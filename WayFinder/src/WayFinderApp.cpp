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

class WayFinderApp : public AppNative {
public:
    void prepareSettings(Settings *);
    void setup();
    void keyUp(KeyEvent);
    void update();
    //void mouseMove(MouseEvent);
    void draw();

private:
    void println(const std::string&);
    void guide();
    void detect();

    std::vector<Destination> destinations;
    // HACK: Need to figure out a better way to maintain center state.
    ci::Vec2f spotlightCenter2D;
    //ci::Vec3f spotlightCenter3D; // 3D vector is required for drawVector.
    float spotlightRadius;
    float arrowLength;
    bool detected;
    bool debugView;
    static const int WIDTH = 640;
    static const int HEIGHT = 360;
    static const int FRAME_RATE = 30;
    static const int FRAME_COUNT_THRESHOLD = 10;

    cv::Mat src_img;
    CaptureRef capture;
    gl::Texture mTexture;


    // Detection sample: http://mateuszstankiewicz.eu/?p=189
    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    cv::BackgroundSubtractorMOG2 bg;
    std::vector<std::vector<cv::Point>> contours;
};

void WayFinderApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    settings->setFrameRate(FRAME_RATE);
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
    spotlightRadius = getWindowWidth() / 16;
    arrowLength = 50.0f;
    spotlightCenter2D = Vec2f(getWindowWidth() / 2, getWindowHeight() / 2);
    detected = false;
    //spotlightCenter3D = Vec3f(getWindowWidth() / 2, getWindowHeight() / 2, 0.0f);

    capture = Capture::create(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    capture->start();

    //bg.set("bShadowDetection", false);
    bg.set("nmixtures", 3);

    debugView = false;
}

// Toggle btwn debug view and normal view.
void WayFinderApp::keyUp(KeyEvent event)
{
    if(event.getChar() == KeyEvent::KEY_d) {
        debugView = !debugView;
    }
}

void WayFinderApp::update()
{
    detected = false;

    // TODO: Consider converting capture to grayscale to improve performance.
    if(capture && capture->checkNewFrame()) {
        frame = toOcv(capture->getSurface());
        //cv::Mat frameGray, frameBlurred, frameThresh, foreGray, backGray;
        //cvtColor(frame, frameGray, CV_BGR2GRAY);
        //int blurAmount = 10;
        //cv::blur(frame, frameBlurred, cv::Size(blurAmount, blurAmount));
        //threshold(frameBlurred, frameThresh, 100, 255, CV_THRESH_BINARY);

        // Get all contours.
        bg.operator()(frame,fore);
        bg.getBackgroundImage(back);
        cv::erode(fore, fore, cv::Mat());
        cv::dilate(fore, fore, cv::Mat());
        cv::findContours(fore, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        // Get largest contour: http://stackoverflow.com/questions/15012073/opencv-draw-draw-contours-of-2-largest-objects
        int largestIndex = 0;
        int largestContour = 0;
        for(int i = 0; i< contours.size(); i++) {
            if(contours[i].size() > largestContour) {
                largestContour = contours[i].size();
                largestIndex = i;
            }
        }
        vector<std::vector<cv::Point>> hack;
        hack.push_back(contours[largestIndex]);

        // Find bounding rectangle for largest countour.
        cv::Rect rect = boundingRect(contours[largestIndex]);

        // Get center of rectangle.
        cv::Point center = cv::Point(
                               rect.x + (rect.width / 2),
                               rect.y + (rect.height / 2)
                           );

        // Show guide.
        spotlightCenter2D.x = center.x;
        spotlightCenter2D.y = center.y;
        //spotlightRadius = (rect.width + rect.y) / 2;
        detected = true;

        if(debugView) {
            cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 2);
            cv::drawContours(frame, hack, -1, cv::Scalar(255, 0, 0), 2);
            rectangle(frame, rect, cv::Scalar(0, 255, 0), 3);
            circle(frame, center, 10, cv::Scalar(0, 255, 0), 3);
            mTexture = gl::Texture(fromOcv(frame));
        }
    }
}

/*
void WayFinderApp::mouseMove(MouseEvent event)
{
    Vec2i pos = event.getPos();
    //println("(x, y) = (" + boost::lexical_cast<string>(pos.x) + ", " + boost::lexical_cast<string>(pos.y) + ")");
    if(pos.x <= 0 || pos.x >= getWindowWidth() || pos.y <= 0 || pos.y >= getWindowHeight()) {
        detected = false;
    } else {
        detected = true;
        // When moving the mouse, move the center.
        spotlightCenter2D = event.getPos();
    }
}
*/

void WayFinderApp::draw()
{
    gl::clear(Color(0, 0, 0));

    if(mTexture)
        gl::draw(mTexture);

    detect();

    if(detected) {
        guide();
    }
}

void WayFinderApp::detect()
{
    if(getElapsedFrames() % FRAME_COUNT_THRESHOLD == 0) {
        cv::Mat src_img(toOcv(capture->getSurface()));



    }
}

void WayFinderApp::guide()
{
    // TODO: Scale spotlight radius based on size of detected rectangle.

    // TODO: USE THIS APPROACH: http://www.codeproject.com/Articles/10248/Motion-Detection-Algorithms

    // TODO: OR THIS APPROACH: http://mateuszstankiewicz.eu/?p=189

    // Draw the spotlight, centered around the detected location.
    gl::drawSolidCircle(spotlightCenter2D, spotlightRadius);

    // TODO: Vectors should be of uniform length. Need a point *along* the vector at a predefined distance from the start point: http://stackoverflow.com/questions/1800138/given-a-start-and-end-point-and-a-distance-calculate-a-point-along-a-line

    for(vector<Destination>::iterator iter = destinations.begin(); iter != destinations.end(); ++iter) {
        // Draw a line from the spotlight center to each of the destinations.
        //gl::drawLine(spotlightCenter3D, iter->getVector3D());
        gl::drawLine(spotlightCenter2D, iter->getVector2D());

        // Display the destination name.
        gl::drawStringCentered(iter->getName(), iter->getVector2D());
    }
}

void WayFinderApp::println(const std::string& msg)
{
    app::console() << msg << std::endl;
}

CINDER_APP_NATIVE(WayFinderApp, RendererGl)
