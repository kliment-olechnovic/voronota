#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "../construction_of_cad_score.h"

#include "generic_command_for_congregation_of_data_managers.h"
#include "basic_assertions.h"

namespace common
{

namespace scripting
{

class CustomCommandsForCongregationOfDataManagers
{
public:
	class list_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.congregation_of_data_managers.assert_objects_availability();
			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(false, false);
			cargs.output_for_log << "Objects:\n";
			for(std::size_t i=0;i<objects.size();i++)
			{
				const CongregationOfDataManagers::ObjectAttributes attributes=cargs.congregation_of_data_managers.get_object_attributes(objects[i]);
				cargs.output_for_log << "  '" << attributes.name << "'";
				if(attributes.picked)
				{
					cargs.output_for_log << " *";
				}
				if(!attributes.visible)
				{
					cargs.output_for_log << " h";
				}
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.congregation_of_data_managers.assert_objects_availability();

			const bool all=cargs.input.get_flag("all");

			if(all)
			{
				cargs.input.assert_nothing_unusable();
				std::vector<DataManager*> ptrs=cargs.congregation_of_data_managers.delete_all_objects();
				cargs.change_indicator.deleted_objects.insert(ptrs.begin(), ptrs.end());
				cargs.output_for_log << "Removed all objects\n";
			}
			else
			{
				const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
				cargs.input.mark_all_unnamed_values_as_used();
				cargs.input.assert_nothing_unusable();

				if(names.empty())
				{
					throw std::runtime_error(std::string("No object names provided."));
				}

				cargs.congregation_of_data_managers.assert_objects_availability(names);

				for(std::size_t i=0;i<names.size();i++)
				{
					DataManager* ptr=cargs.congregation_of_data_managers.delete_object(names[i]);
					if(ptr!=0)
					{
						cargs.change_indicator.deleted_objects.insert(ptr);
					}
				}

				cargs.output_for_log << "Removed objects:\n";
				for(std::size_t i=0;i<names.size();i++)
				{
					cargs.output_for_log << "  '" << names[i] << "'";
				}
				cargs.output_for_log << "\n";
			}
		}
	};

	class rename_object : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();
			cargs.input.assert_nothing_unusable();

			if(names.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two names provided for renaming."));
			}

			const std::string& name_original=names[0];
			const std::string& name_new=names[1];

			assert_new_name_input(name_new);

			cargs.congregation_of_data_managers.assert_object_availability(name_original);

			cargs.congregation_of_data_managers.rename_object(name_original, name_new);

