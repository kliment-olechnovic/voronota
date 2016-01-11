#ifndef WRAPPER_GET_BALLS_FROM_ATOMS_FILE_AND_CALCULATE_VERTICES_H_
#define WRAPPER_GET_BALLS_FROM_ATOMS_FILE_AND_CALCULATE_VERTICES_H_

#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation.h"

#include "auxiliaries/atoms_io.h"
#include "auxiliaries/atom_radius_assigner.h"
#include "auxiliaries/io_utilities.h"

class WrapperGetBallsFromAtomsFileAndCalculateVertices
{
public:
	static const std::size_t null_id=apollota::npos;

	struct Ball
	{
		double x;
		double y;
		double z;
		double r;
		std::string name;

		Ball() : x(0.0), y(0.0), z(0.0), r(0.0), name("")
		{
		}
	};

	struct Vertex
	{
		double x;
		double y;
		double z;
		double r;
		unsigned long ball_id[4];
		bool link_available[4];
		unsigned long link_vertex_id[4];

		Vertex() : x(0.0), y(0.0), z(0.0), r(0.0)
		{
			for(int i=0;i<4;i++)
			{
				ball_id[i]=null_id;
				link_available[i]=false;
				link_vertex_id[i]=null_id;
			}
		}
	};

	struct Result
	{
		std::vector<Ball> balls;
		std::vector<Vertex> vertices;
		std::string error;
	};

	struct Parameters
	{
		bool include_heteroatoms;
		bool include_hydrogens;
		bool multimodel_chains;
		bool mmcif;
		std::string radii_file;
		double default_radius;
		bool only_default_radius;
		double hull_offset;
		bool exclude_hidden_balls;
		bool include_surplus_quadruples;
		double init_radius_for_BSH;

		Parameters() :
			include_heteroatoms(false),
			include_hydrogens(false),
			multimodel_chains(false),
			mmcif(false),
			radii_file(""),
			default_radius(1.70),
			only_default_radius(false),
			hull_offset(2.8),
			exclude_hidden_balls(false),
			include_surplus_quadruples(false),
			init_radius_for_BSH(3.5)
		{
		}
	};

	static Result get_balls_from_atoms_file_and_calculate_vertices(std::ifstream& input_file_stream, const Parameters& parameters)
	{
		Result result;

		if(!input_file_stream.good())
		{
			result.error="Invalid input file stream.";
			return result;
		}

		const std::vector<auxiliaries::AtomsIO::AtomRecord> atoms=(parameters.mmcif ?
				auxiliaries::AtomsIO::MMCIFReader::read_data_from_file_stream(
						input_file_stream,
						parameters.include_heteroatoms,
						parameters.include_hydrogens).atom_records :
				auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(input_file_stream,
						parameters.include_heteroatoms,
						parameters.include_hydrogens,
						parameters.multimodel_chains,
						false).atom_records);

		if(atoms.empty())
		{
			result.error="No atoms found in input stream.";
			return result;
		}

		auxiliaries::AtomRadiusAssigner atom_radius_assigner(parameters.default_radius);
		if(!parameters.only_default_radius)
		{
			if(parameters.radii_file.empty())
			{
				atom_radius_assigner.add_radius_by_descriptor("*", "C*", 1.70);
				atom_radius_assigner.add_radius_by_descriptor("*", "N*", 1.55);
				atom_radius_assigner.add_radius_by_descriptor("*", "O*", 1.52);
				atom_radius_assigner.add_radius_by_descriptor("*", "S*", 1.80);
				atom_radius_assigner.add_radius_by_descriptor("*", "P*", 1.80);
				atom_radius_assigner.add_radius_by_descriptor("*", "H*", 1.20);
			}
			else
			{
				std::ifstream radii_file_stream(parameters.radii_file.c_str(), std::ios::in);
				auxiliaries::IOUtilities().read_lines_to_container(radii_file_stream, auxiliaries::AtomRadiusAssigner::add_descriptor_and_radius_from_stream_to_atom_radius_assigner, atom_radius_assigner);
			}
		}

		std::vector<apollota::SimpleSphere> spheres;
		spheres.reserve(atoms.size()+8);
		std::vector<std::string> names;
		names.reserve(atoms.size()+8);
		for(std::size_t i=0;i<atoms.size();i++)
		{
			const auxiliaries::AtomsIO::AtomRecord& atom=atoms[i];
			const double radius=atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
			spheres.push_back(apollota::SimpleSphere(atom, radius));
			names.push_back(name_atom_ball(atom));
		}
		if(parameters.hull_offset>0.0)
		{
			const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, parameters.hull_offset);
			spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());
			names.insert(names.end(), artificial_boundary.size(), std::string("artificial_hull_ball"));
		}

		if(spheres.size()!=names.size())
		{
			result.error="Could not assign names to all balls.";
			return result;
		}

		const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(
				spheres,
				parameters.init_radius_for_BSH,
				parameters.exclude_hidden_balls,
				parameters.include_surplus_quadruples);
		apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
		apollota::Triangulation::VerticesGraph vertices_graph=apollota::Triangulation::construct_vertices_graph(spheres, triangulation_result.quadruples_map);

		if(vertices_vector.size()!=vertices_graph.size())
		{
			result.error="Could not construct valid vertices graph.";
			return result;
		}

		result.balls.resize(spheres.size());
		for(std::size_t i=0;i<spheres.size();i++)
		{
			const apollota::SimpleSphere& sphere=spheres[i];
			const std::string& name=names[i];
			Ball& ball=result.balls[i];
			ball.x=sphere.x;
			ball.y=sphere.y;
			ball.z=sphere.z;
			ball.r=sphere.r;
			ball.name=name;
		}

		result.vertices.resize(vertices_vector.size());
		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			const apollota::SimpleSphere& tangent_sphere=vertices_vector[i].second;
			const apollota::Quadruple& quadruple=vertices_vector[i].first;
			const std::vector<std::size_t> links=vertices_graph[i];
			Vertex& vertex=result.vertices[i];
			vertex.x=tangent_sphere.x;
			vertex.y=tangent_sphere.y;
			vertex.z=tangent_sphere.z;
			vertex.r=tangent_sphere.r;
			for(unsigned int j=0;j<4;j++)
			{
				vertex.ball_id[j]=quadruple.get(j);
				if(links.size()>j)
				{
					vertex.link_vertex_id[j]=links[j];
					vertex.link_available[j]=(vertex.link_vertex_id[j]!=null_id);
				}
			}
		}

		return result;
	}

	static Result get_balls_from_atoms_file_and_calculate_vertices(const std::string& input_file_path, const Parameters& parameters)
	{
		std::ifstream input_file_stream(input_file_path.c_str(), std::ios::in);
		return get_balls_from_atoms_file_and_calculate_vertices(input_file_stream, parameters);
	}

	static Result get_balls_from_atoms_file_and_calculate_vertices(const std::string& input_file_path)
	{
		return get_balls_from_atoms_file_and_calculate_vertices(input_file_path, Parameters());
	}

private:
	static std::string name_atom_ball(const auxiliaries::AtomsIO::AtomRecord& atom)
	{
		std::ostringstream output;
		output << (atom.chainID) << "," << (atom.resSeq) << "," << (atom.resName) << "," << (atom.name) << "," << (atom.altLoc) << "," << (atom.iCode);
		return output.str();
	}
};

#endif /* WRAPPER_GET_BALLS_FROM_ATOMS_FILE_AND_CALCULATE_VERTICES_H_ */
