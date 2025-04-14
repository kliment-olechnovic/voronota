#ifndef SCRIPTING_OPERATORS_SET_CHAIN_RESIDUE_NUMBERS_BY_SEQUENCE_H_
#define SCRIPTING_OPERATORS_SET_CHAIN_RESIDUE_NUMBERS_BY_SEQUENCE_H_

#include "../operators_common.h"

#include "../../common/sequence_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetChainResidueNumbersBySequences : public OperatorBase<SetChainResidueNumbersBySequences>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string chain_name;
		std::string sequence;
		double sequence_identity;

		Result() : sequence_identity(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("chain_name")=chain_name;
			heterostorage.variant_object.value("sequence")=sequence;
			heterostorage.variant_object.value("sequence_length")=sequence.size();
			heterostorage.variant_object.value("sequence_identity")=sequence_identity;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string sequence_file;
	std::string sequence_string;
	std::string alignment_file;
	bool keep_dashes;
	bool only_equal_pairs;
	bool drop_non_polymeric_atoms;

	SetChainResidueNumbersBySequences() : keep_dashes(false), only_equal_pairs(false), drop_non_polymeric_atoms(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		sequence_file=input.get_value_or_default<std::string>("sequence-file", "");
		sequence_string=input.get_value_or_default<std::string>("sequence-string", "");
		alignment_file=input.get_value_or_default<std::string>("alignment-file", "");
		keep_dashes=input.get_flag("keep-dashes");
		only_equal_pairs=input.get_flag("only-equal-pairs");
		drop_non_polymeric_atoms=input.get_flag("drop-non-polymeric-atoms");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("sequence-file", CDOD::DATATYPE_STRING, "sequence input file", ""));
		doc.set_option_decription(CDOD("sequence-string", CDOD::DATATYPE_STRING, "sequence string", ""));
		doc.set_option_decription(CDOD("alignment-file", CDOD::DATATYPE_STRING, "sequence alignment output file", ""));
		doc.set_option_decription(CDOD("keep-dashes", CDOD::DATATYPE_BOOL, "flag to keep dashes in sequence before alignment"));
		doc.set_option_decription(CDOD("only-equal-pairs", CDOD::DATATYPE_BOOL, "flag to only leave equal pairs from alignment"));
		doc.set_option_decription(CDOD("drop-non-polymeric-atoms", CDOD::DATATYPE_BOOL, "flag to not treat non-polymeric atoms as untouchable"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(sequence_file.empty() && sequence_string.empty())
		{
			throw std::runtime_error(std::string("No sequence input provided."));
		}

		if(!sequence_file.empty() && !sequence_string.empty())
		{
			throw std::runtime_error(std::string("No exactly one sequence input provided."));
		}

		std::string sequence;

		if(!sequence_file.empty())
		{
			InputSelector finput_selector(sequence_file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+sequence_file+"'.");
			}

			sequence=common::SequenceUtilities::read_sequence_from_stream(finput, keep_dashes);

			if(sequence.empty())
			{
				throw std::runtime_error(std::string("No sequence from file '")+sequence_file+"'");
			}
		}
		else
		{
			sequence=sequence_string;
		}

		const std::set<std::size_t> initial_atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(initial_atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::string chain_name;
		std::vector<common::ChainResidueAtomDescriptor> residue_sequence_vector;
		std::set<std::size_t> untouchable_atom_ids;

		{
			std::set<std::size_t> chain_ids;
			for(std::set<std::size_t>::const_iterator it=initial_atom_ids.begin();it!=initial_atom_ids.end();++it)
			{
				chain_ids.insert(data_manager.primary_structure_info().map_of_residues_to_chains[data_manager.primary_structure_info().map_of_atoms_to_residues[*it]]);
			}

			if(chain_ids.size()!=1)
			{
				throw std::runtime_error(std::string("No exactly one chain selected."));
			}

			const common::ConstructionOfPrimaryStructure::Chain& chain_info=data_manager.primary_structure_info().chains[*(chain_ids.begin())];
			chain_name=chain_info.name;
			residue_sequence_vector.reserve(chain_info.residue_ids.size());
			for(std::vector<std::size_t>::const_iterator it=chain_info.residue_ids.begin();it!=chain_info.residue_ids.end();++it)
			{
				const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[*it];
				if(residue.residue_type==common::ConstructionOfPrimaryStructure::RESIDUE_TYPE_OTHER)
				{
					if(!drop_non_polymeric_atoms)
					{
						untouchable_atom_ids.insert(residue.atom_ids.begin(), residue.atom_ids.end());
					}
				}
				else
				{
					residue_sequence_vector.push_back(residue.chain_residue_descriptor);
				}
			}
		}

		double sequence_identity=0.0;
		const std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, sequence, only_equal_pairs, true, &sequence_identity, alignment_file);

		std::vector<std::size_t> atom_ids_to_keep;
		atom_ids_to_keep.reserve(data_manager.atoms().size());
		std::map<std::size_t, int> atom_ids_to_renumber;

		std::set<int> forbidden_residue_numbers_for_untouchable_atoms;
		if(!untouchable_atom_ids.empty())
		{
			for(std::map<common::ChainResidueAtomDescriptor, int>::const_iterator jt=sequence_mapping.begin();jt!=sequence_mapping.end();++jt)
			{
				forbidden_residue_numbers_for_untouchable_atoms.insert(jt->second);
			}
		}

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			if(data_manager.atoms()[i].crad.chainID!=chain_name)
			{
				atom_ids_to_keep.push_back(i);
			}
			else if(untouchable_atom_ids.count(i)>0)
			{
				atom_ids_to_keep.push_back(i);
				const int current_resSeq=data_manager.atoms()[i].crad.resSeq;
				if(forbidden_residue_numbers_for_untouchable_atoms.count(current_resSeq)>0)
				{
					if(current_resSeq<0)
					{
						atom_ids_to_renumber[i]=((*forbidden_residue_numbers_for_untouchable_atoms.begin())-1+current_resSeq);
					}
					else
					{
						atom_ids_to_renumber[i]=((*forbidden_residue_numbers_for_untouchable_atoms.rbegin())+1+current_resSeq);
					}
				}
			}
			else
			{
				const std::size_t residue_id=data_manager.primary_structure_info().map_of_atoms_to_residues[i];
				const common::ChainResidueAtomDescriptor& residue_crad=data_manager.primary_structure_info().residues[residue_id].chain_residue_descriptor;
				std::map<common::ChainResidueAtomDescriptor, int>::const_iterator jt=sequence_mapping.find(residue_crad);
				if(jt!=sequence_mapping.end())
				{
					atom_ids_to_keep.push_back(i);
					atom_ids_to_renumber[i]=jt->second;
				}
			}
		}

		data_manager.restrict_atoms_and_renumber_residues(std::set<std::size_t>(atom_ids_to_keep.begin(), atom_ids_to_keep.end()), atom_ids_to_renumber);

		Result result;
		result.chain_name=chain_name;
		result.sequence=sequence;
		result.sequence_identity=sequence_identity;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_CHAIN_RESIDUE_NUMBERS_BY_SEQUENCE_H_ */
