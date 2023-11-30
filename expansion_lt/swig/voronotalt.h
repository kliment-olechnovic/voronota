#ifndef VORONOTALT_H_
#define VORONOTALT_H_

#include <vector>
#include <fstream>
#include <stdexcept>

#include "../src/voronotalt/radical_tessellation.h"
#include "../src/voronotalt_cli/io_utilities.h"

struct Ball
{
	Ball() : x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	Ball(double x, double y, double z, double r) : x(x), y(y), z(z), r(r)
	{
	}

	double x;
	double y;
	double z;
	double r;
};

struct Contact
{
	Contact() : index_a(0), index_b(0), area(0.0), arc_length(0.0)
	{
	}

	int index_a;
	int index_b;
	double area;
	double arc_length;
};

struct Cell
{
	Cell() : sas_area(0.0), volume(0.0), included(false)
	{
	}

	double sas_area;
	double volume;
	bool included;
};

class RadicalTessellation
{
public:
	std::vector<Ball> balls;
	std::vector<Contact> contacts;
	std::vector<Cell> cells;
	double probe;

	RadicalTessellation(const std::vector<Ball>& balls, double probe) : probe(probe), balls(balls)
	{
		recompute(probe);
	}

	RadicalTessellation(const char* balls_file_path, double probe) : probe(probe)
	{
		if(balls_file_path==0 || std::string(balls_file_path).empty())
		{
			throw std::runtime_error("Invalid file name provided for reading.");
		}

		std::ifstream input(balls_file_path, std::ios::in);

		if(!input.good())
		{
			throw std::runtime_error(std::string("Failed to open file '")+std::string(balls_file_path)+std::string("' for reading."));
		}

		std::vector<double> values;
		if(!voronotalt::read_double_values_from_text_stream(input, values))
		{
			throw std::runtime_error(std::string("Failed to read data from file '")+std::string(balls_file_path)+std::string("'."));
		}

		if(values.empty() || values.size()%4!=0)
		{
			throw std::runtime_error(std::string("Invalid data in file '")+std::string(balls_file_path)+std::string("'."));
		}

		const std::size_t N=values.size()/4;
		balls.resize(N);
		for(std::size_t i=0;i<N;i++)
		{
			Ball& b=balls[i];
			b.x=values[i*4+0];
			b.y=values[i*4+1];
			b.z=values[i*4+2];
			b.r=values[i*4+3];
		}

		recompute(probe);
	}

	int recompute(const double new_probe)
	{
		probe=new_probe;
		contacts.clear();
		cells.clear();

		if(balls.empty())
		{
			throw std::runtime_error("No balls to compute the tessellation for.");
		}

		std::vector<voronotalt::SimpleSphere> spheres(balls.size());
		for(std::size_t i=0;i<balls.size();i++)
		{
			const Ball& b=balls[i];
			voronotalt::SimpleSphere& s=spheres[i];
			s.p.x=b.x;
			s.p.y=b.y;
			s.p.z=b.z;
			s.r=b.r+probe;
		}

		voronotalt::RadicalTessellation::Result result;
		voronotalt::RadicalTessellation::construct_full_tessellation(spheres, result);

		if(result.contacts_summaries.empty())
		{
			throw std::runtime_error("No contacts constructed for the provided balls and probe.");
		}

		if(result.cells_summaries.empty())
		{
			throw std::runtime_error("No cells constructed for the provided balls and probe.");
		}

		{
			contacts.resize(result.contacts_summaries.size());
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				contacts[i].index_a=result.contacts_summaries[i].id_a;
				contacts[i].index_b=result.contacts_summaries[i].id_b;
				contacts[i].area=result.contacts_summaries[i].area;
				contacts[i].arc_length=result.contacts_summaries[i].arc_length;
			}
		}

		{
			cells.resize(spheres.size());
			for(std::size_t i=0;i<result.cells_summaries.size();i++)
			{
				std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
				if(index<spheres.size())
				{
					cells[index].sas_area=result.cells_summaries[i].sas_area;
					cells[index].volume=result.cells_summaries[i].sas_inside_volume;
					cells[index].included=true;
				}
			}
		}

		return static_cast<int>(contacts.size());
	}
};

#endif /* VORONOTALT_H_ */
