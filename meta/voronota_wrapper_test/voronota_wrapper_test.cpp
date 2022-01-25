#include "voronota_calculate_contacts.h"

int main(const int argc, const char** argv)
{
	typedef std::vector<double> Point;
	
	try
	{
		std::vector<Point> ball_centers;
		std::vector<double> ball_radii;

		while(std::cin.good())
		{
			Point center(3, 0.0);
			double radius=0.0;
			std::cin >> center[0] >> center[1] >> center[2] >> radius >> std::ws;
			if(std::cin.fail())
			{
				throw std::runtime_error(std::string("Failed to read balls from stdin."));
			}
			else
			{
				ball_centers.push_back(center);
				ball_radii.push_back(radius);
			}
		}


		const double probe=1.4;

		// adjust probe for your needs, e.g. set it to a higher value if you do not want cavity-induced solvent contacts, for example:
		//
		//    const double probe=6.0;
		//

		// if you want to use the ordinaty, not weighted Voronoi tessellation, set all radii to the same value, for example:
		//
		//     ball_radii=std::vector<double>(ball_centers.size(), 2.0);
		//

		const std::vector<voronota::Contact> contacts=voronota::calculate_contacts(ball_centers, ball_radii, probe);

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
	
	return 1;
}
