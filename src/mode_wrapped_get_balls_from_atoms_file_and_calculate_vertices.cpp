#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/opengl_printer.h"

#include "wrapper_get_balls_from_atoms_file_and_calculate_vertices.h"

void wrapped_get_balls_from_atoms_file_and_calculate_vertices(const auxiliaries::ProgramOptionsHandler& poh)
{
	typedef WrapperGetBallsFromAtomsFileAndCalculateVertices Wrapper;

	const std::string input_file=poh.argument<std::string>("--input-file");
	const std::string sketch_file=poh.argument<std::string>("--sketch-file", "");

	const Wrapper::Result result=Wrapper::get_balls_from_atoms_file_and_calculate_vertices(input_file);

	if(result.error.empty())
	{
		std::cout << "balls " << result.balls.size() << "\n";
		for(std::size_t i=0;i<result.balls.size();i++)
		{
			const Wrapper::Ball& b=result.balls[i];
			std::cout << b.x << " " << b.y << " " << b.z << " " << b.r << " " << b.name << "\n";
		}

		std::cout << "vertices " << result.vertices.size() << "\n";
		for(std::size_t i=0;i<result.vertices.size();i++)
		{
			const Wrapper::Vertex& v=result.vertices[i];
			for(int j=0;j<4;j++)
			{
				std::cout << v.ball_id[j] << " ";
			}
			std::cout << v.x << " " << v.y << " " << v.z << " " << v.r;
			for(int j=0;j<4;j++)
			{
				std::cout << " " << ((v.link_vertex_id[j]!=Wrapper::null_id) ? static_cast<long>(v.link_vertex_id[j]) : static_cast<long>(-1));
			}
			std::cout << "\n";
		}
	}
	else
	{
		throw std::runtime_error(result.error);
	}

	if(!sketch_file.empty())
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0x00FFFF);
		for(std::size_t i=0;i<result.balls.size();i++)
		{
			opengl_printer.add_sphere(result.balls[i]);
		}
		opengl_printer.add_color(0xFFFF00);
		for(std::size_t i=0;i<result.vertices.size();i++)
		{
			const Wrapper::Vertex& v=result.vertices[i];
			for(int j=0;j<4;j++)
			{
				const std::size_t link_vertex_id=v.link_vertex_id[j];
				if(link_vertex_id<i)
				{
					opengl_printer.add_line_strip(v, result.vertices[link_vertex_id]);
				}
			}
		}
		std::ofstream sketch_file_stream(sketch_file.c_str(), std::ios::out);
		opengl_printer.print_pymol_script("sketch", false, sketch_file_stream);
	}
}
