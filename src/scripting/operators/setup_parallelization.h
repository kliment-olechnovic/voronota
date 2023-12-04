#ifndef SCRIPTING_OPERATORS_SETUP_PARALLELIZATION_H_
#define SCRIPTING_OPERATORS_SETUP_PARALLELIZATION_H_

#include "../operators_common.h"
#include "../parallelization.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupParallelization : public OperatorBase<SetupParallelization>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		bool possible;
		Parallelization::Configuration used_config;

		Result() : possible(false)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("possible")=possible;
			heterostorage.variant_object.value("processors")=used_config.processors;
			heterostorage.variant_object.value("dynamic")=used_config.dynamic_adjustment;
			heterostorage.variant_object.value("in_script")=used_config.in_script;
		}
	};

	Parallelization::Configuration config;

	SetupParallelization()
	{
	}

	void initialize(CommandInput& input)
	{
		config=Parallelization::Configuration();
		config.processors=input.get_value_or_default<int>("processors", config.processors);
		config.dynamic_adjustment=input.get_flag("dynamic");
		config.in_script=input.get_flag("in-script");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("processors", CDOD::DATATYPE_INT, "number of processors", Parallelization::Configuration().processors));
		doc.set_option_decription(CDOD("dynamic", CDOD::DATATYPE_BOOL, "flag to enable dynamic adjustment and interpret number of processors as the upper limit"));
		doc.set_option_decription(CDOD("in-script", CDOD::DATATYPE_BOOL, "flag to enable in-script parallelization"));
	}

	Result run(void*) const
	{
		Parallelization::setup_and_apply_default_configuration(config);

		Result result;
		result.possible=Parallelization::possible();
		result.used_config=Parallelization::Configuration::get_default_configuration();

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_PARALLELIZATION_H_ */
