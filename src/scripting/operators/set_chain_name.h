#ifndef SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainName : public OperatorBase<SetChainName>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		std::string chain_name;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("chain_name")=chain_name;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string chain_name;

	SetChainName()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		chain_name=input.get_value_or_first_unused_unnamed_value("chain-name");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("chain-name", CDOD::DATATYPE_STRING, "chain name to set"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(chain_name.empty())
		{
			throw std::runtime_error(std::string("No chain name provided."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::vector<Atom> atoms=data_manager.atoms();

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			atoms[*it].crad.chainID=chain_name;
		}

		data_manager.reset_atoms_by_swapping(atoms);

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.chain_name=chain_name;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_NAME_H_ */
