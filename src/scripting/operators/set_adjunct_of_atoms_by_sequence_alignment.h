#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_SEQUENCE_ALIGNMENT_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_SEQUENCE_ALIGNMENT_H_

#include "../operators_common.h"

#include "../../common/sequence_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsBySequenceAlignment : public OperatorBase<SetAdjunctOfAtomsBySequenceAlignment>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	std::string sequence;
	std::string alignment_file;

	SetAdjunctOfAtomsBySequenceAlignment()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		sequence=input.get_value<std::string>("sequence");
		alignment_file=input.get_value_or_default<std::string>("alignment-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("sequence", CDOD::DATATYPE_STRING, "sequence"));
		doc.set_option_decription(CDOD("alignment-file", CDOD::DATATYPE_STRING, "sequence alignment output file", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(name, false);

		if(sequence.find_first_not_of(" \n\t")!=std::string::npos)
		{
			throw std::runtime_error(std::string("Provided sequence does not contain symbols."));
		}

		std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::set<std::size_t> residue_ids;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			residue_ids.insert(data_manager.primary_structure_info().map_of_atoms_to_residues[*it]);
		}

		std::vector<common::ChainResidueAtomDescriptor> residue_sequence_vector;
		residue_sequence_vector.reserve(residue_ids.size());
		for(std::set<std::size_t>::const_iterator it=residue_ids.begin();it!=residue_ids.end();++it)
		{
			residue_sequence_vector.push_back(data_manager.primary_structure_info().residues[*it].chain_residue_descriptor);
		}

		const std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, sequence, alignment_file);

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			atom_adjuncts.erase(name);
			const std::size_t residue_id=data_manager.primary_structure_info().map_of_atoms_to_residues[*it];
			const common::ChainResidueAtomDescriptor& residue_crad=data_manager.primary_structure_info().residues[residue_id].chain_residue_descriptor;
			std::map<common::ChainResidueAtomDescriptor, int>::const_iterator jt=sequence_mapping.find(residue_crad);
			if(jt!=sequence_mapping.end())
			{
				atom_adjuncts[name]=jt->second;
			}
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_SEQUENCE_ALIGNMENT_H_ */
