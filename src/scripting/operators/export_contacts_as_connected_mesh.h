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
	std::string file;
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
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
		no_reordering=input.get_flag("no-reordering");
	}

	void document(CommandDocumentation& doc) const
	{
		common::ConstructionOfContacts::ParametersToDrawContacts params;
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", params.probe));
		doc.set_option_decription(CDOD("step", CDOD::DATATYPE_FLOAT, "edge step size", params.step));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("no-reordering", CDOD::DATATYPE_BOOL, "flag to disable reordering of vertices"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(file, false);

		const std::set<std::size_t> all_contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(all_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> contact_ids;
		std::set<apollota::Pair> ab_pairs;
		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const std::size_t id=(*it);
			const Contact& contact=data_manager.contacts()[id];
			if(!contact.solvent())
			{
				contact_ids.insert(id);
				ab_pairs.insert(apollota::Pair(contact.ids[0], contact.ids[1]));
			}
		}

		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No non-solvent contacts selected."));
		}

		apollota::ConstrainedContactsInterfaceMesh ccim(
				data_manager.triangulation_info().spheres,
				data_manager.triangulation_info().quadruples_map,
				ab_pairs,
				parameters_to_draw_contacts.probe,
				parameters_to_draw_contacts.step,
				parameters_to_draw_contacts.projections,
				no_reordering);


		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0x777777);
		for(std::size_t i=0;i<ccim.mesh_links().size();i++)
		{
			const apollota::Pair& link=ccim.mesh_links()[i];
			opengl_printer.add_line_strip(ccim.mesh_vertices()[link.get(0)].point, ccim.mesh_vertices()[link.get(1)].point);
		}

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			for(std::size_t i=0;i<ccim.mesh_vertices().size();i++)
			{
				output << "v " << ccim.mesh_vertices()[i].point << "\n";
			}
			for(std::size_t i=0;i<ccim.mesh_faces().size();i++)
			{
				const apollota::ConstrainedContactsInterfaceMesh::MeshFace& mf=ccim.mesh_faces()[i];
				output << "f " << (mf.triple_of_mesh_vertex_ids[0]+1) << " " << (mf.triple_of_mesh_vertex_ids[1]+1) << " " << (mf.triple_of_mesh_vertex_ids[2]+1) << " " << "\n";
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
