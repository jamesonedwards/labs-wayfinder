#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"
#include "boost/lexical_cast.hpp"
#include "Destination.h"
#include <vector>
#include<iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include "cinder/app/KeyEvent.h"

using namespace ci;
using namespace ci::app;
using namespace std;

/*
Reference:
http://www.codeproject.com/Articles/10248/Motion-Detection-Algorithms
http://mateuszstankiewicz.eu/?p=189
http://stackoverflow.com/questions/1800138/given-a-start-and-end-point-and-a-distance-calculate-a-point-along-a-line
http://math.stackexchange.com/questions/175896/finding-a-point-along-a-line-a-certain-distance-away-from-another-point
Not used but possibly important: http://www.codeproject.com/Articles/3274/Drawing-Arrows
*/

// TODO: Add sound when guide is shown ("whom" sci-fi door sound, followed  by "warm glow" while shown)
// TODO: Add a subtle undulating effect to circle

class WayFinderApp : public AppNative {
public:
    void prepareSettings(Settings *);
    void setup();
    void keyUp(KeyEvent);
    void update();
    void draw();

private:
    void println(const std::string&);
    ci::Vec2f calculateLinePoint(ci::Vec2f, ci::Vec2f, float);
    void guide();

    std::vector<Destination> destinations;
    // HACK: Need to figure out a better way to maintain center state.
    ci::Vec2f spotlightCenter2D;
    ci::Vec3f spotlightCenter3D; // 3D vector is required for drawVector.
    float spotlightRadius;
    float arrowLength;
    bool detected;
    bool debugView;
    static const int WIDTH = 640;
    static const int HEIGHT = 360;
    //static const int WIDTH = 1280;
    //static const int HEIGHT = 720;
    static const int FRAME_RATE = 30;
    static const int FRAME_COUNT_THRESHOLD = 10;

    cv::Mat src_img;
    CaptureRef capture;
    gl::Texture mTexture;

    // Detection sample: http://mateuszstankiewicz.eu/?p=189
    cv::BackgroundSubtractorMOG2 bg;
    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    std::vector<std::vector<cv::Point>> contours;
};

void WayFinderApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    settings->setFrameRate((float)FRAME_RATE);
}

void WayFinderApp::setup()
{
    println("WayFinderApp started.");

    // Load destinations from config file.
    destinations = Destination::getDestinations();
    if(destinations.size() == 0) {
        println("No destinations found, check the config file.");
        exit(EXIT_FAILURE);
    }
    println("Destinations loaded.");

    // Initialized state.
    spotlightRadius = (float)getWindowWidth() / 16.0f;
    arrowLength = (float)min(getWindowWidth(), getWindowHeight()) / 2.0f;
    spotlightCenter2D = Vec2f((float)getWindowWidth() / 2.0f, (float)getWindowHeight() / 2.0f);
    spotlightCenter3D = Vec3f((float)getWindowWidth() / 2.0f, (float)getWindowHeight() / 2.0f, 0.0f);
    detected = false;

    //capture = Capture::create(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    capture = Capture::create(getWindowWidth(), getWindowHeight());
    capture->start();

    //bg.set("bShadowDetection", false);
    bg.set("nmixtures", 3);
    bg.setBool("detectShadows", true);

    debugView = false;
}

// Toggle btwn debug view and normal view.
void WayFinderApp::keyUp(KeyEvent event)
{
    if(event.getChar() == KeyEvent::KEY_d) {
        gl::clear(Color(0, 0, 0));
        debugView = !debugView;
    }
}

