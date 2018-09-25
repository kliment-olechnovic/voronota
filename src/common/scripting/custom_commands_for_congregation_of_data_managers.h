#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command_for_congregation_of_data_managers.h"
#include "basic_assertions.h"
#include "loading_of_data.h"
#include "scoring_of_data_managers_using_cad_score.h"

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

	class load : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			const std::string  format=cargs.input.get_value_or_default<std::string>("format", "");
			CommandParametersForTitling parameters_for_titling;
			parameters_for_titling.read(cargs.input);

			cargs.input.assert_nothing_unusable();

			LoadingOfData::Result result;
			LoadingOfData::construct_result(LoadingOfData::Parameters(file, format), result);

			if(result.atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}

			const std::string title=(parameters_for_titling.title_available ? parameters_for_titling.title : get_basename_from_path(file));

			DataManager* object_new=cargs.congregation_of_data_managers.add_object(DataManager(), title);
			DataManager& data_manager=*object_new;

			data_manager.reset_atoms_by_swapping(result.atoms);

			{
				SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["loaded"];
				summary_of_atoms=SummaryOfAtoms(data_manager.atoms());

				cargs.output_for_log << "Read atoms from file '" << file << "' ";
				summary_of_atoms.print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}

			if(!result.contacts.empty())
			{
				data_manager.reset_contacts_by_swapping(result.contacts);

				cargs.output_for_log << "Read contacts from file '" << file << "' ";
				SummaryOfContacts(data_manager.contacts()).print(cargs.output_for_log);
				cargs.output_for_log << "\n";
			}

			cargs.change_indicator.added_objects.insert(object_new);

			cargs.congregation_of_data_managers.set_all_objects_picked(false);
			cargs.congregation_of_data_managers.set_object_picked(object_new, true);
			cargs.change_indicator.changed_objects_picks=true;
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
			ScoringOfDataManagersUsingCADScore::Parameters params;
			params.target_selection_expression=cargs.input.get_value_or_default<std::string>("t-sel", "{--no-solvent --min-seq-sep 1}");
			params.model_selection_expression=cargs.input.get_value_or_default<std::string>("m-sel", "{--no-solvent --min-seq-sep 1}");
			params.target_adjunct_atom_scores=cargs.input.get_value_or_default<std::string>("t-adj-atom", "");
			params.target_adjunct_inter_atom_scores=cargs.input.get_value_or_default<std::string>("t-adj-inter-atom", "");
			params.target_adjunct_residue_scores=cargs.input.get_value_or_default<std::string>("t-adj-residue", "");
			params.target_adjunct_inter_residue_scores=cargs.input.get_value_or_default<std::string>("t-adj-inter-residue", "");
			params.model_adjunct_atom_scores=cargs.input.get_value_or_default<std::string>("m-adj-atom", "");
			params.model_adjunct_inter_atom_scores=cargs.input.get_value_or_default<std::string>("m-adj-inter-atom", "");
			params.model_adjunct_residue_scores=cargs.input.get_value_or_default<std::string>("m-adj-residue", "");
			params.model_adjunct_inter_residue_scores=cargs.input.get_value_or_default<std::string>("m-adj-inter-residue", "");
			params.smoothing_window=cargs.input.get_value_or_default<unsigned int>("smoothing-window", 0);
			params.ignore_residue_names=cargs.input.get_flag("ignore-residue-names");

			cargs.input.assert_nothing_unusable();

			if(target_name==model_name)
			{
				throw std::runtime_error(std::string("Target and model are the same."));
			}

			cargs.congregation_of_data_managers.assert_object_availability(target_name);
			cargs.congregation_of_data_managers.assert_object_availability(model_name);

			DataManager& target_dm=*cargs.congregation_of_data_managers.get_object(target_name);
			DataManager& model_dm=*cargs.congregation_of_data_managers.get_object(model_name);

			ScoringOfDataManagersUsingCADScore::Result result;
			ScoringOfDataManagersUsingCADScore::construct_result(params, target_dm, model_dm, result);

			cargs.change_indicator.handled_objects[&target_dm]=result.target_dm_ci;
			cargs.change_indicator.handled_objects[&model_dm]=result.model_dm_ci;

			if(result.bundle.parameters_of_construction.atom_level)
			{
				cargs.output_for_log << "atom_level_global ";
				print_cad_descriptor(cargs.output_for_log, result.bundle.atom_level_global_descriptor);
				cargs.output_for_log << "\n";
			}

			if(result.bundle.parameters_of_construction.residue_level)
			{
				cargs.output_for_log << "residue_level_global ";
				print_cad_descriptor(cargs.output_for_log, result.bundle.residue_level_global_descriptor);
				cargs.output_for_log << "\n";
			}
		}

	private:
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
