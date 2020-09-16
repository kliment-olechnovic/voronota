#ifndef DUKTAPER_OPERATORS_CHECKSUM_H_
#define DUKTAPER_OPERATORS_CHECKSUM_H_

#include <picosha2/picosha2.h>

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class Checksum : public scripting::OperatorBase<Checksum>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string checksum;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("checksum")=checksum;
		}
	};

	std::string data_file;
	std::string data_string;

	Checksum()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		data_file=input.get_value_or_default<std::string>("data-file", "");
		data_string=input.get_value_or_default<std::string>("data-string", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("data-file", CDOD::DATATYPE_STRING, "path to data file", ""));
		doc.set_option_decription(CDOD("data-string", CDOD::DATATYPE_STRING, "data string", ""));
	}

	Result run(void*) const
	{
		if(data_file.empty() && data_string.empty())
		{
			throw std::runtime_error(std::string("No data provided."));
		}

		std::string full_data;

		if(!data_file.empty())
		{
			scripting::InputSelector finput_selector(data_file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+data_file+"'.");
			}

			std::istreambuf_iterator<char> eos;
			std::string data(std::istreambuf_iterator<char>(finput), eos);

			full_data.swap(data);
		}

		full_data+=data_string;

		Result result;

		result.checksum=picosha2::hash256_hex_string(full_data);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CHECKSUM_H_ */
