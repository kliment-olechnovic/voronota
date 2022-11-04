#ifndef SCRIPTING_OPERATORS_IMPORT_H_
#define SCRIPTING_OPERATORS_IMPORT_H_

#include "../operators_common.h"
#include "../loading_of_data.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Import : public OperatorBase<Import>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string effect;
		SummaryOfAtoms atoms_summary;
		SummaryOfContacts contacts_summary;
		std::vector<std::string> object_names;

		Result() : effect("loaded")
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms[effect]=atoms_summary;
			if(contacts_summary.number_total>0)
			{
				VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
				heterostorage.summaries_of_contacts[effect]=contacts_summary;
			}
			std::vector<VariantValue>& object_names_array=heterostorage.variant_object.values_array("object_names");
			for(std::size_t i=0;i<object_names.size();i++)
			{
				object_names_array.push_back(VariantValue(object_names[i]));
			}
		}
	};

	LoadingOfData::Parameters loading_parameters;
	std::string title;

	Import()
	{
	}

	void initialize(CommandInput& input, const bool managed)
	{
		loading_parameters=LoadingOfData::Parameters();
		if(!managed)
		{
			loading_parameters.file=input.get_value_or_first_unused_unnamed_value("file");
		}
		loading_parameters.format=input.get_value_or_default<std::string>("format", "");
		loading_parameters.format_fallback=input.get_value_or_default<std::string>("format-fallback", "pdb");
		loading_parameters.forced_include_heteroatoms=input.is_option("include-heteroatoms");
		loading_parameters.forced_include_hydrogens=input.is_option("include-hydrogens");
		loading_parameters.forced_multimodel_chains=input.is_option("as-assembly");
		loading_parameters.forced_same_radius_for_all=input.is_option("same-radius-for-all");
		loading_parameters.include_heteroatoms=input.get_flag("include-heteroatoms");
		loading_parameters.include_hydrogens=input.get_flag("include-hydrogens");
		loading_parameters.multimodel_chains=input.get_flag("as-assembly");
		loading_parameters.same_radius_for_all=input.get_value_or_default<double>("same-radius-for-all", LoadingOfData::Configuration::recommended_default_radius());
		title=(input.is_option("title") ? input.get_value<std::string>("title") : OperatorsUtilities::get_basename_from_path(loading_parameters.file));
	}

	void initialize(CommandInput& input)
	{
		initialize(input, false);
	}

	void document(CommandDocumentation& doc, const bool managed) const
	{
		if(!managed)
		{
			doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		}
		doc.set_option_decription(CDOD("format", CDOD::DATATYPE_STRING, "input file format", ""));
		doc.set_option_decription(CDOD("format-fallback", CDOD::DATATYPE_STRING, "input file format fallback", ""));
		doc.set_option_decription(CDOD("include-heteroatoms", CDOD::DATATYPE_BOOL, "flag to include heteroatoms"));
		doc.set_option_decription(CDOD("include-hydrogens", CDOD::DATATYPE_BOOL, "flag to include hydrogens"));
		doc.set_option_decription(CDOD("as-assembly", CDOD::DATATYPE_BOOL, "flag import as a biological assembly"));
		doc.set_option_decription(CDOD("same-radius-for-all", CDOD::DATATYPE_FLOAT, "radius to use for all atoms", LoadingOfData::Configuration::recommended_default_radius()));
		doc.set_option_decription(CDOD("title", CDOD::DATATYPE_STRING, "new object title", ""));
	}

	void document(CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		LoadingOfData::Result loading_result;
		LoadingOfData::construct_result(loading_parameters, loading_result);

		if(loading_result.atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms read."));
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
		result.object_names.push_back(congregation_of_data_managers.get_object_attributes(object_new).name);

		return result;
	}

	Result run(DataManager& data_manager) const
	{
		LoadingOfData::Result loading_result;
		LoadingOfData::construct_result(loading_parameters, loading_result);

		if(loading_result.atoms.empty())
		{
			throw std::runtime_error(std::string("No atoms read."));
		}

		data_manager.reset_atoms_by_swapping(loading_result.atoms);
		if(!loading_result.contacts.empty())
		{
			data_manager.reset_contacts_by_swapping(loading_result.contacts);
		}

		Result result;
		result.effect="reloaded";
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms());
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_H_ */
