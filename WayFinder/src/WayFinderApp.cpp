#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

#include "CinderOpenCv.h"
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
    void draw();

    gl::Texture	mTexture;

private:
    void println(const std::string&);
    void guide();

    std::vector<Destination> destinations;
    cinder::Vec2f spotlightCenter;
    float spotlightRadius;
};

void WayFinderApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(800, 600);
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

    // Initialized spotlight.
    spotlightRadius = 50.0f;
    spotlightCenter = Vec2f(getWindowWidth() / 2, getWindowHeight() / 2);
}

void WayFinderApp::update()
{
}

void WayFinderApp::draw()
{
    gl::clear();

    // TODO: Add state machine (struct?): detecting, guiding.
    guide();
}

void WayFinderApp::guide()
{
    // Draw the spotlight, centered around the detected location.
    gl::drawSolidCircle(spotlightCenter, spotlightRadius);

    // Draw a vector from the spotlight center to each of the destinations.
    for(vector<Destination>::iterator iter = destinations.begin(); iter != destinations.end(); ++iter) {

    }
}

void WayFinderApp::println(const std::string& msg)
{
    app::console() << msg << std::endl;
}

CINDER_APP_NATIVE(WayFinderApp, RendererGl)
