#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include "../../auxiliaries/color_utilities.h"
#include "../../auxiliaries/residue_letters_coding.h"

#include "../writing_atomic_balls_in_pdb_format.h"
#include "../construction_of_structural_cartoon.h"
#include "../conversion_of_descriptors.h"

#include "generic_command_for_data_manager.h"
#include "basic_assertions.h"
#include "scoring_of_data_manager_using_voromqa.h"
#include "variant_serialization.h"

namespace common
{

namespace scripting
{

class CustomCommandsForDataManager
{
public:
	class restrict_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
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

			VariantObject& info=cargs.heterostorage.variant_object.object("restricted_atoms");

			if(ids.size()<cargs.data_manager.atoms().size())
			{
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("old_summary"));

				cargs.data_manager.restrict_atoms(ids);
				cargs.change_indicator.changed_atoms=true;
			}
			else
			{
				info.value("old_summary").set_null();
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("summary"));
		}
	};

	class move_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
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
			if(ids.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms selected."));
			}

			cargs.data_manager.transform_coordinates_of_atoms(ids, pre_translation_vector, rotation_matrix, rotation_axis_and_angle, post_translation_vector);

			cargs.change_indicator.changed_atoms=true;
		}
	};

	class center_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_atoms=true;
		}
	};

	class save_atoms : public GenericCommandForDataManager
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

			std::ofstream output(file.c_str(), std::ios::out);
			assert_io_stream(file, output);

			const std::vector<Atom> atoms=slice_vector_by_ids(cargs.data_manager.atoms(), ids);

			if(as_pdb)
			{
				WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms, pdb_b_factor_name, pdb_ter, output);
			}
			else
			{
				auxiliaries::IOUtilities().write_set(atoms, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("saved_atoms");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfAtoms(atoms), info.object("summary"));
			}
		}
	};

	class select_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool no_marking=cargs.input.get_flag("no-marking");

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
				VariantObject& info=cargs.heterostorage.variant_object.object("selected_atoms");
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}

			if(!no_marking)
			{
				{
					CommandParametersForGenericViewing params;
					params.unmark=true;
					if(params.apply_to_display_states(cargs.data_manager.atoms_display_states_mutable()))
					{
						cargs.change_indicator.changed_atoms_display_states=true;
					}
				}
				{
					CommandParametersForGenericViewing params;
					params.mark=true;
					if(params.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
					{
						cargs.change_indicator.changed_atoms_display_states=true;
					}
				}
			}

			cargs.heterostorage.vectors_of_ids["selection_of_atoms"]=std::vector<std::size_t>(ids.begin(), ids.end());
		}
	};

	class tag_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_atoms_tags=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[*it];
				atom.value.props.tags.insert(tag);
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class tag_atoms_by_secondary_structure : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string tag_for_alpha=cargs.input.get_value_or_default<std::string>("tag-for-alpha", "ss=H");
			const std::string tag_for_beta=cargs.input.get_value_or_default<std::string>("tag-for-beta", "ss=S");

			cargs.input.assert_nothing_unusable();

			cargs.change_indicator.changed_atoms_tags=true;

			for(std::size_t i=0;i<cargs.data_manager.atoms().size();i++)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[i];
				atom.value.props.tags.erase(tag_for_alpha);
				atom.value.props.tags.erase(tag_for_beta);
			}

			for(std::size_t residue_id=0;residue_id<cargs.data_manager.secondary_structure_info().residue_descriptors.size();residue_id++)
			{
				const ConstructionOfSecondaryStructure::ResidueDescriptor& residue_descriptor=cargs.data_manager.secondary_structure_info().residue_descriptors[residue_id];
				const std::vector<std::size_t>& atom_ids=cargs.data_manager.primary_structure_info().residues[residue_id].atom_ids;
				for(std::size_t i=0;i<atom_ids.size();i++)
				{
					Atom& atom=cargs.data_manager.atoms_mutable()[atom_ids[i]];
					if(residue_descriptor.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
					{
						atom.value.props.tags.insert(tag_for_alpha);
					}
					else if(residue_descriptor.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
					{
						atom.value.props.tags.insert(tag_for_beta);
					}
				}
			}
		}
	};

	class delete_tags_of_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_atoms_tags=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[*it];
				if(all)
				{
					atom.value.props.tags.clear();
				}
				else
				{
					for(std::size_t i=0;i<tags.size();i++)
					{
						atom.value.props.tags.erase(tags[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class adjunct_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_atoms_adjuncts=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[*it];
				if(remove)
				{
					atom.value.props.adjuncts.erase(name);
				}
				else
				{
					atom.value.props.adjuncts[name]=value;
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class delete_adjuncts_of_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_atoms_adjuncts=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[*it];
				if(all)
				{
					atom.value.props.adjuncts.clear();
				}
				else
				{
					for(std::size_t i=0;i<adjuncts.size();i++)
					{
						atom.value.props.adjuncts.erase(adjuncts[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class mark_atoms : public GenericCommandForDataManager
	{
	public:
		mark_atoms() : positive_(true)
		{
		}

		explicit mark_atoms(const bool positive) : positive_(positive)
		{
		}

		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			{
				CommandParametersForGenericViewing parameters_for_viewing;
				parameters_for_viewing.mark=positive_;
				parameters_for_viewing.unmark=!positive_;
				if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
				{
					cargs.change_indicator.changed_atoms_display_states=true;
				}
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
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

	class show_atoms : public GenericCommandForDataManager
	{
	public:
		show_atoms() : positive_(true)
		{
		}

		explicit show_atoms(const bool positive) : positive_(positive)
		{
		}

		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
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

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=representation_ids;
			parameters_for_viewing.show=positive_;
			parameters_for_viewing.hide=!positive_;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
			{
				cargs.change_indicator.changed_atoms_display_states=true;
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
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

	class color_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const auxiliaries::ColorUtilities::ColorInteger color_value=read_color(cargs.input);

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

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=representation_ids;
			parameters_for_viewing.color=color_value;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
			{
				cargs.change_indicator.changed_atoms_display_states=true;
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class spectrum_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
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

			if(!auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
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
				std::map<ChainResidueAtomDescriptor, double> residue_ids_to_values;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					residue_ids_to_values[cargs.data_manager.atoms()[*it].crad.without_atom()]=0.5;
				}
				if(residue_ids_to_values.size()>1)
				{
					int i=0;
					for(std::map<ChainResidueAtomDescriptor, double>::iterator it=residue_ids_to_values.begin();it!=residue_ids_to_values.end();++it)
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

			{
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
				}

				const double min_val_to_use=(min_val_present ? min_val : min_val_actual);
				const double max_val_to_use=(max_val_present ? max_val : max_val_actual);

				if(max_val_to_use<=min_val_to_use)
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

			if(!only_summarize)
			{
				CommandParametersForGenericViewing parameters_for_viewing;
				parameters_for_viewing.visual_ids_=representation_ids;
				parameters_for_viewing.assert_state();

				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					parameters_for_viewing.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
					if(parameters_for_viewing.apply_to_display_state(it->first, cargs.data_manager.atoms_display_states_mutable()))
					{
						cargs.change_indicator.changed_atoms_display_states=true;
					}
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("spectrum_summary");
				info.value("count").set_value_int(ids.size());
				info.value("min_value")=min_val_actual;
				info.value("max_value")=max_val_actual;
			}
		}
	};


	class print_atoms : public GenericCommandForDataManager
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
			}

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));
		}
	};

	class print_sequence : public GenericCommandForDataManager
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

			VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.heterostorage.variant_object.object("summary_of_atoms"));

			std::set<std::size_t> residue_ids;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				residue_ids.insert(cargs.data_manager.primary_structure_info().map_of_atoms_to_residues[*it]);
			}

			std::vector< std::vector<std::size_t> > grouping;
			{
				std::map<ChainResidueAtomDescriptor, std::size_t> ordering;
				for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
				{
					const ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[*it];
					ordering[r.chain_residue_descriptor]=(*it);
				}

				std::map<ChainResidueAtomDescriptor, std::size_t>::const_iterator it=ordering.begin();
				while(it!=ordering.end())
				{
					if(it==ordering.begin())
					{
						grouping.push_back(std::vector<std::size_t>(1, it->second));
					}
					else
					{
						std::map<ChainResidueAtomDescriptor, std::size_t>::const_iterator it_prev=it;
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
				const ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[grouping[i].front()];
				chaining[r.chain_residue_descriptor.chainID].push_back(i);
			}

			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=chaining.begin();it!=chaining.end();++it)
			{
				VariantObject chain_info;
				chain_info.value("chain_name")=(it->first);
				const std::vector<std::size_t>& group_ids=it->second;
				chain_info.value("continuous_segments")=group_ids.size();
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
								const ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[group[j]];
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
								const ConstructionOfSecondaryStructure::ResidueDescriptor& r=cargs.data_manager.secondary_structure_info().residue_descriptors[group[j]];
								if(r.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
								{
									output_for_secondary_structure_sequence << "H";
								}
								else if(r.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
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
					chain_info.objects_array("segments").push_back(segment_info);
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

	class write_atoms_as_pymol_cgo : public GenericCommandForDataManager
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

			if(wireframe)
			{
				if(!cargs.data_manager.bonding_links_info().valid(cargs.data_manager.atoms(), cargs.data_manager.primary_structure_info()))
				{
					cargs.data_manager.reset_bonding_links_info_by_creating(ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
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
									const ConstructionOfBondingLinks::DirectedLink& dl=cargs.data_manager.bonding_links_info().bonds_links[link_ids[i]];
									const apollota::SimplePoint pa(cargs.data_manager.atoms()[dl.a].value);
									const apollota::SimplePoint pb(cargs.data_manager.atoms()[dl.b].value);
									opengl_printer.add_line_strip(pa, (pa+pb)*0.5);
								}
							}
							else
							{
								opengl_printer.add_sphere(cargs.data_manager.atoms()[id].value);
							}
						}
					}
				}
			}

			{
				std::ofstream output(file.c_str(), std::ios::out);
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("wrote_atoms_as_pymol_cgo");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("summary"));
			}
		}
	};

	class write_cartoon_as_pymol_cgo : public GenericCommandForDataManager
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
				cargs.data_manager.reset_bonding_links_info_by_creating(ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
			}

			ConstructionOfStructuralCartoon::Parameters parameters_for_cartoon;
			ConstructionOfStructuralCartoon::BundleOfMeshInformation bundle_of_cartoon_mesh;
			if(!ConstructionOfStructuralCartoon::construct_bundle_of_mesh_information(
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

			{
				std::ofstream output(file.c_str(), std::ios::out);
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("wrote_cartoon_as_pymol_cgo");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), ids), info.object("summary"));
			}
		}
	};

	class list_selections_of_atoms : public GenericCommandForDataManager
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
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms(), it->second), info.object("summary"));
				cargs.heterostorage.variant_object.objects_array("selections").push_back(info);
			}
		}
	};

	class delete_selections_of_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_selections_availability();

			const bool all=cargs.input.get_flag("all");

			if(all)
			{
				cargs.input.assert_nothing_unusable();
				cargs.heterostorage.variant_object.value("deleted_all_selections").set_value_int(
						cargs.data_manager.selection_manager().map_of_atoms_selections().size());
				cargs.data_manager.selection_manager().delete_atoms_selections();
			}
			else
			{

				const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
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
		}
	};

	class rename_selection_of_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

	class construct_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
			parameters_to_construct_contacts.probe=cargs.input.get_value_or_default<double>("probe", parameters_to_construct_contacts.probe);
			parameters_to_construct_contacts.calculate_volumes=cargs.input.get_flag("calculate-volumes");
			parameters_to_construct_contacts.step=cargs.input.get_value_or_default<double>("step", parameters_to_construct_contacts.step);
			parameters_to_construct_contacts.projections=cargs.input.get_value_or_default<int>("projections", parameters_to_construct_contacts.projections);
			parameters_to_construct_contacts.sih_depth=cargs.input.get_value_or_default<int>("sih-depth", parameters_to_construct_contacts.sih_depth);
			ConstructionOfContacts::ParametersToEnhanceContacts parameters_to_enhance_contacts;
			parameters_to_enhance_contacts.probe=parameters_to_construct_contacts.probe;
			parameters_to_enhance_contacts.step=cargs.input.get_value_or_default<double>("visual-step", parameters_to_enhance_contacts.step);
			parameters_to_enhance_contacts.projections=cargs.input.get_value_or_default<int>("visual-projections", parameters_to_enhance_contacts.projections);
			parameters_to_enhance_contacts.simplify=cargs.input.get_flag("visual-simplify");
			parameters_to_enhance_contacts.sih_depth=cargs.input.get_value_or_default<int>("visual-sih-depth", parameters_to_enhance_contacts.sih_depth);
			parameters_to_enhance_contacts.tag_centrality=!cargs.input.get_flag("no-tag-centrality");
			parameters_to_enhance_contacts.tag_peripherial=cargs.input.get_flag("tag-peripherial");
			const SelectionManager::Query render_parameters_for_selecting=read_generic_selecting_query("render-", "{--min-seq-sep 1}", cargs.input);
			const bool render=(cargs.input.get_flag("render-default") || render_parameters_for_selecting.altered);

			cargs.input.assert_nothing_unusable();

			ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
			parameters_to_construct_triangulation.artificial_boundary_shift=std::max(parameters_to_construct_contacts.probe*2.0, 5.0);

			const std::vector<apollota::SimpleSphere> atomic_balls=ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(cargs.data_manager.atoms());

			bool using_cached_triangulation=false;

			if(!cargs.data_manager.triangulation_info().equivalent(parameters_to_construct_triangulation, atomic_balls))
			{
				ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;
				if(ConstructionOfTriangulation::construct_bundle_of_triangulation_information(parameters_to_construct_triangulation, atomic_balls, bundle_of_triangulation_information))
				{
					cargs.data_manager.reset_triangulation_info_by_swapping(bundle_of_triangulation_information);
					cargs.change_indicator.changed_contacts=true;
				}
				else
				{
					throw std::runtime_error(std::string("Failed to construct triangulation."));
				}
			}
			else
			{
				using_cached_triangulation=true;
			}

			ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

			if(ConstructionOfContacts::construct_bundle_of_contact_information(parameters_to_construct_contacts, cargs.data_manager.triangulation_info(), bundle_of_contact_information))
			{
				cargs.data_manager.reset_contacts_by_swapping(bundle_of_contact_information.contacts);
				cargs.change_indicator.changed_contacts=true;

				if(parameters_to_construct_contacts.calculate_volumes)
				{
					for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<cargs.data_manager.atoms().size();i++)
					{
						cargs.data_manager.atoms_mutable()[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
					}
				}

				std::set<std::size_t> draw_ids;
				if(render)
				{
					draw_ids=cargs.data_manager.selection_manager().select_contacts(render_parameters_for_selecting);
				}

				ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, cargs.data_manager.triangulation_info(), draw_ids, cargs.data_manager.contacts_mutable());

				cargs.data_manager.reset_contacts_display_states();

				VariantObject& info=cargs.heterostorage.variant_object.object("constructed_contacts");
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("summary"));
				info.value("used_cached_triangulation")=using_cached_triangulation;
			}
			else
			{
				throw std::runtime_error(std::string("Failed to construct contacts."));
			}
		}
	};

	class save_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			assert_file_name_input(file, false);
			const bool no_graphics=cargs.input.get_flag("no-graphics");

			cargs.input.assert_nothing_unusable();

			{
				std::ofstream output(file.c_str(), std::ios::out);
				assert_io_stream(file, output);
				enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("saved_contacts");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("summary"));
			}
		}
	};

	class load_contacts : public GenericCommandForDataManager
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

			std::vector<Contact> contacts;

			auxiliaries::IOUtilities().read_file_lines_to_set(file, contacts);

			if(!contacts.empty())
			{
				cargs.data_manager.reset_contacts_by_swapping(contacts);
				cargs.change_indicator.changed_contacts=true;

				VariantObject& info=cargs.heterostorage.variant_object.object("read_contacts");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("summary"));
			}
			else
			{
				throw std::runtime_error(std::string("Failed to read contacts from file '")+file+"'.");
			}
		}
	};

	class select_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool no_marking=cargs.input.get_flag("no-marking");

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
				VariantObject& info=cargs.heterostorage.variant_object.object("selected_contacts");
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("summary"));
				if(name.empty())
				{
					info.value("selection_name").set_null();
				}
				else
				{
					info.value("selection_name")=name;
				}
			}

			if(!no_marking)
			{
				{
					CommandParametersForGenericViewing params;
					params.unmark=true;
					if(params.apply_to_display_states(cargs.data_manager.contacts_display_states_mutable()))
					{
						cargs.change_indicator.changed_contacts_display_states=true;
					}
				}
				{
					CommandParametersForGenericViewing params;
					params.mark=true;
					if(params.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
					{
						cargs.change_indicator.changed_contacts_display_states=true;
					}
				}
			}

			cargs.heterostorage.vectors_of_ids["selection_of_contacts"]=std::vector<std::size_t>(ids.begin(), ids.end());
		}
	};

	class tag_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_contacts_tags=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Contact& contact=cargs.data_manager.contacts_mutable()[*it];
				contact.value.props.tags.insert(tag);
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
		}
	};

	class delete_tags_of_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_contacts_tags=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Contact& contact=cargs.data_manager.contacts_mutable()[*it];
				if(all)
				{
					contact.value.props.tags.clear();
				}
				else
				{
					for(std::size_t i=0;i<tags.size();i++)
					{
						contact.value.props.tags.erase(tags[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
		}
	};

	class adjunct_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_contacts_adjuncts=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Contact& contact=cargs.data_manager.contacts_mutable()[*it];
				if(remove)
				{
					contact.value.props.adjuncts.erase(name);
				}
				else
				{
					contact.value.props.adjuncts[name]=value;
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
		}
	};

	class delete_adjuncts_of_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			cargs.change_indicator.changed_contacts_adjuncts=true;

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Contact& contact=cargs.data_manager.contacts_mutable()[*it];
				if(all)
				{
					contact.value.props.adjuncts.clear();
				}
				else
				{
					for(std::size_t i=0;i<adjuncts.size();i++)
					{
						contact.value.props.adjuncts.erase(adjuncts[i]);
					}
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
		}
	};


	class mark_contacts : public GenericCommandForDataManager
	{
	public:
		mark_contacts() : positive_(true)
		{
		}

		explicit mark_contacts(const bool positive) : positive_(positive)
		{
		}

		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			{
				CommandParametersForGenericViewing parameters_for_viewing;
				parameters_for_viewing.mark=positive_;
				parameters_for_viewing.unmark=!positive_;
				if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
				{
					cargs.change_indicator.changed_contacts_display_states=true;
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
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

	class show_contacts : public GenericCommandForDataManager
	{
	public:
		show_contacts() : positive_(true)
		{
		}

		explicit show_contacts(const bool positive) : positive_(positive)
		{
		}

		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
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

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=representation_ids;
			parameters_for_viewing.show=positive_;
			parameters_for_viewing.hide=!positive_;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
			{
				cargs.change_indicator.changed_contacts_display_states=true;
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
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

	class color_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			const SelectionManager::Query parameters_for_selecting=read_generic_selecting_query(cargs.input);
			const std::vector<std::string> representation_names=cargs.input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
			const auxiliaries::ColorUtilities::ColorInteger color_value=read_color(cargs.input);

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

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=representation_ids;
			parameters_for_viewing.color=color_value;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
			{
				cargs.change_indicator.changed_contacts_display_states=true;
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
		}
	};

	class spectrum_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
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

			{
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
				}

				const double min_val_to_use=(min_val_present ? min_val : min_val_actual);
				const double max_val_to_use=(max_val_present ? max_val : max_val_actual);

				if(max_val_to_use<=min_val_to_use)
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

			if(!only_summarize)
			{
				CommandParametersForGenericViewing parameters_for_viewing;
				parameters_for_viewing.visual_ids_=representation_ids;
				parameters_for_viewing.assert_state();

				for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
				{
					parameters_for_viewing.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
					if(parameters_for_viewing.apply_to_display_state(it->first, cargs.data_manager.contacts_display_states_mutable()))
					{
						cargs.change_indicator.changed_contacts_display_states=true;
					}
				}
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("spectrum_summary");
				info.value("count").set_value_int(ids.size());
				info.value("min_value")=min_val_actual;
				info.value("max_value")=max_val_actual;
			}
		}
	};

	class print_contacts : public GenericCommandForDataManager
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
				std::map<ChainResidueAtomDescriptorsPair, ContactValue> map_for_output;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=cargs.data_manager.contacts()[*it];
					if(contact.solvent())
					{
						map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), ChainResidueAtomDescriptor::solvent())].add(contact.value);
					}
					else
					{
						map_for_output[ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad.without_atom(), atoms[contact.ids[1]].crad.without_atom())].add(contact.value);
					}
				}
				contacts.reserve(map_for_output.size());
				for(std::map<ChainResidueAtomDescriptorsPair, ContactValue>::const_iterator it=map_for_output.begin();it!=map_for_output.end();++it)
				{
					contacts.push_back(VariantObject());
					VariantSerialization::write(it->first, it->second, contacts.back());
				}
			}
			else
			{
				contacts.reserve(ids.size());
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					contacts.push_back(VariantObject());
					VariantSerialization::write(atoms, cargs.data_manager.contacts()[*it], contacts.back());
				}
			}

			VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.heterostorage.variant_object.object("summary_of_contacts"));
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

	class write_contacts_as_pymol_cgo : public GenericCommandForDataManager
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

			{
				std::ofstream output(file.c_str(), std::ios::out);
				assert_io_stream(file, output);
				opengl_printer.print_pymol_script(name, true, output);
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("wrote_contacts_as_pymol_cgo");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), ids), info.object("summary"));
			}
		}
	};

	class list_selections_of_contacts : public GenericCommandForDataManager
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
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts(), it->second), info.object("summary"));
				cargs.heterostorage.variant_object.objects_array("selections").push_back(info);
			}
		}
	};

	class delete_selections_of_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_selections_availability();

			const bool all=cargs.input.get_flag("all");

			if(all)
			{
				cargs.input.assert_nothing_unusable();
				cargs.heterostorage.variant_object.value("deleted_all_selections").set_value_int(
						cargs.data_manager.selection_manager().map_of_contacts_selections().size());
				cargs.data_manager.selection_manager().delete_contacts_selections();
			}
			else
			{
				const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
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
		}
	};

	class rename_selection_of_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

	class save_atoms_and_contacts : public GenericCommandForDataManager
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

			{
				std::ofstream output(file.c_str(), std::ios::out);
				assert_io_stream(file, output);
				auxiliaries::IOUtilities().write_set(cargs.data_manager.atoms(), output);
				output << "_end_atoms\n";
				enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
				output << "_end_contacts\n";
			}

			{
				VariantObject& info=cargs.heterostorage.variant_object.object("saved_atoms_and_contacts");
				info.value("file")=file;
				VariantSerialization::write(SummaryOfAtoms(cargs.data_manager.atoms()), info.object("summary_of_atoms"));
				VariantSerialization::write(SummaryOfContacts(cargs.data_manager.contacts()), info.object("summary_of_contacts"));
			}
		}
	};

	class calculate_burial_depth : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string name=cargs.input.get_value<std::string>("name");
			const int min_seq_sep=cargs.input.get_value_or_default<int>("min-seq-sep", 1);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(name, false);

			std::set<ChainResidueAtomDescriptorsPair> set_of_contacts;
			for(std::size_t i=0;i<cargs.data_manager.contacts().size();i++)
			{
				const ChainResidueAtomDescriptorsPair crads=ConversionOfDescriptors::get_contact_descriptor(cargs.data_manager.atoms(), cargs.data_manager.contacts()[i]);
				if(ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, min_seq_sep, ChainResidueAtomDescriptor::null_num(), true))
				{
					set_of_contacts.insert(crads);
				}
			}

			const std::map<ChainResidueAtomDescriptor, int> map_crad_to_depth=ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_contacts);

			cargs.change_indicator.changed_atoms_adjuncts=true;

			for(std::size_t i=0;i<cargs.data_manager.atoms_mutable().size();i++)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[i];
				atom.value.props.adjuncts.erase(name);
				std::map<ChainResidueAtomDescriptor, int>::const_iterator it=map_crad_to_depth.find(atom.crad);
				if(it!=map_crad_to_depth.end())
				{
					atom.value.props.adjuncts[name]=it->second;
				}
			}
		}
	};

	class voromqa_global : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

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

			VariantObject& info=cargs.heterostorage.variant_object.object("voromqa_result");
			info.value("quality_score")=result.global_quality_score;
			info.value("atoms_count")=result.bundle_of_quality.atom_quality_scores.size();
			info.value("residues_count")=result.bundle_of_quality.raw_residue_quality_scores.size();
			info.value("contacts_count")=result.bundle_of_energy.global_energy_descriptor.contacts_count;
			info.value("pseudo_energy")=result.bundle_of_energy.global_energy_descriptor.energy;
			info.value("total_area")=result.bundle_of_energy.global_energy_descriptor.total_area;
			info.value("strange_area")=result.bundle_of_energy.global_energy_descriptor.strange_area;
		}
	};

	class voromqa_local : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			const std::string selection_expresion_for_atoms=cargs.input.get_value_or_default<std::string>("atoms", "{}");
			const bool provided_selection_expresion_for_contacts=cargs.input.is_option("contacts");
			const std::string selection_expresion_for_contacts=cargs.input.get_value_or_default<std::string>("contacts", "{}");
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
						SelectionManager::Query(atom_ids, (std::string("{")+"--adjuncts "+adjunct_atom_depth_weights+"&"+adjunct_atom_quality_scores+"}"), false));

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

				VariantObject& info=cargs.heterostorage.variant_object.object("voromqa_for_atoms");
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
						SelectionManager::Query(contact_ids, (std::string("{")+"--adjuncts "+adjunct_inter_atom_energy_scores_raw+"}"), false));

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

				VariantObject& info=cargs.heterostorage.variant_object.object("voromqa_for_contacts");
				info.value("contacts_selected").set_value_int(contact_ids.size());
				info.value("contacts_relevant").set_value_int(contact_ids_with_adjuncts.size());
				info.value("area")=sum_of_areas;
				info.value("energy")=sum_of_energies;
			}
		}
	};

	class voromqa_frustration : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			const std::string adjunct_contact_energy=cargs.input.get_value_or_default<std::string>("adj-contact-energy", "voromqa_energy");
			const std::string adjunct_contact_frustration_area=cargs.input.get_value_or_default<std::string>("adj-contact-frustration-area", "");
			const std::string adjunct_contact_frustration_energy=cargs.input.get_value_or_default<std::string>("adj-contact-frustration-energy", "");
			const std::string adjunct_contact_frustration_energy_mean=cargs.input.get_value_or_default<std::string>("adj-contact-frustration-energy-mean", "frustration_energy_mean");
			const std::string adjunct_atom_frustration_area=cargs.input.get_value_or_default<std::string>("adj-atom-frustration-area", "");
			const std::string adjunct_atom_frustration_energy=cargs.input.get_value_or_default<std::string>("adj-atom-frustration-energy", "");
			const std::string adjunct_atom_frustration_energy_mean=cargs.input.get_value_or_default<std::string>("adj-atom-frustration-energy-mean", "frustration_energy_mean");
			const unsigned int depth=cargs.input.get_value_or_default<unsigned int>("depth", 2);

			cargs.input.assert_nothing_unusable();

			assert_adjunct_name_input(adjunct_contact_energy, false);
			assert_adjunct_name_input(adjunct_contact_frustration_area, true);
			assert_adjunct_name_input(adjunct_contact_frustration_energy, true);
			assert_adjunct_name_input(adjunct_contact_frustration_energy_mean, true);
			assert_adjunct_name_input(adjunct_atom_frustration_area, true);
			assert_adjunct_name_input(adjunct_atom_frustration_energy, true);
			assert_adjunct_name_input(adjunct_atom_frustration_energy_mean, true);

			const std::set<std::size_t> solvent_contact_ids=cargs.data_manager.selection_manager().select_contacts(
					SelectionManager::Query(std::string("{--solvent --adjuncts ")+adjunct_contact_energy+"}", false));

			if(solvent_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No solvent contacts with energy values."));
			}

			const std::set<std::size_t> exterior_atom_ids=cargs.data_manager.selection_manager().select_atoms_by_contacts(solvent_contact_ids, false);

			const std::set<std::size_t> exterior_contact_ids=cargs.data_manager.selection_manager().select_contacts_by_atoms_and_atoms(exterior_atom_ids, exterior_atom_ids, false);

			if(exterior_contact_ids.empty())
			{
				throw std::runtime_error(std::string("No contacts between exterior atoms."));
			}

			std::vector<std::size_t> atom_solvent_contact_ids(cargs.data_manager.atoms().size(), 0);
			for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
			{
				atom_solvent_contact_ids[cargs.data_manager.contacts()[*it].ids[0]]=(*it);
			}

			std::vector< std::set<std::size_t> > graph(cargs.data_manager.atoms().size());
			for(std::set<std::size_t>::const_iterator it=exterior_contact_ids.begin();it!=exterior_contact_ids.end();++it)
			{
				const Contact& contact=cargs.data_manager.contacts()[*it];
				graph[contact.ids[0]].insert(contact.ids[1]);
				graph[contact.ids[1]].insert(contact.ids[0]);
			}

			for(std::set<std::size_t>::const_iterator it=exterior_atom_ids.begin();it!=exterior_atom_ids.end();++it)
			{
				const std::size_t central_id=(*it);

				std::map<std::size_t, bool> neighbors;
				neighbors[central_id]=false;

				for(unsigned int level=0;level<depth;level++)
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
					const Contact& contact=cargs.data_manager.contacts()[atom_solvent_contact_ids[jt->first]];
					sum_of_areas+=contact.value.area;
					sum_of_energies+=contact.value.props.adjuncts.find(adjunct_contact_energy)->second;
				}

				const double energy_mean=(sum_of_areas>0.0 ? (sum_of_energies/sum_of_areas) : 0.0);

				if(!adjunct_contact_frustration_area.empty() || !adjunct_contact_frustration_energy.empty() || !adjunct_contact_frustration_energy_mean.empty())
				{
					Contact& contact=cargs.data_manager.contacts_mutable()[atom_solvent_contact_ids[central_id]];
					if(!adjunct_contact_frustration_area.empty())
					{
						contact.value.props.adjuncts[adjunct_contact_frustration_area]=sum_of_areas;
					}
					if(!adjunct_contact_frustration_energy.empty())
					{
						contact.value.props.adjuncts[adjunct_contact_frustration_energy]=sum_of_energies;
					}
					if(!adjunct_contact_frustration_energy_mean.empty())
					{
						contact.value.props.adjuncts[adjunct_contact_frustration_energy_mean]=energy_mean;
					}
				}

				if(!adjunct_atom_frustration_area.empty() || !adjunct_atom_frustration_energy.empty() || !adjunct_atom_frustration_energy_mean.empty())
				{
					Atom& atom=cargs.data_manager.atoms_mutable()[central_id];
					if(!adjunct_atom_frustration_area.empty())
					{
						atom.value.props.adjuncts[adjunct_atom_frustration_area]=sum_of_areas;
					}
					if(!adjunct_atom_frustration_energy.empty())
					{
						atom.value.props.adjuncts[adjunct_atom_frustration_energy]=sum_of_energies;
					}
					if(!adjunct_atom_frustration_energy_mean.empty())
					{
						atom.value.props.adjuncts[adjunct_atom_frustration_energy_mean]=energy_mean;
					}
				}
			}
		}
	};

