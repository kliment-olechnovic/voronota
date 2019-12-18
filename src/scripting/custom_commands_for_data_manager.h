#ifndef SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include <algorithm>
#include <numeric>
#include <functional>

#include "../auxiliaries/color_utilities.h"
#include "../auxiliaries/residue_letters_coding.h"

#include "../common/writing_atomic_balls_in_pdb_format.h"
#include "../common/construction_of_structural_cartoon.h"
#include "../common/conversion_of_descriptors.h"
#include "../common/construction_of_path_centralities.h"

#include "generic_command_for_data_manager.h"
#include "basic_assertions.h"
#include "scoring_of_data_manager_using_voromqa.h"
#include "variant_serialization.h"
#include "io_selectors.h"
#include "filtering_of_triangulation.h"
#include "membrane_placement_for_data_manager_using_voromqa.h"

namespace scripting
{

class CustomCommandsForDataManager
{
public:
	class restrict_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
//		void document(CommandDocumentation& doc) const
//		{
//			doc.set_short_description("Restrict atoms by keeping only the selected ones.");
//			document_generic_selecting_query(doc);
//		}

		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms selected."));
			}

			VariantObject& info=cargs.heterostorage.variant_object;

			if(ids.size()<cargs.data_manager.atoms().size())
			{
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("atoms_summary_old"));

				cargs.data_manager.restrict_atoms(ids);
			}
			else
			{
				info.value("atoms_summary_old").set_null();
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("atoms_summary_new"));
		}
	};

	class move_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
