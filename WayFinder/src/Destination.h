#pragma once
class Destination
{
	public:
		//Destination(void);
		Destination(std::string, float, float);
		~Destination(void);
		std::string getName();
		void setName(std::string);
		float getX();
		void setX(float);
		float getY();
		void setY(float);
		static std::vector<Destination> getDestinations();

	private:
		static std::vector<Destination> destinations;
		std::string name;
		float x;
		float y;
};

