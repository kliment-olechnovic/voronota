#ifndef SCRIPTING_OPERATORS_IMPORT_H_
#define SCRIPTING_OPERATORS_IMPORT_H_

#include "../operators_common.h"
#include "../loading_of_data.h"

namespace scripting
{

namespace operators
{

class Import : public OperatorBase<Import>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		SummaryOfContacts contacts_summary;
		std::string object_name;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms["loaded"]=atoms_summary;
			if(contacts_summary.number_total>0)
			{
				VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
				heterostorage.summaries_of_contacts["loaded"]=contacts_summary;
			}
			heterostorage.variant_object.value("object_name")=object_name;
			return (*this);
		}
	};

	LoadingOfData::Parameters params;
	std::string title;

	Import()
	{
	}

	Import& init(CommandInput& input)
	{
		params=LoadingOfData::Parameters();
		params.file=input.get_value_or_first_unused_unnamed_value("file");
		params.format=input.get_value_or_default<std::string>("format", "");
		params.forced_include_heteroatoms=input.is_option("include-heteroatoms");
		params.forced_include_hydrogens=input.is_option("include-hydrogens");
		params.forced_multimodel_chains=input.is_option("as-assembly");
		params.include_heteroatoms=input.get_flag("include-heteroatoms");
		params.include_hydrogens=input.get_flag("include-hydrogens");
		params.multimodel_chains=input.get_flag("as-assembly");
		title=(input.is_option("title") ? input.get_value<std::string>("title") : Utilities::get_basename_from_path(params.file));
		return (*this);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("format", CDOD::DATATYPE_STRING, "input file format", ""));
		doc.set_option_decription(CDOD("include-heteroatoms", CDOD::DATATYPE_BOOL, "flag to include heteroatoms"));
		doc.set_option_decription(CDOD("include-hydrogens", CDOD::DATATYPE_BOOL, "flag to include hydrogens"));
		doc.set_option_decription(CDOD("as-assembly", CDOD::DATATYPE_BOOL, "flag import as a biological assembly"));
		doc.set_option_decription(CDOD("title", CDOD::DATATYPE_STRING, "new object title", ""));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		LoadingOfData::Result loading_result;
		LoadingOfData::construct_result(params, loading_result);

		if(loading_result.atoms.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms read."));
		}

		DataManager* object_new=congregation_of_data_managers.add_object(DataManager(), title);
		DataManager& data_manager=*object_new;

		data_manager.reset_atoms_by_swapping(loading_result.atoms);
		if(!loading_result.contacts.empty())
		{
			data_manager.reset_contacts_by_swapping(loading_result.contacts);
		}

		congregation_of_data_managers.set_all_objects_picked(false);
		congregation_of_data_managers.set_object_picked(object_new, true);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms());
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());
		result.object_name=congregation_of_data_managers.get_object_attributes(object_new).name;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_H_ */
