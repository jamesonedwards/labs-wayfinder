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
    this->vector2d = Vec2f(x, y);
    this->vector3d = Vec3f(x, y, 0.0f);
}

Destination::Destination(string name, Vec3f vector)
{
    this->name = name;
    this->setVector(vector);
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

Vec2f Destination::getVector2D()
{
    return this->vector2d;
}

Vec3f Destination::getVector3D()
{
    return this->vector3d;
}

void Destination::setVector(Vec3f vector)
{
    this->vector2d = Vec2d(vector.x, vector.y);
    this->vector3d = vector;
}

vector<Destination> Destination::getDestinations()
{
    // Initialize the destinations vector if not already initialized.
    if(Destination::destinations.size() == 0) {
        // Load destinations from config file.
        string configPath = "destinations.json";
        fs::path path(configPath);
        if(fs::exists(path)) {
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