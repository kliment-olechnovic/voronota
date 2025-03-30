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

//user-defined structure for a point, used for both defining a periodic box and for recording cell vertices
struct Point
{
	Point() : x(0), y(0), z(0) {}

	Point(const double x, const double y, const double z) : x(x), y(y), z(z) {}

	double x;
	double y;
	double z;
};

//user-defined structure for a cell vertex
struct CellVertex
{
	CellVertex() {}

	int ball_indices[4];
	Point position;

	//checks if the vertex lies on the solvent accessible surface
	bool is_on_sas() const
	{
		return (ball_indices[3]<0);
	}
};

//user-defined function that uses voronotalt::RadicalTessellation to fill a vectors cell vertices
bool compute_cell_vertices_with_optional_periodic_box_conditions(
		const std::vector<Ball>& balls,
		const double probe,
		const std::vector<Point>& periodic_box_corners,
		std::vector<CellVertex>& cell_vertices)
{
	cell_vertices.clear();

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
			true,
			result);

	if(result.tessellation_net.tes_vertices.empty())
	{
		std::cerr << "No tessellation cell vertices constructed for the provided balls and probe." << std::endl;
		return false;
	}

	// using the result data about tessellation cell vertices
	cell_vertices.resize(result.tessellation_net.tes_vertices.size());
	for(std::size_t i=0;i<result.tessellation_net.tes_vertices.size();i++)
	{
		const voronotalt::RadicalTessellationContactConstruction::TessellationVertex& tv=result.tessellation_net.tes_vertices[i];
		CellVertex& cv=cell_vertices[i];
		for(int j=0;j<4;j++)
		{
			if(tv.ids_of_spheres[j]==voronotalt::null_id())
			{
				cv.ball_indices[j]=-1;
			}
			else
			{
				cv.ball_indices[j]=static_cast<int>(tv.ids_of_spheres[j]);
			}
		}
		cv.position=Point(tv.position.x, tv.position.y, tv.position.z);
	}

	return true;
}

//user-defined convenience function that redirects to the previously defined function with an empty vector of periodic box corners
bool compute_cell_vertices(
		const std::vector<Ball>& balls,
		const double probe,
		std::vector<CellVertex>& cell_vertices)
{
	return compute_cell_vertices_with_optional_periodic_box_conditions(balls, probe, std::vector<Point>(), cell_vertices);
}

//user-defined function to print input balls
void print_balls(const std::vector<Ball>& balls)
{
	std::cout << "balls:\n";
	for(std::size_t i=0;i<balls.size();i++)
	{
		const Ball& ball=balls[i];
		std::cout << "ball index=" << i << " center=(" << ball.x << ", " << ball.y << ", " << ball.z << ") radius=" << ball.r << "\n";
	}
	std::cout << "\n";
}

//user-defined function to print resulting contacts and cells
void print_cell_vertices(const std::vector<CellVertex>& output_cell_vertices)
{
	std::cout << "tessellation cell vertices:\n";
	for(const CellVertex& cv : output_cell_vertices)
	{
		std::cout << "cell_vertex " << (cv.is_on_sas() ? "on_SAS" : "not_on_SAS");
		std::cout << " ball_indices=(" << cv.ball_indices[0] << ", " << cv.ball_indices[1] << ", " << cv.ball_indices[2] << ", " << cv.ball_indices[3] << ")";
		std::cout << " position=(" << cv.position.x << ", " << cv.position.y << ", " << cv.position.z << ")\n";
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

		std::vector<CellVertex> output_cell_vertices;

		if(compute_cell_vertices(input_balls, probe, output_cell_vertices))
		{
			print_cell_vertices(output_cell_vertices);
		}
		else
		{
			std::cerr << "Failed to compute tessellation cell vertices in basic mode." << std::endl;
			return 1;
		}
	}

	{
		std::cout << "Output in periodic box mode:\n\n";

		std::vector<Point> periodic_box_corners;
		periodic_box_corners.push_back(Point(-1.6, -1.6, -0.6));
		periodic_box_corners.push_back(Point(1.6, 1.6, 3.1));

		std::vector<CellVertex> output_cell_vertices;

		if(compute_cell_vertices_with_optional_periodic_box_conditions(input_balls, probe, periodic_box_corners, output_cell_vertices))
		{
			print_cell_vertices(output_cell_vertices);
		}
		else
		{
			std::cerr << "Failed to compute tessellation cell vertices in periodic box mode." << std::endl;
			return 1;
		}
	}

	return 0;
}

