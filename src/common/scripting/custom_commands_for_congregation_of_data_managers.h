#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "generic_command_for_congregation_of_data_managers.h"

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
			const std::vector<CongregationOfDataManagers::ObjectDescriptor> descriptors=cargs.congregation_of_data_managers.get_descriptors(false);
			cargs.output_for_log << "Objects:\n";
			for(std::size_t i=0;i<descriptors.size();i++)
			{
				cargs.output_for_log << "  '" << descriptors[i].name << "'";
				if(descriptors[i].enabled)
				{
					cargs.output_for_log << " *";
				}
				cargs.output_for_log << "\n";
			}
		}
	};

	class delete_all_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			cargs.congregation_of_data_managers.assert_objects_availability();
			cargs.congregation_of_data_managers.delete_all_objects();
			cargs.output_for_log << "Removed all objects\n";
		}
	};

	class delete_objects : public GenericCommandForCongregationOfDataManagers
	{
	protected:
		void run(CommandArguments& cargs)
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
				cargs.congregation_of_data_managers.delete_object(names[i]);
			}

			cargs.output_for_log << "Removed objects:";
			for(std::size_t i=0;i<names.size();i++)
			{
				cargs.output_for_log << "  '" << names[i] << "'";
			}
			cargs.output_for_log << "\n";
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

			DataManager* data_manager_original=cargs.congregation_of_data_managers.get_descriptor(name_original).at(0).data_manager_ptr;
			data_manager_original->set_title(name_new);
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

			DataManager* data_manager_original=cargs.congregation_of_data_managers.get_descriptor(name_original).at(0).data_manager_ptr;
			DataManager* data_manager_new=cargs.congregation_of_data_managers.add_object(*data_manager_original).at(0).data_manager_ptr;
			data_manager_new->set_title(name_new);
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
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */
