#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_DATA_MANAGER_H_

#include "../../auxiliaries/color_utilities.h"

#include "../writing_atomic_balls_in_pdb_format.h"

#include "generic_command_for_data_manager.h"
#include "table_printing.h"

namespace common
{

namespace scripting
{

class CustomCommandsForDataManager
{
public:
	class load_atoms : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile parameters_to_collect_atoms;
			parameters_to_collect_atoms.include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
			parameters_to_collect_atoms.include_hydrogens=cargs.input.get_flag("include-hydrogens");
			parameters_to_collect_atoms.multimodel_chains=cargs.input.get_flag("as-assembly");
			const std::string atoms_file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const std::string radii_file=cargs.input.get_value_or_default<std::string>("radii-file", "");
			const double default_radius=cargs.input.get_value_or_default<double>("default-radii", ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius());
			const bool only_default_radius=cargs.input.get_flag("same-radius-for-all");
			std::string format=cargs.input.get_value_or_default<std::string>("format", "");
			CommandParametersForTitling parameters_for_titling;
			parameters_for_titling.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(atoms_file.empty())
			{
				throw std::runtime_error(std::string("Empty input atoms file name."));
			}

			if(format.empty())
			{
				format=get_format_from_atoms_file_name(atoms_file);
				if(format.empty())
				{
					throw std::runtime_error(std::string("Could not deduce format from file name '")+atoms_file+"'.");
				}
			}

			if(format!="pdb" && format!="mmcif" && format!="plain")
			{
				throw std::runtime_error(std::string("Unrecognized format '")+format+"', allowed formats are 'pdb', 'mmcif' or 'plain'.");
			}

			if(parameters_for_titling.title_available)
			{
				cargs.data_manager.set_title(parameters_for_titling.title);
			}

			if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::ParametersToCollectAtomicBallsFromFile::default_default_radius())
			{
				parameters_to_collect_atoms.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
			}

			std::vector<Atom> atoms;
			bool success=false;

			if(format=="pdb" || format=="mmcif")
			{
				if(format=="mmcif")
				{
					parameters_to_collect_atoms.mmcif=true;
				}
				success=ConstructionOfAtomicBalls::collect_atomic_balls_from_file(parameters_to_collect_atoms, atoms_file, atoms);
			}
			else if(format=="plain")
			{
				auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_file_lines_to_set(atoms_file, atoms);
				if(!atoms.empty())
				{
					if(!radii_file.empty() || only_default_radius)
					{
						for(std::size_t i=0;i<atoms.size();i++)
						{
							Atom& atom=atoms[i];
							atom.value.r=parameters_to_collect_atoms.atom_radius_assigner.get_atom_radius(atom.crad.resName, atom.crad.name);
						}
					}
					success=true;
				}
			}

