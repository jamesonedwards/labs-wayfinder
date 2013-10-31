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
	void setup();
	void draw();
	
	gl::Texture	mTexture;

private:
	std::vector<Destination> destinations;
};

void WayFinderApp::setup()
{
	// The included image is copyright Trey Ratcliff
	// http://www.flickr.com/photos/stuckincustoms/4045813826/
	
	ci::Surface8u surface( loadImage( loadAsset( "dfw.jpg" ) ) );
	cv::Mat input( toOcv( surface ) );
	cv::Mat output;

	cv::medianBlur( input, output, 11 );
//	cv::Sobel( input, output, CV_8U, 0, 1 ); 
//	cv::threshold( input, output, 128, 255, CV_8U );

	mTexture = gl::Texture( fromOcv( output ) );

	destinations = Destination::getDestinations();
	int cnt = destinations.size();
}   

void WayFinderApp::draw()
{
	gl::clear();
	gl::draw( mTexture );
}

CINDER_APP_NATIVE( WayFinderApp, RendererGl )
