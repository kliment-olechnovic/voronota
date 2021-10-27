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

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
	SelectionManager::Query parameters_for_selecting;
	std::string obj_file;
	std::string mtl_file;
	std::string color_file;
	bool no_reordering;

	ExportContactsAsConnectedMesh() : no_reordering(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_to_draw_contacts=common::ConstructionOfContacts::ParametersToDrawContacts();
		parameters_to_draw_contacts.probe=input.get_value_or_default<double>("probe", parameters_to_draw_contacts.probe);
		parameters_to_draw_contacts.step=input.get_value_or_default<double>("step", parameters_to_draw_contacts.step);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		obj_file=input.get_value<std::string>("obj-file");
		assert_file_name_input(obj_file, false);
		mtl_file=input.get_value<std::string>("mtl-file");
		assert_file_name_input(mtl_file, true);
		color_file=input.get_value<std::string>("color-file");
		assert_file_name_input(color_file, true);
		no_reordering=input.get_flag("no-reordering");
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfContacts::ParametersToDrawContacts params;
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", params.probe));
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("obj-file", CDOD::DATATYPE_STRING, "path to output OBJ file"));
		doc.set_option_decription(CDOD("mtl-file", CDOD::DATATYPE_STRING, "path to output MTL file"));
		doc.set_option_decription(CDOD("color-file", CDOD::DATATYPE_STRING, "path to output face colors file"));
		doc.set_option_decription(CDOD("no-reordering", CDOD::DATATYPE_BOOL, "flag to disable reordering of vertices"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(obj_file, false);
		assert_file_name_input(mtl_file, true);
		assert_file_name_input(color_file, true);

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

		const bool record_coloring=(!mtl_file.empty() || !color_file.empty());

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

		apollota::ConstrainedContactsInterfaceMesh ccim(
				data_manager.triangulation_info().spheres,
				data_manager.triangulation_info().quadruples_map,
				set_of_ab_pairs,
				parameters_to_draw_contacts.probe,
				parameters_to_draw_contacts.step,
				parameters_to_draw_contacts.projections,
				no_reordering);

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
			OutputSelector output_selector(obj_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(obj_file, output);

			if(!mtl_file.empty())
			{
				output << "mtllib " << mtl_file << "\n";
			}

			for(std::size_t i=0;i<ccim.mesh_vertices().size();i++)
			{
				output << "v " << ccim.mesh_vertices()[i].point << "\n";
			}

			if(mtl_file.empty())
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

		if(!mtl_file.empty())
		{
			OutputSelector output_selector(mtl_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(mtl_file, output);

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

		if(!color_file.empty())
		{
			OutputSelector output_selector(color_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(color_file, output);

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

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_CONNECTED_MESH_H_ */
