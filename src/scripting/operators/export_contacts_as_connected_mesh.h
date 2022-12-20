#ifndef SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_CONNECTED_MESH_H_
#define SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_CONNECTED_MESH_H_

#include "../operators_common.h"

#include "../../apollota/constrained_contacts_interface_mesh.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportContactsAsConnectedMesh : public OperatorBase<ExportContactsAsConnectedMesh>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		bool manifold;
		double step;

		Result() : manifold(false), step(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			heterostorage.variant_object.value("manifold")=manifold;
			heterostorage.variant_object.value("step")=step;
		}
	};

	common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
	SelectionManager::Query parameters_for_selecting;
	bool only_largest_component;
	std::vector<double> alt_step_tries;
	std::string output_obj_file;
	std::string output_mtl_file;
	std::string output_color_file;

	ExportContactsAsConnectedMesh() : only_largest_component(false)
	{
	}

	void initialize(CommandInput& input, const bool managed)
	{
		parameters_to_draw_contacts=common::ConstructionOfContacts::ParametersToDrawContacts();
		parameters_to_draw_contacts.probe=input.get_value_or_default<double>("probe", 0.0);
		parameters_to_draw_contacts.step=input.get_value_or_default<double>("step", parameters_to_draw_contacts.step);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		only_largest_component=input.get_flag("only-largest-component");
		alt_step_tries=input.get_value_vector_or_default<double>("alt-step-tries", generate_default_alt_step_tries());
		if(!managed)
		{
			output_obj_file=input.get_value<std::string>("output-obj-file");
			assert_file_name_input(output_obj_file, false);
			output_mtl_file=input.get_value_or_default<std::string>("output-mtl-file", "");
			assert_file_name_input(output_mtl_file, true);
			output_color_file=input.get_value_or_default<std::string>("output-color-file", "");
			assert_file_name_input(output_color_file, true);
		}
	}

	void initialize(CommandInput& input)
	{
		initialize(input, false);
	}

	void document(CommandDocumentation& doc, const bool managed) const
	{
		common::ConstructionOfContacts::ParametersToDrawContacts params;
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius, or 0.0 to use the probe radius of construction", 0.0));
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("only-largest-component", CDOD::DATATYPE_BOOL, "flag to only output the largest connected component"));
		doc.set_option_decription(CDOD("alt-step-tries", CDOD::DATATYPE_FLOAT_ARRAY, "values of step to try for a manifold result", "0.05 0.10 ... 0.45 0.50"));
		if(!managed)
		{
			doc.set_option_decription(CDOD("output-obj-file", CDOD::DATATYPE_STRING, "path to output OBJ file"));
			doc.set_option_decription(CDOD("output-mtl-file", CDOD::DATATYPE_STRING, "path to output MTL file", ""));
			doc.set_option_decription(CDOD("output-color-file", CDOD::DATATYPE_STRING, "path to output face colors file", ""));
		}
	}

	void document(CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(output_obj_file, false);
		assert_file_name_input(output_mtl_file, true);
		assert_file_name_input(output_color_file, true);

		const std::set<std::size_t> all_contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(all_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> contact_ids;
		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const std::size_t id=(*it);
			if(!data_manager.contacts()[id].solvent())
			{
				contact_ids.insert(id);
			}
		}

		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No non-solvent contacts selected."));
		}

		const bool record_coloring=(!output_mtl_file.empty() || !output_color_file.empty());

		std::set<apollota::Pair> set_of_ab_pairs;
		std::map< unsigned int, std::set<apollota::Pair> > map_of_colors_to_ab_pairs;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t id=(*it);
			const Contact& contact=data_manager.contacts()[id];
			const apollota::Pair ab_pair(contact.ids[0], contact.ids[1]);
			set_of_ab_pairs.insert(ab_pair);
			if(record_coloring && !data_manager.contacts_display_states()[id].visuals.empty())
			{
				const DataManager::DisplayState::Visual& dsv=data_manager.contacts_display_states()[id].visuals[0];
				map_of_colors_to_ab_pairs[dsv.color].insert(ab_pair);
			}
		}

		common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts_to_use=parameters_to_draw_contacts;
		if(parameters_to_draw_contacts_to_use.probe<0.0001)
		{
			parameters_to_draw_contacts_to_use.probe=data_manager.history_of_actions_on_contacts().probe();
		}

		apollota::ConstrainedContactsInterfaceMesh ccim(
				data_manager.triangulation_info().spheres,
				data_manager.triangulation_info().quadruples_map,
				set_of_ab_pairs,
				parameters_to_draw_contacts_to_use.probe,
				parameters_to_draw_contacts_to_use.step,
				parameters_to_draw_contacts_to_use.projections,
				only_largest_component);

		double step_used=parameters_to_draw_contacts_to_use.step;

		for(std::size_t i=0;i<alt_step_tries.size() && !ccim.check_manifold();i++)
		{
			const double alt_step=alt_step_tries[i];
			if(alt_step!=parameters_to_draw_contacts_to_use.step)
			{
				ccim=apollota::ConstrainedContactsInterfaceMesh(
						data_manager.triangulation_info().spheres,
						data_manager.triangulation_info().quadruples_map,
						set_of_ab_pairs,
						parameters_to_draw_contacts_to_use.probe,
						alt_step,
						parameters_to_draw_contacts_to_use.projections,
						only_largest_component);
				step_used=alt_step;
			}
		}

		if(!alt_step_tries.empty() && !ccim.check_manifold())
		{
			throw std::runtime_error(std::string("Failed to produce manifold."));
		}

		std::map< apollota::Pair, std::set<std::size_t> > map_of_ab_pairs_to_mesh_faces;
		if(record_coloring)
		{
			for(std::size_t i=0;i<ccim.mesh_faces().size();i++)
			{
				const apollota::ConstrainedContactsInterfaceMesh::MeshFace& mf=ccim.mesh_faces()[i];
				map_of_ab_pairs_to_mesh_faces[mf.pair_of_generator_ids].insert(i);
			}
		}

		{
			OutputSelector output_selector(output_obj_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(output_obj_file, output);

			if(!output_mtl_file.empty())
			{
				output << "mtllib " << output_mtl_file << "\n";
			}

			for(std::size_t i=0;i<ccim.mesh_vertices().size();i++)
			{
				output << "v " << ccim.mesh_vertices()[i].point << "\n";
			}

			if(output_mtl_file.empty())
			{
				for(std::size_t i=0;i<ccim.mesh_faces().size();i++)
				{
					const apollota::ConstrainedContactsInterfaceMesh::MeshFace& mf=ccim.mesh_faces()[i];
					output << "f " << (mf.triple_of_mesh_vertex_ids[0]+1) << " " << (mf.triple_of_mesh_vertex_ids[1]+1) << " " << (mf.triple_of_mesh_vertex_ids[2]+1) << "\n";
				}
			}
			else
			{
				for(std::map< unsigned int, std::set<apollota::Pair> >::const_iterator map_of_colors_to_ab_pairs_it=map_of_colors_to_ab_pairs.begin();map_of_colors_to_ab_pairs_it!=map_of_colors_to_ab_pairs.end();++map_of_colors_to_ab_pairs_it)
				{
					const unsigned int color=map_of_colors_to_ab_pairs_it->first;
					output << "usemtl col" << color << "\n";
					const std::set<apollota::Pair>& face_pairs=map_of_colors_to_ab_pairs_it->second;
					for(std::set<apollota::Pair>::const_iterator face_pairs_it=face_pairs.begin();face_pairs_it!=face_pairs.end();++face_pairs_it)
					{
						std::set<std::size_t>& face_ids=map_of_ab_pairs_to_mesh_faces[*face_pairs_it];
						for(std::set<std::size_t>::const_iterator face_ids_it=face_ids.begin();face_ids_it!=face_ids.end();++face_ids_it)
						{
							const apollota::ConstrainedContactsInterfaceMesh::MeshFace& mf=ccim.mesh_faces()[*face_ids_it];
							output << "f " << (mf.triple_of_mesh_vertex_ids[0]+1) << " " << (mf.triple_of_mesh_vertex_ids[1]+1) << " " << (mf.triple_of_mesh_vertex_ids[2]+1) << "\n";
						}
					}
				}
			}
		}

		if(!output_mtl_file.empty())
		{
			OutputSelector output_selector(output_mtl_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(output_mtl_file, output);

			for(std::map< unsigned int, std::set<apollota::Pair> >::const_iterator map_of_colors_to_ab_pairs_it=map_of_colors_to_ab_pairs.begin();map_of_colors_to_ab_pairs_it!=map_of_colors_to_ab_pairs.end();++map_of_colors_to_ab_pairs_it)
			{
				const unsigned int color=map_of_colors_to_ab_pairs_it->first;
				float rgb[3]={0.0, 0.0, 0.0};
				auxiliaries::ColorUtilities::color_to_components(color, &rgb[0], true);
				output << "newmtl col" << color << "\n";
				output << "Ka " << rgb[0] << " " << rgb[1] << " " << rgb[2] << "\n";
				output << "Kd " << rgb[0] << " " << rgb[1] << " " << rgb[2] << "\n";
				output << "Ks " << rgb[0] << " " << rgb[1] << " " << rgb[2] << "\n";
				output << "Ns 1.0\n";
				output << "Ni 1.0\n";
				output << "d 1.0\n";
				output << "illum 0\n";
			}
		}

		if(!output_color_file.empty())
		{
			OutputSelector output_selector(output_color_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(output_color_file, output);

			for(std::map< unsigned int, std::set<apollota::Pair> >::const_iterator map_of_colors_to_ab_pairs_it=map_of_colors_to_ab_pairs.begin();map_of_colors_to_ab_pairs_it!=map_of_colors_to_ab_pairs.end();++map_of_colors_to_ab_pairs_it)
			{
				const unsigned int color=map_of_colors_to_ab_pairs_it->first;
				const std::set<apollota::Pair>& face_pairs=map_of_colors_to_ab_pairs_it->second;
				for(std::set<apollota::Pair>::const_iterator face_pairs_it=face_pairs.begin();face_pairs_it!=face_pairs.end();++face_pairs_it)
				{
					std::set<std::size_t>& face_ids=map_of_ab_pairs_to_mesh_faces[*face_pairs_it];
					for(std::set<std::size_t>::const_iterator face_ids_it=face_ids.begin();face_ids_it!=face_ids.end();++face_ids_it)
					{
						const apollota::ConstrainedContactsInterfaceMesh::MeshFace& mf=ccim.mesh_faces()[*face_ids_it];
						output << (mf.triple_of_mesh_vertex_ids[0]+1) << " " << (mf.triple_of_mesh_vertex_ids[1]+1) << " " << (mf.triple_of_mesh_vertex_ids[2]+1) << " " << color << "\n";
					}
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);
		result.manifold=ccim.check_manifold();
		result.step=step_used;

		return result;
	}

private:
	static std::vector<double> generate_default_alt_step_tries()
	{
		std::vector<double> steps;
		for(int step=5;step<=50;step+=5)
		{
			steps.push_back(static_cast<double>(step)/100.0);
		}
		return steps;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_CONNECTED_MESH_H_ */
