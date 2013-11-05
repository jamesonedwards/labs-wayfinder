#include <string>
#include <vector>
#include "cinder/Json.h"
#include "cinder/Filesystem.h"
#include "cinder/Exception.h"
#include "cinder/app/App.h"
#include "Destination.h"
using namespace std;
using namespace ci;

vector<Destination> Destination::destinations;

Destination::Destination(void)
{
}

Destination::Destination(string name, float x, float y)
{
	this->name = name;
	this->x = x;
	this->y = y;
}

Destination::~Destination(void)
{
}

string Destination::getName()
{
	return this->name;
}

void Destination::setName(string name)
{
	this->name = name;
}

float Destination::getX()
{
	return this->x;
}

void Destination::setX(float x)
{
	this->x = x;
}

float Destination::getY()
{
	return this->y;
}

void Destination::setY(float y)
{
	this->y = y;
}

vector<Destination> Destination::getDestinations()
{
	// Initialize the destinations vector if not already initialized.
	if (Destination::destinations.size() == 0) {
		// Load destinations from config file.
		string configPath = "destinations.json";
		fs::path path(configPath);
		if(fs::exists(path))
		{
			DataSourceRef dsr = loadFile(path.native());
			JsonTree tree = ci::JsonTree::JsonTree(dsr);
			JsonTree::Container jsonDestinations = tree.getChild("destinations").getChildren();
			for(JsonTree::Iter iter = jsonDestinations.begin(); iter != jsonDestinations.end(); ++iter) {
				Destination::destinations.push_back(Destination(
					iter->getChild("name").getValue(),
					boost::lexical_cast<float>(iter->getChild("x").getValue()),
					boost::lexical_cast<float>(iter->getChild("y").getValue())));
			}
		} else {
			string err = "Cannot find destinations config file: " + configPath;
			app::console() << err << std::endl;
		}
	}
	return Destination::destinations;
}