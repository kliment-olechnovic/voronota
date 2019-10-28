#ifndef SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_
#define SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_

#include "../auxiliaries/time_utilities.h"

#include "generic_command.h"
#include "collection_of_command_documentations.h"
#include "loading_of_data.h"
#include "scoring_of_data_manager_using_voromqa.h"

namespace scripting
{

class CustomsCommandsForExtraActions
{
public:
	class reset_time : public GenericCommand
	{
	public:
		explicit reset_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			elapsed_processor_time_.reset();
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};

	class print_time : public GenericCommand
	{
	public:
		explicit print_time(auxiliaries::ElapsedProcessorTime& elapsed_processor_time) :
			elapsed_processor_time_(elapsed_processor_time)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const bool reset=cargs.input.get_flag("reset");
			cargs.input.assert_nothing_unusable();
			cargs.heterostorage.variant_object.value("elapsed_miliseconds")=elapsed_processor_time_.elapsed_miliseconds();
			if(reset)
			{
				elapsed_processor_time_.reset();
			}
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};

	class exit : public GenericCommand
	{
	public:
		explicit exit(bool& exit_status) :
			exit_status_(exit_status)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();
			exit_status_=true;
		}

	private:
		bool& exit_status_;
	};

	class echo : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& strings=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			for(std::size_t i=0;i<strings.size();i++)
			{
				cargs.heterostorage.variant_object.values_array("lines").push_back(VariantValue(strings[i]));
			}
		}
	};

	class list_virtual_files : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			cargs.input.assert_nothing_unusable();

			for(std::map<std::string, std::string>::const_iterator it=VirtualFileStorage::files().begin();it!=VirtualFileStorage::files().end();++it)
			{
				VariantObject info;
				info.value("name")=it->first;
				info.value("bytes")=it->second.size();
				cargs.heterostorage.variant_object.objects_array("files").push_back(info);
			}

			cargs.heterostorage.variant_object.value("total_count")=VirtualFileStorage::files().size();
			cargs.heterostorage.variant_object.value("total_bytes")=VirtualFileStorage::count_bytes();
		}
	};

	class upload_virtual_file : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string name=cargs.input.get_value<std::string>("name");
			const std::string file=cargs.input.get_value<std::string>("file");

			cargs.input.assert_nothing_unusable();

			VirtualFileStorage::assert_filename_is_valid(name);

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty input file name."));
			}

			std::ifstream finput(file.c_str(), std::ios::in);

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
			}

			std::istreambuf_iterator<char> eos;
			std::string data(std::istreambuf_iterator<char>(finput), eos);

			VirtualFileStorage::set_file(name, data);
		}
	};

	class download_virtual_file : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string name=cargs.input.get_value<std::string>("name");
			const std::string file=cargs.input.get_value<std::string>("file");

			cargs.input.assert_nothing_unusable();

			VirtualFileStorage::assert_file_exists(name);

			if(file.empty())
			{
				throw std::runtime_error(std::string("Empty output file name."));
			}

			std::ofstream foutput(file.c_str(), std::ios::out);

			if(!foutput.good())
			{
				throw std::runtime_error(std::string("Failed to write to file '")+file+"'.");
			}

			foutput << VirtualFileStorage::get_file(name);
		}
	};

	class print_virtual_file : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string filename=cargs.input.get_value_or_first_unused_unnamed_value("name");
			const bool line_by_line=cargs.input.get_flag("line-by-line");

			cargs.input.assert_nothing_unusable();

			cargs.heterostorage.variant_object.value("name")=filename;

			if(line_by_line)
			{
				std::istringstream input(VirtualFileStorage::get_file(filename));
				while(input.good())
				{
					std::string line;
					std::getline(input, line);
					cargs.heterostorage.variant_object.values_array("lines").push_back(VariantValue(line));
				}
			}
			else
			{
				cargs.heterostorage.variant_object.value("data")=VirtualFileStorage::get_file(filename);
			}
		}
	};

	class delete_virtual_files : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& filenames=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();

			cargs.input.assert_nothing_unusable();

			if(filenames.empty())
			{
				VirtualFileStorage::clear();
			}
			else
			{
				for(std::size_t i=0;i<filenames.size();i++)
				{
					VirtualFileStorage::assert_file_exists(filenames[i]);
				}

				for(std::size_t i=0;i<filenames.size();i++)
				{
					VirtualFileStorage::delete_file(filenames[i]);
				}
			}
		}
	};

	class setup_loading : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			LoadingOfData::Configuration config;
			config.include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
			config.include_hydrogens=cargs.input.get_flag("include-hydrogens");
			config.multimodel_chains=cargs.input.get_flag("as-assembly");
			const std::string radii_file=cargs.input.get_value_or_default<std::string>("radii-file", "");
			const double default_radius=cargs.input.get_value_or_default<double>("default-radius", LoadingOfData::Configuration::recommended_default_radius());
			const bool only_default_radius=cargs.input.get_flag("same-radius-for-all");

			cargs.input.assert_nothing_unusable();

			if(!radii_file.empty())
			{
				InputSelector finput_selector(radii_file);
				std::istream& radii_file_stream=finput_selector.stream();
				if(!radii_file_stream.good())
				{
					throw std::runtime_error(std::string("Failed to read radii file."));
				}
				config.atom_radius_assigner=LoadingOfData::Configuration::generate_atom_radius_assigner(default_radius, only_default_radius, &radii_file_stream);
			}
			else
			{
				config.atom_radius_assigner=LoadingOfData::Configuration::generate_atom_radius_assigner(default_radius, only_default_radius, 0);
			}

			LoadingOfData::Configuration::setup_default_configuration(config);
		}
	};

	class setup_voromqa : public GenericCommand
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::string potential_file=cargs.input.get_value<std::string>("potential");
			const std::string means_and_sds_file=cargs.input.get_value<std::string>("means-and-sds");

			cargs.input.assert_nothing_unusable();

			if(!ScoringOfDataManagerUsingVoroMQA::Configuration::setup_default_configuration(potential_file, means_and_sds_file))
			{
				throw std::runtime_error(std::string("Failed to setup VoroMQA."));
			}
		}
	};

	class explain_command : public GenericCommand
	{
	public:
		explicit explain_command(CollectionOfCommandDocumentations& collection_of_docs) :
			collection_of_command_documentations_(collection_of_docs)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const std::string name=cargs.input.get_value_or_first_unused_unnamed_value("name");

			cargs.input.assert_nothing_unusable();

			if(collection_of_command_documentations_.map_of_documentations().count(name)==0)
			{
				throw std::runtime_error(std::string("Invalid command name '")+name+"'.");
			}

			const CommandDocumentation doc=collection_of_command_documentations_.get_documentation(name);

			VariantObject& info=cargs.heterostorage.variant_object;

			info.value("command")=name;

			info.value("description")=doc.get_full_description();

			if(!doc.get_option_descriptions().empty())
			{
				std::vector<VariantObject>& output_array=info.objects_array("options");
				output_array.reserve(doc.get_option_descriptions().size());
				for(std::size_t i=0;i<doc.get_option_descriptions().size();i++)
				{
					const CommandDocumentation::OptionDescription& od=doc.get_option_descriptions()[i];
					VariantObject obj;
					obj.value("name")=od.name;
					obj.value("required")=od.required;
					obj.value("type")=od.value_type;
					obj.value("description")=od.description;
					output_array.push_back(obj);
				}
			}
		}

	private:
		const CollectionOfCommandDocumentations& collection_of_command_documentations_;
	};

	class list_commands : public GenericCommand
	{
	public:
		explicit list_commands(CollectionOfCommandDocumentations& collection_of_docs) :
			collection_of_command_documentations_(collection_of_docs)
		{
		}

	protected:
		void run(CommandArguments& cargs)
		{
			const bool compact=cargs.input.get_flag("compact");

			cargs.input.assert_nothing_unusable();

			const std::vector<std::string> names=collection_of_command_documentations_.get_all_names();

			if(names.empty())
			{
				throw std::runtime_error(std::string("No commands documented."));
			}

			if(compact)
			{
				std::vector<VariantValue>& values_array=cargs.heterostorage.variant_object.values_array("names");

				for(std::size_t i=0;i<names.size();i++)
				{
					values_array.push_back(VariantValue(names[i]));
				}
			}
			else
			{
				std::vector<VariantObject>& output_array=cargs.heterostorage.variant_object.objects_array("outlines");

				for(std::size_t i=0;i<names.size();i++)
				{
					const CommandDocumentation doc=collection_of_command_documentations_.get_documentation(names[i]);
					VariantObject obj;
					obj.value("name")=names[i];
					obj.value("short_description")=doc.get_short_description();
					output_array.push_back(obj);
				}
			}
		}

	private:
		const CollectionOfCommandDocumentations& collection_of_command_documentations_;
	};
};

}

#endif /* SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_ */

