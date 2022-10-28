#ifndef DUKTAPER_OPERATORS_IMPORT_URL_H_
#define DUKTAPER_OPERATORS_IMPORT_URL_H_

#include "../remote_import_preparation.h"

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

		RemoteImportPreparation remote_input_preparation;
		remote_input_preparation.add_request(RemoteImportPreparation::Request(url, import_many_operator));

		RemoteImportPreparation::Request* downloaded_request=remote_input_preparation.download_request_until_first_success();

		if(downloaded_request==0)
		{
			throw std::runtime_error(std::string("No data downloaded."));
		}

		Result result;
		result.url=url;
		result.import_result=downloaded_request->import_downloaded_data(congregation_of_data_managers);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_IMPORT_URL_H_ */
