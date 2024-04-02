#ifndef DUKTAPER_OPERATORS_SETUP_DEFAULTS_REQUESTED_H_
#define DUKTAPER_OPERATORS_SETUP_DEFAULTS_REQUESTED_H_

#include "../../../../src/scripting/operators/setup_akbps.h"

#include "../stocked_data_resources.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class SetupDefaultsRequested : public scripting::OperatorBase<SetupDefaultsRequested>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	bool load_akbps;

	SetupDefaultsRequested() : load_akbps(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		load_akbps=input.get_flag("load-akbps");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("load-akbps", CDOD::DATATYPE_BOOL, "flag to load area KBPs"));
	}

	Result run(void*) const
	{
		if(load_akbps)
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_akbps;
			voronota::scripting::VirtualFileStorage::set_file(tmp_akbps.filename(), voronota::duktaper::resources::data_voromqalike_area_kbps());
			scripting::operators::SetupAKBPs().init(CMDIN().set("potentials", tmp_akbps.filename())).run(0);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SETUP_DEFAULTS_REQUESTED_H_ */
