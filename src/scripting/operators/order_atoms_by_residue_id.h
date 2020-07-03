#ifndef SCRIPTING_OPERATORS_ORDER_ATOMS_BY_RESIDUE_ID_H_
#define SCRIPTING_OPERATORS_ORDER_ATOMS_BY_RESIDUE_ID_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class OrderAtomsByResidueID : public OperatorBase<OrderAtomsByResidueID>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	OrderAtomsByResidueID()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> > ordered_atom_ids;

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			ordered_atom_ids[data_manager.atoms()[i].crad.without_atom()].push_back(i);
		}

		std::vector<Atom> atoms;
		atoms.reserve(data_manager.atoms().size());

		for(std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=ordered_atom_ids.begin();it!=ordered_atom_ids.end();++it)
		{
			const std::vector<std::size_t>& atom_ids=it->second;
			for(std::size_t i=0;i<atom_ids.size();i++)
			{
				atoms.push_back(data_manager.atoms()[atom_ids[i]]);
			}
		}

		data_manager.reset_atoms_by_swapping(atoms);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ORDER_ATOMS_BY_RESIDUE_ID_H_ */