//		void document(CommandDocumentation& doc) const
//		{
//			doc.set_short_description("Transform coordinates of atoms.");
//			document_generic_selecting_query(doc);
//			doc.set_option_decription("translate-before", "3 numeric values", "translation vector to apply before rotation");
//			doc.set_option_decription("rotate-by-matrix", "9 numeric values", "3x3 rotation matrix");
//			doc.set_option_decription("rotate-by-axis-and-angle", "4 numeric values", "axis and angle for rotation");
//			doc.set_option_decription("translate", "3 numeric values", "translation vector to apply after rotation");
//		}

		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<double> pre_translation_vector=cargs.input.get_value_vector_or_default<double>("translate-before", std::vector<double>());
			const std::vector<double> rotation_matrix=cargs.input.get_value_vector_or_default<double>("rotate-by-matrix", std::vector<double>());
			const std::vector<double> rotation_axis_and_angle=cargs.input.get_value_vector_or_default<double>("rotate-by-axis-and-angle", std::vector<double>());
			const std::vector<double> post_translation_vector=cargs.input.get_value_vector_or_default<double>("translate", std::vector<double>());

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			cargs.data_manager.transform_coordinates_of_atoms(ids, pre_translation_vector, rotation_matrix, rotation_axis_and_angle, post_translation_vector);
		}
	};

	class center_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(SelectionManager::Query());
			if(ids.size()!=cargs.data_manager.atoms().size())
			{
				throw std::runtime_error(std::string("Not all atoms selected."));
			}

			const SummaryOfAtoms summary_of_atoms(cargs.data_manager.atoms(), ids);

			const apollota::SimplePoint translation=apollota::SimplePoint(0.0, 0.0, 0.0)-((summary_of_atoms.bounding_box.p_min+summary_of_atoms.bounding_box.p_max)*0.5);

			std::vector<double> translation_vector(3);
			translation_vector[0]=translation.x;
			translation_vector[1]=translation.y;
			translation_vector[2]=translation.z;

			cargs.data_manager.transform_coordinates_of_atoms(ids, translation_vector, std::vector<double>(), std::vector<double>(), std::vector<double>());
		}
	};

	class export_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool as_pdb=cargs.input.get_flag("as-pdb");
			const std::string pdb_b_factor_name=cargs.input.get_value_or_default<std::string>("pdb-b-factor", "tf");
			const bool pdb_ter=cargs.input.get_flag("pdb-ter");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			OutputSelector output_selector(file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			const std::vector<Atom> atoms=slice_vector_by_ids(cargs.data_manager.atoms(), ids);

			if(as_pdb)
			{
				common::WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms, pdb_b_factor_name, pdb_ter, output);
			}
			else
			{
				auxiliaries::IOUtilities().write_set(atoms, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfAtoms(atoms), info.object("atoms_summary"));
			}
		}

	private:
		template<typename T>
		static T slice_vector_by_ids(const T& full_vector, const std::set<std::size_t>& ids)
		{
			T sliced_vector;
			sliced_vector.reserve(ids.size());
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				sliced_vector.push_back(full_vector.at(*it));
			}
			return sliced_vector;
		}
	};

	class select_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool mark=cargs.input.get_flag("mark");

			cargs.input.assert_nothing_unusable();

			assert_selection_name_input(name, true);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_atoms_selection(name, ids);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("atoms_summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}

			if(mark)
			{
				cargs.data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_unmark(true));
				cargs.data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(true), ids);
			}

			cargs.heterostorage.vectors_of_ids["selection_of_atoms"]=std::vector<std::size_t>(ids.begin(), ids.end());
		}
	};

	class export_selection_of_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			OutputSelector output_selector(file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			std::set<common::ChainResidueAtomDescriptor> set_of_crads;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				set_of_crads.insert(cargs.data_manager.atoms()[*it].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName));
			}

			auxiliaries::IOUtilities().write_set(set_of_crads, output);

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("atoms_summary"));
				info.value("number_of_descriptors_written")=set_of_crads.size();
			}
		}
	};

	class import_selection_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const std::string name=cargs.input.get_value<std::string>("name");
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");

			cargs.input.assert_nothing_unusable();

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input selection file name."));
			}

			assert_selection_name_input(name, false);

			InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::set<common::ChainResidueAtomDescriptor> set_of_crads;

			auxiliaries::IOUtilities().read_lines_to_set(finput, set_of_crads);

			if(set_of_crads.empty())
			{
				throw std::runtime_error(std::string("No descriptors in file '")+file+"'.");
			}

			if(no_serial || no_name || no_resSeq || no_resName)
			{
				std::set<common::ChainResidueAtomDescriptor> refined_set_of_crads;
				for(std::set<common::ChainResidueAtomDescriptor>::const_iterator it=set_of_crads.begin();it!=set_of_crads.end();++it)
				{
					refined_set_of_crads.insert(it->without_some_info(no_serial, no_name, no_resSeq, no_resName));
				}
				set_of_crads.swap(refined_set_of_crads);
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms_by_set_of_crads(set_of_crads);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_atoms_selection(name, ids);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("atoms_summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}
		}
	};

	class set_tag_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

			cargs.input.assert_nothing_unusable();

			assert_tag_input(tag, false);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				cargs.data_manager.atom_tags_mutable(*it).insert(tag);
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class set_tag_of_atoms_by_secondary_structure : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string tag_for_alpha=cargs.input.get_value_or_default<std::string>("tag-for-alpha", "ss=H");
			const std::string tag_for_beta=cargs.input.get_value_or_default<std::string>("tag-for-beta", "ss=S");

			cargs.input.assert_nothing_unusable();

			for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
			{
				std::set<std::string>& atom_tags=cargs.data_manager.atom_tags_mutable(i);
				atom_tags.erase(tag_for_alpha);
				atom_tags.erase(tag_for_beta);
			}

			for(std::size_t residue_id=0;residue_id<cargs.data_manager.secondary_structure_info().residue_descriptors.size();residue_id++)
			{
				const common::ConstructionOfSecondaryStructure::ResidueDescriptor& residue_descriptor=cargs.data_manager.secondary_structure_info().residue_descriptors[residue_id];
				const std::vector<std::size_t>& atom_ids=cargs.data_manager.primary_structure_info().residues[residue_id].atom_ids;
				for(std::size_t i=0;i<atom_ids.size();i++)
				{
					std::set<std::string>& atom_tags=cargs.data_manager.atom_tags_mutable(atom_ids[i]);
					if(residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
					{
						atom_tags.insert(tag_for_alpha);
					}
					else if(residue_descriptor.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
					{
						atom_tags.insert(tag_for_beta);
					}
				}
			}
		}
	};

	class delete_tags_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool all=cargs.input.get_flag("all");
			const std::vector<std::string> tags=cargs.input.get_value_vector_or_default<std::string>("tags", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			if(!all && tags.empty())
			{
				throw std::runtime_error(std::string("No tags specified."));
			}

			if(all && !tags.empty())
			{
				throw std::runtime_error(std::string("Conflicting specification of tags."));
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::set<std::string>& atom_tags=cargs.data_manager.atom_tags_mutable(*it);
				if(all)
				{
					atom_tags.clear();
				}
				else
				{
					for(std::size_t i=0;i<tags.size();i++)
					{
						atom_tags.erase(tags[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class set_adjunct_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=cargs.input.get_value<std::string>("name");
			const bool value_present=cargs.input.is_option("value");
			const double value=cargs.input.get_value_or_default<double>("value", 0.0);
			const bool remove=cargs.input.get_flag("remove");

			cargs.input.assert_nothing_unusable();

			if(value_present && remove)
			{
				throw std::runtime_error(std::string("Value setting and removing options used together."));
			}

			assert_adjunct_name_input(name, false);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=cargs.data_manager.atom_adjuncts_mutable(*it);
				if(remove)
				{
					atom_adjuncts.erase(name);
				}
				else
				{
					atom_adjuncts[name]=value;
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class set_adjunct_of_atoms_by_contact_areas : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query("", "[--min-seq-sep 1]", cargs.input);
			const std::string name=cargs.input.get_value<std::string>("name");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(name, false);

			const std::set<std::size_t> contacts_ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(contacts_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
			{
				cargs.data_manager.atom_adjuncts_mutable(i).erase(name);
			}

			std::set<std::size_t> atom_ids;

			for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];

				for(int i=0;i<(contact.solvent() ? 1 : 2);i++)
				{
					cargs.data_manager.atom_adjuncts_mutable(contact.ids[i])[name]+=contact.value.area;
					atom_ids.insert(contact.ids[i]);
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), contacts_ids), cargs.heterostorage.variant_object.object("contacts_summary"));
			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), atom_ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class set_adjunct_of_atoms_by_contact_adjuncts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string source_name=cargs.input.get_value<std::string>("source-name");
			const std::string destination_name=cargs.input.get_value<std::string>("destination-name");
			const std::string pooling_mode=cargs.input.get_value<std::string>("pooling-mode");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(source_name, false);
			assert_adjunct_name_input(destination_name, false);

			if(pooling_mode!="sum" && pooling_mode!="min" && pooling_mode!="max")
			{
				throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'sum', 'min', 'max'."));
			}

			const std::set<std::size_t> contacts_ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(contacts_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
			{
				cargs.data_manager.atom_adjuncts_mutable(i).erase(destination_name);
			}

			std::set<std::size_t> atom_ids;

			for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];

				if(contact.value.props.adjuncts.count(source_name)>0)
				{
					const double source_value=(contact.value.props.adjuncts.find(source_name)->second);
					for(int i=0;i<(contact.solvent() ? 1 : 2);i++)
					{
						std::map<std::string, double>& atom_adjuncts=cargs.data_manager.atom_adjuncts_mutable(contact.ids[i]);
						const bool first_setting=(atom_adjuncts.count(destination_name)==0);
						double& destination_value=atom_adjuncts[destination_name];
						if(pooling_mode=="sum")
						{
							destination_value+=source_value;
						}
						else if(pooling_mode=="min")
						{
							destination_value=(first_setting ? source_value : std::min(source_value, destination_value));
						}
						else if(pooling_mode=="max")
						{
							destination_value=(first_setting ? source_value : std::max(source_value, destination_value));
						}
						atom_ids.insert(contact.ids[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), contacts_ids), cargs.heterostorage.variant_object.object("contacts_summary"));
			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), atom_ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class set_adjunct_of_atoms_by_residue_pooling : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting_source_atoms=read_generic_selecting_query("source-atoms-", "[]", cargs.input);
			const SelectionManager::Query parameters_for_selecting_destination_atoms=read_generic_selecting_query("destination-atoms-", "[]", cargs.input);
			const std::string source_name=cargs.input.get_value<std::string>("source-name");
			const std::string destination_name=cargs.input.get_value<std::string>("destination-name");
			const std::string pooling_mode=cargs.input.get_value<std::string>("pooling-mode");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(source_name, false);
			assert_adjunct_name_input(destination_name, false);

			if(pooling_mode!="mean" && pooling_mode!="sum" && pooling_mode!="product" && pooling_mode!="min" && pooling_mode!="max")
			{
				throw std::runtime_error(std::string("Invalid pooling mode, valid options are: 'mean', 'sum', 'product', 'min', 'max'."));
			}

			const std::set<std::size_t> source_atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_source_atoms);
			if(source_atom_ids.empty())
			{
				throw std::runtime_error(std::string("No source atoms selected."));
			}

			const std::set<std::size_t> destination_atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_destination_atoms);
			if(destination_atom_ids.empty())
			{
				throw std::runtime_error(std::string("No destination atoms selected."));
			}

			std::map< std::size_t, std::vector<double> > residue_source_values;
			std::set<std::size_t> used_source_atom_ids;

			for(std::set<std::size_t>::const_iterator it=destination_atom_ids.begin();it!=destination_atom_ids.end();++it)
			{
				const std::size_t residue_id=cargs.data_manager.primary_structure_info().map_of_atoms_to_residues[*it];
				if(residue_source_values.count(residue_id)==0)
				{
					const std::vector<std::size_t>& residue_atom_ids=cargs.data_manager.primary_structure_info().residues[residue_id].atom_ids;
					for(std::size_t i=0;i<residue_atom_ids.size();i++)
					{
						const std::size_t atom_id=residue_atom_ids[i];
						if(source_atom_ids.count(atom_id)>0)
						{
							const Atom& atom=cargs.data_manager.atoms()[atom_id];
							std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(source_name);
							if(jt!=atom.value.props.adjuncts.end())
							{
								residue_source_values[residue_id].push_back(jt->second);
								used_source_atom_ids.insert(atom_id);
							}
						}
					}
				}
			}

			std::map<std::size_t, double> residue_pooled_values;
			for(std::map< std::size_t, std::vector<double> >::const_iterator it=residue_source_values.begin();it!=residue_source_values.end();++it)
			{
				const std::size_t residue_id=it->first;
				const std::vector<double>& values=it->second;
				if(!values.empty())
				{
					if(pooling_mode=="mean")
					{
						residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 0.0)/values.size();
					}
					else if(pooling_mode=="sum")
					{
						residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 0.0);
					}
					else if(pooling_mode=="product")
					{
						residue_pooled_values[residue_id]=std::accumulate(values.begin(), values.end(), 1.0, std::multiplies<double>());
					}
					else if(pooling_mode=="min")
					{
						residue_pooled_values[residue_id]=(*std::min_element(values.begin(), values.end()));
					}
					else if(pooling_mode=="max")
					{
						residue_pooled_values[residue_id]=(*std::max_element(values.begin(), values.end()));
					}
					else
					{
						throw std::runtime_error(std::string("Invalid pooling mode."));
					}
				}
			}

			for(std::set<std::size_t>::const_iterator it=destination_atom_ids.begin();it!=destination_atom_ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=cargs.data_manager.atom_adjuncts_mutable(*it);
				atom_adjuncts.erase(destination_name);
				const std::size_t residue_id=cargs.data_manager.primary_structure_info().map_of_atoms_to_residues[*it];
				if(residue_pooled_values.count(residue_id)>0)
				{
					atom_adjuncts[destination_name]=residue_pooled_values[residue_id];
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), used_source_atom_ids), cargs.heterostorage.variant_object.object("used_source_atoms_summary"));
			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), destination_atom_ids), cargs.heterostorage.variant_object.object("destination_atoms_summary"));
		}
	};

	class delete_adjuncts_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool all=cargs.input.get_flag("all");
			const std::vector<std::string> adjuncts=cargs.input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			if(!all && adjuncts.empty())
			{
				throw std::runtime_error(std::string("No adjuncts specified."));
			}

			if(all && !adjuncts.empty())
			{
				throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& atom_adjuncts=cargs.data_manager.atom_adjuncts_mutable(*it);
				if(all)
				{
					atom_adjuncts.clear();
				}
				else
				{
					for(std::size_t i=0;i<adjuncts.size();i++)
					{
						atom_adjuncts.erase(adjuncts[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class export_adjuncts_of_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");
			const bool all=cargs.input.get_flag("all");
			std::vector<std::string> adjuncts=cargs.input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			if(!all && adjuncts.empty())
			{
				throw std::runtime_error(std::string("No adjuncts specified."));
			}

			if(all && !adjuncts.empty())
			{
				throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			if(all)
			{
				std::set<std::string> all_adjuncts;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Atom& atom=cargs.data_manager.atoms()[*it];
					for(std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.begin();jt!=atom.value.props.adjuncts.end();++jt)
					{
						all_adjuncts.insert(jt->first);
					}
				}
				if(all_adjuncts.empty())
				{
					throw std::runtime_error(std::string("Selected atoms have no adjuncts."));
				}
				adjuncts=std::vector<std::string>(all_adjuncts.begin(), all_adjuncts.end());
			}

			OutputSelector output_selector(file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			output << "ID";
			for(std::size_t i=0;i<adjuncts.size();i++)
			{
				output << " " << adjuncts[i];
			}
			output << "\n";

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Atom& atom=cargs.data_manager.atoms()[*it];
				output << atom.crad.without_some_info(no_serial, no_name, no_resSeq, no_resName);
				for(std::size_t i=0;i<adjuncts.size();i++)
				{
					output << " ";
					std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(adjuncts[i]);
					if(jt!=atom.value.props.adjuncts.end())
					{
						output << jt->second;
					}
					else
					{
						output << "NA";
					}
				}
				output << "\n";
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class import_adjuncts_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");

			cargs.input.assert_nothing_unusable();

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input adjuncts file name."));
			}

			InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::vector<std::string> header;
			{
				std::string line;
				std::getline(finput, line);
				if(!line.empty())
				{
					std::istringstream linput(line);
					while(linput.good())
					{
						std::string token;
						linput >> token;
						if(!token.empty())
						{
							header.push_back(token);
						}
					}
				}
			}

			if(header.size()<2 || header[0]!="ID")
			{
				throw std::runtime_error(std::string("Invalid header in file."));
			}

			std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> > maps_of_adjuncts;

			while(finput.good())
			{
				std::string line;
				std::getline(finput, line);
				if(!line.empty())
				{
					std::istringstream linput(line);
					common::ChainResidueAtomDescriptor crad;
					linput >> crad;
					if(linput.fail() || !crad.valid())
					{
						throw std::runtime_error(std::string("Invalid ID in file."));
					}
					for(std::size_t i=1;i<header.size();i++)
					{
						std::string token;
						linput >> token;
						if(linput.fail() || token.empty())
						{
							throw std::runtime_error(std::string("Missing value in file."));
						}
						if(token!="NA")
						{
							std::istringstream vinput(token);
							double value=0.0;
							vinput >> value;
							if(vinput.fail())
							{
								throw std::runtime_error(std::string("Invalid value '")+token+"'.");
							}
							maps_of_adjuncts[header[i]][crad.without_some_info(no_serial, no_name, no_resSeq, no_resName)]=value;
						}
					}
				}
			}

			for(std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> >::const_iterator it=maps_of_adjuncts.begin();it!=maps_of_adjuncts.end();++it)
			{
				const std::string& adjunct_name=it->first;
				const std::map<common::ChainResidueAtomDescriptor, double>& adjunct_map=it->second;

				for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
				{
					const Atom& atom=cargs.data_manager.atoms()[i];
					const std::pair<bool, double> value=common::MatchingUtilities::match_crad_with_map_of_crads(true, atom.crad, adjunct_map);
					if(value.first)
					{
						cargs.data_manager.atom_adjuncts_mutable(i)[adjunct_name]=value.second;
					}
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				std::vector<VariantValue>& adjunct_names=info.values_array("adjunct_names");
				for(std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> >::const_iterator it=maps_of_adjuncts.begin();it!=maps_of_adjuncts.end();++it)
				{
					adjunct_names.push_back(VariantValue(it->first));
				}
			}
		}
	};

	class mark_atoms : public GenericCommandForDataManagerScaled
	{
	public:
		mark_atoms() : positive_(true)
		{
		}

		explicit mark_atoms(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			cargs.data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_mark(positive_).set_unmark(!positive_), ids);

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}

	private:
		bool positive_;
	};

	class unmark_atoms : public mark_atoms
	{
	public:
		unmark_atoms() : mark_atoms(false)
		{
		}
	};

	class show_atoms : public GenericCommandForDataManagerScaled
	{
	public:
		show_atoms() : positive_(true)
		{
		}

		explicit show_atoms(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			std::set<std::size_t> representation_ids=cargs.data_manager.atoms_representation_descriptor().ids_by_names(representation_names);
			
			if(positive_ && representation_ids.empty())
			{
				representation_ids.insert(0);
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			cargs.data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive_).set_hide(!positive_), ids);

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}

	private:
		bool positive_;
	};

	class hide_atoms : public show_atoms
	{
	public:
		hide_atoms() : show_atoms(false)
		{
		}
	};

	class color_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const bool next_random_color=cargs.input.get_flag("next-random-color");
			const auxiliaries::ColorUtilities::ColorInteger color_value=(next_random_color ? get_next_random_color() : read_color(cargs.input));

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

			if(!auxiliaries::ColorUtilities::color_valid(color_value))
			{
				throw std::runtime_error(std::string("Atoms color not specified."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			cargs.data_manager.update_atoms_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class spectrum_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
			const std::string by=adjunct.empty() ? cargs.input.get_value_or_default<std::string>("by", "residue-number") : std::string("adjunct");
			const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
			const bool as_z_scores=cargs.input.get_flag("as-z-scores");
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", (as_z_scores ? -2.0 : 0.0));
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", (as_z_scores ? 2.0 : 1.0));
			const bool only_summarize=cargs.input.get_flag("only-summarize");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

			if(by!="residue-number" && by!="adjunct" && by!="chain" && by!="residue-id")
			{
				throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
			}

			if(by=="adjunct" && adjunct.empty())
			{
				throw std::runtime_error(std::string("No adjunct name provided."));
			}

			if(by!="adjunct" && !adjunct.empty())
			{
				throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
			}

			if(scheme!="random" && !auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
			{
				throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
			}

			if(min_val_present && max_val_present && max_val<=min_val)
			{
				throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			std::map<std::size_t, double> map_of_ids_values;

			if(by=="adjunct")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::map<std::string, double>& adjuncts=cargs.data_manager.atoms()[*it].value.props.adjuncts;
					std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
					if(jt!=adjuncts.end())
					{
						map_of_ids_values[*it]=jt->second;
					}
				}
			}
			else if(by=="residue-number")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					map_of_ids_values[*it]=cargs.data_manager.atoms()[*it].crad.resSeq;
				}
			}
			else if(by=="chain")
			{
				std::map<std::string, double> chains_to_values;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					chains_to_values[cargs.data_manager.atoms()[*it].crad.chainID]=0.5;
				}
				if(chains_to_values.size()>1)
				{
					int i=0;
					for(std::map<std::string, double>::iterator it=chains_to_values.begin();it!=chains_to_values.end();++it)
					{
						it->second=static_cast<double>(i)/static_cast<double>(chains_to_values.size()-1);
						i++;
					}
				}
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					map_of_ids_values[*it]=chains_to_values[cargs.data_manager.atoms()[*it].crad.chainID];
				}
			}
			else if(by=="residue-id")
			{
				std::map<common::ChainResidueAtomDescriptor, double> residue_ids_to_values;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					residue_ids_to_values[cargs.data_manager.atoms()[*it].crad.without_atom()]=0.5;
				}
				if(residue_ids_to_values.size()>1)
				{
					int i=0;
					for(std::map<common::ChainResidueAtomDescriptor, double>::iterator it=residue_ids_to_values.begin();it!=residue_ids_to_values.end();++it)
					{
						it->second=static_cast<double>(i)/static_cast<double>(residue_ids_to_values.size()-1);
						i++;
					}
				}
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					map_of_ids_values[*it]=residue_ids_to_values[cargs.data_manager.atoms()[*it].crad.without_atom()];
				}
			}

			if(map_of_ids_values.empty())
			{
				throw std::runtime_error(std::string("Nothing colorable."));
			}

			double min_val_actual=0.0;
			double max_val_actual=0.0;
			int num_of_vals=0;
			double mean_of_values=0.0;
			double sd_of_values=0.0;

			calculate_spectrum_info(
					as_z_scores,
					min_val_present,
					min_val,
					max_val_present,
					max_val,
					min_val_actual,
					max_val_actual,
					num_of_vals,
					mean_of_values,
					sd_of_values,
					map_of_ids_values);

			if(!only_summarize)
			{
				DataManager::DisplayStateUpdater dsu;
				dsu.visual_ids=representation_ids;
				if(scheme=="random")
				{
					std::map<double, auxiliaries::ColorUtilities::ColorInteger> map_of_values_colors;
					for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
					{
						map_of_values_colors[it->second]=0;
					}
					for(std::map<double, auxiliaries::ColorUtilities::ColorInteger>::iterator it=map_of_values_colors.begin();it!=map_of_values_colors.end();++it)
					{
						it->second=get_next_random_color();
					}
					for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
					{
						dsu.color=map_of_values_colors[it->second];
						cargs.data_manager.update_atoms_display_state(dsu, it->first);
					}
				}
				else
				{
					for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
					{
						dsu.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
						cargs.data_manager.update_atoms_display_state(dsu, it->first);
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("spectrum_summary");
				info.value("min_value")=min_val_actual;
				info.value("max_value")=max_val_actual;
				info.value("number_of_values")=num_of_vals;
				info.value("mean_of_values")=mean_of_values;
				info.value("sd_of_values")=sd_of_values;
			}
		}
	};


	class print_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			std::vector<VariantObject>& atoms=cargs.heterostorage.variant_object.objects_array("atoms");
			atoms.reserve(ids.size());
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				atoms.push_back(VariantObject());
				VariantSerialization::write(cargs.data_manager.atoms()[*it], atoms.back());
				atoms.back().value("id")=(*it);
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));
		}
	};

	class print_sequence : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool secondary_structure=cargs.input.get_flag("secondary-structure");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("atoms_summary"));

			std::set<std::size_t> residue_ids;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				residue_ids.insert(cargs.data_manager.primary_structure_info().map_of_atoms_to_residues[*it]);
			}

			std::vector< std::vector<std::size_t> > grouping;
			{
				std::map<common::ChainResidueAtomDescriptor, std::size_t> ordering;
				for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
				{
					const common::ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[*it];
					ordering[r.chain_residue_descriptor]=(*it);
				}

				std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it=ordering.begin();
				while(it!=ordering.end())
				{
					if(it==ordering.begin())
					{
						grouping.push_back(std::vector<std::size_t>(1, it->second));
					}
					else
					{
						std::map<common::ChainResidueAtomDescriptor, std::size_t>::const_iterator it_prev=it;
						--it_prev;
						if(it->first.chainID==it_prev->first.chainID && (it->first.resSeq-it_prev->first.resSeq)<=1)
						{
							grouping.back().push_back(it->second);
						}
						else
						{
							grouping.push_back(std::vector<std::size_t>(1, it->second));
						}
					}
					++it;
				}
			}

			std::map< std::string, std::vector<std::size_t> > chaining;
			for(std::size_t i=0;i<grouping.size();i++)
			{
				const common::ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[grouping[i].front()];
				chaining[r.chain_residue_descriptor.chainID].push_back(i);
			}

			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=chaining.begin();it!=chaining.end();++it)
			{
				VariantObject chain_info;
				chain_info.value("chain_name")=(it->first);
				const std::vector<std::size_t>& group_ids=it->second;
				for(std::size_t i=0;i<group_ids.size();i++)
				{
					std::ostringstream output_for_residue_sequence;
					std::ostringstream output_for_secondary_structure_sequence;
					VariantObject segment_info;
					const std::vector<std::size_t>& group=grouping[group_ids[i]];
					segment_info.value("length")=group.size();
					if(!group.empty())
					{
						for(std::size_t j=0;j<group.size();j++)
						{
							{
								const common::ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[group[j]];
								output_for_residue_sequence << auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(r.chain_residue_descriptor.resName);
								if(j==0)
								{
									segment_info.values_array("range").push_back(VariantValue(r.chain_residue_descriptor.resSeq));
								}
								if((j+1)==group.size())
								{
									segment_info.values_array("range").push_back(VariantValue(r.chain_residue_descriptor.resSeq));
								}
							}
							if(secondary_structure)
							{
								const common::ConstructionOfSecondaryStructure::ResidueDescriptor& r=cargs.data_manager.secondary_structure_info().residue_descriptors[group[j]];
								if(r.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
								{
									output_for_secondary_structure_sequence << "H";
								}
								else if(r.secondary_structure_type==common::ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
								{
									output_for_secondary_structure_sequence << "S";
								}
								else
								{
									output_for_secondary_structure_sequence << "-";
								}
							}
						}
						segment_info.value("residue_sequence")=output_for_residue_sequence.str();
						if(secondary_structure)
						{
							segment_info.value("secondary_structure")=output_for_secondary_structure_sequence.str();
						}
					}
					chain_info.objects_array("continuous_segments").push_back(segment_info);
				}
				cargs.heterostorage.variant_object.objects_array("chains").push_back(chain_info);
			}
		}
	};

	class zoom_by_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["zoomed"];
			summary_of_atoms=SummaryOfAtoms(cargs.data_manager.atoms(), ids);

			VariantSerialization::write(summary_of_atoms.bounding_box, cargs.heterostorage.variant_object.object("bounding_box"));
		}
	};

	class export_atoms_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const std::string name=cargs.input.get_value_or_default<std::string>("name", "atoms");
			const bool wireframe=cargs.input.get_flag("wireframe");
			const double radius_subtraction=cargs.input.get_value_or_default<double>("radius-subtraction", 0.0);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string file=cargs.input.get_value<std::string>("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			std::set<std::size_t> representation_ids=cargs.data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

			if(representation_ids.empty())
			{
				representation_ids.insert(0);
			}

			if(representation_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable visible atoms selected."));
			}

			if(wireframe)
			{
				if(!cargs.data_manager.bonding_links_info().valid(cargs.data_manager.atoms(), cargs.data_manager.primary_structure_info()))
				{
					cargs.data_manager.reset_bonding_links_info_by_creating(common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
				}
			}

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
					{
						const std::size_t visual_id=(*jt);
						if(visual_id<cargs.data_manager.atoms_display_states()[id].visuals.size())
						{
							const DataManager::DisplayState::Visual& dsv=cargs.data_manager.atoms_display_states()[id].visuals[visual_id];
							if(prev_color==0 || dsv.color!=prev_color)
							{
								opengl_printer.add_color(dsv.color);
							}
							prev_color=dsv.color;
							if(wireframe)
							{
								const std::vector<std::size_t>& link_ids=cargs.data_manager.bonding_links_info().map_of_atoms_to_bonds_links[id];
								for(std::size_t i=0;i<link_ids.size();i++)
								{
									const common::ConstructionOfBondingLinks::DirectedLink& dl=cargs.data_manager.bonding_links_info().bonds_links[link_ids[i]];
									const apollota::SimplePoint pa(cargs.data_manager.atoms()[dl.a].value);
									const apollota::SimplePoint pb(cargs.data_manager.atoms()[dl.b].value);
									opengl_printer.add_line_strip(pa, (pa+pb)*0.5);
								}
							}
							else
							{
								if(radius_subtraction>0.0)
								{
									apollota::SimpleSphere reduced_ball(cargs.data_manager.atoms()[id].value);
									reduced_ball.r=std::max(0.0, reduced_ball.r-radius_subtraction);
									opengl_printer.add_sphere(reduced_ball);
								}
								else
								{
									opengl_printer.add_sphere(cargs.data_manager.atoms()[id].value);
								}
							}
						}
					}
				}
			}

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("atoms_summary"));
			}
		}
	};

	class export_cartoon_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const std::string name=cargs.input.get_value_or_default<std::string>("name", "atoms");
			const bool wireframe=cargs.input.get_flag("wireframe");
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string file=cargs.input.get_value<std::string>("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			std::set<std::size_t> representation_ids=cargs.data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

			if(representation_ids.empty())
			{
				representation_ids.insert(0);
			}

			if(representation_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable visible atoms selected."));
			}

			if(!cargs.data_manager.bonding_links_info().valid(cargs.data_manager.atoms(), cargs.data_manager.primary_structure_info()))
			{
				cargs.data_manager.reset_bonding_links_info_by_creating(common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
			}

			common::ConstructionOfStructuralCartoon::Parameters parameters_for_cartoon;
			common::ConstructionOfStructuralCartoon::BundleOfMeshInformation bundle_of_cartoon_mesh;
			if(!common::ConstructionOfStructuralCartoon::construct_bundle_of_mesh_information(
					parameters_for_cartoon,
					cargs.data_manager.atoms(),
					cargs.data_manager.primary_structure_info(),
					cargs.data_manager.secondary_structure_info(),
					cargs.data_manager.bonding_links_info(),
					bundle_of_cartoon_mesh))
			{
				throw std::runtime_error(std::string("Failed to construct cartoon mesh."));
			}

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				std::vector<apollota::SimplePoint> vertices(3);
				std::vector<apollota::SimplePoint> normals(3);
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					const std::vector<unsigned int>& indices=bundle_of_cartoon_mesh.mapped_indices[id];
					if(!indices.empty() && indices.size()%3==0)
					{
						for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
						{
							const std::size_t visual_id=(*jt);
							if(visual_id<cargs.data_manager.atoms_display_states()[id].visuals.size())
							{
								const DataManager::DisplayState::Visual& dsv=cargs.data_manager.atoms_display_states()[id].visuals[visual_id];
								if(prev_color==0 || dsv.color!=prev_color)
								{
									opengl_printer.add_color(dsv.color);
								}
								prev_color=dsv.color;
								for(std::size_t i=0;i<indices.size();i+=3)
								{
									for(std::size_t j=0;j<3;j++)
									{
										vertices[j].x=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+0];
										vertices[j].y=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+1];
										vertices[j].z=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+2];
										normals[j].x=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+0];
										normals[j].y=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+1];
										normals[j].z=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+2];
									}
									if(wireframe)
									{
										opengl_printer.add_line_loop(vertices);
									}
									else
									{
										opengl_printer.add_triangle_strip(vertices, normals);
									}
								}
							}
						}
					}
				}
			}

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("atoms_summary"));
			}
		}
	};

	class list_selections_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.data_manager.sync_atoms_selections_with_display_states();
			cargs.data_manager.assert_atoms_selections_availability();
			const std::map< std::string, std::set<std::size_t> >& map_of_selections=cargs.data_manager.selection_manager().map_of_atoms_selections();

			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				VariantObject info;
				info.value("name")=(it->first);
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), it->second), info.object("atoms_summary"));
				cargs.heterostorage.variant_object.objects_array("selections").push_back(info);
			}
		}
	};

	class delete_selections_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_selections_availability();

			const bool all=cargs.input.get_flag("all");

			std::vector<std::string> names;

			if(all)
			{
				names=cargs.data_manager.selection_manager().get_names_of_atoms_selections();
			}
			else
			{
				names=cargs.input.get_list_of_unnamed_values();
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No atoms selections names specified."));
			}

			cargs.data_manager.assert_atoms_selections_availability(names);

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.data_manager.selection_manager().delete_atoms_selection(names[i]);
			}

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.heterostorage.variant_object.values_array("deleted_selections").push_back(VariantValue(names[i]));
			}
		}
	};

	class rename_selection_of_atoms : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_selections_availability();

			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();

			if(names.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().get_atoms_selection(names[0]);
			cargs.data_manager.selection_manager().set_atoms_selection(names[1], ids);
			cargs.data_manager.selection_manager().delete_atoms_selection(names[0]);
		}
	};

	class construct_triangulation : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			common::ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
			parameters_to_construct_triangulation.artificial_boundary_shift=cargs.input.get_value_or_default<double>("boundary-shift", 5.0);
			parameters_to_construct_triangulation.init_radius_for_BSH=cargs.input.get_value_or_default<double>("init-radius-for-BSH", parameters_to_construct_triangulation.init_radius_for_BSH);
			parameters_to_construct_triangulation.exclude_hidden_balls=cargs.input.get_flag("exclude-hidden-balls");
			const bool force=cargs.input.get_flag("force");

			cargs.input.assert_nothing_unusable();

			if(force)
			{
				cargs.data_manager.remove_triangulation_info();
			}

			cargs.data_manager.reset_triangulation_info_by_creating(parameters_to_construct_triangulation);

			VariantSerialization::write(SummaryOfTriangulation(cargs.data_manager.triangulation_info()), cargs.heterostorage.variant_object.object("triangulation_summary"));
		}
	};

	class export_triangulation : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();

			const SelectionManager::Query parameters_for_selecting_atoms=read_generic_selecting_query(cargs.input);
			FilteringOfTriangulation::Query filtering_query=read_filtering_of_triangulation_query(cargs.input);
			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			filtering_query.atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(filtering_query.atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(cargs.data_manager.triangulation_info(), filtering_query);

			if(filtering_result.vertices_info.empty())
			{
				throw std::runtime_error(std::string("No triangulation parts selected."));
			}

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);

				for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
				{
					const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];
					for(int j=0;j<4;j++)
					{
						output << vi.quadruple.get(j) << " ";
					}
					output << vi.sphere.x << " " << vi.sphere.y << " " << vi.sphere.z << " " << vi.sphere.r << " ";
					output << vi.tetrahedron_volume << "\n";
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}

				VariantSerialization::write(SummaryOfTriangulation(cargs.data_manager.triangulation_info()), cargs.heterostorage.variant_object.object("full_triangulation_summary"));

				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), filtering_query.atom_ids), cargs.heterostorage.variant_object.object("atoms_summary"));

				cargs.heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=filtering_result.vertices_info.size();

				cargs.heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=filtering_result.total_relevant_tetrahedron_volume;
			}
		}
	};

	class export_triangulation_voxels : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();

			const SelectionManager::Query parameters_for_selecting_atoms=read_generic_selecting_query(cargs.input);
			FilteringOfTriangulation::Query filtering_query=read_filtering_of_triangulation_query(cargs.input);
			const double search_step_factor=cargs.input.get_value_or_default<double>("search-step-factor", 1.0);
			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			filtering_query.atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(filtering_query.atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(cargs.data_manager.triangulation_info(), filtering_query);

			if(filtering_result.vertices_info.empty())
			{
				throw std::runtime_error(std::string("No triangulation parts selected."));
			}

			OutputSelector output_selector(file);

			int number_of_voxels=0;

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);

				const std::vector<apollota::SimpleSphere>& balls=cargs.data_manager.triangulation_info().spheres;
				const double search_step=1.0/search_step_factor;

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

					const double search_r=(vi.sphere.r-filtering_query.min_radius);
					if(search_r<search_step)
					{
						const apollota::SimpleSphere candidate_ball(vi.sphere, filtering_query.min_radius);
						output << candidate_ball.x << " " << candidate_ball.y << " " << candidate_ball.z << " " << candidate_ball.r << "\n";
						number_of_voxels++;
					}
					else
					{
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
										}
									}
								}
							}
						}
					}
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}

				cargs.heterostorage.variant_object.value("number_of_voxels")=number_of_voxels;
			}
		}
	};

	class print_triangulation : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();

			const SelectionManager::Query parameters_for_selecting_atoms=read_generic_selecting_query(cargs.input);
			FilteringOfTriangulation::Query filtering_query=read_filtering_of_triangulation_query(cargs.input);
			const bool only_summary=cargs.input.get_flag("only-summary");

			cargs.input.assert_nothing_unusable();

			filtering_query.atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(filtering_query.atom_ids.empty())
			{
			    throw std::runtime_error(std::string("No atoms selected."));
			}

			const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(cargs.data_manager.triangulation_info(), filtering_query);

			if(filtering_result.vertices_info.empty())
			{
			    throw std::runtime_error(std::string("No triangulation parts selected."));
			}

			if(!only_summary)
			{
				std::vector<VariantObject> output_array;

				for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
				{
					const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];

					VariantObject info;
					info.value("id")=vi.id;

					{
						std::vector<VariantValue>& suboutput=info.values_array("quadruple");
						for(std::size_t j=0;j<4;j++)
						{
							suboutput.push_back(VariantValue(vi.quadruple.get(j)));
						}
					}

					{
						std::vector<VariantValue>& suboutput=info.values_array("tangent_sphere");
						suboutput.push_back(VariantValue(vi.sphere.x));
						suboutput.push_back(VariantValue(vi.sphere.y));
						suboutput.push_back(VariantValue(vi.sphere.z));
						suboutput.push_back(VariantValue(vi.sphere.r));
					}

					info.value("tetrahedron_volume")=vi.tetrahedron_volume;

					output_array.push_back(info);
				}

				cargs.heterostorage.variant_object.objects_array("vertices")=output_array;
			}

			VariantSerialization::write(SummaryOfTriangulation(cargs.data_manager.triangulation_info()), cargs.heterostorage.variant_object.object("full_triangulation_summary"));

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), filtering_query.atom_ids), cargs.heterostorage.variant_object.object("atoms_summary"));

			cargs.heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=filtering_result.vertices_info.size();

			cargs.heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=filtering_result.total_relevant_tetrahedron_volume;
		}
	};

	class select_atoms_by_triangulation_query : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();

			const SelectionManager::Query parameters_for_selecting_atoms=read_generic_selecting_query(cargs.input);
			FilteringOfTriangulation::Query filtering_query=read_filtering_of_triangulation_query(cargs.input);
			const std::string name=cargs.input.get_value_or_default<std::string>("name", "");

			cargs.input.assert_nothing_unusable();

			filtering_query.atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(filtering_query.atom_ids.empty())
			{
			    throw std::runtime_error(std::string("No initial atoms selected."));
			}

			const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(cargs.data_manager.triangulation_info(), filtering_query);

			if(filtering_result.vertices_info.empty())
			{
			    throw std::runtime_error(std::string("No triangulation parts selected."));
			}

			std::set<std::size_t> result_ids;

			for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
			{
				const FilteringOfTriangulation::VertexInfo& vi=filtering_result.vertices_info[i];
				for(std::size_t j=0;j<4;j++)
				{
					result_ids.insert(vi.quadruple.get(j));
				}
			}

			if(result_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_atoms_selection(name, result_ids);
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), filtering_query.atom_ids), cargs.heterostorage.variant_object.object("initial_atoms_summary"));

			cargs.heterostorage.variant_object.value("number_of_relevant_voronoi_vertices")=filtering_result.vertices_info.size();

			cargs.heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=filtering_result.total_relevant_tetrahedron_volume;

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), result_ids), cargs.heterostorage.variant_object.object("selected_atoms_summary"));

			if(name.empty())
			{
				cargs.heterostorage.variant_object.value("selection_name").set_null();
			}
			else
			{
				cargs.heterostorage.variant_object.value("selection_name")=name;
			}
		}
	};

	class construct_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
			parameters_to_construct_contacts.probe=cargs.input.get_value_or_default<double>("probe", parameters_to_construct_contacts.probe);
			parameters_to_construct_contacts.calculate_volumes=!cargs.input.get_flag("no-calculate-volumes");
			parameters_to_construct_contacts.step=cargs.input.get_value_or_default<double>("step", parameters_to_construct_contacts.step);
			parameters_to_construct_contacts.projections=cargs.input.get_value_or_default<int>("projections", parameters_to_construct_contacts.projections);
			parameters_to_construct_contacts.sih_depth=cargs.input.get_value_or_default<int>("sih-depth", parameters_to_construct_contacts.sih_depth);
			common::ConstructionOfContacts::ParametersToEnhanceContacts parameters_to_enhance_contacts;
			parameters_to_enhance_contacts.probe=parameters_to_construct_contacts.probe;
			parameters_to_enhance_contacts.sih_depth=parameters_to_construct_contacts.sih_depth;
			parameters_to_enhance_contacts.tag_centrality=!cargs.input.get_flag("no-tag-centrality");
			parameters_to_enhance_contacts.tag_peripherial=!cargs.input.get_flag("no-tag-peripherial");
			parameters_to_enhance_contacts.adjunct_solvent_direction=cargs.input.get_flag("adjunct-solvent-direction");
			const bool force=cargs.input.get_flag("force");

			cargs.input.assert_nothing_unusable();

			if(force)
			{
				cargs.data_manager.remove_contacts();
			}

			cargs.data_manager.reset_contacts_by_creating(parameters_to_construct_contacts, parameters_to_enhance_contacts);

			VariantObject& info=cargs.heterostorage.variant_object;
			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("contacts_summary"));
		}
	};

	class make_drawable_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			common::ConstructionOfContacts::ParametersToDrawContacts parameters_to_draw_contacts;
			parameters_to_draw_contacts.probe=cargs.input.get_value_or_default<double>("probe", parameters_to_draw_contacts.probe);
			parameters_to_draw_contacts.step=cargs.input.get_value_or_default<double>("step", parameters_to_draw_contacts.step);
			parameters_to_draw_contacts.projections=cargs.input.get_value_or_default<int>("projections", parameters_to_draw_contacts.projections);
			parameters_to_draw_contacts.simplify=cargs.input.get_flag("simplify");
			parameters_to_draw_contacts.sih_depth=cargs.input.get_value_or_default<int>("sih-depth", parameters_to_draw_contacts.sih_depth);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query("", "[--min-seq-sep 1]", cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			cargs.data_manager.reset_contacts_graphics_by_creating(parameters_to_draw_contacts, ids);

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class make_undrawable_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			cargs.data_manager.remove_contacts_graphics(ids);

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class export_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const bool no_graphics=cargs.input.get_flag("no-graphics");

			cargs.input.assert_nothing_unusable();

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				common::enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("contacts_summary"));
			}
		}
	};

	class import_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");

			cargs.input.assert_nothing_unusable();

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input contacts file name."));
			}

			InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::vector<Contact> contacts;

			auxiliaries::IOUtilities().read_lines_to_set(finput, contacts);

			if(!contacts.empty())
			{
				cargs.data_manager.reset_contacts_by_swapping(contacts);

				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("contacts_summary"));
			}
			else
			{
				throw std::runtime_error(std::string("Failed to read contacts from file '")+file+"'.");
			}
		}
	};

	class select_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool mark=cargs.input.get_flag("mark");

			cargs.input.assert_nothing_unusable();

			assert_selection_name_input(name, true);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_contacts_selection(name, ids);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("contacts_summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}

			if(mark)
			{
				cargs.data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_unmark(true));
				cargs.data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_mark(true), ids);
			}

			cargs.heterostorage.vectors_of_ids["selection_of_contacts"]=std::vector<std::size_t>(ids.begin(), ids.end());
		}
	};

	class export_selection_of_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			OutputSelector output_selector(file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			std::set<common::ChainResidueAtomDescriptorsPair> set_of_crads_pairs;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				set_of_crads_pairs.insert(
						common::ConversionOfDescriptors::get_contact_descriptor(
								cargs.data_manager.atoms(), cargs.data_manager.contacts()[*it]).without_some_info(
										no_serial, no_name, no_resSeq, no_resName));
			}

			auxiliaries::IOUtilities().write_set(set_of_crads_pairs, output);

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("contacts_summary"));
				info.value("number_of_descriptors_written")=set_of_crads_pairs.size();
			}
		}
	};

	class import_selection_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const std::string name=cargs.input.get_value<std::string>("name");
			const bool no_serial=cargs.input.get_flag("no-serial");
			const bool no_name=cargs.input.get_flag("no-name");
			const bool no_resSeq=cargs.input.get_flag("no-resSeq");
			const bool no_resName=cargs.input.get_flag("no-resName");

			cargs.input.assert_nothing_unusable();

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input selection file name."));
			}

			assert_selection_name_input(name, false);

			InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::set<common::ChainResidueAtomDescriptorsPair> set_of_crads_pairs;

			auxiliaries::IOUtilities().read_lines_to_set(finput, set_of_crads_pairs);

			if(set_of_crads_pairs.empty())
			{
				throw std::runtime_error(std::string("No descriptors in file '")+file+"'.");
			}

			if(no_serial || no_name || no_resSeq || no_resName)
			{
				std::set<common::ChainResidueAtomDescriptorsPair> refined_set_of_crads_pairs;
				for(std::set<common::ChainResidueAtomDescriptorsPair>::const_iterator it=set_of_crads_pairs.begin();it!=set_of_crads_pairs.end();++it)
				{
					refined_set_of_crads_pairs.insert(it->without_some_info(no_serial, no_name, no_resSeq, no_resName));
				}
				set_of_crads_pairs.swap(refined_set_of_crads_pairs);
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts_by_set_of_crads_pairs(set_of_crads_pairs);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_contacts_selection(name, ids);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("contacts_summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}
		}
	};

	class set_tag_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

			cargs.input.assert_nothing_unusable();

			assert_tag_input(tag, false);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				cargs.data_manager.contact_tags_mutable(*it).insert(tag);
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class delete_tags_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool all=cargs.input.get_flag("all");
			const std::vector<std::string> tags=cargs.input.get_value_vector_or_default<std::string>("tags", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			if(!all && tags.empty())
			{
				throw std::runtime_error(std::string("No tags specified."));
			}

			if(all && !tags.empty())
			{
				throw std::runtime_error(std::string("Conflicting specification of tags."));
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::set<std::string>& contact_tags=cargs.data_manager.contact_tags_mutable(*it);
				if(all)
				{
					contact_tags.clear();
				}
				else
				{
					for(std::size_t i=0;i<tags.size();i++)
					{
						contact_tags.erase(tags[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class set_adjunct_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=cargs.input.get_value<std::string>("name");
			const bool value_present=cargs.input.is_option("value");
			const double value=cargs.input.get_value_or_default<double>("value", 0.0);
			const bool remove=cargs.input.get_flag("remove");

			cargs.input.assert_nothing_unusable();

			if(value_present && remove)
			{
				throw std::runtime_error(std::string("Value setting and removing options used together."));
			}

			assert_adjunct_name_input(name, false);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& contact_adjuncts=cargs.data_manager.contact_adjuncts_mutable(*it);
				if(remove)
				{
					contact_adjuncts.erase(name);
				}
				else
				{
					contact_adjuncts[name]=value;
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class delete_adjuncts_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool all=cargs.input.get_flag("all");
			const std::vector<std::string> adjuncts=cargs.input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			if(!all && adjuncts.empty())
			{
				throw std::runtime_error(std::string("No adjuncts specified."));
			}

			if(all && !adjuncts.empty())
			{
				throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
			}

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				std::map<std::string, double>& contact_adjuncts=cargs.data_manager.contact_adjuncts_mutable(*it);
				if(all)
				{
					contact_adjuncts.clear();
				}
				else
				{
					for(std::size_t i=0;i<adjuncts.size();i++)
					{
						contact_adjuncts.erase(adjuncts[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};


	class mark_contacts : public GenericCommandForDataManagerScaled
	{
	public:
		mark_contacts() : positive_(true)
		{
		}

		explicit mark_contacts(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			{
				cargs.data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_mark(positive_).set_unmark(!positive_), ids);
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}

	private:
		bool positive_;
	};

	class unmark_contacts : public mark_contacts
	{
	public:
		unmark_contacts() : mark_contacts(false)
		{
		}
	};

	class show_contacts : public GenericCommandForDataManagerScaled
	{
	public:
		show_contacts() : positive_(true)
		{
		}

		explicit show_contacts(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			std::set<std::size_t> representation_ids=cargs.data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

			if(positive_ && representation_ids.empty() && cargs.data_manager.contacts_representation_descriptor().names.size()>1)
			{
				representation_ids.insert(0);
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			cargs.data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive_).set_hide(!positive_), ids);

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}

	private:
		bool positive_;
	};

	class hide_contacts : public show_contacts
	{
	public:
		hide_contacts() : show_contacts(false)
		{
		}
	};

	class color_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const bool next_random_color=cargs.input.get_flag("next-random-color");
			const auxiliaries::ColorUtilities::ColorInteger color_value=(next_random_color ? get_next_random_color() : read_color(cargs.input));

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

			if(!auxiliaries::ColorUtilities::color_valid(color_value))
			{
				throw std::runtime_error(std::string("Contacts color not specified."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			cargs.data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class spectrum_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
			const std::string by=adjunct.empty() ? cargs.input.get_value<std::string>("by") : std::string("adjunct");
			const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
			const bool as_z_scores=cargs.input.get_flag("as-z-scores");
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", (as_z_scores ? -2.0 : 0.0));
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", (as_z_scores ? 2.0 : 1.0));
			const bool only_summarize=cargs.input.get_flag("only-summarize");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

			if(by!="area" && by!="adjunct" && by!="dist-centers" && by!="dist-balls" && by!="seq-sep")
			{
				throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
			}

			if(by=="adjunct" && adjunct.empty())
			{
				throw std::runtime_error(std::string("No adjunct name provided."));
			}

			if(by!="adjunct" && !adjunct.empty())
			{
				throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
			}

			if(!auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
			{
				throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
			}

			if(min_val_present && max_val_present && max_val<=min_val)
			{
				throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			std::map<std::size_t, double> map_of_ids_values;

			if(by=="adjunct")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::map<std::string, double>& adjuncts=cargs.data_manager.contacts()[*it].value.props.adjuncts;
					std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
					if(jt!=adjuncts.end())
					{
						map_of_ids_values[*it]=jt->second;
					}
				}
			}
			else if(by=="area")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					map_of_ids_values[*it]=cargs.data_manager.contacts()[*it].value.area;
				}
			}
			else if(by=="dist-centers")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					map_of_ids_values[*it]=cargs.data_manager.contacts()[*it].value.dist;
				}
			}
			else if(by=="dist-balls")
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id0=cargs.data_manager.contacts()[*it].ids[0];
					const std::size_t id1=cargs.data_manager.contacts()[*it].ids[1];
					if(cargs.data_manager.contacts()[*it].solvent())
					{
						map_of_ids_values[*it]=(cargs.data_manager.contacts()[*it].value.dist-cargs.data_manager.atoms()[id0].value.r)/3.0*2.0;
					}
					else
					{
						map_of_ids_values[*it]=apollota::minimal_distance_from_sphere_to_sphere(cargs.data_manager.atoms()[id0].value, cargs.data_manager.atoms()[id1].value);
					}
				}
			}
			else if(by=="seq-sep")
			{
				double max_seq_sep=0.0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id0=cargs.data_manager.contacts()[*it].ids[0];
					const std::size_t id1=cargs.data_manager.contacts()[*it].ids[1];
					if(cargs.data_manager.atoms()[id0].crad.chainID==cargs.data_manager.atoms()[id1].crad.chainID)
					{
						const double seq_sep=fabs(static_cast<double>(cargs.data_manager.atoms()[id0].crad.resSeq-cargs.data_manager.atoms()[id1].crad.resSeq));
						map_of_ids_values[*it]=seq_sep;
						max_seq_sep=((max_seq_sep<seq_sep) ? seq_sep : max_seq_sep);
					}
				}
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id0=cargs.data_manager.contacts()[*it].ids[0];
					const std::size_t id1=cargs.data_manager.contacts()[*it].ids[1];
					if(cargs.data_manager.atoms()[id0].crad.chainID!=cargs.data_manager.atoms()[id1].crad.chainID)
					{
						map_of_ids_values[*it]=max_seq_sep+1.0;
					}
				}
			}

			if(map_of_ids_values.empty())
			{
				throw std::runtime_error(std::string("Nothing colorable."));
			}

			double min_val_actual=0.0;
			double max_val_actual=0.0;
			int num_of_vals=0;
			double mean_of_values=0.0;
			double sd_of_values=0.0;

			calculate_spectrum_info(
					as_z_scores,
					min_val_present,
					min_val,
					max_val_present,
					max_val,
					min_val_actual,
					max_val_actual,
					num_of_vals,
					mean_of_values,
					sd_of_values,
					map_of_ids_values);

			if(!only_summarize)
			{
				DataManager::DisplayStateUpdater dsu;
				dsu.visual_ids=representation_ids;

				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					dsu.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
					cargs.data_manager.update_contacts_display_state(dsu, it->first);
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("spectrum_summary");
				info.value("min_value")=min_val_actual;
				info.value("max_value")=max_val_actual;
				info.value("number_of_values")=num_of_vals;
				info.value("mean_of_values")=mean_of_values;
				info.value("sd_of_values")=sd_of_values;
			}
		}
	};

	class print_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const bool inter_residue=cargs.input.get_flag("inter-residue");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			const std::vector<Atom>& atoms=cargs.data_manager.atoms();
			std::vector<VariantObject>& contacts=cargs.heterostorage.variant_object.objects_array("contacts");

			if(inter_residue)
			{
				std::map<common::ChainResidueAtomDescriptorsPair, common::ContactValue> map_for_output;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=cargs.data_manager.contacts()[*it];
					if(contact.solvent())
					{
						map_for_output[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), common::ChainResidueAtomDescriptor::solvent())].add(contact.value);
					}
					else
					{
						map_for_output[common::ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), atoms[contact.ids[1]].crad.without_atom())].add(contact.value);
					}
				}
				contacts.reserve(map_for_output.size());
				for(std::map<common::ChainResidueAtomDescriptorsPair, common::ContactValue>::const_iterator it=map_for_output.begin();it!=map_for_output.end();++it)
				{
					contacts.push_back(VariantObject());
					VariantSerialization::write(it->first, it->second, contacts.back());
				}
				cargs.heterostorage.variant_object.value("number_of_inter_residue_contacts")=map_for_output.size();
			}
			else
			{
				contacts.reserve(ids.size());
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					contacts.push_back(VariantObject());
					VariantSerialization::write(atoms, cargs.data_manager.contacts()[*it], contacts.back());
					contacts.back().value("id")=(*it);
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class zoom_by_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> contacts_ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);

			if(contacts_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			std::set<std::size_t> atoms_ids;
			for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
			{
				atoms_ids.insert(cargs.data_manager.contacts()[*it].ids[0]);
				atoms_ids.insert(cargs.data_manager.contacts()[*it].ids[1]);
			}

			if(atoms_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["zoomed"];
			summary_of_atoms=SummaryOfAtoms(cargs.data_manager.atoms(), atoms_ids);

			VariantSerialization::write(summary_of_atoms.bounding_box, cargs.heterostorage.variant_object.object("bounding_box"));
		}
	};

	class export_contacts_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			const std::string name=cargs.input.get_value_or_default<std::string>("name", "contacts");
			const bool wireframe=cargs.input.get_flag("wireframe");
			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string file=cargs.input.get_value<std::string>("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			std::set<std::size_t> representation_ids=cargs.data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

			if(representation_ids.empty())
			{
				representation_ids.insert(0);
			}

			if(representation_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					representation_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable visible contacts selected."));
			}

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
					{
						const std::size_t visual_id=(*jt);
						if(visual_id<cargs.data_manager.contacts_display_states()[id].visuals.size())
						{
							const DataManager::DisplayState::Visual& dsv=cargs.data_manager.contacts_display_states()[id].visuals[visual_id];
							if(prev_color==0 || dsv.color!=prev_color)
							{
								opengl_printer.add_color(dsv.color);
							}
							prev_color=dsv.color;
							if(wireframe)
							{
								opengl_printer.add_as_wireframe(cargs.data_manager.contacts()[id].value.graphics);
							}
							else
							{
								opengl_printer.add(cargs.data_manager.contacts()[id].value.graphics);
							}
						}
					}
				}
			}

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("contacts_summary"));
			}
		}
	};

	class list_selections_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.data_manager.sync_contacts_selections_with_display_states();
			cargs.data_manager.assert_contacts_selections_availability();
			const std::map< std::string, std::set<std::size_t> >& map_of_selections=cargs.data_manager.selection_manager().map_of_contacts_selections();

			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				VariantObject info;
				info.value("name")=(it->first);
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), it->second), info.object("contacts_summary"));
				cargs.heterostorage.variant_object.objects_array("selections").push_back(info);
			}
		}
	};

	class delete_selections_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_selections_availability();

			const bool all=cargs.input.get_flag("all");

			std::vector<std::string> names;

			if(all)
			{
				names=cargs.data_manager.selection_manager().get_names_of_contacts_selections();
			}
			else
			{
				names=cargs.input.get_list_of_unnamed_values();
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No contacts selections names specified."));
			}

			cargs.data_manager.assert_contacts_selections_availability(names);

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.data_manager.selection_manager().delete_contacts_selection(names[i]);
			}

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.heterostorage.variant_object.values_array("deleted_selections").push_back(VariantValue(names[i]));
			}
		}
	};

	class rename_selection_of_contacts : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_selections_availability();

			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();

			if(names.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
			}

			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().get_contacts_selection(names[0]);
			cargs.data_manager.selection_manager().set_contacts_selection(names[1], ids);
			cargs.data_manager.selection_manager().delete_contacts_selection(names[0]);
		}
	};

	class export_atoms_and_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_contacts_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const bool no_graphics=cargs.input.get_flag("no-graphics");

			cargs.input.assert_nothing_unusable();

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				auxiliaries::IOUtilities().write_set(cargs.data_manager.atoms(), output);
				output << "_end_atoms\n";
				common::enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
				output << "_end_contacts\n";
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("atoms_summary"));
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("contacts_summary"));
			}
		}
	};

	class calculate_burial_depth : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string name=cargs.input.get_value<std::string>("name");
			const int min_seq_sep=cargs.input.get_value_or_default<int>("min-seq-sep", 1);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(name, false);

			std::set<common::ChainResidueAtomDescriptorsPair> set_of_contacts;
			for(std::size_t i=0;i<cargs.data_manager.contacts().size();i++)
			{
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(cargs.data_manager.atoms(), cargs.data_manager.contacts()[i]);
				if(common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, min_seq_sep, common::ChainResidueAtomDescriptor::null_num(), true))
				{
					set_of_contacts.insert(crads);
				}
			}

			const std::map<common::ChainResidueAtomDescriptor, int> map_crad_to_depth=common::ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_contacts);

			for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
			{
				const Atom& atom=cargs.data_manager.atoms()[i];
				std::map<std::string, double>& atom_adjuncts=cargs.data_manager.atom_adjuncts_mutable(i);
				atom_adjuncts.erase(name);
				std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=map_crad_to_depth.find(atom.crad);
				if(it!=map_crad_to_depth.end())
				{
					atom_adjuncts[name]=it->second;
				}
			}
		}
	};

	class voromqa_global : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			ScoringOfDataManagerUsingVoroMQA::Parameters params;
			params.adjunct_inter_atom_energy_scores_raw=cargs.input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
			params.adjunct_inter_atom_energy_scores_normalized=cargs.input.get_value_or_default<std::string>("adj-contact-energy-normalized", "");
			params.adjunct_atom_depth_weights=cargs.input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
			params.adjunct_atom_quality_scores=cargs.input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");
			params.adjunct_residue_quality_scores_raw=cargs.input.get_value_or_default<std::string>("adj-residue-quality-raw", "");
			params.adjunct_residue_quality_scores_smoothed=cargs.input.get_value_or_default<std::string>("adj-residue-quality", "voromqa_score_r");
			params.smoothing_window=cargs.input.get_value_or_default<unsigned int>("smoothing-window", params.smoothing_window);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(params.adjunct_inter_atom_energy_scores_raw, true);
			assert_adjunct_name_input(params.adjunct_inter_atom_energy_scores_normalized, true);
			assert_adjunct_name_input(params.adjunct_atom_depth_weights, true);
			assert_adjunct_name_input(params.adjunct_atom_quality_scores, true);
			assert_adjunct_name_input(params.adjunct_residue_quality_scores_raw, true);
			assert_adjunct_name_input(params.adjunct_residue_quality_scores_smoothed, true);

			ScoringOfDataManagerUsingVoroMQA::Result result;
			ScoringOfDataManagerUsingVoroMQA::construct_result(params, cargs.data_manager, result);

			VariantObject& info=cargs.heterostorage.variant_object;
			info.value("quality_score")=result.global_quality_score;
			info.value("atoms_count")=result.bundle_of_quality.atom_quality_scores.size();
			info.value("residues_count")=result.bundle_of_quality.raw_residue_quality_scores.size();
			info.value("contacts_count")=result.bundle_of_energy.global_energy_descriptor.contacts_count;
			info.value("pseudo_energy")=result.bundle_of_energy.global_energy_descriptor.energy;
			info.value("total_area")=result.bundle_of_energy.global_energy_descriptor.total_area;
			info.value("strange_area")=result.bundle_of_energy.global_energy_descriptor.strange_area;
		}
	};

	class voromqa_local : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string selection_expresion_for_atoms=cargs.input.get_value_or_default<std::string>("atoms", "[]");
			const bool provided_selection_expresion_for_contacts=cargs.input.is_option("contacts");
			const std::string selection_expresion_for_contacts=cargs.input.get_value_or_default<std::string>("contacts", "[]");
			const std::string adjunct_inter_atom_energy_scores_raw=cargs.input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
			const std::string adjunct_atom_depth_weights=cargs.input.get_value_or_default<std::string>("adj-atom-depth", "voromqa_depth");
			const std::string adjunct_atom_quality_scores=cargs.input.get_value_or_default<std::string>("adj-atom-quality", "voromqa_score_a");

			cargs.input.assert_nothing_unusable();

			if(provided_selection_expresion_for_contacts)
			{
				cargs.data_manager.assert_contacts_availability();
			}

			assert_adjunct_name_input(adjunct_inter_atom_energy_scores_raw, false);
			assert_adjunct_name_input(adjunct_atom_depth_weights, false);
			assert_adjunct_name_input(adjunct_atom_quality_scores, false);

			std::set<std::size_t> atom_ids;
			std::set<std::size_t> contact_ids;

			atom_ids=cargs.data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms, false));

			if(!cargs.data_manager.contacts().empty())
			{
				contact_ids=cargs.data_manager.selection_manager().select_contacts(SelectionManager::Query(selection_expresion_for_contacts, false));
				atom_ids=cargs.data_manager.selection_manager().select_atoms_by_contacts(atom_ids, contact_ids, false);
				contact_ids=cargs.data_manager.selection_manager().select_contacts_by_atoms(contact_ids, atom_ids, false);
			}

			{
				if(atom_ids.empty())
				{
					throw std::runtime_error(std::string("No atoms selected."));
				}

				const std::set<std::size_t> atom_ids_with_adjuncts=cargs.data_manager.selection_manager().select_atoms(
						SelectionManager::Query(atom_ids, (std::string("[")+"--adjuncts "+adjunct_atom_depth_weights+"&"+adjunct_atom_quality_scores+"]"), false));

				double sum_of_atom_weights=0.0;
				double sum_of_atom_weighted_scores=0.0;

				for(std::set<std::size_t>::const_iterator it=atom_ids_with_adjuncts.begin();it!=atom_ids_with_adjuncts.end();++it)
				{
					const std::size_t id=(*it);
					const double weight=cargs.data_manager.atoms()[id].value.props.adjuncts.find(adjunct_atom_depth_weights)->second;
					const double score=cargs.data_manager.atoms()[id].value.props.adjuncts.find(adjunct_atom_quality_scores)->second;
					sum_of_atom_weights+=weight;
					sum_of_atom_weighted_scores+=(weight*score);
				}

				const double quality_score=(sum_of_atom_weights>0.0 ? (sum_of_atom_weighted_scores/sum_of_atom_weights) : 0.0);

				VariantObject& info=cargs.heterostorage.variant_object.object("atoms_result");
				info.value("atoms_selected").set_value_int(atom_ids.size());
				info.value("atoms_relevant").set_value_int(atom_ids_with_adjuncts.size());
				info.value("quality_score")=quality_score;
			}

			if(!cargs.data_manager.contacts().empty())
			{
				if(contact_ids.empty())
				{
					throw std::runtime_error(std::string("No contacts selected."));
				}

				const std::set<std::size_t> contact_ids_with_adjuncts=cargs.data_manager.selection_manager().select_contacts(
						SelectionManager::Query(contact_ids, (std::string("[")+"--adjuncts "+adjunct_inter_atom_energy_scores_raw+"]"), false));

				double sum_of_areas=0.0;
				double sum_of_energies=0.0;

				for(std::set<std::size_t>::const_iterator it=contact_ids_with_adjuncts.begin();it!=contact_ids_with_adjuncts.end();++it)
				{
					const std::size_t id=(*it);
					const double area=cargs.data_manager.contacts()[id].value.area;
					const double energy=cargs.data_manager.contacts()[id].value.props.adjuncts.find(adjunct_inter_atom_energy_scores_raw)->second;
					sum_of_areas+=area;
					sum_of_energies+=energy;
				}

				VariantObject& info=cargs.heterostorage.variant_object.object("contacts_result");
				info.value("contacts_selected").set_value_int(contact_ids.size());
				info.value("contacts_relevant").set_value_int(contact_ids_with_adjuncts.size());
				info.value("area")=sum_of_areas;
				info.value("pseudo_energy")=sum_of_energies;
			}
		}
	};

	class voromqa_frustration : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string adjunct_contact_energy=cargs.input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
			const std::string adjunct_contact_frustration_energy_mean=cargs.input.get_value_or_default<std::string>("adj-contact-frustration-energy-mean", "frustration_energy_mean");
			const std::string adjunct_atom_frustration_energy_mean=cargs.input.get_value_or_default<std::string>("adj-atom-frustration-energy-mean", "frustration_energy_mean");
			const unsigned int smoothing_iterations=cargs.input.get_value_or_default<unsigned int>("smoothing-iterations", 1);
			const unsigned int smoothing_depth=cargs.input.get_value_or_default<unsigned int>("smoothing-depth", 3);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_contact_energy, false);
			assert_adjunct_name_input(adjunct_contact_frustration_energy_mean, true);
			assert_adjunct_name_input(adjunct_atom_frustration_energy_mean, true);

			const std::set<std::size_t> solvent_contact_ids=cargs.data_manager.selection_manager().select_contacts(
					SelectionManager::Query(std::string("[--solvent --adjuncts ")+adjunct_contact_energy+"]", false));

			if(solvent_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No solvent contacts with energy values."));
			}

			const std::set<std::size_t> exterior_atom_ids=cargs.data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

			const std::set<std::size_t> exterior_contact_ids=cargs.data_manager.selection_manager().select_contacts(
					SelectionManager::Query("[--tags peripherial]", false));

			if(exterior_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No peripherial contacts."));
			}

			std::vector<std::size_t> atom_solvent_contact_ids(cargs.data_manager.atoms().size(), 0);
			std::vector<double> atom_solvent_contact_areas(cargs.data_manager.atoms().size(), 0.0);
			std::vector<double> atom_solvent_contact_energy_means(cargs.data_manager.atoms().size(), 0.0);

			for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];
				atom_solvent_contact_ids[contact.ids[0]]=(*it);
				atom_solvent_contact_areas[contact.ids[0]]=contact.value.area;
				if(contact.value.area>0.0 && contact.value.props.adjuncts.count(adjunct_contact_energy)>0)
				{
					atom_solvent_contact_energy_means[contact.ids[0]]=(contact.value.props.adjuncts.find(adjunct_contact_energy)->second)/contact.value.area;
				}
			}

			std::vector< std::set<std::size_t> > graph(cargs.data_manager.atoms().size());
			for(std::set<std::size_t>::const_iterator it=exterior_contact_ids.begin();it!=exterior_contact_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];
				graph[contact.ids[0]].insert(contact.ids[1]);
				graph[contact.ids[1]].insert(contact.ids[0]);
			}

			for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
			{
				std::vector<double> updated_atom_solvent_contact_energy_means=atom_solvent_contact_energy_means;

				for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
				{
					const std::size_t central_id=(*it);

					std::map<std::size_t, bool> neighbors;
					neighbors[central_id]=false;

					for(unsigned int level=0;level<smoothing_depth;level++)
					{
						std::map<std::size_t, bool> more_neighbors;
						for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							const std::size_t id=jt->first;
							if(!jt->second)
							{
								for(std::set<std::size_t>::const_iterator et=graph[id].begin();et!=graph[id].end();++et)
								{
									more_neighbors[*et]=(neighbors.count(*et)>0 && neighbors.find(*et)->second);
								}
							}
							more_neighbors[id]=true;
						}
						neighbors.swap(more_neighbors);
					}

					double sum_of_areas=0.0;
					double sum_of_energies=0.0;

					for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
					{
						sum_of_areas+=atom_solvent_contact_areas[jt->first];
						sum_of_energies+=atom_solvent_contact_energy_means[jt->first]*atom_solvent_contact_areas[jt->first];
					}

					updated_atom_solvent_contact_energy_means[central_id]=(sum_of_areas>0.0 ? (sum_of_energies/sum_of_areas) : 0.0);
				}

				atom_solvent_contact_energy_means.swap(updated_atom_solvent_contact_energy_means);
			}

			if(!adjunct_contact_frustration_energy_mean.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.contacts().size();i++)
				{
					cargs.data_manager.contact_adjuncts_mutable(i).erase(adjunct_contact_frustration_energy_mean);
				}
			}

			if(!adjunct_atom_frustration_energy_mean.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
				{
					cargs.data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_frustration_energy_mean);
				}
			}

			if(!adjunct_contact_frustration_energy_mean.empty() || !adjunct_atom_frustration_energy_mean.empty())
			{
				for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
				{
					const std::size_t central_id=(*it);

					if(!adjunct_contact_frustration_energy_mean.empty())
					{
						cargs.data_manager.contact_adjuncts_mutable(atom_solvent_contact_ids[central_id])[adjunct_contact_frustration_energy_mean]=atom_solvent_contact_energy_means[central_id];
					}

					if(!adjunct_atom_frustration_energy_mean.empty())
					{
						cargs.data_manager.atom_adjuncts_mutable(central_id)[adjunct_atom_frustration_energy_mean]=atom_solvent_contact_energy_means[central_id];
					}
				}
			}
		}
	};

	class voromqa_membrane_place : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string adjunct_contact_frustration_value=cargs.input.get_value_or_default<std::string>("adj-contact-frustration-value", "frustration_energy_mean");
			const std::string adjunct_atom_exposure_value=cargs.input.get_value_or_default<std::string>("adj-atom-exposure-value", "exposure_value");
			const std::string adjunct_atom_membrane_place_value=cargs.input.get_value_or_default<std::string>("adj-atom-membrane-place-value", "membrane_place_value");
			const double membrane_width=cargs.input.get_value<double>("membrane-width");
			const double membrane_width_extended=cargs.input.get_value_or_default<double>("membrane-width-extended", membrane_width);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_contact_frustration_value, false);
			assert_adjunct_name_input(adjunct_atom_exposure_value, true);
			assert_adjunct_name_input(adjunct_atom_membrane_place_value, true);

			if(membrane_width<6.0)
			{
				throw std::runtime_error(std::string("Invalid membrane width."));
			}

			if(membrane_width_extended<membrane_width)
			{
				throw std::runtime_error(std::string("Invalid extended membrane width."));
			}

			std::vector<MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor> atom_descriptors=
					MembranePlacementForDataManagerUsingVoroMQA::init_atom_descriptors(cargs.data_manager, adjunct_contact_frustration_value, adjunct_atom_exposure_value);

			const MembranePlacementForDataManagerUsingVoroMQA::OrientationScore best_score=
					MembranePlacementForDataManagerUsingVoroMQA::score_orientation(atom_descriptors, membrane_width, membrane_width_extended);

			if(!adjunct_atom_membrane_place_value.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
				{
					cargs.data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_membrane_place_value);
				}

				for(std::size_t i=0;i<atom_descriptors.size();i++)
				{
					const MembranePlacementForDataManagerUsingVoroMQA::AtomDescriptor& ad=atom_descriptors[i];
					cargs.data_manager.atom_adjuncts_mutable(ad.atom_id)[adjunct_atom_membrane_place_value]=ad.membrane_place_value;
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("number_of_checks")=best_score.number_of_checks;
				info.value("best_score")=best_score.value();
				std::vector<VariantValue>& direction=info.values_array("direction");
				direction.resize(3);
				direction[0]=best_score.direction.x;
				direction[1]=best_score.direction.y;
				direction[2]=best_score.direction.z;
				info.value("projection_center")=best_score.projection_center;
			}
		}
	};

	class describe_exposure : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();
			cargs.data_manager.assert_contacts_availability();

			const std::string adjunct_atom_exposure_value=cargs.input.get_value_or_default<std::string>("adj-atom-exposure-value", "exposure_value");
			const double probe_min=cargs.input.get_value_or_default<double>("probe-min", 0);
			const double probe_max=cargs.input.get_value_or_default<double>("probe-max", 30.0);
			const double expansion=cargs.input.get_value_or_default<double>("expansion", 1.0);
			const unsigned int smoothing_iterations=cargs.input.get_value_or_default<unsigned int>("smoothing-iterations", 2);
			const unsigned int smoothing_depth=cargs.input.get_value_or_default<unsigned int>("smoothing-depth", 1);
			const double weight_power=cargs.input.get_value_or_default<double>("weight-power", 3.0);
			const bool use_sas_areas=cargs.input.get_flag("use-sas-areas");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_atom_exposure_value, false);

			const std::vector<apollota::SimpleSphere>& balls=common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(cargs.data_manager.atoms());

			const apollota::BoundingSpheresHierarchy bsh(balls, 3.5, 1);

			const apollota::Triangulation::VerticesVector t_vertices=
					apollota::Triangulation::collect_vertices_vector_from_quadruples_map(cargs.data_manager.triangulation_info().quadruples_map);

			std::vector<double> atoms_values(cargs.data_manager.atoms().size(), 0.0);
			std::vector<double> atoms_weights(cargs.data_manager.atoms().size(), 0.0);

			for(std::size_t i=0;i<t_vertices.size();i++)
			{
				const apollota::Quadruple& q=t_vertices[i].first;
				const apollota::SimpleSphere& s=t_vertices[i].second;
				if(s.r>probe_min)
				{
					double t_vertex_value=0.0;
					double t_vertex_weight=0.0;

					const std::vector<std::size_t> near_ids=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, apollota::SimpleSphere(s, s.r+expansion));
					const std::size_t N=near_ids.size();

					if(s.r>probe_max || q.get_min_max().second>=balls.size())
					{
						t_vertex_weight=std::pow(probe_max, weight_power);
					}
					else
					{
						if(N>=4)
						{
							std::vector<apollota::SimplePoint> touches(N);
							for(std::size_t j=0;j<N;j++)
							{
								touches[j]=(apollota::SimplePoint(balls[near_ids[j]])-apollota::SimplePoint(s)).unit();
							}

							double d_sum=0.0;
							for(std::size_t j=0;(j+1)<N;j++)
							{
								for(std::size_t k=j+1;k<N;k++)
								{
									d_sum+=apollota::distance_from_point_to_point(touches[j], touches[k]);
								}
							}

							const double pi=3.14159265358979323846;
							const double max_d_sum=N*(1.0/tan(pi/(2.0*N)));
							t_vertex_value=std::min(d_sum/max_d_sum, 1.0);
						}
						t_vertex_weight=std::pow(s.r, weight_power);
					}

					for(std::size_t j=0;j<N;j++)
					{
						const std::size_t id=near_ids[j];
						atoms_values[id]+=t_vertex_value*t_vertex_weight;
						atoms_weights[id]+=t_vertex_weight;
					}
				}
			}

			for(std::size_t i=0;i<atoms_values.size();i++)
			{
				if(atoms_weights[i]>0.0)
				{
					atoms_values[i]=atoms_values[i]/atoms_weights[i];
				}
			}

			{
				const std::set<std::size_t> solvent_contact_ids=cargs.data_manager.selection_manager().select_contacts(
						SelectionManager::Query(std::string("[--solvent]"), false));

				if(solvent_contact_ids.empty())
				{
					throw std::runtime_error(std::string("No solvent contacts."));
				}

				if(use_sas_areas)
				{
					for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
					{
						const Contact& contact=cargs.data_manager.contacts()[*it];
						atoms_weights[contact.ids[0]]*=contact.value.area;
					}
				}

				const std::set<std::size_t> exterior_atom_ids=cargs.data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

				const std::set<std::size_t> exterior_contact_ids=cargs.data_manager.selection_manager().select_contacts(
						SelectionManager::Query("[--tags peripherial]", false));

				if(exterior_contact_ids.empty())
				{
					throw std::runtime_error(std::string("No peripherial contacts."));
				}

				std::vector< std::set<std::size_t> > graph(cargs.data_manager.atoms().size());
				for(std::set<std::size_t>::const_iterator it=exterior_contact_ids.begin();it!=exterior_contact_ids.end();++it)
				{
					const Contact& contact=cargs.data_manager.contacts()[*it];
					graph[contact.ids[0]].insert(contact.ids[1]);
					graph[contact.ids[1]].insert(contact.ids[0]);
				}

				for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
				{
					std::vector<double> updated_atoms_values=atoms_values;

					for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
					{
						const std::size_t central_id=(*it);

						std::map<std::size_t, bool> neighbors;
						neighbors[central_id]=false;

						for(unsigned int level=0;level<smoothing_depth;level++)
						{
							std::map<std::size_t, bool> more_neighbors;
							for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
							{
								const std::size_t id=jt->first;
								if(!jt->second)
								{
									for(std::set<std::size_t>::const_iterator et=graph[id].begin();et!=graph[id].end();++et)
									{
										more_neighbors[*et]=(neighbors.count(*et)>0 && neighbors.find(*et)->second);
									}
								}
								more_neighbors[id]=true;
							}
							neighbors.swap(more_neighbors);
						}

						double sum_of_weights=0.0;
						double sum_of_weighted_values=0.0;

						for(std::map<std::size_t, bool>::const_iterator jt=neighbors.begin();jt!=neighbors.end();++jt)
						{
							sum_of_weights+=atoms_weights[jt->first];
							sum_of_weighted_values+=atoms_values[jt->first]*atoms_weights[jt->first];
						}

						updated_atoms_values[central_id]=(sum_of_weights>0.0 ? (sum_of_weighted_values/sum_of_weights) : 0.0);
					}

					atoms_values.swap(updated_atoms_values);
				}

				if(!adjunct_atom_exposure_value.empty())
				{
					for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
					{
						cargs.data_manager.atom_adjuncts_mutable(i).erase(adjunct_atom_exposure_value);
					}

					for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
					{
						const std::size_t central_id=(*it);
						if(atoms_weights[central_id]>0.0)
						{
							cargs.data_manager.atom_adjuncts_mutable(central_id)[adjunct_atom_exposure_value]=atoms_values[central_id];
						}
					}
				}
			}
		}
	};

	class find_connected_components : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting_atoms_core=read_generic_selecting_query("atoms-core-", "[]", cargs.input);
			const SelectionManager::Query parameters_for_selecting_atoms_all=read_generic_selecting_query("atoms-all-", parameters_for_selecting_atoms_core.expression_string, cargs.input);
			const SelectionManager::Query parameters_for_selecting_contacts=read_generic_selecting_query("contacts-", "[]", cargs.input);
			const std::string adjunct_component_number=cargs.input.get_value_or_default<std::string>("adj-component-number", "component_number");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_component_number, false);

			const std::set<std::size_t> core_atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms_core);
			if(core_atom_ids.empty())
			{
				throw std::runtime_error(std::string("No core atoms selected."));
			}

			std::set<std::size_t> all_atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms_all);
			all_atom_ids.insert(core_atom_ids.begin(), core_atom_ids.end());
			if(all_atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			const std::set<std::size_t> contact_ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
			if(contact_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			std::vector<int> atoms_enabled(cargs.data_manager.atoms().size(), 0);
			for(std::set<std::size_t>::const_iterator it=all_atom_ids.begin();it!=all_atom_ids.end();++it)
			{
				atoms_enabled[*it]=1;
			}
			for(std::set<std::size_t>::const_iterator it=core_atom_ids.begin();it!=core_atom_ids.end();++it)
			{
				atoms_enabled[*it]=2;
			}

			std::vector< std::set<std::size_t> > graph(cargs.data_manager.atoms().size());
			for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];
				if(!contact.solvent() && atoms_enabled[contact.ids[0]]>0 && atoms_enabled[contact.ids[1]]>0)
				{
					graph[contact.ids[0]].insert(contact.ids[1]);
					graph[contact.ids[1]].insert(contact.ids[0]);
				}
			}

			std::vector<int> atoms_component_nums(cargs.data_manager.atoms().size(), 0);
			{
				int current_component_num=0;
				for(std::set<std::size_t>::const_iterator start_it=core_atom_ids.begin();start_it!=core_atom_ids.end();++start_it)
				{
					const std::size_t start_id=(*start_it);
					if(atoms_enabled[start_id]>0 && atoms_component_nums[start_id]==0)
					{
						++current_component_num;
						atoms_component_nums[start_id]=current_component_num;
						std::vector<std::size_t> stack;
						stack.push_back(start_id);
						while(!stack.empty())
						{
							const std::size_t central_id=stack.back();
							stack.pop_back();
							for(std::set<std::size_t>::const_iterator neighbor_it=graph[central_id].begin();neighbor_it!=graph[central_id].end();++neighbor_it)
							{
								const std::size_t neignbor_id=(*neighbor_it);
								if(atoms_enabled[neignbor_id]>0 && atoms_component_nums[neignbor_id]==0)
								{
									atoms_component_nums[neignbor_id]=current_component_num;
									stack.push_back(neignbor_id);
								}
							}
						}
					}
				}
			}

			VariantObject components_info;

			{
				std::map<int, int> map_of_component_nums_to_counts;
				for(std::size_t i=0;i<atoms_component_nums.size();i++)
				{
					if(atoms_component_nums[i]>0)
					{
						map_of_component_nums_to_counts[atoms_component_nums[i]]++;
					}
				}

				std::map<int, int> map_of_counts_to_ranks;
				for(std::map<int, int>::const_iterator it=map_of_component_nums_to_counts.begin();it!=map_of_component_nums_to_counts.end();++it)
				{
					map_of_counts_to_ranks[it->second]=0;
				}
				{
					int rank=0;
					for(std::map<int, int>::reverse_iterator it=map_of_counts_to_ranks.rbegin();it!=map_of_counts_to_ranks.rend();++it)
					{
						++rank;
						it->second=rank;
					}
				}

				for(std::size_t i=0;i<atoms_component_nums.size();i++)
				{
					if(atoms_component_nums[i]>0)
					{
						atoms_component_nums[i]=map_of_counts_to_ranks[map_of_component_nums_to_counts[atoms_component_nums[i]]];
					}
				}

				components_info.value("number_of_components")=map_of_component_nums_to_counts.size();
				for(std::map<int, int>::reverse_iterator it=map_of_counts_to_ranks.rbegin();it!=map_of_counts_to_ranks.rend();++it)
				{
					VariantObject obj;
					obj.value("rank")=it->second;
					obj.value("size")=it->first;
					components_info.objects_array("component_sizes").push_back(obj);
				}
			}

			if(!adjunct_component_number.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
				{
					cargs.data_manager.atom_adjuncts_mutable(i).erase(adjunct_component_number);
				}

				for(std::set<std::size_t>::const_iterator it=all_atom_ids.begin();it!=all_atom_ids.end();++it)
				{
					const std::size_t central_id=(*it);
					if(atoms_component_nums[central_id]>0)
					{
						cargs.data_manager.atom_adjuncts_mutable(central_id)[adjunct_component_number]=atoms_component_nums[central_id];
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), all_atom_ids), cargs.heterostorage.variant_object.object("atoms_summary"));

			cargs.heterostorage.variant_object.object("components_summary")=components_info;
		}
	};

	class add_figure : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string> name=cargs.input.get_value_vector<std::string>("name");
			const std::vector<float> vertices=cargs.input.get_value_vector<float>("vertices");
			const std::vector<float> normals=cargs.input.get_value_vector<float>("normals");
			const std::vector<unsigned int> indices=cargs.input.get_value_vector<unsigned int>("indices");

			cargs.input.assert_nothing_unusable();

			Figure figure;
			figure.name=name;
			figure.vertices=vertices;
			figure.normals=normals;
			figure.indices=indices;

			cargs.data_manager.add_figure(figure);

			{
				const Figure& figure=cargs.data_manager.figures().back();
				VariantObject& info=cargs.heterostorage.variant_object.object("added_figure");
				for(std::size_t i=0;i<figure.name.size();i++)
				{
					info.values_array("name").push_back(VariantValue(figure.name[i]));
				}
			}
		}
	};

	class print_figures : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();

			const std::vector<std::string> name=cargs.input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
			const std::vector<std::size_t> forced_ids=cargs.input.get_value_vector_or_default<std::size_t>("id", std::vector<std::size_t>());

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=
					Figure::match_name(cargs.data_manager.figures(), forced_ids.empty(), std::set<std::size_t>(forced_ids.begin(), forced_ids.end()), name);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No figures selected."));
			}

			std::vector<VariantObject>& figures=cargs.heterostorage.variant_object.objects_array("figures");
			figures.reserve(ids.size());
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Figure& figure=cargs.data_manager.figures()[*it];
				figures.push_back(VariantObject());
				for(std::size_t i=0;i<figure.name.size();i++)
				{
					figures.back().values_array("name").push_back(VariantValue(figure.name[i]));
				}
				if(!figure.props.empty())
				{
					VariantSerialization::write(figure.props, figures.back());
				}
				figures.back().value("id")=(*it);
			}
		}
	};

	class show_figures : public GenericCommandForDataManagerScaled
	{
	public:
		show_figures() : positive_(true)
		{
		}

		explicit show_figures(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();
			cargs.data_manager.assert_figures_representations_availability();

			const std::vector<std::string> name=cargs.input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.figures_representation_descriptor().ids_by_names(representation_names);

			const std::set<std::size_t> ids=cargs.data_manager.filter_figures_drawable_implemented_ids(
					representation_ids,
					Figure::match_name(cargs.data_manager.figures(), name),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable figures selected."));
			}

			cargs.data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_show(positive_).set_hide(!positive_), ids);
		}

	private:
		bool positive_;
	};

	class hide_figures : public show_figures
	{
	public:
		hide_figures() : show_figures(false)
		{
		}
	};

	class color_figures : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();
			cargs.data_manager.assert_figures_representations_availability();

			const std::vector<std::string> name=cargs.input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const bool next_random_color=cargs.input.get_flag("next-random-color");
			const auxiliaries::ColorUtilities::ColorInteger color_value=(next_random_color ? get_next_random_color() : read_color(cargs.input));

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> representation_ids=cargs.data_manager.figures_representation_descriptor().ids_by_names(representation_names);

			if(!auxiliaries::ColorUtilities::color_valid(color_value))
			{
				throw std::runtime_error(std::string("Figure color not specified."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_figures_drawable_implemented_ids(
					representation_ids,
					Figure::match_name(cargs.data_manager.figures(), name),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable figures selected."));
			}

			cargs.data_manager.update_figures_display_states(DataManager::DisplayStateUpdater().set_visual_ids(representation_ids).set_color(color_value), ids);
		}
	};

	class list_figures : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();

			const std::vector<std::string> name=cargs.input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=Figure::match_name(cargs.data_manager.figures(), name);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No figures selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Figure& figure=cargs.data_manager.figures().at(*it);
				VariantObject info;
				for(std::size_t i=0;i<figure.name.size();i++)
				{
					info.values_array("name").push_back(VariantValue(figure.name[i]));
				}
				cargs.heterostorage.variant_object.objects_array("figures").push_back(info);
			}
		}
	};

	class delete_figures : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();

			const std::vector<std::string> name=cargs.input.get_value_vector_or_default<std::string>("name", std::vector<std::string>());

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=Figure::match_name(cargs.data_manager.figures(), name);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No figures selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Figure& figure=cargs.data_manager.figures().at(*it);
				VariantObject info;
				for(std::size_t i=0;i<figure.name.size();i++)
				{
					info.values_array("name").push_back(VariantValue(figure.name[i]));
				}
				cargs.heterostorage.variant_object.objects_array("deleted_figures").push_back(info);
			}

			cargs.data_manager.remove_figures(ids);
		}
	};

	class add_figure_of_triangulation : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_triangulation_info_availability();

			const SelectionManager::Query parameters_for_selecting_atoms=read_generic_selecting_query(cargs.input);
			FilteringOfTriangulation::Query filtering_query=read_filtering_of_triangulation_query(cargs.input);
			const std::vector<std::string> figure_name=cargs.input.get_value_vector<std::string>("figure-name");
			const bool as_spheres=cargs.input.get_flag("as-spheres");

			cargs.input.assert_nothing_unusable();

			filtering_query.atom_ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

			if(filtering_query.atom_ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			const FilteringOfTriangulation::MatchingResult filtering_result=FilteringOfTriangulation::match_vertices(cargs.data_manager.triangulation_info(), filtering_query);

			if(filtering_result.vertices_info.empty())
			{
				throw std::runtime_error(std::string("No triangulation parts selected."));
			}

			Figure figure;
			figure.name=figure_name;

			if(as_spheres)
			{
				const apollota::SubdividedIcosahedron sih0(0);
				const apollota::SubdividedIcosahedron sih1(1);
				const apollota::SubdividedIcosahedron sih2(2);
				for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
				{
					const apollota::SimpleSphere& sphere=filtering_result.vertices_info[i].sphere;
					if(sphere.r>0.0)
					{
						figure.add_sphere((sphere.r<0.5 ? sih0 : (sphere.r<1.5 ? sih1 : sih2)), apollota::SimplePoint(sphere), sphere.r);
					}
				}
			}
			else
			{
				const std::vector<apollota::SimpleSphere>& balls=cargs.data_manager.triangulation_info().spheres;

				for(std::size_t i=0;i<filtering_result.vertices_info.size();i++)
				{
					const apollota::Quadruple& quadruple=filtering_result.vertices_info[i].quadruple;

					for(unsigned int j=0;j<4;j++)
					{
						apollota::Triple triple=quadruple.exclude(j);
						const apollota::SimplePoint normal=apollota::plane_normal_from_three_points<apollota::SimplePoint>(balls[triple.get(0)], balls[triple.get(1)], balls[triple.get(2)]);
						figure.add_triangle(balls[triple.get(0)], balls[triple.get(1)], balls[triple.get(2)], normal);
					}
				}
			}

			figure.props.adjuncts["total_relevant_tetrahedron_volume"]=filtering_result.total_relevant_tetrahedron_volume;

			cargs.data_manager.add_figure(figure);
		}
	};

	class export_figures_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_figures_availability();
			cargs.data_manager.assert_figures_representations_availability();

			const std::vector<std::string> figure_name=cargs.input.get_value_vector_or_default<std::string>("figure-name", std::vector<std::string>());
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const std::string cgo_name=cargs.input.get_value_or_default<std::string>("cgo-name", "figures");
			const std::string file=cargs.input.get_value<std::string>("file");
			assert_file_name_input(file, false);

			cargs.input.assert_nothing_unusable();

			if(cgo_name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			std::set<std::size_t> representation_ids=cargs.data_manager.figures_representation_descriptor().ids_by_names(representation_names);

			if(representation_ids.empty())
			{
				representation_ids.insert(0);
			}

			if(representation_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_figures_drawable_implemented_ids(
					representation_ids,
					Figure::match_name(cargs.data_manager.figures(), figure_name),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable figures selected."));
			}

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
					{
						const std::size_t visual_id=(*jt);
						if(visual_id<cargs.data_manager.figures_display_states()[id].visuals.size())
						{
							const DataManager::DisplayState::Visual& dsv=cargs.data_manager.figures_display_states()[id].visuals[visual_id];
							if(prev_color==0 || dsv.color!=prev_color)
							{
								opengl_printer.add_color(dsv.color);
							}
							prev_color=dsv.color;
							const Figure& figure=cargs.data_manager.figures()[id];
							for(std::size_t i=0;i<figure.indices.size();i+=3)
							{
								std::vector<apollota::SimplePoint> vertices(3);
								std::vector<apollota::SimplePoint> normals(3);
								for(std::size_t j=0;j<3;j++)
								{
									const std::size_t v_id=(i+j);
									vertices[j].x=figure.vertices[v_id*3+0];
									vertices[j].y=figure.vertices[v_id*3+1];
									vertices[j].z=figure.vertices[v_id*3+2];
									normals[j].x=figure.normals[v_id*3+0];
									normals[j].y=figure.normals[v_id*3+1];
									normals[j].z=figure.normals[v_id*3+2];
								}
								opengl_printer.add_triangle_strip(vertices, normals);
							}
						}
					}
				}
			}

			OutputSelector output_selector(file);

			{
				std::ostream& output=output_selector.stream();
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(cgo_name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object;
				info.value("file")=file;
				if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
				{
					info.value("dump")=output_selector.str();
				}
			}
		}
	};

	class calculate_betweenness : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query("", "[--min-seq-sep 1]", cargs.input);
			const bool inter_residue=!cargs.input.get_flag("not-inter-residue");
			const bool uniform=cargs.input.get_flag("uniform");
			const bool normalize=!cargs.input.get_flag("not-normalize");
			const std::string adjunct_atoms=cargs.input.get_value_or_default<std::string>("adj-atoms", "betweenness");
			const std::string adjunct_contacts=cargs.input.get_value_or_default<std::string>("adj-contacts", "betweenness");

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_atoms, true);
			assert_adjunct_name_input(adjunct_contacts, true);

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			typedef common::ConstructionOfPathCentralities COPC;

			std::map<COPC::CRADsPair, double> map_of_contacts;
			std::map< COPC::CRADsPair, std::set<std::size_t> > map_of_contacts_ids;
			std::map< COPC::CRAD, std::set<std::size_t> > map_of_atoms_ids;

			{
				const std::vector<Atom>& atoms=cargs.data_manager.atoms();
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=cargs.data_manager.contacts()[*it];
					if(!contact.solvent())
					{
						COPC::CRADsPair crads=
								COPC::CRADsPair(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad).without_some_info(inter_residue, inter_residue, false, false);
						map_of_contacts[crads]+=contact.value.area;
						map_of_contacts_ids[crads].insert(*it);
						map_of_atoms_ids[atoms[contact.ids[0]].crad.without_some_info(inter_residue, inter_residue, false, false)].insert(contact.ids[0]);
						map_of_atoms_ids[atoms[contact.ids[1]].crad.without_some_info(inter_residue, inter_residue, false, false)].insert(contact.ids[1]);
					}
				}
			}

			for(std::map<COPC::CRADsPair, double>::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
			{
				it->second=(uniform ? 1.0 : (100.0/it->second));
			}

			const COPC::Graph graph=COPC::init_graph(map_of_contacts);

			COPC::BetweennessCentralitiesResult result=COPC::calculate_betweenness_centralities(graph, 0.0, normalize);

			if(!adjunct_atoms.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
				{
					cargs.data_manager.atom_adjuncts_mutable(i).erase(adjunct_atoms);
				}

				for(COPC::ID id=0;id<graph.vertices.size();id++)
				{
					const std::set<std::size_t>& atom_ids=map_of_atoms_ids[graph.vertices[id].crad];
					for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
					{
						cargs.data_manager.atom_adjuncts_mutable(*it)[adjunct_atoms]=result.vertex_centralities[id];
					}
				}
			}

			if(!adjunct_contacts.empty())
			{
				for(std::size_t i=0;i<cargs.data_manager.contacts().size();i++)
				{
					cargs.data_manager.contact_adjuncts_mutable(i).erase(adjunct_contacts);
				}

				for(std::size_t i=0;i<graph.edges.size();i++)
				{
					const COPC::Edge& edge=graph.edges[i];
					const COPC::ID id1=edge.vertex_ids[0];
					const COPC::ID id2=edge.vertex_ids[1];
					const COPC::CRADsPair crads(graph.vertices[id1].crad, graph.vertices[id2].crad);
					const std::set<std::size_t>& contact_ids=map_of_contacts_ids[crads];
					for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
					{
						cargs.data_manager.contact_adjuncts_mutable(*it)[adjunct_contacts]=result.edge_centralities[COPC::ordered_pair_of_ids(id1, id2)];
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("contacts_summary"));
		}
	};

	class set_text_description : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string text=cargs.input.get_value_or_first_unused_unnamed_value("text");

			cargs.input.assert_nothing_unusable();

			cargs.data_manager.set_text_description(text);

			cargs.heterostorage.variant_object.value("text_description")=cargs.data_manager.text_description();
		}
	};

	class print_text_description : public GenericCommandForDataManagerScaled
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();

			cargs.heterostorage.variant_object.value("text_description")=cargs.data_manager.text_description();
		}
	};

