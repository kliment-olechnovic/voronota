#ifndef DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CACHED_CONTACTS_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CACHED_CONTACTS_H_

#include "../../../../src/scripting/operators_all.h"

#include "../stocked_data_resources.h"

#include "checksum.h"
#include "call_shell.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConstructOrLoadCachedContacts : public scripting::operators::OperatorBase<ConstructOrLoadCachedContacts>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		scripting::SummaryOfContacts contacts_summary;
		std::string created_cache_file;
		std::string loaded_cache_file;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			heterostorage.variant_object.value("created_cache_file")=created_cache_file;
			heterostorage.variant_object.value("used_cache_file")=loaded_cache_file;
		}
	};

	scripting::operators::ConstructContacts construct_contacts_operator;
	std::string cache_dir;

	ConstructOrLoadCachedContacts()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		construct_contacts_operator.initialize(input);
		cache_dir=input.get_value<std::string>("cache-dir");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		construct_contacts_operator.document(doc);
		doc.set_option_decription(scripting::CDOD("cache-dir", scripting::CDOD::DATATYPE_STRING, "path to cache directory"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		if(cache_dir.empty())
		{
			throw std::runtime_error(std::string("No cache directory provided."));
		}

		data_manager.assert_atoms_availability();

		std::string checksum;

		{
			std::ostringstream summary_output;

			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				const scripting::Atom& a=data_manager.atoms()[i];
				summary_output << a.crad << " " << a.value.x << " " << a.value.y << " " << a.value.z << " " << a.value.r << "\n";
			}

			summary_output << "construct_contacts_parameters";
			summary_output << "_" << construct_contacts_operator.parameters_to_construct_contacts.probe;
			summary_output << "_" << construct_contacts_operator.parameters_to_construct_contacts.step;
			summary_output << "_" << construct_contacts_operator.parameters_to_construct_contacts.projections;
			summary_output << "_" << construct_contacts_operator.parameters_to_construct_contacts.sih_depth;
			summary_output << "_" << construct_contacts_operator.parameters_to_enhance_contacts.tag_centrality;
			summary_output << "_" << construct_contacts_operator.parameters_to_enhance_contacts.tag_peripherial;
			summary_output << "_" << construct_contacts_operator.parameters_to_enhance_contacts.adjunct_solvent_direction;

			scripting::VirtualFileStorage::TemporaryFile tmp_file_for_checksum;
			scripting::VirtualFileStorage::set_file(tmp_file_for_checksum.filename(), summary_output.str());

			std::ostringstream args;
			args << " -data-file " << tmp_file_for_checksum.filename();
			checksum=Checksum().init(args.str()).run(0).checksum;
		}

		if(checksum.empty())
		{
			throw std::runtime_error(std::string("Failed to compute checksum."));
		}

		const std::string cache_file=cache_dir+"/"+checksum+".pac";

		bool cache_file_available=false;

		{
			std::ostringstream args;
			args << " -command-string \"test \\-s '" << cache_file << "'\"";
			cache_file_available=CallShell().init(args.str()).run(0).exit_status==0;
		}

		Result result;

		if(!cache_file_available || construct_contacts_operator.force)
		{
			construct_contacts_operator.run(data_manager);
			{
				std::ostringstream args;
				args << " -command-string \"mkdir \\-p '" << cache_dir << "'\"";
				CallShell().init(args.str()).run(0);
			}
			{
				std::ostringstream args;
				args << " -file " << cache_file;
				scripting::operators::ExportAtomsAndContacts().init(args.str()).run(data_manager);
			}
			result.created_cache_file=cache_file;
		}
		else
		{
			std::ostringstream args;
			args << " -file " << cache_file;
			scripting::operators::Import().init(args.str()).run(data_manager);
			result.loaded_cache_file=cache_file;
		}

		result.contacts_summary=scripting::SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CONSTRUCT_OR_LOAD_CACHED_CONTACTS_H_ */
