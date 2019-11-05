#ifndef SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command_for_congregation_of_data_managers.h"
#include "basic_assertions.h"
#include "loading_of_data.h"
#include "scoring_of_data_managers_using_cad_score.h"
#include "variant_serialization.h"

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
			cargs.congregation_of_data_managers.assert_objects_availability();

			const CongregationOfDataManagers::ObjectQuery query=read_query(cargs.input);
			cargs.input.assert_nothing_unusable();

			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(query);
			if(objects.empty())
			{
				throw std::runtime_error(std::string("No objects selected."));
			}

			for(std::size_t i=0;i<objects.size();i++)
			{
				const CongregationOfDataManagers::ObjectAttributes attributes=cargs.congregation_of_data_managers.get_object_attributes(objects[i]);
				VariantObject info;
				info.value("name")=attributes.name;
				info.value("picked")=attributes.picked;
				info.value("visible")=attributes.visible;
				cargs.heterostorage.variant_object.objects_array("objects").push_back(info);
			}
		}
	};

	class delete_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.congregation_of_data_managers.assert_objects_availability();

			const CongregationOfDataManagers::ObjectQuery query=read_query(cargs.input);
			cargs.input.assert_nothing_unusable();

			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(query);
			if(objects.empty())
			{
				throw std::runtime_error(std::string("No objects selected."));
			}

			for(std::size_t i=0;i<objects.size();i++)
			{
				cargs.heterostorage.variant_object.values_array("deleted_objects").push_back(
						VariantValue(cargs.congregation_of_data_managers.get_object_attributes(objects[i]).name));
				DataManager* ptr=cargs.congregation_of_data_managers.delete_object(objects[i]);
				if(ptr!=0)
				{
					cargs.change_indicator.deleted_objects.insert(ptr);
				}
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

			cargs.heterostorage.variant_object.value("object_name")=cargs.congregation_of_data_managers.get_object_attributes(object_new).name;
		}
	};

	class import : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			LoadingOfData::Parameters params;
			params.file=cargs.input.get_value_or_first_unused_unnamed_value("file");
			params.format=cargs.input.get_value_or_default<std::string>("format", "");
			params.forced_include_heteroatoms=cargs.input.is_option("include-heteroatoms");
			params.forced_include_hydrogens=cargs.input.is_option("include-hydrogens");
			params.forced_multimodel_chains=cargs.input.is_option("as-assembly");
			params.include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
			params.include_hydrogens=cargs.input.get_flag("include-hydrogens");
			params.multimodel_chains=cargs.input.get_flag("as-assembly");
			const std::string title=(cargs.input.is_option("title") ? cargs.input.get_value<std::string>("title") : get_basename_from_path(params.file));

			cargs.input.assert_nothing_unusable();

			LoadingOfData::Result result;
			LoadingOfData::construct_result(params, result);

			if(result.atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}

			DataManager* object_new=cargs.congregation_of_data_managers.add_object(DataManager(), title);
			DataManager& data_manager=*object_new;

			data_manager.reset_atoms_by_swapping(result.atoms);

			{
				SummaryOfAtoms& summary_of_atoms=cargs.heterostorage.summaries_of_atoms["loaded"];
				summary_of_atoms=SummaryOfAtoms(data_manager.atoms());

				VariantSerialization::write(summary_of_atoms, cargs.heterostorage.variant_object.object("atoms_summary"));
			}

			if(!result.contacts.empty())
			{
				data_manager.reset_contacts_by_swapping(result.contacts);

				SummaryOfContacts& summary_of_contacts=cargs.heterostorage.summaries_of_contacts["loaded"];
				summary_of_contacts=SummaryOfContacts(data_manager.contacts());

				VariantSerialization::write(summary_of_contacts, cargs.heterostorage.variant_object.object("contacts_summary"));
			}

			cargs.change_indicator.added_objects.insert(object_new);

			cargs.congregation_of_data_managers.set_all_objects_picked(false);
			cargs.congregation_of_data_managers.set_object_picked(object_new, true);
			cargs.change_indicator.changed_objects_picks=true;

			cargs.heterostorage.variant_object.value("object_name")=cargs.congregation_of_data_managers.get_object_attributes(object_new).name;;
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

			const CongregationOfDataManagers::ObjectQuery query=read_query(cargs.input);
			cargs.input.assert_nothing_unusable();

			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(query);
			if(objects.empty())
			{
				throw std::runtime_error(std::string("No objects selected."));
			}

			cargs.change_indicator.changed_objects_picks=true;

			if(positive_ && !add_)
			{
				cargs.congregation_of_data_managers.set_all_objects_picked(false);
			}

			for(std::size_t i=0;i<objects.size();i++)
			{
				cargs.congregation_of_data_managers.set_object_picked(objects[i], positive_);
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

			const CongregationOfDataManagers::ObjectQuery query=read_query(cargs.input);
			cargs.input.assert_nothing_unusable();

			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(query);
			if(objects.empty())
			{
				throw std::runtime_error(std::string("No objects selected."));
			}

			cargs.change_indicator.changed_objects_visibilities=true;

			for(std::size_t i=0;i<objects.size();i++)
			{
				cargs.congregation_of_data_managers.set_object_visible(objects[i], positive_);
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
			cargs.congregation_of_data_managers.assert_objects_availability();

			const CongregationOfDataManagers::ObjectQuery query=read_query(cargs.input);
			cargs.input.assert_nothing_unusable();

			const std::vector<DataManager*> objects=cargs.congregation_of_data_managers.get_objects(query);
			if(objects.empty())
			{
				throw std::runtime_error(std::string("No objects selected."));
			}

			SummaryOfAtoms summary_of_atoms;

			for(std::size_t i=0;i<objects.size();i++)
			{
				summary_of_atoms.feed(SummaryOfAtoms(objects[i]->atoms()));
			}

			if(summary_of_atoms.bounding_box.filled)
			{
				cargs.heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
				VariantSerialization::write(summary_of_atoms.bounding_box, cargs.heterostorage.variant_object.object("bounding_box"));
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
			params.target_selection_expression=cargs.input.get_value_or_default<std::string>("t-sel", "[--no-solvent --min-seq-sep 1]");
			params.model_selection_expression=cargs.input.get_value_or_default<std::string>("m-sel", params.target_selection_expression);
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
			params.binarize=cargs.input.get_flag("binarize");

			cargs.input.assert_nothing_unusable();

			if(target_name==model_name)
			{
				throw std::runtime_error(std::string("Target and model are the same."));
			}

			assert_adjunct_name_input(params.target_adjunct_atom_scores, true);
			assert_adjunct_name_input(params.target_adjunct_inter_atom_scores, true);
			assert_adjunct_name_input(params.target_adjunct_residue_scores, true);
			assert_adjunct_name_input(params.target_adjunct_inter_residue_scores, true);
			assert_adjunct_name_input(params.model_adjunct_atom_scores, true);
			assert_adjunct_name_input(params.model_adjunct_inter_atom_scores, true);
			assert_adjunct_name_input(params.model_adjunct_residue_scores, true);
			assert_adjunct_name_input(params.model_adjunct_inter_residue_scores, true);

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
				write_cad_descriptor(result.bundle.atom_level_global_descriptor, cargs.heterostorage.variant_object.object("atom_level_result"));
			}

			if(result.bundle.parameters_of_construction.residue_level)
			{
				write_cad_descriptor(result.bundle.residue_level_global_descriptor, cargs.heterostorage.variant_object.object("residue_level_result"));
			}
		}

	private:
		static void write_cad_descriptor(const common::ConstructionOfCADScore::CADDescriptor& cadd, VariantObject& output)
		{
			output.value("score")=cadd.score();
			output.value("target_area_sum")=cadd.target_area_sum;
			output.value("model_area_sum")=cadd.model_area_sum;
			output.value("raw_differences_sum")=cadd.raw_differences_sum;
			output.value("constrained_differences_sum")=cadd.constrained_differences_sum;
			output.value("model_target_area_sum")=cadd.model_target_area_sum;
		}
	};

private:
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

	static CongregationOfDataManagers::ObjectQuery read_query(CommandInput& input)
	{
		CongregationOfDataManagers::ObjectQuery query;

		query.picked=input.get_flag("picked");
		query.not_picked=input.get_flag("not-picked");
		query.visible=input.get_flag("visible");
		query.not_visible=input.get_flag("not-visible");

		std::vector<std::string> names;
		if(input.is_option("names"))
		{
			names=input.get_value_vector<std::string>("names");
		}
		else
		{
			names=input.get_list_of_unnamed_values();
			input.mark_all_unnamed_values_as_used();
		}
		query.names.insert(names.begin(), names.end());

		return query;
	}
};

}

#endif /* SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
