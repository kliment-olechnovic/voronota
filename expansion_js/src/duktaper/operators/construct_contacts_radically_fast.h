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
		std::map<std::string, double> map_of_subareas;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			if(!map_of_subareas.empty())
			{
				std::vector<scripting::VariantValue>& subareas_names=heterostorage.variant_object.values_array("subareas_names");
				std::vector<scripting::VariantValue>& subareas_values=heterostorage.variant_object.values_array("subareas_values");
				for(std::map<std::string, double>::const_iterator it=map_of_subareas.begin();it!=map_of_subareas.end();++it)
				{
					subareas_names.push_back(scripting::VariantValue(it->first));
					subareas_values.push_back(scripting::VariantValue(it->second));
				}
			}
		}
	};

	double probe;
	double restrict_circle;
	double thicken_graphics;
	bool no_intra_chain;
	bool no_intra_residue;
	bool generate_graphics;
	bool no_remove_triangulation_info;
	std::vector<double> adjunct_circle_restrictions;

	ConstructContactsRadicallyFast() : probe(1.4), restrict_circle(0.0), thicken_graphics(0.0), no_intra_chain(false), no_intra_residue(false), generate_graphics(false), no_remove_triangulation_info(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		probe=input.get_value_or_default<double>("probe", 1.4);
		restrict_circle=input.get_value_or_default<double>("restrict-circle", 0.0);
		thicken_graphics=input.get_value_or_default<double>("thicken-graphics", 0.0);
		no_intra_chain=input.get_flag("no-intra-chain");
		no_intra_residue=input.get_flag("no-intra-residue");
		generate_graphics=input.get_flag("generate-graphics");
		no_remove_triangulation_info=input.get_flag("no-remove-triangulation-info");
		adjunct_circle_restrictions=input.get_value_vector_or_default<double>("adjunct-circle-restrictions", std::vector<double>());
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("restrict-circle", CDOD::DATATYPE_FLOAT, "max circle restriction radius", 1.4));
		doc.set_option_decription(CDOD("thicken-graphics", CDOD::DATATYPE_FLOAT, "thickness of generated graphics", 0.0));
		doc.set_option_decription(CDOD("no-intra-chain", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-chain contacts"));
		doc.set_option_decription(CDOD("no-intra-residue", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-residue contacts"));
		doc.set_option_decription(CDOD("generate-graphics", CDOD::DATATYPE_BOOL, "flag to generate graphics"));
		doc.set_option_decription(CDOD("no-remove-triangulation-info", CDOD::DATATYPE_BOOL, "flag to not remove triangulation info"));
		doc.set_option_decription(CDOD("adjunct-circle-restrictions", CDOD::DATATYPE_FLOAT_ARRAY, "adjunct circle restriction radii", ""));
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

		bool restrictions_positive=false;
		bool restrictions_negative=false;

		if(!adjunct_circle_restrictions.empty())
		{
			for(std::size_t i=0;i<adjunct_circle_restrictions.size();i++)
			{
				if(i>0 && adjunct_circle_restrictions[i-1]>=adjunct_circle_restrictions[i])
				{
					throw std::runtime_error("Invalid order of circle restrictions, must be ascending.");
				}
				restrictions_positive=restrictions_positive || adjunct_circle_restrictions[i]>0.0;
				restrictions_negative=restrictions_negative || adjunct_circle_restrictions[i]<0.0;
			}
		}

		if(restrictions_positive && restrictions_negative)
		{
			throw std::runtime_error("Invalid signs of circle restrictions, must be either all positive or all negative.");
		}

		std::vector<double> descending_adjunct_circle_restrictions;
		if(!adjunct_circle_restrictions.empty())
		{
			descending_adjunct_circle_restrictions=adjunct_circle_restrictions;
			std::reverse(descending_adjunct_circle_restrictions.begin(), descending_adjunct_circle_restrictions.end());
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
				descending_adjunct_circle_restrictions,
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

		std::vector<std::string> names_for_adjunct_subareas;
		std::vector<std::string> names_for_adjunct_levelareas;
		if(!adjunct_circle_restrictions.empty())
		{
			names_for_adjunct_subareas.resize(adjunct_circle_restrictions.size());
			names_for_adjunct_levelareas.resize(adjunct_circle_restrictions.size());
			for(std::size_t j=0;j<adjunct_circle_restrictions.size();j++)
			{
				{
					std::ostringstream name_output;
					name_output << "subarea";
					print_pretty_number((j==0 ? (restrictions_positive ? 0.0 : -999.0) : adjunct_circle_restrictions[j-1]), name_output);
					name_output << "to";
					print_pretty_number(adjunct_circle_restrictions[j], name_output);
					names_for_adjunct_subareas[j]=name_output.str();
				}
				{
					std::ostringstream name_output;
					name_output << "levelarea";
					print_pretty_number(adjunct_circle_restrictions[j], name_output);
					names_for_adjunct_levelareas[j]=name_output.str();
				}
			}
		}

		std::vector<scripting::Contact> contacts;
		contacts.reserve(radical_tessellation_result.contacts_summaries.size()+radical_tessellation_result.cells_summaries.size());

		std::map<std::string, double> total_subareas;

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

				for(std::size_t j=0;j<adjunct_circle_restrictions.size();j++)
				{
					std::size_t rj=(adjunct_circle_restrictions.size()-1-j);
					double& subarea=contact.value.props.adjuncts[names_for_adjunct_subareas[j]];
					double& levelarea=contact.value.props.adjuncts[names_for_adjunct_levelareas[j]];
					if(rj<cdsa.level_areas.size())
					{
						subarea=(((rj+1)<cdsa.level_areas.size()) ? (cdsa.level_areas[rj]-cdsa.level_areas[rj+1]) : cdsa.level_areas[rj]);
						levelarea=cdsa.level_areas[rj];
					}
					else
					{
						subarea=0.0;
						levelarea=0.0;
					}
					total_subareas[names_for_adjunct_subareas[j]]+=subarea;
				}
			}
			if(generate_graphics && i<radical_tessellation_result_graphics.contacts_graphics.size())
			{
				const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& cdg=radical_tessellation_result_graphics.contacts_graphics[i];
				auxiliaries::OpenGLPrinter opengl_printer;
				if(thicken_graphics>0.0)
				{
					for(int j=0;j<2;j++)
					{
						voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics cdg_mod=cdg;
						if(j>0)
						{
							cdg_mod.plane_normal=voronotalt::point_and_number_product(cdg.plane_normal, -1.0);
						}
						const voronotalt::SimplePoint pos_shift=voronotalt::point_and_number_product(cdg_mod.plane_normal, thicken_graphics);
						cdg_mod.barycenter=voronotalt::sum_of_points(cdg_mod.barycenter, pos_shift);
						for(std::size_t l=0;l<cdg_mod.outer_points.size();l++)
						{
							cdg_mod.outer_points[l]=voronotalt::sum_of_points(cdg_mod.outer_points[l], pos_shift);
						}
						opengl_printer.add_triangle_fan(cdg_mod.barycenter, cdg_mod.outer_points, cdg_mod.plane_normal);
					}
				}
				else
				{
					opengl_printer.add_triangle_fan(cdg.barycenter, cdg.outer_points, cdg.plane_normal);
				}
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
		result.map_of_subareas=total_subareas;

		return result;
	}

private:
	static void print_pretty_number(const double val, std::ostream& output)
	{
		const int ival=std::abs(static_cast<int>(std::floor(val*100.0+0.5)));
		if(val<0.0)
		{
			output << "M";
		}
		output << (ival<10 ? "0000" : (ival<100 ? "000" : (ival<1000 ? "00" : (ival<10000 ? "0" : "")))) << ival;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_ */
