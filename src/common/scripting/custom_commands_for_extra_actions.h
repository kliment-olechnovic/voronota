#ifndef COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_
#define COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_

#include "../../auxiliaries/time_utilities.h"

#include "generic_command_for_extra_actions.h"
#include "loading_of_data.h"
#include "scoring_of_data_manager_using_voromqa.h"

namespace common
{

namespace scripting
{

class CustomsCommandsForExtraActions
{
public:
	class reset_time : public GenericCommandForExtraActions
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

	class print_time : public GenericCommandForExtraActions
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
			cargs.output_for_log << "Elapsed time: " << elapsed_processor_time_.elapsed_miliseconds() << " ms\n";
			if(reset)
			{
				elapsed_processor_time_.reset();
			}
		}

	private:
		auxiliaries::ElapsedProcessorTime& elapsed_processor_time_;
	};

	class exit : public GenericCommandForExtraActions
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

	class echo : public GenericCommandForExtraActions
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const std::vector<std::string>& strings=cargs.input.get_list_of_unnamed_values();
			cargs.input.mark_all_unnamed_values_as_used();
			cargs.input.assert_nothing_unusable();

			for(std::size_t i=0;i<strings.size();i++)
			{
				cargs.output_for_log << strings[i] << "\n";
			}
		}
	};

	class setup_loading : public GenericCommandForExtraActions
	{
	protected:
		void run(CommandArguments& cargs)
		{
			const bool include_heteroatoms=cargs.input.get_flag("include-heteroatoms");
			const bool include_hydrogens=cargs.input.get_flag("include-hydrogens");
			const bool multimodel_chains=cargs.input.get_flag("as-assembly");
			const std::string radii_file=cargs.input.get_value_or_default<std::string>("radii-file", "");
			const double default_radius=cargs.input.get_value_or_default<double>("default-radius", LoadingOfData::Configuration::recommended_default_radius());
			const bool only_default_radius=cargs.input.get_flag("same-radius-for-all");

			cargs.input.assert_nothing_unusable();

			if(!radii_file.empty())
			{
				std::ifstream radii_file_stream(radii_file.c_str(), std::ios::in);
				if(!radii_file_stream.good())
				{
					throw std::runtime_error(std::string("Failed to read radii file."));
				}
				LoadingOfData::Configuration::setup_default_configuration(
						include_heteroatoms, include_hydrogens, multimodel_chains, default_radius, only_default_radius, &radii_file_stream);
			}
			else
			{
				LoadingOfData::Configuration::setup_default_configuration(
						include_heteroatoms, include_hydrogens, multimodel_chains, default_radius, only_default_radius, 0);
			}
		}
	};

	class setup_voromqa : public GenericCommandForExtraActions
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
};

}

}

#endif /* COMMON_SCRIPTING_CUSTOM_COMMANDS_FOR_EXTRA_ACTIONS_H_ */

