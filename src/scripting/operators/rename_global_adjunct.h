#ifndef SCRIPTING_OPERATORS_RENAME_GLOBAL_ADJUNCT_H_
#define SCRIPTING_OPERATORS_RENAME_GLOBAL_ADJUNCT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RenameGlobalAdjunct : public OperatorBase<RenameGlobalAdjunct>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name_original;
	std::string name_new;

	RenameGlobalAdjunct()
	{
	}

	void initialize(CommandInput& input)
	{
		name_original=input.get_value_or_first_unused_unnamed_value("original");
		name_new=input.get_value_or_first_unused_unnamed_value("new");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("original", CDOD::DATATYPE_STRING, "original selection name"));
		doc.set_option_decription(CDOD("new", CDOD::DATATYPE_STRING, "new selection name"));
	}

	Result run(DataManager& data_manager) const
	{
		if(name_original.empty())
		{
			throw std::runtime_error(std::string("Empty first name provided for renaming."));
		}

		if(name_new.empty())
		{
			throw std::runtime_error(std::string("Empty second name provided for renaming."));
		}

		assert_adjunct_name_input(name_new, false);

		if(data_manager.global_numeric_adjuncts().count(name_original)==0)
		{
			throw std::runtime_error(std::string("No global adjunct '")+name_original+"'");
		}

		if(name_original!=name_new)
		{
			const double value=data_manager.global_numeric_adjuncts().find(name_original)->second;
			data_manager.global_numeric_adjuncts_mutable().erase(name_original);
			data_manager.global_numeric_adjuncts_mutable()[name_new]=value;
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RENAME_GLOBAL_ADJUNCT_H_ */
