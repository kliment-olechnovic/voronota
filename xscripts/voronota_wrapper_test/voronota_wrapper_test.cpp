#include "voronota_calculate_contacts.h"

int main(const int argc, const char** argv)
{
	typedef std::vector<double> Point;
	
	std::vector<Point> ball_centers;
	std::vector<double> ball_radii;
	
	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			std::istringstream line_input(line);
			Point center(3, 0.0);
			double radius=1.0;
			line_input >> center[0] >> center[1] >> center[2] >> radius;
			ball_centers.push_back(center);
			ball_radii.push_back(radius);
		}
	}
	
	try
	{
		std::vector<voronota::Contact> contacts=voronota::calculate_contacts(ball_centers, ball_radii, 1.4);
		for(std::size_t i=0;i<contacts.size();i++)
		{
			std::cout << contacts[i].ids[0] << " " << contacts[i].ids[1] << " " << contacts[i].area << "\n";
		}
		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
	}
	
	return 2;
}
