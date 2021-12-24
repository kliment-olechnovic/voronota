#ifndef DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_
#define DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_

#include "../../../../src/scripting/operators/setup_loading.h"
#include "../../../../src/scripting/operators/setup_voromqa.h"
#include "../../../../src/scripting/operators/setup_mock_voromqa.h"

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

	bool no_load_voromqa_potentials;
	bool no_load_alt_voromqa_potential;
	bool faster_load_voromqa_potentials;
	bool no_load_more_atom_types;
	bool no_load_mock_voromqa_potential;

	SetupDefaults() : no_load_voromqa_potentials(false), no_load_alt_voromqa_potential(false), faster_load_voromqa_potentials(false), no_load_more_atom_types(false), no_load_mock_voromqa_potential(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		no_load_voromqa_potentials=input.get_flag("no-load-voromqa-potentials");
		no_load_alt_voromqa_potential=input.get_flag("no-load-alt-voromqa-potential");
		faster_load_voromqa_potentials=input.get_flag("faster-load-voromqa-potentials");
		no_load_more_atom_types=input.get_flag("no-load-more-atom-types");
		no_load_mock_voromqa_potential=input.get_flag("no-load-mock-voromqa-potential");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("no-load-voromqa-potentials", CDOD::DATATYPE_BOOL, "flag to not load VoroMQA potentials, to save time"));
		doc.set_option_decription(CDOD("no-load-alt-voromqa-potential", CDOD::DATATYPE_BOOL, "flag to not load alternative VoroMQA potential, to save time"));
		doc.set_option_decription(CDOD("faster-load-voromqa-potentials", CDOD::DATATYPE_BOOL, "flag to load VoroMQA potentials faster"));
		doc.set_option_decription(CDOD("no-load-more-atom-types", CDOD::DATATYPE_BOOL, "flag to not load more atom types, to save time"));
		doc.set_option_decription(CDOD("no-load-mock-voromqa-potential", CDOD::DATATYPE_BOOL, "flag to not load mock VoroMQA potential, to save time"));
	}

	Result run(void*) const
	{
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_radii;
			voronota::scripting::VirtualFileStorage::set_file(tmp_radii.filename(), voronota::duktaper::resources::data_radii());
			scripting::operators::SetupLoading().init(CMDIN().set("radii-file", tmp_radii.filename())).run(0);
		}

		if(!no_load_voromqa_potentials)
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_potential;
			scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_means_and_sds;
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_potential.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_potential());
			voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_means_and_sds.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_means_and_sds());
			if(no_load_alt_voromqa_potential)
			{
				scripting::operators::SetupVoroMQA().init(CMDIN()
						.set("potential", tmp_voromqa_v1_energy_potential.filename())
						.set("means-and-sds", tmp_voromqa_v1_energy_means_and_sds.filename())
						.set("faster", faster_load_voromqa_potentials)).run(0);
			}
			else
			{
				scripting::VirtualFileStorage::TemporaryFile tmp_voromqa_v1_energy_potential_alt;
				voronota::scripting::VirtualFileStorage::set_file(tmp_voromqa_v1_energy_potential_alt.filename(), voronota::duktaper::resources::data_voromqa_v1_energy_potential_alt());
				scripting::operators::SetupVoroMQA().init(CMDIN()
						.set("potential", tmp_voromqa_v1_energy_potential.filename())
						.set("potential-alt", tmp_voromqa_v1_energy_potential_alt.filename())
						.set("means-and-sds", tmp_voromqa_v1_energy_means_and_sds.filename())
						.set("faster", faster_load_voromqa_potentials)).run(0);
			}
		}

		if(!no_load_more_atom_types)
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_more_atom_types;
			voronota::scripting::VirtualFileStorage::set_file(tmp_more_atom_types.filename(), voronota::duktaper::resources::data_more_atom_types());
			scripting::operators::SetupChemistryAnnotating().init(CMDIN().set("more-atom-types-file", tmp_more_atom_types.filename())).run(0);
		}

		if(!no_load_mock_voromqa_potential)
		{
			scripting::VirtualFileStorage::TemporaryFile tmp_mock_voromqa_energy_potential;
			voronota::scripting::VirtualFileStorage::set_file(tmp_mock_voromqa_energy_potential.filename(), voronota::duktaper::resources::data_mock_voromqa_energy_potential());
			scripting::operators::SetupMockVoroMQA().init(CMDIN().set("potential", tmp_mock_voromqa_energy_potential.filename())).run(0);
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_SETUP_DEFAULTS_H_ */
