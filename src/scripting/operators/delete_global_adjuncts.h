#ifndef SCRIPTING_OPERATORS_DELETE_GLOBAL_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_DELETE_GLOBAL_ADJUNCTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DeleteGlobalAdjuncts : public OperatorBase<DeleteGlobalAdjuncts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	bool all;
	std::vector<std::string> adjuncts;

	DeleteGlobalAdjuncts() : all(false)
	{
	}

	void initialize(CommandInput& input)
	{
		all=input.get_flag("all");
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("all", CDOD::DATATYPE_BOOL, "flag to delete all adjuncts"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "adjunct names", ""));
	}

	Result run(DataManager& data_manager) const
	{
		if(!all && adjuncts.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		if(all && !adjuncts.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of adjuncts."));
		}

		std::vector<std::string> adjuncts_filled;

		if(all)
		{
			for(std::map<std::string, double>::const_iterator it=data_manager.global_numeric_adjuncts().begin();it!=data_manager.global_numeric_adjuncts().end();++it)
			{
				adjuncts_filled.push_back(it->first);
			}
		}
		else
		{
			adjuncts_filled=adjuncts;
		}

		if(adjuncts_filled.empty())
		{
			throw std::runtime_error(std::string("No adjuncts specified."));
		}

		for(std::size_t j=0;j<adjuncts_filled.size();j++)
		{
			data_manager.global_numeric_adjuncts_mutable().erase(adjuncts_filled[j]);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_GLOBAL_ADJUNCTS_H_ */
