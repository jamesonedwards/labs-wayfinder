#pragma once
class Destination {
public:
    Destination(void);
    Destination(std::string, float, float);
    Destination(std::string, ci::Vec3f);
    ~Destination(void);
    std::string getName();
    void setName(std::string);
    ci::Vec3f getVector();
    void setVector(ci::Vec3f);
    static std::vector<Destination> getDestinations();

private:
    static std::vector<Destination> destinations;
    std::string name;
    ci::Vec3f vector;
};

