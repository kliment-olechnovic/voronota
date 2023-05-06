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

	SetChainResidueNumbersBySequences() : keep_dashes(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		sequence_file=input.get_value_or_default<std::string>("sequence-file", "");
		sequence_string=input.get_value_or_default<std::string>("sequence-string", "");
		keep_dashes=input.get_flag("keep-dashes");
		alignment_file=input.get_value_or_default<std::string>("alignment-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("sequence-file", CDOD::DATATYPE_STRING, "sequence input file", ""));
		doc.set_option_decription(CDOD("sequence-file", CDOD::DATATYPE_STRING, "sequence string", ""));
		doc.set_option_decription(CDOD("alignment-file", CDOD::DATATYPE_STRING, "sequence alignment output file", ""));
		doc.set_option_decription(CDOD("keep-dashes", CDOD::DATATYPE_BOOL, "flag to keep dashes in sequence before alignment"));
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

		if(!sequence_string.empty() && keep_dashes)
		{
			throw std::runtime_error(std::string("Keeping dashes not supported for direct string input."));
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
				residue_sequence_vector.push_back(data_manager.primary_structure_info().residues[*it].chain_residue_descriptor);
			}
		}

		double sequence_identity=0.0;
		const std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, sequence, false, &sequence_identity, alignment_file);

		std::vector<std::size_t> atom_ids_to_keep;
		atom_ids_to_keep.reserve(data_manager.atoms().size());
		std::map<std::size_t, int> atom_ids_to_renumber;

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			if(data_manager.atoms()[i].crad.chainID!=chain_name)
			{
				atom_ids_to_keep.push_back(i);
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
