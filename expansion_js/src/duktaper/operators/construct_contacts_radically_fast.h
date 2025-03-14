#ifndef DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_CONTACTS_RADICALLY_FAST_H_

#include "../../../../expansion_lt/src/voronotalt/voronotalt.h"

#include "../../../../expansion_lt/src/voronotalt_cli/mesh_writer.h"

#include "../../../../src/auxiliaries/opengl_printer.h"

#include "../../../../src/scripting/primitive_atom_directions_assignment.h"

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
	bool calculate_adjacencies;
	bool generate_graphics;
	bool characterize_topology;
	bool no_remove_triangulation_info;
	int precutting_variant;
	bool add_collapsed_adjuncts;
	std::vector<double> adjunct_circle_restrictions;
	std::vector<double> precutting_shifts;
	std::string initial_selection_expression;

	ConstructContactsRadicallyFast() : probe(1.4), restrict_circle(0.0), thicken_graphics(0.0), no_intra_chain(false), no_intra_residue(false), calculate_adjacencies(false), generate_graphics(false), characterize_topology(false), no_remove_triangulation_info(false), precutting_variant(-1), add_collapsed_adjuncts(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		probe=input.get_value_or_default<double>("probe", 1.4);
		restrict_circle=input.get_value_or_default<double>("restrict-circle", 0.0);
		thicken_graphics=input.get_value_or_default<double>("thicken-graphics", 0.0);
		no_intra_chain=input.get_flag("no-intra-chain");
		no_intra_residue=input.get_flag("no-intra-residue");
		calculate_adjacencies=input.get_flag("calculate-adjacencies");
		generate_graphics=input.get_flag("generate-graphics");
		characterize_topology=input.get_flag("characterize-topology");
		no_remove_triangulation_info=input.get_flag("no-remove-triangulation-info");
		precutting_variant=input.get_value_or_default<int>("precutting-variant", -1);
		add_collapsed_adjuncts=input.get_flag("add-collapsed-adjuncts");
		adjunct_circle_restrictions=input.get_value_vector_or_default<double>("adjunct-circle-restrictions", std::vector<double>());
		precutting_shifts=input.get_value_vector_or_default<double>("precutting-shifts", std::vector<double>());
		initial_selection_expression=input.get_value_or_default<std::string>("initial-sel", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("restrict-circle", CDOD::DATATYPE_FLOAT, "max circle restriction radius", 1.4));
		doc.set_option_decription(CDOD("thicken-graphics", CDOD::DATATYPE_FLOAT, "thickness of generated graphics", 0.0));
		doc.set_option_decription(CDOD("no-intra-chain", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-chain contacts"));
		doc.set_option_decription(CDOD("no-intra-residue", CDOD::DATATYPE_BOOL, "flag to skip constructing intra-residue contacts"));
		doc.set_option_decription(CDOD("calculate-adjacencies", CDOD::DATATYPE_BOOL, "flag to calculate contact-contact adjacency values"));
		doc.set_option_decription(CDOD("generate-graphics", CDOD::DATATYPE_BOOL, "flag to generate graphics"));
		doc.set_option_decription(CDOD("characterize-topology", CDOD::DATATYPE_BOOL, "flag to characterize topology of the merged contacts mesh surface"));
		doc.set_option_decription(CDOD("no-remove-triangulation-info", CDOD::DATATYPE_BOOL, "flag to not remove triangulation info"));
		doc.set_option_decription(CDOD("adjunct-circle-restrictions", CDOD::DATATYPE_FLOAT_ARRAY, "adjunct circle restriction radii", ""));
		doc.set_option_decription(CDOD("precutting-variant", CDOD::DATATYPE_INT, "precutting variant", -1));
		doc.set_option_decription(CDOD("add-collapsed-adjuncts", CDOD::DATATYPE_BOOL, "flag to add collapse adjuncts of subarea value"));
		doc.set_option_decription(CDOD("precutting-shifts", CDOD::DATATYPE_FLOAT_ARRAY, "precutting plane shift values", ""));
		doc.set_option_decription(CDOD("initial-sel", CDOD::DATATYPE_STRING, "initial selection expression", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		const bool with_grouping_for_filtering=(no_intra_chain || no_intra_residue);
		const bool summarize_cells=!with_grouping_for_filtering;

		data_manager.assert_atoms_availability();

		if(with_grouping_for_filtering || !precutting_shifts.empty())
		{
			data_manager.assert_primary_structure_info_valid();
		}

		if(precutting_shifts.size()>2)
		{
			throw std::runtime_error("Invalid number of precutting plane shifts, must be not greater than two.");
		}

		if(precutting_variant>=(1 << (precutting_shifts.size()*2)))
		{
			throw std::runtime_error("Invalid precutting variant, must be less than pow(2, 2*number_ofprecutting_shifts).");
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

		if(!precutting_shifts.empty())
		{
			precutting_parameters.apply_with_single_mask=(precutting_variant>=0);
			precutting_parameters.apply_with_all_masks=(precutting_variant<0);
			precutting_parameters.permissions.resize(spheres.size(), 0);
			precutting_parameters.cutting_planes.resize(precutting_shifts.size(), std::vector<voronotalt::RadicalTessellation::ParametersForPreliminaryCuts::Plane>(spheres.size()));
			if(precutting_parameters.apply_with_single_mask)
			{
				precutting_parameters.single_mask=static_cast<voronotalt::UnsignedInt>(precutting_variant);
			}

			scripting::PrimitiveAtomDirectionsAssignment::Result atom_directions_assignment_result;
			scripting::PrimitiveAtomDirectionsAssignment::construct_result(data_manager, atom_directions_assignment_result);

			for(std::size_t i=0;i<spheres.size();i++)
			{
				if(!atom_directions_assignment_result.basic_directions[i].empty())
				{
					precutting_parameters.permissions[i]=1;
					const apollota::SimplePoint& bd=atom_directions_assignment_result.basic_directions[i][0];
					for(std::size_t j=0;j<precutting_parameters.cutting_planes.size();j++)
					{
						precutting_parameters.cutting_planes[j][i].normal=voronotalt::SimplePoint(bd.x, bd.y, bd.z);
						const apollota::SimplePoint shift=bd*precutting_shifts[j];
						precutting_parameters.cutting_planes[j][i].center=voronotalt::SimplePoint(shift.x, shift.y, shift.z);
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
					calculate_adjacencies,
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
		std::vector<std::string> names_for_adjunct_subareas_collapse_layers;
		std::vector<std::string> names_for_adjunct_subareas_collapse_directions;
		std::string name_for_adjunct_subareas_collapse_all;
		std::vector<std::string> names_for_adjunct_levelareas;

		if(!radical_tessellation_result.adjuncts_for_contacts_summaries.empty())
		{
			std::size_t number_of_adjuncts=0;
			for(std::size_t i=0;i<radical_tessellation_result.adjuncts_for_contacts_summaries.size();i++)
			{
				const std::size_t n=radical_tessellation_result.adjuncts_for_contacts_summaries[i].level_areas.size();
				if(n>0)
				{
					if(number_of_adjuncts==0)
					{
						number_of_adjuncts=n;
						names_for_adjunct_subareas.resize(number_of_adjuncts);
						names_for_adjunct_subareas_collapse_layers.resize(number_of_adjuncts);
						names_for_adjunct_subareas_collapse_directions.resize(number_of_adjuncts);
						name_for_adjunct_subareas_collapse_all="subareaM99900toM00000";
						names_for_adjunct_levelareas.resize(number_of_adjuncts);
						for(std::size_t j=0;j<number_of_adjuncts;j++)
						{
							const voronotalt::RadicalTessellation::ContactDescriptorSummaryAdjunct::LevelArea& la=radical_tessellation_result.adjuncts_for_contacts_summaries[i].level_areas[j];
							const int mask_class=static_cast<int>(precutting_parameters.calculate_mask_class(la.zone));
							{
								std::string name_for_direction;
								if(number_of_adjuncts>adjunct_circle_restrictions.size())
								{
									std::ostringstream name_output_for_direction;
									name_output_for_direction << "pcut";
									print_pretty_integer_number(mask_class, name_output_for_direction);
									name_for_direction=name_output_for_direction.str();
								}

								std::string name_for_layer;
								{
									std::ostringstream name_output_for_layer;
									name_output_for_layer << "subarea";
									{
										double smaller_restriction=(restrictions_positive ? 0.0 : -999.0);
										if(j+1<number_of_adjuncts)
										{
											const voronotalt::RadicalTessellation::ContactDescriptorSummaryAdjunct::LevelArea& smaller_la=radical_tessellation_result.adjuncts_for_contacts_summaries[i].level_areas[j+1];
											if(smaller_la.zone==la.zone)
											{
												smaller_restriction=smaller_la.restriction;
											}
										}
										print_pretty_number(smaller_restriction, name_output_for_layer);
									}
									name_output_for_layer << "to";
									print_pretty_number(la.restriction, name_output_for_layer);
									name_for_layer=name_output_for_layer.str();
								}

								if(!name_for_direction.empty())
								{
									names_for_adjunct_subareas[j]=name_for_direction+name_for_layer;
									names_for_adjunct_subareas_collapse_layers[j]=name_for_direction+name_for_adjunct_subareas_collapse_all;
									names_for_adjunct_subareas_collapse_directions[j]=name_for_layer;
								}
								else
								{
									names_for_adjunct_subareas[j]=name_for_layer;
								}
							}
							{
								std::ostringstream name_output;
								if(number_of_adjuncts>adjunct_circle_restrictions.size())
								{
									name_output << "pcut";
									print_pretty_integer_number(mask_class, name_output);
								}
								name_output << "levelarea";
								print_pretty_number(la.restriction, name_output);
								names_for_adjunct_levelareas[j]=name_output.str();
							}
						}
					}
					else if(n!=number_of_adjuncts)
					{
						throw std::runtime_error("Inconsistent number of level areas.");
					}
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
				if(!names_for_adjunct_subareas_collapse_layers[j].empty())
				{
					contact.value.props.adjuncts.erase(names_for_adjunct_subareas_collapse_layers[j]);
				}
				if(!names_for_adjunct_subareas_collapse_directions[j].empty())
				{
					contact.value.props.adjuncts.erase(names_for_adjunct_subareas_collapse_directions[j]);
				}
			}
			if(!name_for_adjunct_subareas_collapse_all.empty())
			{
				contact.value.props.adjuncts.erase(name_for_adjunct_subareas_collapse_all);
			}
			for(std::size_t j=0;j<names_for_adjunct_levelareas.size();j++)
			{
				contact.value.props.adjuncts.erase(names_for_adjunct_levelareas[j]);
			}
			if(i<radical_tessellation_result.adjuncts_for_contacts_summaries.size())
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummaryAdjunct& cdsa=radical_tessellation_result.adjuncts_for_contacts_summaries[i];
				for(std::size_t j=0;j<cdsa.level_areas.size();j++)
				{
					double subarea_to_add=0.0;
					if(j+1<cdsa.level_areas.size() && cdsa.level_areas[j+1].zone==cdsa.level_areas[j].zone)
					{
						subarea_to_add=cdsa.level_areas[j].area-cdsa.level_areas[j+1].area;
					}
					else
					{
						subarea_to_add=cdsa.level_areas[j].area;
					}
					const std::string& name_for_adjunct_subarea=names_for_adjunct_subareas[j];
					const std::string& name_for_adjunct_subarea_collapse1=names_for_adjunct_subareas_collapse_layers[j];
					const std::string& name_for_adjunct_subarea_collapse2=names_for_adjunct_subareas_collapse_directions[j];
					contact.value.props.adjuncts[name_for_adjunct_subarea]+=subarea_to_add;
					total_subareas[name_for_adjunct_subarea]+=subarea_to_add;
					if(add_collapsed_adjuncts)
					{
						if(!name_for_adjunct_subarea_collapse1.empty() && name_for_adjunct_subarea_collapse1!=name_for_adjunct_subarea)
						{
							contact.value.props.adjuncts[name_for_adjunct_subarea_collapse1]+=subarea_to_add;
							total_subareas[name_for_adjunct_subarea_collapse1]+=subarea_to_add;
						}
						if(!name_for_adjunct_subarea_collapse2.empty() && name_for_adjunct_subarea_collapse2!=name_for_adjunct_subarea && name_for_adjunct_subarea_collapse2!=name_for_adjunct_subarea_collapse1)
						{
							contact.value.props.adjuncts[name_for_adjunct_subarea_collapse2]+=subarea_to_add;
							total_subareas[name_for_adjunct_subarea_collapse2]+=subarea_to_add;
						}
						if(!name_for_adjunct_subareas_collapse_all.empty() && name_for_adjunct_subareas_collapse_all!=name_for_adjunct_subarea && name_for_adjunct_subareas_collapse_all!=name_for_adjunct_subarea_collapse1 && name_for_adjunct_subareas_collapse_all!=name_for_adjunct_subarea_collapse2)
						{
							contact.value.props.adjuncts[name_for_adjunct_subareas_collapse_all]+=subarea_to_add;
							total_subareas[name_for_adjunct_subareas_collapse_all]+=subarea_to_add;
						}
					}
					contact.value.props.adjuncts[names_for_adjunct_levelareas[j]]+=cdsa.level_areas[j].area;
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

		if(calculate_adjacencies)
		{
			std::map< std::pair<std::size_t, std::size_t>, std::size_t > map_of_id_pairs_to_contact_indices;
			for(std::size_t i=0;i<data_manager.contacts().size();i++)
			{
				const scripting::Contact& contact=data_manager.contacts()[i];
				map_of_id_pairs_to_contact_indices[std::make_pair(contact.ids[0], contact.ids[1])]=i;
			}
			std::vector< std::map<std::size_t, double> > adjacencies(data_manager.contacts().size());
			std::vector<double> adjacency_perimeters(data_manager.contacts().size(), 0.0);
			for(std::size_t i=0;i<radical_tessellation_result.tessellation_net.tes_edges.size();i++)
			{
				const voronotalt::RadicalTessellationContactConstruction::TessellationEdge te=radical_tessellation_result.tessellation_net.tes_edges[i];
				if(te.ids_of_spheres[2]<spheres.size())
				{
					std::map< std::pair<std::size_t, std::size_t>, std::size_t >::const_iterator it1=map_of_id_pairs_to_contact_indices.find(std::pair<std::size_t, std::size_t>(te.ids_of_spheres[0], te.ids_of_spheres[1]));
					std::map< std::pair<std::size_t, std::size_t>, std::size_t >::const_iterator it2=map_of_id_pairs_to_contact_indices.find(std::pair<std::size_t, std::size_t>(te.ids_of_spheres[0], te.ids_of_spheres[2]));
					std::map< std::pair<std::size_t, std::size_t>, std::size_t >::const_iterator it3=map_of_id_pairs_to_contact_indices.find(std::pair<std::size_t, std::size_t>(te.ids_of_spheres[1], te.ids_of_spheres[2]));
					if(it1!=map_of_id_pairs_to_contact_indices.end() && it2!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacencies[it1->second][it2->second]=te.length;
						adjacencies[it2->second][it1->second]=te.length;
					}
					if(it1!=map_of_id_pairs_to_contact_indices.end() && it3!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacencies[it1->second][it3->second]=te.length;
						adjacencies[it3->second][it1->second]=te.length;
					}
					if(it2!=map_of_id_pairs_to_contact_indices.end() && it3!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacencies[it2->second][it3->second]=te.length;
						adjacencies[it3->second][it2->second]=te.length;
					}
					if(it1!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacency_perimeters[it1->second]+=te.length;
					}
					if(it2!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacency_perimeters[it2->second]+=te.length;
					}
					if(it3!=map_of_id_pairs_to_contact_indices.end())
					{
						adjacency_perimeters[it3->second]+=te.length;
					}
				}
			}
			data_manager.reset_contacts_adjacencies_by_swapping(adjacencies, adjacency_perimeters);
		}

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
