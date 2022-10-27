#ifndef DUKTAPER_OPERATORS_IMPORT_URL_H_
#define DUKTAPER_OPERATORS_IMPORT_URL_H_

#include "../../../../src/scripting/operators/import_many.h"

#include "../file_downloader.h"

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
		scripting::operators::ImportMany::Result import_result;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("url")=url;
			import_result.store(heterostorage);
		}
	};

	std::string url;
	scripting::operators::ImportMany import_many_operator;

	ImportUrl()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		url=input.get_value_or_first_unused_unnamed_value("url");
		import_many_operator.initialize(input, true);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("url", CDOD::DATATYPE_STRING, "resource URL"));
		import_many_operator.document(doc, true);
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

		scripting::operators::ImportMany import_many_operator_to_use=import_many_operator;

		const std::string url_basename=scripting::OperatorsUtilities::remove_suffix(scripting::OperatorsUtilities::get_basename_from_path(url), ".gz");

		scripting::VirtualFileStorage::TemporaryFile tmpfile(url_basename);

		Result result;
		result.url=url;

		if(url[0]=='.' || url[0]=='/')
		{
			import_many_operator_to_use.files.push_back(url);
		}
		else
		{
			std::string download_result;
			if(FileDownloader::download_file(url, true, download_result))
			{
				scripting::VirtualFileStorage::set_file(tmpfile.filename(), download_result);
				import_many_operator_to_use.files.push_back(tmpfile.filename());
			}
			else
			{
				throw std::runtime_error(std::string("No data downloaded."));
			}
		}

		if(import_many_operator_to_use.import_operator.title.empty())
		{
			import_many_operator_to_use.import_operator.title=url_basename;
		}

		result.import_result=import_many_operator_to_use.run(congregation_of_data_managers);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_IMPORT_URL_H_ */