void WayFinderApp::update()
{
    if(getElapsedFrames() % FRAME_COUNT_THRESHOLD == 0) {
        detected = false;

        // TODO: Consider converting capture to grayscale or blurring then thresholding to improve performance.
        if(capture && capture->checkNewFrame()) {
            frame = toOcv(capture->getSurface());
            //cv::Mat frameGray, frameBlurred, frameThresh, foreGray, backGray;
            //cvtColor(frame, frameGray, CV_BGR2GRAY);
            int blurAmount = 10;
            //cv::blur(frame, frameBlurred, cv::Size(blurAmount, blurAmount));
            //threshold(frameBlurred, frameThresh, 100, 255, CV_THRESH_BINARY);

            // Get all contours.
            //bg.operator()(frameThresh,fore);
            bg.operator()(frame, fore);
            bg.getBackgroundImage(back);
            cv::erode(fore, fore, cv::Mat());
            cv::dilate(fore, fore, cv::Mat());
            cv::findContours(fore, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

            // Get largest contour: http://stackoverflow.com/questions/15012073/opencv-draw-draw-contours-of-2-largest-objects
            unsigned largestIndex = 0;
            unsigned largestContour = 0;
            for(unsigned i = 0; i < contours.size(); i++) {
                if(contours[i].size() > largestContour) {
                    largestContour = contours[i].size();
                    largestIndex = i;
                }
            }

            vector<std::vector<cv::Point>> hack;
            cv::Rect rect;
            cv::Point center;

            if(contours.size() > 0) {
                hack.push_back(contours[largestIndex]);

                // Find bounding rectangle for largest countour.
                rect = boundingRect(contours[largestIndex]);

                // Make sure the blog is large enough to be a track-worthy.
                println("Rext area = " + boost::lexical_cast<std::string>(rect.area()));
                if(rect.area() >= 5000) { // TODO: Tweak this value.
                    // Get center of rectangle.
                    center = cv::Point(
                                 rect.x + (rect.width / 2),
                                 rect.y + (rect.height / 2)
                             );

                    // Show guide.
                    spotlightCenter2D.x = (float)center.x;
                    spotlightCenter2D.y = (float)center.y;
                    spotlightCenter3D.x = (float)center.x;
                    spotlightCenter3D.y = (float)center.y;
                    //spotlightRadius = (rect.width + rect.y) / 2;
                    detected = true;
                }
            }

            // When debug mode is off, the background should be black.
            if(debugView) {
                if(contours.size() > 0) {
                    cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 2);
                    cv::drawContours(frame, hack, -1, cv::Scalar(255, 0, 0), 2);
                    rectangle(frame, rect, cv::Scalar(0, 255, 0), 3);
                    circle(frame, center, 10, cv::Scalar(0, 255, 0), 3);
                }
                mTexture = gl::Texture(fromOcv(frame));
            }
        }

        // TODO: Create control panel for all inputs.
    }
}

void WayFinderApp::draw()
{
    gl::clear(Color(0, 0, 0));

    if(mTexture && debugView)
        gl::draw(mTexture);

    if(detected) {
        guide();
    }
}

ci::Vec2f WayFinderApp::calculateLinePoint(ci::Vec2f start, ci::Vec2f end, float distance)
{
    ci::Vec2f diff = end - start;
    ci::Vec2f normalized = diff.safeNormalized();
    return start + (distance * normalized);
}

void WayFinderApp::guide()
{
    // Draw the spotlight, centered around the detected location.
    gl::color(255, 255, 255);
    gl::drawSolidCircle(spotlightCenter2D, spotlightRadius);
    gl::color(0, 0, 0);
    gl::drawSolidCircle(spotlightCenter2D, spotlightRadius * 2 / 3);
    //gl::drawStrokedCircle(spotlightCenter2D, spotlightRadius);
    gl::color(255, 255, 255);

    for(vector<Destination>::iterator iter = destinations.begin(); iter != destinations.end(); ++iter) {
        // Vectors should be of uniform length. Need a point *along* the vector at a predefined distance from the start point.
        ci::Vec2f endPt = calculateLinePoint(spotlightCenter2D, iter->getVector2D(), arrowLength);
        ci::Vec3f endPt3d = ci::Vec3f(endPt.x, endPt.y, 0);
        ci::Vec2f namePt = calculateLinePoint(spotlightCenter2D, iter->getVector2D(), arrowLength * 2 / 3);

        // Draw a line from the spotlight center to each of the destinations.
        gl::drawVector(spotlightCenter3D, endPt3d, 15.0f, 5.0f);

        // Display the destination name.
        gl::drawStringCentered(iter->getName(), namePt);
    }
}

void WayFinderApp::println(const std::string& msg)
{
    app::console() << msg << std::endl;
}

CINDER_APP_NATIVE(WayFinderApp, RendererGl)
