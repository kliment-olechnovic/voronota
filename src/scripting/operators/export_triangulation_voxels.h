#ifndef SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_VOXELS_H_
#define SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_VOXELS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportTriangulationVoxels : public OperatorBase<ExportTriangulationVoxels>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		int number_of_voxels;

		Result() : number_of_voxels(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			heterostorage.variant_object.value("number_of_voxels")=number_of_voxels;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	FilteringOfTriangulation::Query filtering_query_without_ids;
	double search_step_factor;
	std::string file;

	ExportTriangulationVoxels() : search_step_factor(1.0)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		filtering_query_without_ids=OperatorsUtilities::read_filtering_of_triangulation_query(input);
		search_step_factor=input.get_value_or_default<double>("search-step-factor", 1.0);
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		OperatorsUtilities::document_read_filtering_of_triangulation_query(doc);
		doc.set_option_decription(CDOD("search-step-factor", CDOD::DATATYPE_FLOAT, "factor value for voxel precision, must be from 0.4 to 1.0, 1.0 for the most detailed", 1.0));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_triangulation_info_availability();

		assert_file_name_input(file, false);

		FilteringOfTriangulation::Query filtering_query=filtering_query_without_ids;
		filtering_query.atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(filtering_query.atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(data_manager.triangulation_info(), filtering_query);

		if(filtering_result.vertices_info.empty())
		{
			throw std::runtime_error(std::string("No triangulation parts selected."));
		}

		OutputSelector output_selector(file);

		int number_of_voxels=0;

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			const std::vector<apollota::SimpleSphere>& balls=data_manager.triangulation_info().spheres;

			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];

				apollota::SimplePoint q_plane_normals[4];
				apollota::SimplePoint q_plane_points[4];
				int q_plane_halfspaces[4];
				for(unsigned int j=0;j<4;j++)
				{
					const apollota::Triple triple=vi.quadruple.exclude(j);
					q_plane_normals[j]=apollota::plane_normal_from_three_points<apollota::SimplePoint>(balls[triple.get(0)], balls[triple.get(1)], balls[triple.get(2)]);
					q_plane_points[j]=apollota::SimplePoint(balls[triple.get(0)]);
					q_plane_halfspaces[j]=apollota::halfspace_of_point(q_plane_points[j], q_plane_normals[j], balls[vi.quadruple.get(j)]);
				}

				{
					const double search_r=(vi.sphere.r-filtering_query.min_radius);
					int number_of_voxels_local=0;
					for(double bx=ceil((vi.sphere.x-search_r)*search_step_factor);bx<=floor((vi.sphere.x+search_r)*search_step_factor);bx+=1.0)
					{
						for(double by=ceil((vi.sphere.y-search_r)*search_step_factor);by<=floor((vi.sphere.y+search_r)*search_step_factor);by+=1.0)
						{
							for(double bz=ceil((vi.sphere.z-search_r)*search_step_factor);bz<=floor((vi.sphere.z+search_r)*search_step_factor);bz+=1.0)
							{
								const apollota::SimpleSphere candidate_ball(bx/search_step_factor, by/search_step_factor, bz/search_step_factor, filtering_query.min_radius);
								if(apollota::sphere_contains_sphere(vi.sphere, candidate_ball))
								{
									bool center_inside_tetrahedron=true;
									for(unsigned int j=0;j<4 && center_inside_tetrahedron;j++)
									{
										center_inside_tetrahedron=(apollota::halfspace_of_point(q_plane_points[j], q_plane_normals[j], candidate_ball)==q_plane_halfspaces[j]);
									}
									if(center_inside_tetrahedron)
									{
										output << candidate_ball.x << " " << candidate_ball.y << " " << candidate_ball.z << " " << candidate_ball.r << "\n";
										number_of_voxels++;
										number_of_voxels_local++;
									}
								}
							}
						}
					}
					if(number_of_voxels_local<1)
					{
						const apollota::SimpleSphere candidate_ball(vi.sphere, filtering_query.min_radius);
						if(apollota::sphere_contains_sphere(vi.sphere, candidate_ball))
						{
							bool center_inside_tetrahedron=true;
							for(unsigned int j=0;j<4 && center_inside_tetrahedron;j++)
							{
								center_inside_tetrahedron=(apollota::halfspace_of_point(q_plane_points[j], q_plane_normals[j], candidate_ball)==q_plane_halfspaces[j]);
							}
							if(center_inside_tetrahedron)
							{
								output << candidate_ball.x << " " << candidate_ball.y << " " << candidate_ball.z << " " << candidate_ball.r << "\n";
								number_of_voxels++;
							}
						}
					}
				}
			}
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.number_of_voxels=number_of_voxels;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_TRIANGULATION_VOXELS_H_ */
