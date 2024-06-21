#ifndef DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_

#include "../../../../expansion_lt/src/voronotalt/voronotalt.h"

#include "../../../../src/auxiliaries/opengl_printer.h"

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConstructContactsRadicallyFast : public scripting::OperatorBase<ConstructContactsRadicallyFast>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::SummaryOfContacts contacts_summary;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	double probe;
	double restrict_circle;
	bool no_intra_chain;
	bool no_intra_residue;
	bool generate_graphics;
	bool no_remove_triangulation_info;
	std::vector<double> adjunct_circle_restrictions;

	ConstructContactsRadicallyFast() : probe(1.4), restrict_circle(0.0), no_intra_chain(false), no_intra_residue(false), generate_graphics(false), no_remove_triangulation_info(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		probe=input.get_value_or_default<double>("probe", 1.4);
		restrict_circle=input.get_value_or_default<double>("restrict-circle", 0.0);
		no_intra_chain=input.get_flag("no-intra-chain");
		no_intra_residue=input.get_flag("no-intra-residue");
		generate_graphics=input.get_flag("generate-graphics");
		no_remove_triangulation_info=input.get_flag("no-remove-triangulation-info");
		adjunct_circle_restrictions=input.get_value_vector_or_default<double>("adjunct-circle-resctrictions", std::vector<double>());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("restrict-circle", CDOD::DATATYPE_FLOAT, "max circle restriction radius", 1.4));
		doc.set_option_decription(CDOD("no-intra-chain", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-chain contacts"));
		doc.set_option_decription(CDOD("no-intra-residue", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-residue contacts"));
		doc.set_option_decription(CDOD("generate-graphics", CDOD::DATATYPE_BOOL, "flag to generate graphics"));
		doc.set_option_decription(CDOD("no-remove-triangulation-info", CDOD::DATATYPE_BOOL, "flag to not remove triangulation info"));
		doc.set_option_decription(CDOD("adjunct-circle-resctrictions", CDOD::DATATYPE_FLOAT_ARRAY, "adjunct circle restriction radii", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		const bool with_grouping_for_filtering=(no_intra_chain || no_intra_residue);
		const bool summarize_cells=!with_grouping_for_filtering;

		data_manager.assert_atoms_availability();

		if(with_grouping_for_filtering)
		{
			data_manager.assert_primary_structure_info_valid();
		}

		std::vector<voronotalt::SimpleSphere> spheres(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			voronotalt::fill_sphere_from_ball(data_manager.atoms()[i].value, probe, spheres[i]);
		}

		std::vector<int> grouping_for_filtering;
		if(with_grouping_for_filtering)
		{
			grouping_for_filtering.resize(spheres.size(), 0);
			for(std::size_t i=0;i<spheres.size();i++)
			{
				const std::size_t residue_index=data_manager.primary_structure_info().map_of_atoms_to_residues[i];
				if(no_intra_chain)
				{
					const std::size_t chain_index=data_manager.primary_structure_info().map_of_residues_to_chains[residue_index];
					grouping_for_filtering[i]=static_cast<int>(chain_index);
				}
				else if(no_intra_residue)
				{
					grouping_for_filtering[i]=static_cast<int>(residue_index);
				}
			}
		}

		voronotalt::TimeRecorder mock_time_recorder;

		voronotalt::SpheresContainer spheres_container;
		spheres_container.init(spheres, mock_time_recorder);

		voronotalt::RadicalTessellation::Result radical_tessellation_result;
		voronotalt::RadicalTessellation::ResultGraphics radical_tessellation_result_graphics;
		voronotalt::RadicalTessellation::construct_full_tessellation(
				spheres_container,
				std::vector<int>(),
				grouping_for_filtering,
				generate_graphics,
				summarize_cells,
				restrict_circle,
				adjunct_circle_restrictions,
				radical_tessellation_result,
				radical_tessellation_result_graphics,
				mock_time_recorder);


		if(radical_tessellation_result.contacts_summaries.empty())
		{
			throw std::runtime_error("No contacts constructed for the provided atoms and probe.");
		}

		if(summarize_cells && radical_tessellation_result.cells_summaries.empty())
		{
			throw std::runtime_error("No cells constructed for the provided atoms and probe.");
		}

		std::vector<std::string> names_for_adjunct_circle_resctrictions;
		if(!adjunct_circle_restrictions.empty())
		{
			names_for_adjunct_circle_resctrictions.resize(adjunct_circle_restrictions.size());
			for(std::size_t j=0;j<names_for_adjunct_circle_resctrictions.size();j++)
			{
				std::ostringstream name_output;
				name_output << "subarea" << j;
				names_for_adjunct_circle_resctrictions[j]=name_output.str();
			}
		}

		std::vector<scripting::Contact> contacts;
		contacts.reserve(radical_tessellation_result.contacts_summaries.size()+radical_tessellation_result.cells_summaries.size());

		for(std::size_t i=0;i<radical_tessellation_result.contacts_summaries.size();i++)
		{
			const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=radical_tessellation_result.contacts_summaries[i];
			contacts.push_back(scripting::Contact());
			scripting::Contact& contact=contacts.back();
			contact.ids[0]=cds.id_a;
			contact.ids[1]=cds.id_b;
			contact.value.area=cds.area;
			contact.value.dist=cds.distance;
			contact.value.props.adjuncts["boundary"]=cds.arc_length;
			if(cds.flags>0)
			{
				contact.value.props.tags.insert("central");
			}
			if(!adjunct_circle_restrictions.empty() && i<radical_tessellation_result.adjuncts_for_contacts_summaries.size())
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummaryAdjunct& cdsa=radical_tessellation_result.adjuncts_for_contacts_summaries[i];
				for(std::size_t j=0;j<cdsa.level_areas.size() && j<names_for_adjunct_circle_resctrictions.size();j++)
				{
					contact.value.props.adjuncts[names_for_adjunct_circle_resctrictions[j]]=cdsa.level_areas[j];
				}
			}
			if(generate_graphics && i<radical_tessellation_result_graphics.contacts_graphics.size())
			{
				auxiliaries::OpenGLPrinter opengl_printer;
				opengl_printer.add_triangle_fan(
						radical_tessellation_result_graphics.contacts_graphics[i].barycenter,
						radical_tessellation_result_graphics.contacts_graphics[i].outer_points,
						voronotalt::unit_point(voronotalt::sub_of_points(spheres[cds.id_b].p, spheres[cds.id_a].p)));
				contact.value.graphics=opengl_printer.str();
			}
		}

		for(std::size_t i=0;i<radical_tessellation_result.cells_summaries.size();i++)
		{
			const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=radical_tessellation_result.cells_summaries[i];
			if(ccds.sas_area>0.0)
			{
				contacts.push_back(scripting::Contact());
				scripting::Contact& contact=contacts.back();
				contact.ids[0]=ccds.id;
				contact.ids[1]=ccds.id;
				contact.value.area=ccds.sas_area;
				contact.value.dist=spheres[ccds.id].r+probe*2.0;
			}
			data_manager.atom_adjuncts_mutable(ccds.id)["volume"]=ccds.sas_inside_volume;
		}

		if(!no_remove_triangulation_info)
		{
			data_manager.remove_triangulation_info();
		}

		data_manager.reset_contacts_by_swapping(contacts);

		Result result;
		result.contacts_summary=scripting::SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_ */
