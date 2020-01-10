#ifndef SCRIPTING_OPERATORS_IMPORT_CONTACTS_H_
#define SCRIPTING_OPERATORS_IMPORT_CONTACTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportContacts : public OperatorBase<ImportContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	std::string file;

	ImportContacts()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input contacts file name."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::vector<Contact> contacts;

		auxiliaries::IOUtilities().read_lines_to_set(finput, contacts);

		if(!contacts.empty())
		{
			data_manager.reset_contacts_by_swapping(contacts);
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read contacts from file '")+file+"'.");
		}

		Result result;
		result.file=file;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_CONTACTS_H_ */
