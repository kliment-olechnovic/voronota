#ifndef SCRIPTING_OPERATORS_SET_CHAIN_NAMES_BY_GUESSING_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_NAMES_BY_GUESSING_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainNamesByGuessing : public OperatorBase<SetChainNamesByGuessing>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string chains;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("chains")=chains;
		}
	};

	SetChainNamesByGuessing()
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

		std::vector<Atom> atoms=data_manager.atoms();

		Result result;

		{
			char current_chain_name='A';
			for(std::size_t i=0;i<atoms.size();i++)
			{
				if(i>0 && atoms[i].crad.resSeq<atoms[i-1].crad.resSeq)
				{
					if(current_chain_name=='Z')
					{
						current_chain_name='a';
					}
					else if(current_chain_name=='z')
					{
						throw std::runtime_error("Too many chains to guess their names.");
					}
					else
					{
						current_chain_name++;
					}
				}
				atoms[i].crad.chainID=std::string(1, current_chain_name);
				if(result.chains.empty() || result.chains[result.chains.size()-1]!=current_chain_name)
				{
					result.chains.push_back(current_chain_name);
				}
			}
		}

		data_manager.reset_atoms_by_swapping(atoms);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_NAMES_BY_GUESSING_H_ */
