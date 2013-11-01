#include <string>
#include <vector>
#include "cinder/Json.h"
#include "cinder/Filesystem.h"
#include "Destination.h"
using namespace std;
using namespace ci;

vector<Destination> Destination::destinations;

//Destination::Destination(void)
//{
//}

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
		// FIXME: Load destinations from config file.
		fs::path path("C:\Dev\labs-wayfinder\src\c\WayFinder\destinations.json");
		if(fs::exists(path))
		{
			loadFile(path.native());
		}

		Destination::destinations.push_back(Destination("test", 1, 1));
	}
	return Destination::destinations;
}