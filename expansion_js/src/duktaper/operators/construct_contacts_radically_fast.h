#ifndef DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_

#include "../../../../expansion_lt/src/voronotalt/voronotalt.h"

#include "../../../../expansion_lt/src/voronotalt_cli/mesh_writer.h"

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
		bool mesh_topology_characterized;
		scripting::SummaryOfContacts mesh_contacts_summary;
		int mesh_genus;
		int mesh_euler_characteristic;
		int mesh_connected_components;
		int mesh_boundary_components;

		Result() : mesh_topology_characterized(false), mesh_genus(-1), mesh_euler_characteristic(0), mesh_connected_components(0), mesh_boundary_components(0)
		{
		}

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
			if(mesh_topology_characterized)
			{
				scripting::VariantSerialization::write(mesh_contacts_summary, heterostorage.variant_object.object("mesh_contacts_summary"));
				heterostorage.variant_object.value("mesh_genus")=mesh_genus;
				heterostorage.variant_object.value("mesh_euler_characteristic")=mesh_euler_characteristic;
				heterostorage.variant_object.value("mesh_connected_components")=mesh_connected_components;
				heterostorage.variant_object.value("mesh_boundary_components")=mesh_boundary_components;
			}
		}
	};

	double probe;
	double restrict_circle;
	double thicken_graphics;
	bool no_intra_chain;
	bool no_intra_residue;
	bool generate_graphics;
	bool characterize_topology;
	bool no_remove_triangulation_info;
	int precutting;
	std::vector<double> adjunct_circle_restrictions;
	std::string initial_selection_expression;

	ConstructContactsRadicallyFast() : probe(1.4), restrict_circle(0.0), thicken_graphics(0.0), no_intra_chain(false), no_intra_residue(false), generate_graphics(false), characterize_topology(false), no_remove_triangulation_info(false), precutting(0)
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
		characterize_topology=input.get_flag("characterize-topology");
		no_remove_triangulation_info=input.get_flag("no-remove-triangulation-info");
		precutting=input.get_value_or_default<int>("precutting", 0);
		adjunct_circle_restrictions=input.get_value_vector_or_default<double>("adjunct-circle-restrictions", std::vector<double>());
		initial_selection_expression=input.get_value_or_default<std::string>("initial-sel", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("restrict-circle", CDOD::DATATYPE_FLOAT, "max circle restriction radius", 1.4));
		doc.set_option_decription(CDOD("thicken-graphics", CDOD::DATATYPE_FLOAT, "thickness of generated graphics", 0.0));
		doc.set_option_decription(CDOD("no-intra-chain", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-chain contacts"));
		doc.set_option_decription(CDOD("no-intra-residue", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-residue contacts"));
		doc.set_option_decription(CDOD("generate-graphics", CDOD::DATATYPE_BOOL, "flag to generate graphics"));
		doc.set_option_decription(CDOD("characterize-topology", CDOD::DATATYPE_BOOL, "flag to characterize topology of the merged contacts mesh surface"));
		doc.set_option_decription(CDOD("no-remove-triangulation-info", CDOD::DATATYPE_BOOL, "flag to not remove triangulation info"));
		doc.set_option_decription(CDOD("adjunct-circle-restrictions", CDOD::DATATYPE_FLOAT_ARRAY, "adjunct circle restriction radii", ""));
		doc.set_option_decription(CDOD("initial-sel", CDOD::DATATYPE_STRING, "initial selection expression", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		const bool with_grouping_for_filtering=(no_intra_chain || no_intra_residue);
		const bool summarize_cells=!with_grouping_for_filtering;

		data_manager.assert_atoms_availability();

		if(with_grouping_for_filtering || precutting>0)
		{
			data_manager.assert_primary_structure_info_valid();
		}

		if(precutting!=0 && !(precutting>=1100 && precutting<1104) && !(precutting>=1200 && precutting<1216) && !(precutting>=2100 && precutting<2104) && !(precutting>=2200 && precutting<2216))
		{
			throw std::runtime_error("Invalid precutting mode.");
		}

		if(precutting>0 && characterize_topology)
		{
			throw std::runtime_error("Simultaneous enabling of pre-cutting and topology characterization is not implemented yet.");
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

		voronotalt::RadicalTessellation::ParametersForPreliminaryCuts precutting_parameters;

		if(precutting>0)
		{
			const int precutting_application_regime=(precutting/1000);
			const int precutting_number_of_planes=(precutting%1000)/100;

			precutting_parameters.apply_with_single_mask=(precutting_application_regime==1);
			precutting_parameters.apply_with_all_masks=(precutting_application_regime==2);
			precutting_parameters.global_preliminary_cutting_plane_normals.resize(precutting_number_of_planes, std::vector<voronotalt::SimplePoint>(spheres.size(), voronotalt::unit_point(voronotalt::SimplePoint(1.0, 1.0, 1.0))));
			if(precutting_parameters.apply_with_single_mask)
			{
				precutting_parameters.single_mask=(precutting%100);
			}

			for(std::size_t i=0;i<spheres.size();i++)
			{
				std::vector< std::pair< std::pair<double, std::string>, std::size_t > > rneighbors;
				std::size_t j=data_manager.primary_structure_info().map_of_atoms_to_residues[i];
				if(data_manager.primary_structure_info().residues[j].atom_ids.size()>1)
				{
					for(std::size_t l=0;l<data_manager.primary_structure_info().residues[j].atom_ids.size();l++)
					{
						const std::size_t rnid=data_manager.primary_structure_info().residues[j].atom_ids[l];
						if(rnid!=i)
						{
							double dist=voronotalt::distance_from_point_to_point(spheres[i].p, spheres[rnid].p);
							if(dist<1.7)
							{
								dist=1.7;
							}
							else if(dist<3.1)
							{
								dist=3.1;
							}
							const std::pair<double, std::string> named_dist(dist, data_manager.atoms()[rnid].crad.name);
							rneighbors.push_back(std::pair< std::pair<double, std::string>, std::size_t >(named_dist, rnid));
						}
					}
					std::sort(rneighbors.begin(), rneighbors.end());
				}
				if(precutting_parameters.global_preliminary_cutting_plane_normals.size()==1)
				{
					if(rneighbors.size()==1)
					{
						precutting_parameters.global_preliminary_cutting_plane_normals[0][i]=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[0].second].p));
					}
					else if(rneighbors.size()>=2)
					{
						const voronotalt::SimplePoint dir0=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[0].second].p));
						const voronotalt::SimplePoint dir1=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[1].second].p));
						precutting_parameters.global_preliminary_cutting_plane_normals[0][i]=voronotalt::unit_point(voronotalt::sum_of_points(dir0, dir1));
					}
				}
				else if(precutting_parameters.global_preliminary_cutting_plane_normals.size()==2)
				{
					if(rneighbors.size()>=2)
					{
						precutting_parameters.global_preliminary_cutting_plane_normals[0][i]=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[0].second].p));
						precutting_parameters.global_preliminary_cutting_plane_normals[1][i]=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[1].second].p));
					}
					else if(rneighbors.size()==1)
					{
						precutting_parameters.global_preliminary_cutting_plane_normals[0][i]=voronotalt::unit_point(voronotalt::sub_of_points(spheres[i].p, spheres[rneighbors[0].second].p));
					}
				}
			}
		}

		voronotalt::RadicalTessellation::Result radical_tessellation_result;
		voronotalt::RadicalTessellation::ResultGraphics radical_tessellation_result_graphics;

		{
			std::vector<double> descending_adjunct_circle_restrictions;
			
			if(!adjunct_circle_restrictions.empty())
			{
				descending_adjunct_circle_restrictions=adjunct_circle_restrictions;
				std::reverse(descending_adjunct_circle_restrictions.begin(), descending_adjunct_circle_restrictions.end());
			}

			voronotalt::TimeRecorder mock_time_recorder;

			voronotalt::SpheresContainer spheres_container;
			spheres_container.init(spheres, mock_time_recorder);

			voronotalt::RadicalTessellation::construct_full_tessellation(
					spheres_container,
					std::vector<int>(),
					grouping_for_filtering,
					(generate_graphics || characterize_topology),
					summarize_cells,
					restrict_circle,
					descending_adjunct_circle_restrictions,
					precutting_parameters,
					radical_tessellation_result,
					radical_tessellation_result_graphics,
					mock_time_recorder);
		}

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
			const std::size_t number_of_applications_of_preliminary_cuts=(!precutting_parameters.apply_with_all_masks ? static_cast<std::size_t>(1) : static_cast<std::size_t>(precutting_parameters.calculate_number_of_possible_masks()));
			names_for_adjunct_subareas.resize(adjunct_circle_restrictions.size()*number_of_applications_of_preliminary_cuts);
			names_for_adjunct_levelareas.resize(names_for_adjunct_subareas.size());
			std::size_t lindex=0;
			for(std::size_t k=0;k<number_of_applications_of_preliminary_cuts;k++)
			{
				const int mask_class=((number_of_applications_of_preliminary_cuts>1) ? static_cast<int>(precutting_parameters.calculate_mask_class(k)) : static_cast<int>(1));
				for(std::size_t j=0;j<adjunct_circle_restrictions.size();j++)
				{
					{
						std::ostringstream name_output;
						if(number_of_applications_of_preliminary_cuts>1)
						{
							name_output << "pcut";
							print_pretty_integer_number(mask_class, name_output);
						}
						name_output << "subarea";
						print_pretty_number((j==0 ? (restrictions_positive ? 0.0 : -999.0) : adjunct_circle_restrictions[j-1]), name_output);
						name_output << "to";
						print_pretty_number(adjunct_circle_restrictions[j], name_output);
						names_for_adjunct_subareas[lindex]=name_output.str();
					}
					{
						std::ostringstream name_output;
						if(number_of_applications_of_preliminary_cuts>1)
						{
							name_output << "pcut";
							print_pretty_integer_number(mask_class, name_output);
						}
						name_output << "levelarea";
						print_pretty_number(adjunct_circle_restrictions[j], name_output);
						names_for_adjunct_levelareas[lindex]=name_output.str();
					}
					lindex++;
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
			for(std::size_t j=0;j<names_for_adjunct_subareas.size();j++)
			{
				contact.value.props.adjuncts.erase(names_for_adjunct_subareas[j]);
			}
			for(std::size_t j=0;j<names_for_adjunct_levelareas.size();j++)
			{
				contact.value.props.adjuncts.erase(names_for_adjunct_levelareas[j]);
			}
			if(!names_for_adjunct_subareas.empty() && i<radical_tessellation_result.adjuncts_for_contacts_summaries.size() && names_for_adjunct_subareas.size()==radical_tessellation_result.adjuncts_for_contacts_summaries[i].level_areas.size())
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummaryAdjunct& cdsa=radical_tessellation_result.adjuncts_for_contacts_summaries[i];
				for(std::size_t jo=0;jo<names_for_adjunct_subareas.size();jo+=adjunct_circle_restrictions.size())
				{
					for(std::size_t j=0;j<adjunct_circle_restrictions.size();j++)
					{
						std::size_t rj=jo+(adjunct_circle_restrictions.size()-1-j);
						std::size_t sj=jo+j;
						double& subarea=contact.value.props.adjuncts[names_for_adjunct_subareas[sj]];
						double& levelarea=contact.value.props.adjuncts[names_for_adjunct_levelareas[sj]];
						if(rj<cdsa.level_areas.size())
						{
							subarea+=(((rj+1-jo)<adjunct_circle_restrictions.size()) ? (cdsa.level_areas[rj]-cdsa.level_areas[rj+1]) : cdsa.level_areas[rj]);
							levelarea+=cdsa.level_areas[rj];
						}
						total_subareas[names_for_adjunct_subareas[sj]]+=subarea;
					}
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

		std::set<std::size_t> initial_contact_ids;
		voronotalt::MeshWriter mesh_writer(characterize_topology);

		if(!initial_selection_expression.empty())
		{
			initial_contact_ids=data_manager.selection_manager().select_contacts(scripting::SelectionManager::Query(initial_selection_expression, false));
			if(initial_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No initial contacts selected."));
			}

			if(mesh_writer.enabled())
			{
				for(std::set<std::size_t>::const_iterator it_contact_ids=initial_contact_ids.begin();it_contact_ids!=initial_contact_ids.end();++it_contact_ids)
				{
					const std::size_t contact_id=(*it_contact_ids);
					if(contact_id<radical_tessellation_result_graphics.contacts_graphics.size())
					{
						const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& cdg=radical_tessellation_result_graphics.contacts_graphics[contact_id];
						mesh_writer.add_triangle_fan(cdg.outer_points, cdg.boundary_mask, cdg.barycenter);
					}
				}
			}
		}
		else
		{
			if(mesh_writer.enabled())
			{
				for(std::size_t i=0;i<radical_tessellation_result_graphics.contacts_graphics.size();i++)
				{
					const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& cdg=radical_tessellation_result_graphics.contacts_graphics[i];
					mesh_writer.add_triangle_fan(cdg.outer_points, cdg.boundary_mask, cdg.barycenter);
				}
			}
		}

		Result result;
		result.contacts_summary=scripting::SummaryOfContacts(data_manager.contacts());
		result.map_of_subareas=total_subareas;
		if(mesh_writer.enabled())
		{
			result.mesh_contacts_summary=scripting::SummaryOfContacts(data_manager.contacts(), initial_contact_ids);
			result.mesh_topology_characterized=true;
			result.mesh_genus=mesh_writer.calculate_genus();
			result.mesh_euler_characteristic=mesh_writer.get_euler_characteristic();
			result.mesh_connected_components=mesh_writer.get_number_of_connected_components();
			result.mesh_boundary_components=mesh_writer.get_number_of_boundary_components();
		}

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

	static void print_pretty_integer_number(const int ival, std::ostream& output)
	{
		output << (ival<10 ? "0000" : (ival<100 ? "000" : (ival<1000 ? "00" : (ival<10000 ? "0" : "")))) << ival;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_ */
