#ifndef DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_QUALITY_SCORES_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_QUALITY_SCORES_H_

#include "../../../../src/scripting/operators/construct_contacts.h"
#include "../../../../src/scripting/operators/voromqa_global.h"
#include "../../../../src/scripting/operators/export_atoms_and_contacts.h"

#include "../cache_file.h"

#include "voromqa_dark_global.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConstructOrLoadQualityScores : public scripting::OperatorBase<ConstructOrLoadQualityScores>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string cache_file_path;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("cache_file_path")=cache_file_path;
		}
	};

	std::string cache_dir;

	ConstructOrLoadQualityScores()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		cache_dir=input.get_value_or_default<std::string>("cache-dir", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("cache-dir", CDOD::DATATYPE_STRING, "path to cache directory", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		CacheFile cache_file;

		if(!cache_dir.empty())
		{
			std::ostringstream data_for_checksum;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				const scripting::Atom& a=data_manager.atoms()[i];
				data_for_checksum << a.crad << " " << a.value.x << " " << a.value.y << " " << a.value.z << " " << a.value.r << "\n";
			}
			data_for_checksum << "quality_scores";
			cache_file=CacheFile(cache_dir, data_for_checksum.str(), ".pac");
		}

		if(cache_file.file_available())
		{
			scripting::operators::Import().init(CMDIN().set("file", cache_file.file_path())).run(data_manager);
		}
		else
		{
			{
				scripting::operators::ConstructContacts().init().run(data_manager);
				scripting::operators::VoroMQAGlobal().init().run(data_manager);
				operators::VoroMQADarkGlobal().init().run(data_manager);
			}
			if(!cache_file.file_path().empty())
			{
				scripting::operators::ExportAtomsAndContacts().init(CMDIN().set("file", cache_file.file_path())).run(data_manager);
			}
		}

		Result result;
		result.cache_file_path=cache_file.file_path();

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_QUALITY_SCORES_H_ */
