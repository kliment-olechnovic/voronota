#ifndef SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportSelectionOfContacts : public OperatorBase<ExportSelectionOfContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfContacts contacts_summary;
		std::size_t number_of_descriptors_written;

		Result() : number_of_descriptors_written(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			heterostorage.variant_object.value("number_of_descriptors_written")=number_of_descriptors_written;
		}
	};

	std::string file;
	SelectionManager::Query parameters_for_selecting;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;

	ExportSelectionOfContacts() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("no-serial", CDOD::DATATYPE_BOOL, "flag to exclude atom serials"));
		doc.set_option_decription(CDOD("no-name", CDOD::DATATYPE_BOOL, "flag to exclude atom names"));
		doc.set_option_decription(CDOD("no-resSeq", CDOD::DATATYPE_BOOL, "flag to exclude residue sequence numbers"));
		doc.set_option_decription(CDOD("no-resName", CDOD::DATATYPE_BOOL, "flag to exclude residue names"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_file_name_input(file, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		std::set<common::ChainResidueAtomDescriptorsPair> set_of_crads_pairs;

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			set_of_crads_pairs.insert(
					common::ConversionOfDescriptors::get_contact_descriptor(
							data_manager.atoms(), data_manager.contacts()[*it]).without_some_info(
									no_serial, no_name, no_resSeq, no_resName));
		}

		auxiliaries::IOUtilities().write_set(set_of_crads_pairs, output);

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);
		result.number_of_descriptors_written=set_of_crads_pairs.size();

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_CONTACTS_H_ */