			if(success)
			{
				if(atoms.size()<4)
				{
					throw std::runtime_error(std::string("Less than 4 atoms read."));
				}
				else
				{
					cargs.data_manager.reset_atoms_by_swapping(atoms);
					cargs.changed_atoms=true;
					cargs.summary_of_atoms=SummaryOfAtoms(cargs.data_manager.atoms());
					cargs.output_for_log << "Read atoms from file '" << atoms_file << "' ";
					print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms()), cargs.output_for_log);
					cargs.output_for_log << "\n";
				}
			}
			else
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+atoms_file+"' in '"+format+"' format.");
			}
		}

	private:
		static std::string get_format_from_atoms_file_name(const std::string& filename)
		{
			std::multimap<std::string, std::string> map_of_format_extensions;
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".pdb"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".PDB"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ent"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("pdb", ".ENT"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".cif"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".CIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmcif"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".mmCIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("mmcif", ".MMCIF"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".atoms"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pa"));
			map_of_format_extensions.insert(std::pair<std::string, std::string>("plain", ".pac"));
			for(std::multimap<std::string, std::string>::const_iterator it=map_of_format_extensions.begin();it!=map_of_format_extensions.end();++it)
			{
				const std::string& format=it->first;
				const std::string& extension=it->second;
				const std::size_t pos=filename.find(extension);
				if(pos<filename.size() && (pos+extension.size())==filename.size())
				{
					return format;
				}
			}
			return std::string();
		}
	};

	class restrict_atoms : public GenericCommandForDataManager
	{
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

				std::vector<Atom> atoms=slice_vector_by_ids(cargs.data_manager.atoms(), ids);
				cargs.data_manager.reset_atoms_by_swapping(atoms);
				cargs.changed_atoms=true;

				cargs.output_for_log << "Restricted atoms from ";
				print_summary_of_atoms(old_summary, cargs.output_for_log);
				cargs.output_for_log << " to ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms()), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
			else
			{
				cargs.output_for_log << "No need to restrict because all atoms were selected ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms()), cargs.output_for_log);
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
				print_summary_of_atoms(SummaryOfAtoms(atoms), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class select_atoms : public GenericCommandForDataManager
	{
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
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
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
						cargs.changed_atoms_display_states=true;
					}
				}
				{
					CommandParametersForGenericViewing params;
					params.mark=true;
					if(params.apply_to_display_states(ids, cargs.data_manager.atoms_display_states_mutable()))
					{
						cargs.changed_atoms_display_states=true;
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
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
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

	class mark_atoms : public GenericCommandForDataManager
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
					cargs.changed_atoms_display_states=true;
				}
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
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
				cargs.changed_atoms_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
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
				cargs.changed_atoms_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of atoms: ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}
	};

	class spectrum_atoms : public GenericCommandForDataManager
	{
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
						cargs.changed_atoms_display_states=true;
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
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), ids), cargs.output_for_log, true);
				cargs.output_for_log << "\n";
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

			cargs.output_for_log << "Bounding box: (" << cargs.summary_of_atoms.bounding_box.p_min << ") (" << cargs.summary_of_atoms.bounding_box.p_max << ")\n";
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
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms(), it->second), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_all_selections_of_atoms : public GenericCommandForDataManager
	{
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
			parameters_to_enhance_contacts.tag_centrality=cargs.input.get_flag("tag-centrality");
			parameters_to_enhance_contacts.tag_peripherial=cargs.input.get_flag("tag-peripherial");
			CommandParametersForGenericSelecting render_parameters_for_selecting("render-", "{--min-seq-sep 1}");
			render_parameters_for_selecting.read(cargs.input);
			const bool render=(cargs.input.get_flag("render-default") ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_expression) ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_full_residues) ||
					cargs.input.is_option(render_parameters_for_selecting.type_for_forced_id));

			cargs.input.assert_nothing_unusable();

			ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;
			ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

			if(ConstructionOfContacts::construct_bundle_of_contact_information(parameters_to_construct_contacts, common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(cargs.data_manager.atoms()), bundle_of_triangulation_information, bundle_of_contact_information))
			{
				cargs.data_manager.reset_contacts_by_swapping(bundle_of_contact_information.contacts);
				cargs.changed_contacts=true;

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

				ConstructionOfContacts::enhance_contacts(parameters_to_enhance_contacts, bundle_of_triangulation_information, draw_ids, cargs.data_manager.contacts_mutable());

				cargs.data_manager.reset_contacts_display_states();

				cargs.output_for_log << "Constructed contacts ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts()), cargs.output_for_log);
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts()), cargs.output_for_log);
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
				cargs.changed_contacts=true;

				cargs.output_for_log << "Read contacts from file '" << file << "' ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts()), cargs.output_for_log);
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
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
						cargs.changed_contacts_display_states=true;
					}
				}
				{
					CommandParametersForGenericViewing params;
					params.mark=true;
					if(params.apply_to_display_states(ids, cargs.data_manager.contacts_display_states_mutable()))
					{
						cargs.changed_contacts_display_states=true;
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
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
					cargs.changed_contacts_display_states=true;
				}
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
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
				cargs.changed_contacts_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
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
				cargs.changed_contacts_display_states=true;
			}

			{
				cargs.output_for_log << "Summary of contacts: ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
				cargs.output_for_log << "\n";
			}
		}
	};

	class spectrum_contacts : public GenericCommandForDataManager
	{
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
						cargs.changed_contacts_display_states=true;
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log, true);
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), ids), cargs.output_for_log);
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
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts(), it->second), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_all_selections_of_contacts : public GenericCommandForDataManager
	{
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
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms()), cargs.output_for_log);
				cargs.output_for_log << " ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts()), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}
		}
	};

	class load_atoms_and_contacts : public GenericCommandForDataManager
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			CommandParametersForTitling parameters_for_titling;
			parameters_for_titling.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input file name."));
			}

			if(parameters_for_titling.title_available)
			{
				cargs.data_manager.set_title(parameters_for_titling.title);
			}

			std::ifstream finput(file.c_str(), std::ios::in);
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::vector<Atom> atoms;
			std::vector<Contact> contacts;

			auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_lines_to_set(finput, atoms);

			if(atoms.empty())
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+file+"'.");
			}
			else if(atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}
			else
			{
				cargs.data_manager.reset_atoms_by_swapping(atoms);
				cargs.changed_atoms=true;

				cargs.output_for_log << "Read atoms from file '" << file << "' ";
				print_summary_of_atoms(SummaryOfAtoms(cargs.data_manager.atoms()), cargs.output_for_log);
				cargs.output_for_log << "\n";
			}

			auxiliaries::IOUtilities(true, '\n', ' ', "_end_contacts").read_lines_to_set(finput, contacts);

			if(contacts.empty())
			{
				cargs.output_for_log << "No contacts read from file '" << file << "'.";
			}
			else
			{
				cargs.data_manager.reset_contacts_by_swapping(contacts);
				cargs.changed_contacts=true;

				cargs.output_for_log << "Read contacts from file '" << file << "' ";
				print_summary_of_contacts(SummaryOfContacts(cargs.data_manager.contacts()), cargs.output_for_log);
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

	class CommandParametersForTitling
	{
	public:
		bool title_available;
		std::string title;

		CommandParametersForTitling() : title_available(false)
		{
		}

		void read(CommandInput& input)
		{
			title_available=input.is_option("title");
			if(title_available)
			{
				title=input.get_value<std::string>("title");
			}
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

	static void print_summary_of_atoms(const SummaryOfAtoms& summary, std::ostream& output, bool no_brackets=false)
	{
		output << (no_brackets ? "" : "(");
		output << "count=" << summary.number_total;
		if(summary.volume>0.0)
		{
			output << " volume=" << summary.volume;
		}
		output << (no_brackets ? "" : ")");
	}

	static void print_summary_of_contacts(const SummaryOfContacts& summary, std::ostream& output, bool no_brackets=false)
	{
		output << (no_brackets ? "" : "(");
		output << "count=" << summary.number_total;
		output << " drawable=" << summary.number_drawable;
		output << " area=" << summary.area;
		output << (no_brackets ? "" : ")");
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
