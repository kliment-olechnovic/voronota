#ifndef VORONOTALT_H_
#define VORONOTALT_H_

#include <vector>
#include <stdexcept>

#include "../src/voronotalt/voronotalt.h"

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

	int recompute(const double new_probe)
	{
		probe=new_probe;
		contacts.clear();
		cells.clear();

		if(balls.empty())
		{
			throw std::runtime_error("No balls to compute the tessellation for.");
		}

		voronotalt::RadicalTessellation::Result result;
		voronotalt::RadicalTessellation::construct_full_tessellation(voronotalt::get_spheres_from_balls(balls, probe), result);

		if(result.contacts_summaries.empty())
		{
			throw std::runtime_error("No contacts constructed for the provided balls and probe.");
		}

		if(result.cells_summaries.empty())
		{
			throw std::runtime_error("No cells constructed for the provided balls and probe.");
		}

		contacts.resize(result.contacts_summaries.size());
		for(std::size_t i=0;i<result.contacts_summaries.size();i++)
		{
			contacts[i].index_a=result.contacts_summaries[i].id_a;
			contacts[i].index_b=result.contacts_summaries[i].id_b;
			contacts[i].area=result.contacts_summaries[i].area;
			contacts[i].arc_length=result.contacts_summaries[i].arc_length;
		}

		cells.resize(balls.size());
		for(std::size_t i=0;i<result.cells_summaries.size();i++)
		{
			const std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
			cells[index].sas_area=result.cells_summaries[i].sas_area;
			cells[index].volume=result.cells_summaries[i].sas_inside_volume;
			cells[index].included=true;
		}

		return static_cast<int>(contacts.size());
	}
};

#endif /* VORONOTALT_H_ */
