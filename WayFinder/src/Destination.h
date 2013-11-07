#pragma once
class Destination {
public:
    Destination(void);
    Destination(std::string, float, float);
    Destination(std::string, ci::Vec3f);
    ~Destination(void);
    std::string getName();
    void setName(std::string);
    ci::Vec2f getVector2D();
    // TODO: Remove 3D vector if it isn't necessary.
    ci::Vec3f getVector3D();
    void setVector(ci::Vec3f);
    static std::vector<Destination> getDestinations();

private:
    static std::vector<Destination> destinations;
    std::string name;
    ci::Vec2f vector2d;
    ci::Vec3f vector3d;
};

