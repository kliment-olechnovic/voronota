#ifndef DUKTAPER_OPERATORS_IMPORT_URL_H_
#define DUKTAPER_OPERATORS_IMPORT_URL_H_

#include "../../../../src/scripting/operators_all.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ImportUrl : public scripting::OperatorBase<ImportUrl>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string url;
		scripting::operators::Import::Result import_result;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("url")=url;
			import_result.store(heterostorage);
		}
	};

	std::string url;
	scripting::operators::Import import_operator;

	ImportUrl()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		url=input.get_value_or_first_unused_unnamed_value("url");
		import_operator.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("url", CDOD::DATATYPE_STRING, "resource URL"));
		import_operator.document(doc, true);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(url.empty())
		{
			throw std::runtime_error(std::string("Missing URL."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("curl"))
		{
			throw std::runtime_error(std::string("'curl' command not available."));
		}

		if(!CallShellUtilities::test_if_shell_command_available("zless"))
		{
			throw std::runtime_error(std::string("'zless' command not available."));
		}

		scripting::operators::Import import_operator_to_use=import_operator;

		scripting::VirtualFileStorage::TemporaryFile tmpfile;

		Result result;
		result.url=url;

		if(url[0]=='.' || url[0]=='/')
		{
			import_operator_to_use.loading_parameters.file=url;
		}
		else
		{
			std::ostringstream command_output;
			command_output << "curl '" << url << "' | zless";
			operators::CallShell::Result download_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(download_result.exit_status==0 && !download_result.stdout_str.empty())
			{
				scripting::VirtualFileStorage::set_file(tmpfile.filename(), download_result.stdout_str);
				import_operator_to_use.loading_parameters.file=tmpfile.filename();
			}
			else
			{
				throw std::runtime_error(std::string("No data downloaded."));
			}
		}

		if(import_operator_to_use.title.empty())
		{
			import_operator_to_use.title=scripting::Utilities::get_basename_from_path(url);
		}

		result.import_result=import_operator_to_use.run(congregation_of_data_managers);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_IMPORT_URL_H_ */
