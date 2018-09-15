#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include "../../auxiliaries/color_utilities.h"
#include "../../auxiliaries/residue_letters_coding.h"

#include "../writing_atomic_balls_in_pdb_format.h"
#include "../construction_of_structural_cartoon.h"

#include "generic_command_for_data_manager.h"
#include "table_printing.h"

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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms selected."));
			}

			if(ids.size()<cargs.data_manager.atoms().size())
			{
				const SummaryOfAtoms old_summary=SummaryOfAtoms(cargs.data_manager.atoms());

				cargs.data_manager.restrict_atoms(ids);
				cargs.change_indicator.changed_atoms=true;

				cargs.output_for_log << "Restricted atoms from ";
				old_summary.print(cargs.output_for_log);
				cargs.output_for_log << " to ";
				SummaryOfAtoms(cargs.data_manager.atoms()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
			else
			{
				cargs.output_for_log << "No need to restrict because all atoms were selected ";
				SummaryOfAtoms(cargs.data_manager.atoms()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class save_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(true, cargs.input);
			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			const bool as_pdb=cargs.input.get_flag("as-pdb");
			const std::string pdb_b_factor_name=cargs.input.get_value_or_default<std::string>("pdb-b-factor", "tf");
			const bool pdb_ter=cargs.input.get_flag("pdb-ter");

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

			const std::vector<Atom> atoms=slice_vector_by_ids(cargs.data_manager.atoms(), ids);

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				if(as_pdb)
				{
					WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms, pdb_b_factor_name, pdb_ter, output);
				}
				else
				{
					auxiliaries::IOUtilities().write_set(atoms, output);
				}
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote atoms to file '" << parameters_for_output_destinations.file << "' ";
				SummaryOfAtoms(atoms).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool no_marking=cargs.input.get_flag("no-marking");

			cargs.input.assert_nothing_unusable();

			assert_selection_name_input(name, true);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_atoms_selection(name, ids);
				cargs.output_for_log << "Set selection of atoms named '" << name << "'\n";
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

			cargs.output_set_of_atoms_ids.swap(ids);
		}
	};

	class tag_atoms : public GenericCommandForDataManager
	{
	public:
		tag_atoms() : positive_(true)
		{
		}

		explicit tag_atoms(const bool positive) : positive_(positive)
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

			cargs.input.assert_nothing_unusable();

			assert_tag_input(tag);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Atom& atom=cargs.data_manager.atoms_mutable()[*it];
				if(positive_)
				{
					atom.value.props.tags.insert(tag);
				}
				else
				{
					atom.value.props.tags.erase(tag);
				}
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}

	private:
		bool positive_;
	};

	class untag_atoms : public tag_atoms
	{
	public:
		untag_atoms() : tag_atoms(false)
		{
		}
	};

	class tag_atoms_by_secondary_structure : public GenericCommandForDataManager
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.atoms_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(positive_ && parameters_for_representation_selecting.visual_ids.empty())
			{
				parameters_for_representation_selecting.visual_ids.insert(0);
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
			parameters_for_viewing.show=positive_;
			parameters_for_viewing.hide=!positive_;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
			{
				cargs.change_indicator.changed_atoms_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.atoms_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			CommandParametersForGenericColoring parameters_for_coloring;
			parameters_for_coloring.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(!auxiliaries::ColorUtilities::color_valid(parameters_for_coloring.color))
			{
				throw std::runtime_error(std::string("Atoms color not specified."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable atoms selected."));
			}

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
			parameters_for_viewing.color=parameters_for_coloring.color;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
			{
				cargs.change_indicator.changed_atoms_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.atoms_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
			const std::string by=adjunct.empty() ? cargs.input.get_value_or_default<std::string>("by", "residue-number") : std::string("adjunct");
			const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
			const bool only_summarize=cargs.input.get_flag("only-summarize");

			cargs.input.assert_nothing_unusable();

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
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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
				parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
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
				cargs.output_for_log << "Summary: ";
				cargs.output_for_log << "count=" << ids.size() << " ";
				cargs.output_for_log << "min=" << min_val_actual << " ";
				cargs.output_for_log << "max=" << max_val_actual;
				cargs.output_for_log << "\n";
			}
		}
	};


	class print_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericTablePrinting parameters_for_printing;
			parameters_for_printing.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(true);
			parameters_for_output_destinations.read(false, cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(&cargs.output_for_text);

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				TablePrinting::print_atoms(cargs.data_manager.atoms(), ids, parameters_for_printing.values, output);
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}
	};

	class print_sequence : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(true);
			parameters_for_output_destinations.read(false, cargs.input);
			const unsigned int line_width=cargs.input.get_value_or_default<unsigned int>("line-width", 50);
			const bool secondary_structure=cargs.input.get_flag("secondary-structure");

			cargs.input.assert_nothing_unusable();

			if(line_width<1)
			{
				throw std::runtime_error(std::string("Line width is too small."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(&cargs.output_for_text);

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

			std::ostringstream output;
			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=chaining.begin();it!=chaining.end();++it)
			{
				output << ">Chain '" << (it->first) << "'";
				const std::vector<std::size_t>& group_ids=it->second;
				if(group_ids.size()>1)
				{
					output << " with " << group_ids.size() << " continuous segments";
				}
				output << "\n";
				unsigned int item_counter=0;
				for(std::size_t i=0;i<group_ids.size();i++)
				{
					if(i!=0)
					{
						output << "+";
						if((++item_counter)%line_width==0)
						{
							output << "\n";
						}
					}
					const std::vector<std::size_t>& group=grouping[group_ids[i]];
					for(std::size_t j=0;j<group.size();j++)
					{
						if(secondary_structure)
						{
							const ConstructionOfSecondaryStructure::ResidueDescriptor& r=cargs.data_manager.secondary_structure_info().residue_descriptors[group[j]];
							if(r.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX)
							{
								output << "H";
							}
							else if(r.secondary_structure_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND)
							{
								output << "S";
							}
							else
							{
								output << "-";
							}
						}
						else
						{
							const ConstructionOfPrimaryStructure::Residue& r=cargs.data_manager.primary_structure_info().residues[group[j]];
							output << auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(r.chain_residue_descriptor.resName);
						}
						if((++item_counter)%line_width==0)
						{
							output << "\n";
						}
					}
				}
				output << "\n";
			}

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& suboutput=(*(outputs[i]));
				suboutput << output.str();
			}
		}
	};

	class zoom_by_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No atoms selected."));
			}

			cargs.summary_of_atoms=SummaryOfAtoms(cargs.data_manager.atoms(), ids);
			cargs.extra_values["zoom"]=true;

			cargs.output_for_log << "Bounding box: (" << cargs.summary_of_atoms.bounding_box.p_min << ") (" << cargs.summary_of_atoms.bounding_box.p_max << ")\n";
		}
	};

	class write_atoms_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_atoms_representations_availability();

			std::string name=cargs.input.get_value_or_default<std::string>("name", "atoms");
			bool wireframe=cargs.input.get_flag("wireframe");
			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.atoms_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(false, cargs.input);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			if(parameters_for_representation_selecting.visual_ids.empty())
			{
				parameters_for_representation_selecting.visual_ids.insert(0);
			}

			if(parameters_for_representation_selecting.visual_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					for(std::set<std::size_t>::const_iterator jt=parameters_for_representation_selecting.visual_ids.begin();jt!=parameters_for_representation_selecting.visual_ids.end();++jt)
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

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				opengl_printer.print_pymol_script(name, true, output);
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote atoms as PyMol CGO to file '" << parameters_for_output_destinations.file << "' ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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

			std::string name=cargs.input.get_value_or_default<std::string>("name", "atoms");
			bool wireframe=cargs.input.get_flag("wireframe");
			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.atoms_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(false, cargs.input);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			if(parameters_for_representation_selecting.visual_ids.empty())
			{
				parameters_for_representation_selecting.visual_ids.insert(0);
			}

			if(parameters_for_representation_selecting.visual_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_atoms_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_atoms(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

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
						for(std::set<std::size_t>::const_iterator jt=parameters_for_representation_selecting.visual_ids.begin();jt!=parameters_for_representation_selecting.visual_ids.end();++jt)
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

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				opengl_printer.print_pymol_script(name, true, output);
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote cartoon as PyMol CGO to file '" << parameters_for_output_destinations.file << "' ";
				SummaryOfAtoms(cargs.data_manager.atoms(), ids).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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
			cargs.output_for_log << "Selections of atoms:\n";
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				cargs.output_for_log << "  name='" << (it->first) << "' ";
				SummaryOfAtoms(cargs.data_manager.atoms(), it->second).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_all_selections_of_atoms : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.data_manager.assert_atoms_selections_availability();
			cargs.data_manager.selection_manager().delete_atoms_selections();
			cargs.output_for_log << "Removed all selections of atoms\n";
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

			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No atoms selections names provided."));
			}

			cargs.data_manager.assert_atoms_selections_availability(names);

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.data_manager.selection_manager().delete_atoms_selection(names[i]);
			}

			cargs.output_for_log << "Removed selections of atoms:";
			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.output_for_log << " " << names[i];
			}
			cargs.output_for_log << "\n";
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
			cargs.output_for_log << "Renamed selection of atoms from '" << names[0] << "' to '" << names[1] << "'\n";
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
			parameters_to_enhance_contacts.tag_centrality=cargs.input.get_flag("tag-centrality");
			parameters_to_enhance_contacts.tag_peripherial=cargs.input.get_flag("tag-peripherial");
			CommandParametersForGenericSelecting render_parameters_for_selecting("render-", "{--min-seq-sep 1}");
			render_parameters_for_selecting.read(cargs.input);
			const bool render=(cargs.input.get_flag("render-default") ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_expression) ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_full_residues) ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_forced_id));

			cargs.input.assert_nothing_unusable();

			ConstructionOfTriangulation::ParametersToConstructBundleOfTriangulationInformation parameters_to_construct_triangulation;
			parameters_to_construct_triangulation.artificial_boundary_shift=std::max(parameters_to_construct_contacts.probe*2.0, 5.0);

			const std::vector<apollota::SimpleSphere> atomic_balls=ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(cargs.data_manager.atoms());

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
				cargs.output_for_log << "Using cached triangulation\n";
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
					draw_ids=cargs.data_manager.selection_manager().select_contacts(render_parameters_for_selecting.forced_ids, render_parameters_for_selecting.expression, render_parameters_for_selecting.full_residues);
				}

				ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, cargs.data_manager.triangulation_info(), draw_ids, cargs.data_manager.contacts_mutable());

				cargs.data_manager.reset_contacts_display_states();

				cargs.output_for_log << "Constructed contacts ";
				SummaryOfContacts(cargs.data_manager.contacts()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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

			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(true, cargs.input);
			const bool no_graphics=cargs.input.get_flag("no-graphics");

			cargs.input.assert_nothing_unusable();

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote contacts to file '" << parameters_for_output_destinations.file << "' ";
				SummaryOfContacts(cargs.data_manager.contacts()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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

				cargs.output_for_log << "Read contacts from file '" << file << "' ";
				SummaryOfContacts(cargs.data_manager.contacts()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			const std::string name=(cargs.input.is_any_unnamed_value_unused() ?
					cargs.input.get_value_or_first_unused_unnamed_value("name") :
					cargs.input.get_value_or_default<std::string>("name", ""));
			const bool no_marking=cargs.input.get_flag("no-marking");

			cargs.input.assert_nothing_unusable();

			assert_selection_name_input(name, true);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}

			if(!name.empty())
			{
				cargs.data_manager.selection_manager().set_contacts_selection(name, ids);
				cargs.output_for_log << "Set selection of contacts named '" << name << "'\n";
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

			cargs.output_set_of_contacts_ids.swap(ids);
		}
	};

	class tag_contacts : public GenericCommandForDataManager
	{
	public:
		tag_contacts() : positive_(true)
		{
		}

		explicit tag_contacts(const bool positive) : positive_(positive)
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			const std::string tag=cargs.input.get_value_or_first_unused_unnamed_value("tag");

			cargs.input.assert_nothing_unusable();

			assert_tag_input(tag);

			std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				Contact& contact=cargs.data_manager.contacts_mutable()[*it];
				if(positive_)
				{
					contact.value.props.tags.insert(tag);
				}
				else
				{
					contact.value.props.tags.erase(tag);
				}
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}

	private:
		bool positive_;
	};

	class untag_contacts : public tag_contacts
	{
	public:
		untag_contacts() : tag_contacts(false)
		{
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.contacts_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(positive_ && parameters_for_representation_selecting.visual_ids.empty() && cargs.data_manager.contacts_representation_descriptor().names.size()>1)
			{
				parameters_for_representation_selecting.visual_ids.insert(0);
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
			parameters_for_viewing.show=positive_;
			parameters_for_viewing.hide=!positive_;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
			{
				cargs.change_indicator.changed_contacts_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.contacts_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			CommandParametersForGenericColoring parameters_for_coloring;
			parameters_for_coloring.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(!auxiliaries::ColorUtilities::color_valid(parameters_for_coloring.color))
			{
				throw std::runtime_error(std::string("Contacts color not specified."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable contacts selected."));
			}

			CommandParametersForGenericViewing parameters_for_viewing;
			parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
			parameters_for_viewing.color=parameters_for_coloring.color;

			parameters_for_viewing.assert_state();

			if(parameters_for_viewing.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
			{
				cargs.change_indicator.changed_contacts_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
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

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.contacts_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			const std::string adjunct=cargs.input.get_value_or_default<std::string>("adjunct", "");
			const std::string by=adjunct.empty() ? cargs.input.get_value<std::string>("by") : std::string("adjunct");
			const std::string scheme=cargs.input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
			const bool min_val_present=cargs.input.is_option("min-val");
			const double min_val=cargs.input.get_value_or_default<double>("min-val", 0.0);
			const bool max_val_present=cargs.input.is_option("max-val");
			const double max_val=cargs.input.get_value_or_default<double>("max-val", 1.0);
			const bool only_summarize=cargs.input.get_flag("only-summarize");

			cargs.input.assert_nothing_unusable();

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
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
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
				parameters_for_viewing.visual_ids_=parameters_for_representation_selecting.visual_ids;
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
				cargs.output_for_log << "Summary: ";
				cargs.output_for_log << "count=" << ids.size() << " ";
				cargs.output_for_log << "min=" << min_val_actual << " ";
				cargs.output_for_log << "max=" << max_val_actual;
				cargs.output_for_log << "\n";
			}
		}
	};

	class print_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForContactsTablePrinting parameters_for_printing;
			parameters_for_printing.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(true);
			parameters_for_output_destinations.read(false, cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(&cargs.output_for_text);

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				TablePrinting::print_contacts(cargs.data_manager.atoms(), cargs.data_manager.contacts(), ids, parameters_for_printing.values, output);
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}
	};

	class zoom_by_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();

			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			const std::set<std::size_t> contacts_ids=cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues);

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

			cargs.summary_of_atoms=SummaryOfAtoms(cargs.data_manager.atoms(), atoms_ids);
			cargs.extra_values["zoom"]=true;

			cargs.output_for_log << "Bounding box: (" << cargs.summary_of_atoms.bounding_box.p_min << ") (" << cargs.summary_of_atoms.bounding_box.p_max << ")\n";
		}
	};

	class write_contacts_as_pymol_cgo : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_contacts_availability();
			cargs.data_manager.assert_contacts_representations_availability();

			std::string name=cargs.input.get_value_or_default<std::string>("name", "contacts");
			bool wireframe=cargs.input.get_flag("wireframe");
			CommandParametersForGenericSelecting parameters_for_selecting;
			parameters_for_selecting.read(cargs.input);
			CommandParametersForGenericRepresentationSelecting parameters_for_representation_selecting(cargs.data_manager.contacts_representation_descriptor().names);
			parameters_for_representation_selecting.read(cargs.input);
			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(false, cargs.input);

			cargs.input.assert_nothing_unusable();

			if(name.empty())
			{
				throw std::runtime_error(std::string("Missing object name."));
			}

			if(parameters_for_representation_selecting.visual_ids.empty())
			{
				parameters_for_representation_selecting.visual_ids.insert(0);
			}

			if(parameters_for_representation_selecting.visual_ids.size()>1)
			{
				throw std::runtime_error(std::string("More than one representation requested."));
			}

			const std::set<std::size_t> ids=cargs.data_manager.filter_contacts_drawable_implemented_ids(
					parameters_for_representation_selecting.visual_ids,
					cargs.data_manager.selection_manager().select_contacts(parameters_for_selecting.forced_ids, parameters_for_selecting.expression, parameters_for_selecting.full_residues),
					false);

			if(ids.empty())
			{
				throw std::runtime_error(std::string("No drawable visible contacts selected."));
			}

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

			auxiliaries::OpenGLPrinter opengl_printer;
			{
				unsigned int prev_color=0;
				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const std::size_t id=(*it);
					for(std::set<std::size_t>::const_iterator jt=parameters_for_representation_selecting.visual_ids.begin();jt!=parameters_for_representation_selecting.visual_ids.end();++jt)
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

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				opengl_printer.print_pymol_script(name, true, output);
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote contacts as PyMol CGO to file '" << parameters_for_output_destinations.file << "' ";
				SummaryOfContacts(cargs.data_manager.contacts(), ids).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
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
			cargs.output_for_log << "Selections of contacts:\n";
			for(std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_selections.begin();it!=map_of_selections.end();++it)
			{
				cargs.output_for_log << "  name='" << (it->first) << "' ";
				SummaryOfContacts(cargs.data_manager.contacts(), it->second).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_all_selections_of_contacts : public GenericCommandForDataManager
	{
	public:
		bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
		{
			return true;
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.data_manager.assert_contacts_selections_availability();
			cargs.data_manager.selection_manager().delete_contacts_selections();
			cargs.output_for_log << "Removed all selections of contacts\n";
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

			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No contacts selections names provided."));
			}

			cargs.data_manager.assert_contacts_selections_availability(names);

			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.data_manager.selection_manager().delete_contacts_selection(names[i]);
			}

			cargs.output_for_log << "Removed selections of contacts:";
			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.output_for_log << " " << names[i];
			}
			cargs.output_for_log << "\n";
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
			cargs.output_for_log << "Renamed selection of contacts from '" << names[0] << "' to '" << names[1] << "'\n";
		}
	};

	class save_atoms_and_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.data_manager.assert_atoms_availability();
			cargs.data_manager.assert_contacts_availability();

			CommandParametersForGenericOutputDestinations parameters_for_output_destinations(false);
			parameters_for_output_destinations.read(true, cargs.input);
			const bool no_graphics=cargs.input.get_flag("no-graphics");

			cargs.input.assert_nothing_unusable();

			std::vector<std::ostream*> outputs=parameters_for_output_destinations.get_output_destinations(0);

			for(std::size_t i=0;i<outputs.size();i++)
			{
				std::ostream& output=(*(outputs[i]));
				auxiliaries::IOUtilities().write_set(cargs.data_manager.atoms(), output);
				output << "_end_atoms\n";
				enabled_output_of_ContactValue_graphics()=!no_graphics;
				auxiliaries::IOUtilities().write_set(cargs.data_manager.contacts(), output);
				output << "_end_contacts\n";
			}

			if(!parameters_for_output_destinations.file.empty())
			{
				cargs.output_for_log << "Wrote atoms and contacts to file '" << parameters_for_output_destinations.file << "'";
				cargs.output_for_log << " ";
				SummaryOfAtoms(cargs.data_manager.atoms()).print(cargs.output_for_log);
				cargs.output_for_log << " ";
				SummaryOfContacts(cargs.data_manager.contacts()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

private:
	class CommandParametersForGenericSelecting
	{
	public:
		std::string type_for_expression;
		std::string type_for_full_residues;
		std::string type_for_forced_id;
		std::string expression;
		bool full_residues;
		std::set<std::size_t> forced_ids;

		CommandParametersForGenericSelecting() :
			type_for_expression("use"),
			type_for_full_residues("full-residues"),
			type_for_forced_id("id"),
			expression("{}"),
			full_residues(false)
		{
		}

		CommandParametersForGenericSelecting(const std::string& types_prefix, const std::string& default_expression) :
			type_for_expression(types_prefix+"use"),
			type_for_full_residues(types_prefix+"full-residues"),
			type_for_forced_id(types_prefix+"id"),
			expression(default_expression),
			full_residues(false)
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option(type_for_expression))
			{
				expression=input.get_value<std::string>(type_for_expression);
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
							expression=candidate;
							input.mark_unnamed_value_as_used(i);
							found=true;
						}
					}
				}
			}

			full_residues=input.get_flag(type_for_full_residues);

			{
				const std::vector<std::size_t> forced_ids_vector=input.get_value_vector_or_default<std::size_t>(type_for_forced_id, std::vector<std::size_t>());
				if(!forced_ids_vector.empty())
				{
					forced_ids.insert(forced_ids_vector.begin(), forced_ids_vector.end());
				}
			}
		}
	};

	class CommandParametersForGenericOutputDestinations
	{
	public:
		std::string file;
		bool use_stdout;
		std::ofstream foutput;

		explicit CommandParametersForGenericOutputDestinations(const bool use_stdout) : use_stdout(use_stdout)
		{
		}

		void read(const bool allow_use_of_unnamed_value, CommandInput& input)
		{
			if(input.is_option("file") || (allow_use_of_unnamed_value && input.is_any_unnamed_value_unused()))
			{
				const std::string str=(allow_use_of_unnamed_value ? input.get_value_or_first_unused_unnamed_value("file") : input.get_value<std::string>("file"));
				if(!str.empty() && str.find_first_of("?*$'\";:<>,|")==std::string::npos)
				{
					file=str;
				}
				else
				{
					throw std::runtime_error(std::string("Invalid file name '")+str+"'.");
				}
			}

			if(input.is_option("use-stdout"))
			{
				use_stdout=input.get_flag("use-stdout");
			}
		}

		std::vector<std::ostream*> get_output_destinations(std::ostream* stdout_ptr, const bool allow_empty_list=false)
		{
			std::vector<std::ostream*> list;

			if(use_stdout && stdout_ptr!=0)
			{
				list.push_back(stdout_ptr);
			}

			if(!file.empty())
			{
				if(!foutput.is_open())
				{
					foutput.open(file.c_str(), std::ios::out);
					if(!foutput.good())
					{
						throw std::runtime_error(std::string("Failed to open file '")+file+"' for writing.");
					}
				}
				if(!foutput.good())
				{
					throw std::runtime_error(std::string("Failed to use file '")+file+"' for writing.");
				}
				list.push_back(&foutput);
			}

			if(list.empty() && !allow_empty_list)
			{
				throw std::runtime_error(std::string("No output destinations specified."));
			}

			return list;
		}
	};

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
			color(0)
		{
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
			if((show || hide || mark || unmark || color>0) && id<display_states.size())
			{
				DataManager::DisplayState& ds=display_states[id];
				if(ds.implemented())
				{
					if(mark || unmark)
					{
						updated=(updated || (ds.marked!=mark));
						ds.marked=mark;
					}

					if(show || hide || color>0)
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
			if(show || hide || mark || unmark || color>0)
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
			if(show || hide || mark || unmark || color>0)
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

				if(color>0)
				{
					updated=(updated || (visual.color!=color));
					visual.color=color;
				}
			}

			return updated;
		}
	};

	class CommandParametersForGenericRepresentationSelecting
	{
	public:
		const std::vector<std::string>& available_representations;
		std::set<std::size_t> visual_ids;

		explicit CommandParametersForGenericRepresentationSelecting(const std::vector<std::string>& available_representations) : available_representations(available_representations)
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option("rep"))
			{
				const std::vector<std::string> names=input.get_value_vector<std::string>("rep");
				std::set<std::size_t> ids;
				for(std::size_t i=0;i<names.size();i++)
				{
					const std::string& name=names[i];
					std::size_t id=find_name_id(available_representations, name);
					if(id<available_representations.size())
					{
						ids.insert(id);
					}
					else
					{
						throw std::runtime_error(std::string("Representation '")+name+"' does not exist.");
					}
				}
				visual_ids.swap(ids);
			}
		}
	};

	class CommandParametersForGenericColoring
	{
	public:
		auxiliaries::ColorUtilities::ColorInteger color;

		CommandParametersForGenericColoring() : color(auxiliaries::ColorUtilities::null_color())
		{
		}

		void read(CommandInput& input)
		{
			if(input.is_option("col"))
			{
				color=auxiliaries::ColorUtilities::color_from_name(input.get_value<std::string>("col"));
			}
			else if(input.is_any_unnamed_value_unused())
			{
				bool found=false;
				for(std::size_t i=0;i<input.get_list_of_unnamed_values().size() && !found;i++)
				{
					if(!input.is_unnamed_value_used(i))
					{
						const std::string& candidate_str=input.get_list_of_unnamed_values()[i];
						if(candidate_str.size()>2 && candidate_str.rfind("0x", 0)==0)
						{
							auxiliaries::ColorUtilities::ColorInteger candidate_color=auxiliaries::ColorUtilities::color_from_name(candidate_str);
							if(candidate_color!=auxiliaries::ColorUtilities::null_color())
							{
								color=candidate_color;
								input.mark_unnamed_value_as_used(i);
								found=true;
							}
						}
					}
				}
			}
		}
	};

	class CommandParametersForGenericTablePrinting
	{
	public:
		TablePrinting::ParametersForGenericTablePrinting values;

		CommandParametersForGenericTablePrinting()
		{
		}

		void read(CommandInput& input)
		{
			values.reversed_sorting=input.get_flag("desc");
			values.expanded_descriptors=input.get_flag("expand");
			values.limit=input.get_value_or_default<std::size_t>("limit", std::numeric_limits<std::size_t>::max());
			values.sort_column=input.get_value_or_default<std::string>("sort", "");
		}
	};

	class CommandParametersForContactsTablePrinting
	{
	public:
		TablePrinting::ParametersForContactsTablePrinting values;

		CommandParametersForContactsTablePrinting()
		{
		}

		void read(CommandInput& input)
		{
			values.reversed_sorting=input.get_flag("desc");
			values.expanded_descriptors=input.get_flag("expand");
			values.limit=input.get_value_or_default<std::size_t>("limit", std::numeric_limits<std::size_t>::max());
			values.sort_column=input.get_value_or_default<std::string>("sort", "");
			values.inter_residue=input.get_flag("inter-residue");
		}
	};

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

	static void assert_selection_name_input(const std::string& name, const bool allow_empty)
	{
		if(name.empty())
		{
			if(!allow_empty)
			{
				throw std::runtime_error(std::string("Selection name is empty."));
			}
		}
		else if(name.find_first_of("{}()[]<>,;.:\\/+*/='\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("Selection name contains invalid symbols."));
		}
		else if(name.rfind("-", 0)==0 || name.rfind("_", 0)==0)
		{
			throw std::runtime_error(std::string("Selection name starts with invalid symbol."));
		}
	}

	static void assert_tag_input(const std::string& tag)
	{
		if(tag.empty())
		{
			throw std::runtime_error(std::string("Tag is empty."));
		}
		else if(tag.find_first_of("{}()[]<>,;.:\\/+*/'\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("Tag contains invalid symbols."));
		}
		else if(tag.rfind("-", 0)==0)
		{
			throw std::runtime_error(std::string("Tag starts with invalid symbol."));
		}
	}

	static std::size_t find_name_id(const std::vector<std::string>& names, const std::string& name)
	{
		std::size_t id=names.size();
		for(std::size_t i=0;i<names.size() && !(id<names.size());i++)
		{
			if(names[i]==name)
			{
				id=i;
			}
		}
		return id;
	}
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_ */