private:
	static SelectionManager::Query read_generic_selecting_query(const std::string& prefix, const std::string& default_expression, CommandInput& input)
	{
		const std::string type_for_expression=prefix+"use";
		const std::string type_for_full_residues=prefix+"full-residues";
		const std::string type_for_forced_id=prefix+"id";

		SelectionManager::Query query(default_expression);

		if(input.is_option(type_for_expression))
		{
			query.expression_string=input.get_value<std::string>(type_for_expression);
		}
		else if(type_for_expression=="use" && input.is_any_unnamed_value_unused())
		{
			bool found=false;
			for(std::size_t i=0;i<input.get_list_of_unnamed_values().size() && !found;i++)
			{
				if(!input.is_unnamed_value_used(i))
				{
					const std::string& candidate=input.get_list_of_unnamed_values()[i];
					if(!candidate.empty() && candidate.find_first_of("([")==0)
					{
						query.expression_string=candidate;
						input.mark_unnamed_value_as_used(i);
						found=true;
					}
				}
			}
		}

		if(input.is_option(type_for_full_residues))
		{
			query.full_residues=input.get_flag(type_for_full_residues);
		}

		{
			const std::vector<std::size_t> forced_ids_vector=input.get_value_vector_or_default<std::size_t>(type_for_forced_id, std::vector<std::size_t>());
			if(!forced_ids_vector.empty())
			{
				query.from_ids.insert(forced_ids_vector.begin(), forced_ids_vector.end());
			}
		}

		return query;
	}

	static SelectionManager::Query read_generic_selecting_query(CommandInput& input)
	{
		return read_generic_selecting_query("", "[]", input);
	}

	static void document_generic_selecting_query(const std::string& prefix, const std::string& default_expression, CommandDocumentation& doc)
	{
		doc.set_option_decription(prefix+"use", "string", "selection expression, default is "+default_expression);
		doc.set_option_decription(prefix+"full-residues", "boolean flag", "flag to select full residues");
		doc.set_option_decription(prefix+"id", "vector of numbers", "one or more indices");
	}

	static void document_generic_selecting_query(CommandDocumentation& doc)
	{
		document_generic_selecting_query("", "[]", doc);
	}

	static auxiliaries::ColorUtilities::ColorInteger read_color(CommandInput& input)
	{
		if(input.is_option("col"))
		{
			return auxiliaries::ColorUtilities::color_from_name(input.get_value<std::string>("col"));
		}
		else if(input.is_any_unnamed_value_unused())
		{
			for(std::size_t i=0;i<input.get_list_of_unnamed_values().size();i++)
			{
				if(!input.is_unnamed_value_used(i))
				{
					const std::string& candidate_str=input.get_list_of_unnamed_values()[i];
					if(candidate_str.size()>2 && candidate_str.rfind("0x", 0)==0)
					{
						auxiliaries::ColorUtilities::ColorInteger candidate_color=auxiliaries::ColorUtilities::color_from_name(candidate_str);
						if(candidate_color!=auxiliaries::ColorUtilities::null_color())
						{
							input.mark_unnamed_value_as_used(i);
							return candidate_color;
						}
					}
				}
			}
		}
		return auxiliaries::ColorUtilities::null_color();
	}

	static auxiliaries::ColorUtilities::ColorInteger get_next_random_color()
	{
		const unsigned long m=2147483647L;
		const unsigned long a=48271L;
		const unsigned long c=0L;

		static unsigned long x=111L;

		static double value_prev=-1.0;

		static std::vector<auxiliaries::ColorUtilities::ColorInteger> anchors;

		if(anchors.empty())
		{
			anchors.push_back(0xFF4444);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFF4444);
			anchors.push_back(0x888888);
			anchors.push_back(0xFF4444);

			anchors.push_back(0xFF6644);

			anchors.push_back(0xFF8844);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFF8844);
			anchors.push_back(0x888888);
			anchors.push_back(0xFF8844);

			anchors.push_back(0xFFCC44);

			anchors.push_back(0xFFFF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFFFF44);
			anchors.push_back(0x888888);
			anchors.push_back(0xFFFF44);

			anchors.push_back(0xCCFF44);

			anchors.push_back(0x88FF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x88FF44);
			anchors.push_back(0x888888);
			anchors.push_back(0x88FF44);

			anchors.push_back(0x66FF44);

			anchors.push_back(0x44FF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FF44);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FF44);

			anchors.push_back(0x44FF66);

			anchors.push_back(0x44FF88);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FF88);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FF88);

			anchors.push_back(0x44FFCC);

			anchors.push_back(0x44FFFF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FFFF);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FFFF);

			anchors.push_back(0x44CCFF);

			anchors.push_back(0x4488FF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x4488FF);
			anchors.push_back(0x888888);
			anchors.push_back(0x4488FF);

			anchors.push_back(0x4466FF);

			anchors.push_back(0x4444FF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x4444FF);
			anchors.push_back(0x888888);
			anchors.push_back(0x4444FF);
		}

		x=((a*x+c)%m);

		double value=static_cast<double>(x%100000L)/static_cast<double>(99999L);

		if(value_prev>=0.0 && std::abs(value-value_prev)<0.1)
		{
			value+=((value<value_prev) ? -0.1 : 0.1);
			if(value<0.0)
			{
				value+=1.0;
			}
			if(value>1.0)
			{
				value-=1.0;
			}
		}

		value_prev=value;

		return auxiliaries::ColorUtilities::color_from_gradient(anchors, value);
	}

	static double calculate_zscore_reverse(const double zscore, const double mean_of_values, const double sd_of_values)
	{
		return ((zscore*sd_of_values)+mean_of_values);
	}

	static void calculate_spectrum_info(
			const bool as_z_scores,
			const bool min_val_present,
			const double min_val,
			const bool max_val_present,
			const double max_val,
			double& min_val_actual,
			double& max_val_actual,
			int& num_of_vals,
			double& mean_of_values,
			double& sd_of_values,
			std::map<std::size_t, double>& map_of_ids_values)
	{
		{
			double sum_of_vals=0.0;
			double sum_of_squared_vals=0.0;

			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				const double val=it->second;
				if(it==map_of_ids_values.begin() || min_val_actual>val)
				{
					min_val_actual=val;
				}
				if(it==map_of_ids_values.begin() || max_val_actual<val)
				{
					max_val_actual=val;
				}
				num_of_vals++;
				sum_of_vals+=val;
				sum_of_squared_vals+=val*val;
			}

			mean_of_values=(sum_of_vals/static_cast<double>(num_of_vals));
			sd_of_values=sqrt((sum_of_squared_vals/static_cast<double>(num_of_vals))-(mean_of_values*mean_of_values));
		}

		const double min_val_to_use=(min_val_present ?
				(as_z_scores ? calculate_zscore_reverse(min_val, mean_of_values, sd_of_values) : min_val) :
				min_val_actual);
		const double max_val_to_use=(max_val_present ?
				(as_z_scores ? calculate_zscore_reverse(max_val, mean_of_values, sd_of_values) : max_val) :
				max_val_actual);

		if(max_val_to_use<min_val_to_use)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		for(std::map<std::size_t, double>::iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
		{
			double& val=it->second;
			if(val<=min_val_to_use)
			{
				val=0.0;
			}
			else if(val>=max_val_to_use)
			{
				val=1.0;
			}
			else
			{
				val=(val-min_val_to_use)/(max_val_to_use-min_val_to_use);
			}
		}
	}

	static FilteringOfTriangulation::Query read_filtering_of_triangulation_query(CommandInput& input)
	{
		FilteringOfTriangulation::Query query;
		query.strict=input.get_flag("strict");
		query.max_edge=input.get_value_or_default<double>("max-edge", query.max_edge);
		query.min_radius=input.get_value_or_default<double>("min-radius", query.min_radius);
		query.max_radius=input.get_value_or_default<double>("max-radius", query.max_radius);
		query.expansion=input.get_value_or_default<double>("expansion", query.expansion);
		return query;
	}
};

}

#endif /* SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_ */
