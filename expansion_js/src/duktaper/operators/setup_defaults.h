#ifndef DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_
#define DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_

#include "../../../../src/scripting/operators/setup_loading.h"
#include "../../../../src/scripting/operators/setup_voromqa.h"

#include "../stocked_data_resources.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class SetupDefaults : public scripting::OperatorBase<SetupDefaults>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	SetupDefaults()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(void*) const
	{
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_radii;
			voronota::scripting::VirtualFileStorage::set_file(tmp_radii.filename(), voronota::duktaper::resources::data_radii());
			scripting::operators::SetupLoading().init(CMDIN().set("radii-file", tmp_radii.filename())).run(0);
		}

		{
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_potential;
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_potential_alt;
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_means_and_sds;
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_potential.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_potential());
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_potential_alt.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_potential_alt());
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_means_and_sds.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_means_and_sds());
			scripting::operators::SetupVoroMQA().init(CMDIN()
					.set("potential", tmp_voromqa_v1_energy_potential.filename())
					.set("potential-alt", tmp_voromqa_v1_energy_potential_alt.filename())
					.set("means-and-sds", tmp_voromqa_v1_energy_means_and_sds.filename())).run(0);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_ */
