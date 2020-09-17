#ifndef DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CONTACTS_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CONTACTS_H_

#include "../../../../src/scripting/operators/export_atoms_and_contacts.h"

#include "../cache_file.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConstructOrLoadContacts : public scripting::OperatorBase<ConstructOrLoadContacts>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		scripting::SummaryOfContacts contacts_summary;
		std::string cache_file_path;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			heterostorage.variant_object.value("cache_file_path")=cache_file_path;
		}
	};

	scripting::operators::ConstructContacts construct_contacts_operator;
	std::string cache_dir;

	ConstructOrLoadContacts()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		construct_contacts_operator.initialize(input);
		cache_dir=input.get_value_or_default<std::string>("cache-dir", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		construct_contacts_operator.document(doc);
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
			data_for_checksum << "construct_contacts_parameters";
			data_for_checksum << " " << construct_contacts_operator.parameters_to_construct_contacts.probe;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_construct_contacts.step;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_construct_contacts.projections;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_construct_contacts.sih_depth;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_enhance_contacts.tag_centrality;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_enhance_contacts.tag_peripherial;
			data_for_checksum << " " << construct_contacts_operator.parameters_to_enhance_contacts.adjunct_solvent_direction;
			cache_file=CacheFile(cache_dir, data_for_checksum.str(), ".pac");
		}

		if(cache_file.file_available() && !construct_contacts_operator.force)
		{
			scripting::operators::Import().init(CMDIN().set("file", cache_file.file_path())).run(data_manager);
		}
		else
		{
			construct_contacts_operator.run(data_manager);
			if(!cache_file.file_path().empty())
			{
				scripting::operators::ExportAtomsAndContacts().init(CMDIN().set("file", cache_file.file_path())).run(data_manager);
			}
		}

		Result result;
		result.cache_file_path=cache_file.file_path();
		result.contacts_summary=scripting::SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CONTACTS_H_ */
