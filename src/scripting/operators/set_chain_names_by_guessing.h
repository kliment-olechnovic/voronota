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

	bool detect_gly_linkers;
	int min_gly_linker_length;

	SetChainNamesByGuessing() : detect_gly_linkers(false), min_gly_linker_length(30)
	{
	}

	void initialize(CommandInput& input)
	{
		detect_gly_linkers=input.get_flag("detect-gly-linkers");
		min_gly_linker_length=input.get_value_or_default<int>("min-gly-linker-length", 30);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("detect-gly-linkers", CDOD::DATATYPE_BOOL, "flag to detect long GLY linkers and treat them as chain separators"));
		doc.set_option_decription(CDOD("min-sequence-identity", CDOD::DATATYPE_INT, "min length of long GLY linker, ignored if less than 10", "30"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::vector<Atom> atoms=data_manager.atoms();

		std::set<std::size_t> residue_ids_of_gly_linker_middles;

		if(detect_gly_linkers && min_gly_linker_length>=10)
		{
			std::vector< std::vector<std::size_t> > gly_segments;
			for(std::size_t i=0;i<data_manager.primary_structure_info().residues.size();i++)
			{
				const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[i];
				if(residue.chain_residue_descriptor.resName=="GLY")
				{
					if(gly_segments.empty())
					{
						gly_segments.push_back(std::vector<std::size_t>(1, i));
					}
					else
					{
						const common::ConstructionOfPrimaryStructure::Residue& prev_residue=data_manager.primary_structure_info().residues[gly_segments.back().back()];
						if(common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(prev_residue.chain_residue_descriptor, residue.chain_residue_descriptor, 1, 1, false))
						{
							gly_segments.back().push_back(i);
						}
						else
						{
							gly_segments.push_back(std::vector<std::size_t>(1, i));
						}
					}
				}
			}
			for(std::size_t i=0;i<gly_segments.size();i++)
			{
				if(gly_segments[i].size()>=static_cast<std::size_t>(min_gly_linker_length))
				{
					const std::size_t j=gly_segments[i].size()/2;
					residue_ids_of_gly_linker_middles.insert(gly_segments[i][j]);
				}
			}
		}

		Result result;

		{
			char current_chain_name='A';
			for(std::size_t i=0;i<atoms.size();i++)
			{
				if(i>0)
				{
					const bool break_due_to_seq_num=(atoms[i].crad.resSeq<atoms[i-1].crad.resSeq);
					const bool break_due_to_gly_linker=(!residue_ids_of_gly_linker_middles.empty() && residue_ids_of_gly_linker_middles.count(data_manager.primary_structure_info().map_of_atoms_to_residues[i])>0);
					if(break_due_to_seq_num || break_due_to_gly_linker)
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
					if(break_due_to_gly_linker)
					{
						residue_ids_of_gly_linker_middles.erase(data_manager.primary_structure_info().map_of_atoms_to_residues[i]);
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