private:
	class CommandParametersForGenericViewing
	{
	public:
		bool mark;
		bool unmark;
		bool show;
		bool hide;
		unsigned int color;
		std::set<std::size_t> visual_ids_;

		CommandParametersForGenericViewing() :
			mark(false),
			unmark(false),
			show(false),
			hide(false),
			color(auxiliaries::ColorUtilities::null_color())
		{
		}

		bool color_valid() const
		{
			return auxiliaries::ColorUtilities::color_valid(color);
		}

		void assert_state() const
		{
			if(hide && show)
			{
				throw std::runtime_error(std::string("Cannot show and hide at the same time."));
			}

			if(mark && unmark)
			{
				throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
			}
		}

		bool apply_to_display_state(const std::size_t id, std::vector<DataManager::DisplayState>& display_states) const
		{
			bool updated=false;
			if((show || hide || mark || unmark || color_valid()) && id<display_states.size())
			{
				DataManager::DisplayState& ds=display_states[id];
				if(ds.implemented())
				{
					if(mark || unmark)
					{
						updated=(updated || (ds.marked!=mark));
						ds.marked=mark;
					}

					if(show || hide || color_valid())
					{
						if(visual_ids_.empty())
						{
							for(std::size_t i=0;i<ds.visuals.size();i++)
							{
								if(apply_to_display_state_visual(ds.visuals[i]))
								{
									updated=true;
								}
							}
						}
						else
						{
							for(std::set<std::size_t>::const_iterator jt=visual_ids_.begin();jt!=visual_ids_.end();++jt)
							{
								const std::size_t visual_id=(*jt);
								if(visual_id<ds.visuals.size())
								{
									if(apply_to_display_state_visual(ds.visuals[visual_id]))
									{
										updated=true;
									}
								}
							}
						}
					}
				}
			}
			return updated;
		}

		bool apply_to_display_states(const std::set<std::size_t>& ids, std::vector<DataManager::DisplayState>& display_states) const
		{
			bool updated=false;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					if(apply_to_display_state((*it), display_states))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

		bool apply_to_display_states(std::vector<DataManager::DisplayState>& display_states) const
		{
			bool updated=false;
			if(show || hide || mark || unmark || color_valid())
			{
				for(std::size_t i=0;i<display_states.size();i++)
				{
					if(apply_to_display_state(i, display_states))
					{
						updated=true;
					}
				}
			}
			return updated;
		}

		bool apply_to_display_state_visual(DataManager::DisplayState::Visual& visual) const
		{
			bool updated=false;

			if(visual.implemented)
			{
				if(show || hide)
				{
					updated=(updated || (visual.visible!=show));
					visual.visible=show;
				}

				if(color_valid())
				{
					updated=(updated || (visual.color!=color));
					visual.color=color;
				}
			}

			return updated;
		}
	};

	static SelectionManager::Query read_generic_selecting_query(const std::string& prefix, const std::string& default_expression, CommandInput& input)
	{
		const std::string type_for_expression=prefix+"use";
		const std::string type_for_full_residues=prefix+"full-residues";
		const std::string type_for_forced_id=prefix+"id";

		SelectionManager::Query query(default_expression);

		if(input.is_option(type_for_expression))
		{
			query.expression_string=input.get_value<std::string>(type_for_expression);
			query.altered=true;
		}
		else if(type_for_expression=="use" && input.is_any_unnamed_value_unused())
		{
			bool found=false;
			for(std::size_t i=0;i<input.get_list_of_unnamed_values().size() && !found;i++)
			{
				if(!input.is_unnamed_value_used(i))
				{
					const std::string& candidate=input.get_list_of_unnamed_values()[i];
					if(!candidate.empty() && candidate.find_first_of("({")==0)
					{
						query.expression_string=candidate;
						query.altered=true;
						input.mark_unnamed_value_as_used(i);
						found=true;
					}
				}
			}
		}

		if(input.is_option(type_for_full_residues))
		{
			query.full_residues=input.get_flag(type_for_full_residues);
			query.altered=true;
		}

		{
			const std::vector<std::size_t> forced_ids_vector=input.get_value_vector_or_default<std::size_t>(type_for_forced_id, std::vector<std::size_t>());
			if(!forced_ids_vector.empty())
			{
				query.from_ids.insert(forced_ids_vector.begin(), forced_ids_vector.end());
				query.altered=true;
			}
		}

		return query;
	}

	static SelectionManager::Query read_generic_selecting_query(CommandInput& input)
	{
		return read_generic_selecting_query("", "{}", input);
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

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_ */
