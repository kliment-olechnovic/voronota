#ifndef SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_
#define SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportObjects : public OperatorBase<ImportObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string file;

	ImportObjects()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		congregation_of_data_managers.load_from_stream(finput);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_ */
