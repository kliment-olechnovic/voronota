#ifndef SCRIPTING_OPERATORS_PRINT_GLOBAL_ADJUNCTS_H_
#define SCRIPTING_OPERATORS_PRINT_GLOBAL_ADJUNCTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class PrintGlobalAdjuncts : public OperatorBase<PrintGlobalAdjuncts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::map<std::string, double> numeric_adjuncts;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantObject& obj=heterostorage.variant_object.object("numeric_adjuncts");
			for(std::map<std::string, double>::const_iterator it=numeric_adjuncts.begin();it!=numeric_adjuncts.end();++it)
			{
				obj.value(it->first)=it->second;
			}
		}
	};

	PrintGlobalAdjuncts()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(DataManager& data_manager) const
	{
		Result result;
		result.numeric_adjuncts=data_manager.global_numeric_adjuncts();
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_PRINT_GLOBAL_ADJUNCTS_H_ */
