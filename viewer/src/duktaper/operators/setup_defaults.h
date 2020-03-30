#ifndef DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_
#define DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_

#include "../../../src/scripting/operators_all.h"

#include "../stocked_data_resources.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class SetupDefaults : public scripting::operators::OperatorBase<SetupDefaults>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
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
			std::ostringstream args;
			args << "--radii-file " << tmp_radii.filename();
			scripting::operators::SetupLoading().init(args.str()).run(0);
		}

		{
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_potential;
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_means_and_sds;
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_potential.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_potential());
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_means_and_sds.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_means_and_sds());
			std::ostringstream args;
			args << "--potential " << tmp_voromqa_v1_energy_potential.filename();
			args << " --means-and-sds " << tmp_voromqa_v1_energy_means_and_sds.filename();
			scripting::operators::SetupVoroMQA().init(args.str()).run(0);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_ */
