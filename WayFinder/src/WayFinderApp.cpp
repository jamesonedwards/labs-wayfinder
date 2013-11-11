#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"
#include "boost/lexical_cast.hpp"
#include "Destination.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class WayFinderApp : public AppNative {
public:
    void prepareSettings(Settings *);
    void setup();
    void update();
    void mouseMove(MouseEvent);
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
    static const int WIDTH = 320;
    static const int HEIGHT = 180;
    static const int FRAME_COUNT_THRESHOLD = 10;
    int frameCount;

    // HOG Detection:
    cv::Mat src_img;
    //cv::VideoCapture capture;
    CaptureRef capture;
    gl::Texture mTexture;
    cv::HOGDescriptor hog;
    cv::Mat mono_img;
    vector<cv::Rect> found;
};

void WayFinderApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    settings->setFrameRate(60.0f);
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
    frameCount = 0;
    //spotlightCenter3D = Vec3f(getWindowWidth() / 2, getWindowHeight() / 2, 0.0f);

    // HOG Detection:
    //capture = Capture(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    capture = Capture::create(WayFinderApp::WIDTH, WayFinderApp::HEIGHT);
    capture->start();
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    //cv::namedWindow("Frame");
    //cv::namedWindow("Background");
}

void WayFinderApp::update()
{
    if(capture && capture->checkNewFrame()) {
        cv::Mat input(toOcv(capture->getSurface())), output;

        //cv::Sobel(input, output, CV_8U, 1, 0);
        //cv::threshold(input, output, 128, 255, CV_8U);
        //cv::Laplacian(input, output, CV_8U);
        //cv::circle(output, toOcv(Vec2f(200, 200)), 300, toOcv(Color(0, 0.5f, 1)), -1);
        //cv::line(output, cv::Point(1, 1), cv::Point(30, 30), toOcv(Color(1, 0.5f, 0)));

        mTexture = gl::Texture(fromOcv(input));
        //mTexture = gl::Texture(fromOcv(output));
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
    frameCount++;

    if(frameCount >= FRAME_COUNT_THRESHOLD) {
        cv::Mat src_img(toOcv(capture->getSurface()));
        cv::cvtColor(src_img, mono_img, CV_BGR2GRAY);
        hog.detectMultiScale(mono_img, found);
        detected = false;

        for(unsigned i = 0; i < found.size(); i++) {
            cv::Rect r = found[i];
            rectangle(src_img, r.tl(), r.br(), cv::Scalar(0,255,0), 2);

            detected = true;
            spotlightCenter2D = Vec2f(r.x, r.y);
        }
        //cv::imshow("test", src_img);
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
