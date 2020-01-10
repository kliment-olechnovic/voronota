#ifndef SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportSelectionOfContacts : public OperatorBase<ImportSelectionOfContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;
		std::string selection_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			if(selection_name.empty())
			{
				heterostorage.variant_object.value("selection_name").set_null();
			}
			else
			{
				heterostorage.variant_object.value("selection_name")=selection_name;
			}
		}
	};

	std::string file;
	std::string name;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;

	ImportSelectionOfContacts() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		name=input.get_value<std::string>("name");
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "selection name"));
		doc.set_option_decription(CDOD("no-serial", CDOD::DATATYPE_BOOL, "flag to exclude atom serials"));
		doc.set_option_decription(CDOD("no-name", CDOD::DATATYPE_BOOL, "flag to exclude atom names"));
		doc.set_option_decription(CDOD("no-resSeq", CDOD::DATATYPE_BOOL, "flag to exclude residue sequence numbers"));
		doc.set_option_decription(CDOD("no-resName", CDOD::DATATYPE_BOOL, "flag to exclude residue names"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input selection file name."));
		}

		assert_selection_name_input(name, false);

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::set<common::ChainResidueAtomDescriptorsPair> set_of_crads_pairs;

		auxiliaries::IOUtilities().read_lines_to_set(finput, set_of_crads_pairs);

		if(set_of_crads_pairs.empty())
		{
			throw std::runtime_error(std::string("No descriptors in file '")+file+"'.");
		}

		if(no_serial || no_name || no_resSeq || no_resName)
		{
			std::set<common::ChainResidueAtomDescriptorsPair> refined_set_of_crads_pairs;
			for(std::set<common::ChainResidueAtomDescriptorsPair>::const_iterator it=set_of_crads_pairs.begin();it!=set_of_crads_pairs.end();++it)
			{
				refined_set_of_crads_pairs.insert(it->without_some_info(no_serial, no_name, no_resSeq, no_resName));
			}
			set_of_crads_pairs.swap(refined_set_of_crads_pairs);
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts_by_set_of_crads_pairs(set_of_crads_pairs);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_contacts_selection(name, ids);
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);
		result.selection_name=name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_CONTACTS_H_ */