			cargs.change_indicator.changed_objects_names=true;
		}
	};

	class copy_object : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();
			cargs.input.assert_nothing_unusable();

			if(names.size()!=2)
			{
				throw std::runtime_error(std::string("Not exactly two names provided for copying."));
			}

			const std::string& name_original=names[0];
			const std::string& name_new=names[1];

			assert_new_name_input(name_new);

			cargs.congregation_of_data_managers.assert_object_availability(name_original);

			DataManager* object_original=cargs.congregation_of_data_managers.get_object(name_original);
			DataManager* object_new=cargs.congregation_of_data_managers.add_object(*object_original, name_new);
			cargs.change_indicator.added_objects.insert(object_new);
		}
	};

	class load_atoms : public GenericCommandForCongregationOfDataManagers
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
				assert_new_name_input(parameters_for_titling.title);
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
					const std::string title=(parameters_for_titling.title_available ? parameters_for_titling.title : get_basename_from_path(atoms_file));

					DataManager* object_new=cargs.congregation_of_data_managers.add_object(DataManager(), title);
					DataManager& data_manager=*object_new;

					data_manager.reset_atoms_by_swapping(atoms);

					SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["loaded"];
					summary_of_atoms=SummaryOfAtoms(data_manager.atoms());

					cargs.output_for_log << "Read atoms from file '" << atoms_file << "' ";
					summary_of_atoms.print(cargs.output_for_log);
					cargs.output_for_log << "\n";

					cargs.change_indicator.added_objects.insert(object_new);

					cargs.congregation_of_data_managers.set_all_objects_picked(false);
					cargs.congregation_of_data_managers.set_object_picked(object_new, true);
					cargs.change_indicator.changed_objects_picks=true;
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

	class load_atoms_and_contacts : public GenericCommandForCongregationOfDataManagers
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
				assert_new_name_input(parameters_for_titling.title);
			}

			std::ifstream finput(file.c_str(), std::ios::in);
			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::vector<Atom> atoms;
			auxiliaries::IOUtilities(true, '\n', ' ', "_end_atoms").read_lines_to_set(finput, atoms);

			if(atoms.empty())
			{
				throw std::runtime_error(std::string("Failed to read atoms from file '")+file+"'.");
			}
			else if(atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}

			std::vector<Contact> contacts;
			auxiliaries::IOUtilities(true, '\n', ' ', "_end_contacts").read_lines_to_set(finput, contacts);

			{
				const std::string title=(parameters_for_titling.title_available ? parameters_for_titling.title : get_basename_from_path(file));

				DataManager* object_new=cargs.congregation_of_data_managers.add_object(DataManager(), title);
				DataManager& data_manager=*object_new;

				data_manager.reset_atoms_by_swapping(atoms);

				SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["loaded"];
				summary_of_atoms=SummaryOfAtoms(data_manager.atoms());

				cargs.output_for_log << "Read atoms from file '" << file << "' ";
				summary_of_atoms.print(cargs.output_for_log);
				cargs.output_for_log << "\n";

				if(!contacts.empty())
				{
					data_manager.reset_contacts_by_swapping(contacts);

					cargs.output_for_log << "Read contacts from file '" << file << "' ";
					SummaryOfContacts(data_manager.contacts()).print(cargs.output_for_log);
					cargs.output_for_log << "\n";
				}
				else
				{
					cargs.output_for_log << "No contacts read from file '" << file << "'.";
				}

				cargs.change_indicator.added_objects.insert(object_new);

				cargs.congregation_of_data_managers.set_all_objects_picked(false);
				cargs.congregation_of_data_managers.set_object_picked(object_new, true);
				cargs.change_indicator.changed_objects_picks=true;
			}
		}
	};

	class pick_objects : public GenericCommandForCongregationOfDataManagers
	{
	public:
		pick_objects() : positive_(true), add_(false)
		{
		}

		explicit pick_objects(const bool positive, const bool add) : positive_(positive), add_(add)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.congregation_of_data_managers.assert_objects_availability();

			const bool all=cargs.input.get_flag("all");

			if(all)
			{
				cargs.input.assert_nothing_unusable();
				cargs.congregation_of_data_managers.set_all_objects_picked(positive_);
				cargs.change_indicator.changed_objects_picks=true;
			}
			else
			{
				const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
				cargs.input.mark_all_unnamed_values_as_used();
				cargs.input.assert_nothing_unusable();

				if(names.empty())
				{
					throw std::runtime_error(std::string("No object names provided."));
				}

				cargs.congregation_of_data_managers.assert_objects_availability(names);

				if(positive_ && !add_)
				{
					cargs.congregation_of_data_managers.set_all_objects_picked(false);
				}

				cargs.change_indicator.changed_objects_picks=true;

				for(std::size_t i=0;i<names.size();i++)
				{
					cargs.congregation_of_data_managers.set_object_picked(names[i], positive_);
				}
			}
		}

	private:
		bool positive_;
		bool add_;
	};

	class pick_more_objects : public pick_objects
	{
	public:
		pick_more_objects() : pick_objects(true, true)
		{
		}
	};

	class unpick_objects : public pick_objects
	{
	public:
		unpick_objects() : pick_objects(false, false)
		{
		}
	};

	class show_objects : public GenericCommandForCongregationOfDataManagers
	{
	public:
		show_objects() : positive_(true)
		{
		}

		explicit show_objects(const bool positive) : positive_(positive)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.congregation_of_data_managers.assert_objects_availability();

			const bool all=cargs.input.get_flag("all");

			if(all)
			{
				cargs.input.assert_nothing_unusable();
				cargs.congregation_of_data_managers.set_all_objects_visible(positive_);
				cargs.change_indicator.changed_objects_visibilities=true;
			}
			else
			{
				const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
				cargs.input.mark_all_unnamed_values_as_used();

				cargs.input.assert_nothing_unusable();

				if(names.empty())
				{
					throw std::runtime_error(std::string("No object names provided."));
				}

				cargs.congregation_of_data_managers.assert_objects_availability(names);

				cargs.change_indicator.changed_objects_visibilities=true;

				for(std::size_t i=0;i<names.size();i++)
				{
					cargs.congregation_of_data_managers.set_object_visible(names[i], positive_);
				}
			}
		}

	private:
		bool positive_;
	};

	class hide_objects : public show_objects
	{
	public:
		hide_objects() : show_objects(false)
		{
		}
	};

	class zoom_by_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& names=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();
			cargs.input.assert_nothing_unusable();

			SummaryOfAtoms summary_of_atoms;

			if(names.empty())
			{
				std::vector<DataManager*> dms=cargs.congregation_of_data_managers.get_objects(false, true);
				for(std::size_t i=0;i<dms.size();i++)
				{
					summary_of_atoms.feed(SummaryOfAtoms(dms[i]->atoms()));
				}
			}
			else
			{
				cargs.congregation_of_data_managers.assert_objects_availability(names);
				for(std::size_t i=0;i<names.size();i++)
				{
					DataManager* dm=cargs.congregation_of_data_managers.get_object(names[i]);
					if(dm!=0)
					{
						summary_of_atoms.feed(SummaryOfAtoms(dm->atoms()));
					}
				}
			}

			if(summary_of_atoms.bounding_box.filled)
			{
				cargs.heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
				cargs.output_for_log << "Bounding box: (" << summary_of_atoms.bounding_box.p_min << ") (" << summary_of_atoms.bounding_box.p_max << ")\n";
			}
			else
			{
				cargs.output_for_log << "No visible objects to zoom by.\n";
			}

		}
	};

	class cad_score : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string target_name=cargs.input.get_value<std::string>("target");
			const std::string model_name=cargs.input.get_value<std::string>("model");
			const std::string target_sel=cargs.input.get_value_or_default<std::string>("t-sel", "{--no-solvent --min-seq-sep 1}");
			const std::string model_sel=cargs.input.get_value_or_default<std::string>("m-sel", "{--no-solvent --min-seq-sep 1}");
			const std::string target_atom_scores=cargs.input.get_value_or_default<std::string>("t-atom-scores", "");
			const std::string model_atom_scores=cargs.input.get_value_or_default<std::string>("m-atom-scores", "");
			const std::string target_inter_atom_scores=cargs.input.get_value_or_default<std::string>("t-inter-atom-scores", "");
			const std::string model_inter_atom_scores=cargs.input.get_value_or_default<std::string>("m-inter-atom-scores", "");
			const std::string target_residue_scores=cargs.input.get_value_or_default<std::string>("t-residue-scores", "");
			const std::string model_residue_scores=cargs.input.get_value_or_default<std::string>("m-residue-scores", "");
			const std::string target_inter_residue_scores=cargs.input.get_value_or_default<std::string>("t-inter-residue-scores", "");
			const std::string model_inter_residue_scores=cargs.input.get_value_or_default<std::string>("m-inter-residue-scores", "");
			const unsigned int smoothing_window=cargs.input.get_value_or_default<unsigned int>("smoothing-window", 0);
			const bool ignore_residue_names=cargs.input.get_flag("ignore-residue-names");

			cargs.input.assert_nothing_unusable();

			if(target_name==model_name)
			{
				throw std::runtime_error(std::string("Target and model are the same."));
			}

			assert_adjunct_name_input(target_atom_scores, true);
			assert_adjunct_name_input(model_atom_scores, true);
			assert_adjunct_name_input(target_inter_atom_scores, true);
			assert_adjunct_name_input(model_inter_atom_scores, true);
			assert_adjunct_name_input(target_residue_scores, true);
			assert_adjunct_name_input(model_residue_scores, true);
			assert_adjunct_name_input(target_inter_residue_scores, true);
			assert_adjunct_name_input(model_inter_residue_scores, true);

			cargs.congregation_of_data_managers.assert_object_availability(target_name);
			cargs.congregation_of_data_managers.assert_object_availability(model_name);

			DataManager& target_dm=*cargs.congregation_of_data_managers.get_object(target_name);
			DataManager& model_dm=*cargs.congregation_of_data_managers.get_object(model_name);

			target_dm.assert_contacts_availability();
			model_dm.assert_contacts_availability();

			const std::set<std::size_t> target_ids=target_dm.selection_manager().select_contacts(std::set<std::size_t>(), target_sel, false);
			if(target_ids.empty())
			{
				throw std::runtime_error(std::string("No target contacts selected."));
			}

			const std::set<std::size_t> model_ids=model_dm.selection_manager().select_contacts(std::set<std::size_t>(), model_sel, false);
			if(model_ids.empty())
			{
				throw std::runtime_error(std::string("No model contacts selected."));
			}

			ConstructionOfCADScore::ParametersToConstructBundleOfCADScoreInformation parameters;
			parameters.ignore_residue_names=ignore_residue_names;
			parameters.atom_level=!(
					target_atom_scores.empty()
					&& model_atom_scores.empty()
					&& target_inter_atom_scores.empty()
					&& model_inter_atom_scores.empty());

			ConstructionOfCADScore::BundleOfCADScoreInformation bundle;
			if(!ConstructionOfCADScore::construct_bundle_of_cadscore_information(
					parameters,
					target_dm.atoms(),
					target_dm.contacts(),
					target_ids,
					model_dm.atoms(),
					model_dm.contacts(),
					model_ids,
					bundle))
			{
				throw std::runtime_error(std::string("Failed to calculate CAD-score."));
			}

			DataManager::ChangeIndicator target_dm_ci;
			DataManager::ChangeIndicator model_dm_ci;

			write_adjuncts(bundle, smoothing_window, target_ids, target_atom_scores, target_residue_scores, target_inter_atom_scores, target_inter_residue_scores, target_dm, target_dm_ci);
			write_adjuncts(bundle, smoothing_window, model_ids, model_atom_scores, model_residue_scores, model_inter_atom_scores, model_inter_residue_scores, model_dm, model_dm_ci);

			cargs.change_indicator.handled_objects[&target_dm]=target_dm_ci;
			cargs.change_indicator.handled_objects[&model_dm]=model_dm_ci;

			if(parameters.atom_level)
			{
				cargs.output_for_log << "atom_level_global ";
				print_cad_descriptor(cargs.output_for_log, bundle.atom_level_global_descriptor);
				cargs.output_for_log << "\n";
			}

			if(parameters.residue_level)
			{
				cargs.output_for_log << "residue_level_global ";
				print_cad_descriptor(cargs.output_for_log, bundle.residue_level_global_descriptor);
				cargs.output_for_log << "\n";
			}
		}

	private:
		static void write_adjuncts(
				const ConstructionOfCADScore::BundleOfCADScoreInformation& bundle,
				const unsigned int smoothing_window,
				const std::set<std::size_t>& contact_ids,
				const std::string& adjunct_atom_scores,
				const std::string& adjunct_residue_scores,
				const std::string& adjunct_inter_atom_scores,
				const std::string& adjunct_inter_residue_scores,
				DataManager& dm,
				DataManager::ChangeIndicator& dm_ci)
		{
			if(!adjunct_atom_scores.empty() || !adjunct_residue_scores.empty())
			{
				for(std::size_t i=0;i<dm.atoms_mutable().size();i++)
				{
					ConstructionOfCADScore::Atom& atom=dm.atoms_mutable()[i];
					if(!adjunct_atom_scores.empty())
					{
						std::map<ConstructionOfCADScore::CRAD, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
								bundle.map_of_atom_cad_descriptors.find(atom.crad);
						if(jt!=bundle.map_of_atom_cad_descriptors.end())
						{
							atom.value.props.adjuncts[adjunct_atom_scores]=jt->second.score();
							dm_ci.changed_atoms_adjuncts=true;
						}
					}
					if(!adjunct_residue_scores.empty())
					{
						std::map<ConstructionOfCADScore::CRAD, double> smoothed_residue_scores=bundle.residue_scores(smoothing_window);
						std::map<ConstructionOfCADScore::CRAD, double>::const_iterator jt=
								smoothed_residue_scores.find(atom.crad.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
						if(jt!=smoothed_residue_scores.end())
						{
							atom.value.props.adjuncts[adjunct_residue_scores]=jt->second;
							dm_ci.changed_atoms_adjuncts=true;
						}
					}
				}
			}

			if(!adjunct_inter_atom_scores.empty() || !adjunct_inter_residue_scores.empty())
			{
				for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
				{
					ConstructionOfCADScore::Contact& contact=dm.contacts_mutable()[*it];
					const ConstructionOfCADScore::CRADsPair crads=ConversionOfDescriptors::get_contact_descriptor(dm.atoms(), contact);
					if(crads.valid())
					{
						if(!adjunct_inter_atom_scores.empty())
						{
							std::map<ConstructionOfCADScore::CRADsPair, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
									bundle.map_of_inter_atom_cad_descriptors.find(crads);
							if(jt!=bundle.map_of_inter_atom_cad_descriptors.end())
							{
								if(jt->second.target_area_sum>0.0)
								{
									contact.value.props.adjuncts[adjunct_inter_atom_scores]=jt->second.score();
									dm_ci.changed_contacts_adjuncts=true;
								}
							}
						}
						if(!adjunct_inter_residue_scores.empty())
						{
							const ConstructionOfCADScore::CRADsPair ir_crads(
									crads.a.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names),
									crads.b.without_some_info(true, true, false, bundle.parameters_of_construction.ignore_residue_names));
							std::map<ConstructionOfCADScore::CRADsPair, ConstructionOfCADScore::CADDescriptor>::const_iterator jt=
									bundle.map_of_inter_residue_cad_descriptors.find(ir_crads);
							if(jt!=bundle.map_of_inter_residue_cad_descriptors.end())
							{
								if(jt->second.target_area_sum>0.0)
								{
									contact.value.props.adjuncts[adjunct_inter_residue_scores]=jt->second.score();
									dm_ci.changed_contacts_adjuncts=true;
								}
							}
						}
					}
				}
			}
		}

		static void print_cad_descriptor(std::ostream& output, const ConstructionOfCADScore::CADDescriptor& cadd)
		{
			output << cadd.score();
			output << " " << cadd.target_area_sum
					<< " " << cadd.model_area_sum
					<< " " << cadd.raw_differences_sum
					<< " " << cadd.constrained_differences_sum
					<< " " << cadd.model_target_area_sum;
		}
	};

private:
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

	static void assert_new_name_input(const std::string& name)
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("New name is empty."));
		}
		if(name.find_first_of("{}()[]<>\\/*/'\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("New name '")+name+"' contains invalid symbols.");
		}
		else if(name.find_first_of("-+,;.: ", 0)==0)
		{
			throw std::runtime_error(std::string("New name '")+name+"' starts with invalid symbol.");
		}
	}

	static std::string get_basename_from_path(const std::string& path)
	{
		const std::size_t pos=path.find_last_of("/\\");
		if(pos==std::string::npos)
		{
			return path;
		}
		else if((pos+1)<path.size())
		{
			return path.substr(pos+1);
		}
		else
		{
			return std::string();
		}
	}
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
