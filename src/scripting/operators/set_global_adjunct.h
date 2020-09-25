#ifndef SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_H_
#define SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetGlobalAdjunct : public OperatorBase<SetGlobalAdjunct>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name;
	bool value_present;
	double value;
	bool remove;

	SetGlobalAdjunct() : value_present(false), value(0.0), remove(false)
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		value_present=input.is_option("value");
		value=input.get_value_or_default<double>("value", 0.0);
		remove=input.get_flag("remove");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("value", CDOD::DATATYPE_FLOAT, "adjunct value", ""));
		doc.set_option_decription(CDOD("remove", CDOD::DATATYPE_BOOL, "flag to remove adjunct"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(value_present && remove)
		{
			throw std::runtime_error(std::string("Value setting and removing options used together."));
		}

		assert_adjunct_name_input(name, false);

		if(remove)
		{
			data_manager.global_numeric_adjuncts_mutable().erase(name);
		}
		else
		{
			data_manager.global_numeric_adjuncts_mutable()[name]=value;
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_GLOBAL_ADJUNCT_H_ */
