#include <iostream>

#include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path

//user-defined structure for a ball
struct Ball
{
	Ball(const double x, const double y, const double z, const double r) : x(x), y(y), z(z), r(r) {}

	double x;
	double y;
	double z;
	double r;
};

//user-defined structure for a contact descriptor
struct Contact
{
	Contact() : index_a(0), index_b(0), area(0.0), arc_length(0.0) {}

	int index_a;
	int index_b;
	double area;
	double arc_length;
};

//user-defined structure for a cell descriptor
struct Cell
{
	Cell() : index(0), sas_area(0.0), volume(0.0), included(false) {}

	int index;
	double sas_area;
	double volume;
	bool included;
};

//user-defined structure for a point, to define optonal periodic box corners
struct Point
{
	Point(const double x, const double y, const double z) : x(x), y(y), z(z) {}

	double x;
	double y;
	double z;
};

//user-defined function that uses voronotalt::RadicalTessellation to fill vectors of contact and cell descriptors
bool compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(
		const std::vector<Ball>& balls,
		const double probe,
		const std::vector<Point>& periodic_box_corners,
		std::vector<Contact>& contacts,
		std::vector<Cell>& cells)
{
	contacts.clear();
	cells.clear();

	if(balls.empty())
	{
		std::cerr << "No balls to compute the tessellation for." << std::endl;
		return false;
	}

	if(!periodic_box_corners.empty() && periodic_box_corners.size()<2)
	{
		std::cerr << "Invalid number of provided periodic box corners, there must be either none or more than one corners." << std::endl;
		return false;
	}

	// computing Voronota-LT radical tessellation results
	voronotalt::RadicalTessellation::Result result;
	voronotalt::RadicalTessellation::construct_full_tessellation(
			voronotalt::get_spheres_from_balls(balls, probe),
			voronotalt::PeriodicBox::create_periodic_box_from_corners(voronotalt::get_simple_points_from_points(periodic_box_corners)),
			result);

	if(result.contacts_summaries.empty())
	{
		std::cerr << "No contacts constructed for the provided balls and probe." << std::endl;
		return false;
	}

	if(result.cells_summaries.empty())
	{
		std::cerr << "No cells constructed for the provided balls and probe.";
		return false;
	}

	// using the result data about contacts
	contacts.resize(result.contacts_summaries.size());
	for(std::size_t i=0;i<result.contacts_summaries.size();i++)
	{
		contacts[i].index_a=result.contacts_summaries[i].id_a;
		contacts[i].index_b=result.contacts_summaries[i].id_b;
		contacts[i].area=result.contacts_summaries[i].area;
		contacts[i].arc_length=result.contacts_summaries[i].arc_length;
	}

	// using the result data about cells
	cells.resize(balls.size());
	for(std::size_t i=0;i<result.cells_summaries.size();i++)
	{
		const std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
		cells[index].index=static_cast<int>(result.cells_summaries[i].id);
		cells[index].sas_area=result.cells_summaries[i].sas_area;
		cells[index].volume=result.cells_summaries[i].sas_inside_volume;
		cells[index].included=true;
	}

	return true;
}

//user-defined convenience function that redirects to the previously defined function with an empty vector of periodic box corners
bool compute_contact_and_cell_descriptors(
		const std::vector<Ball>& balls,
		const double probe,
		std::vector<Contact>& contacts,
		std::vector<Cell>& cells)
{
	return compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(balls, probe, std::vector<Point>(), contacts, cells);
}

//user-defined function to print input balls
void print_balls(const std::vector<Ball>& balls)
{
	std::cout << "balls:\n";
	for(std::size_t i=0;i<balls.size();i++)
	{
		const Ball& ball=balls[i];
		std::cout << "ball " << i << " " << ball.x << " " << ball.y << " " << ball.z << " " << ball.r << "\n";
	}
	std::cout << "\n";
}

//user-defined function to print resulting contacts and cells
void print_contacts_and_cells(const std::vector<Contact>& output_contacts, const std::vector<Cell>& output_cells)
{
	std::cout << "contacts:\n";
	for(const Contact& contact : output_contacts)
	{
		std::cout << "contact " << contact.index_a << " " << contact.index_b << " " << contact.area << " " << contact.arc_length << "\n";
	}
	std::cout << "\n";

	std::cout << "cells:\n";
	for(const Cell& cell : output_cells)
	{
		if(cell.included)
		{
			std::cout << "cell " << cell.index << " " << cell.sas_area << " " << cell.volume << "\n";
		}
	}
	std::cout << "\n";
}

int main(const int, const char**)
{
	std::vector<Ball> input_balls;

	input_balls.push_back(Ball(0, 0, 2, 1));
	input_balls.push_back(Ball(0, 1, 0, 0.5));
	input_balls.push_back(Ball(0.382683, 0.92388, 0, 0.5));
	input_balls.push_back(Ball(0.707107, 0.707107, 0, 0.5));
	input_balls.push_back(Ball(0.92388, 0.382683, 0, 0.5));
	input_balls.push_back(Ball(1, 0, 0, 0.5));
	input_balls.push_back(Ball(0.92388, -0.382683, 0, 0.5));
	input_balls.push_back(Ball(0.707107, -0.707107, 0, 0.5));
	input_balls.push_back(Ball(0.382683, -0.92388, 0, 0.5));
	input_balls.push_back(Ball(0, -1, 0, 0.5));
	input_balls.push_back(Ball(-0.382683, -0.92388, 0, 0.5));
	input_balls.push_back(Ball(-0.707107, -0.707107, 0, 0.5));
	input_balls.push_back(Ball(-0.92388, -0.382683, 0, 0.5));
	input_balls.push_back(Ball(-1, 0, 0, 0.5));
	input_balls.push_back(Ball(-0.92388, 0.382683, 0, 0.5));
	input_balls.push_back(Ball(-0.707107, 0.707107, 0, 0.5));
	input_balls.push_back(Ball(-0.382683, 0.92388, 0, 0.5));

	std::cout << "Input:\n\n";

	print_balls(input_balls);

	const double probe=1.0;

	{
		std::cout << "Output in basic mode:\n\n";

		std::vector<Contact> output_contacts;
		std::vector<Cell> output_cells;

		if(compute_contact_and_cell_descriptors(input_balls, probe, output_contacts, output_cells))
		{
			print_contacts_and_cells(output_contacts, output_cells);
		}
		else
		{
			std::cerr << "Failed to compute contact and cell descriptors in basic mode." << std::endl;
			return 1;
		}
	}

	{
		std::cout << "Output in periodic box mode:\n\n";

		std::vector<Point> periodic_box_corners;
		periodic_box_corners.push_back(Point(-1.6, -1.6, -0.6));
		periodic_box_corners.push_back(Point(1.6, 1.6, 3.1));

		std::vector<Contact> output_contacts;
		std::vector<Cell> output_cells;

		if(compute_contact_and_cell_descriptors_with_optional_periodic_box_conditions(input_balls, probe, periodic_box_corners, output_contacts, output_cells))
		{
			print_contacts_and_cells(output_contacts, output_cells);
		}
		else
		{
			std::cerr << "Failed to compute contact and cell descriptors in periodic box mode." << std::endl;
			return 1;
		}
	}

	return 0;
}

