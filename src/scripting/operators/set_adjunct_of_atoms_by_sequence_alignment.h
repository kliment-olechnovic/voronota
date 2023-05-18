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
		std::string sequence;
		double sequence_identity;

		Result() : sequence_identity(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("sequence")=sequence;
			heterostorage.variant_object.value("sequence_length")=sequence.size();
			heterostorage.variant_object.value("sequence_identity")=sequence_identity;
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name;
	std::string sequence_file;
	std::string alignment_file;
	bool keep_dashes;

	SetAdjunctOfAtomsBySequenceAlignment() : keep_dashes(false)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name=input.get_value<std::string>("name");
		sequence_file=input.get_value<std::string>("sequence-file");
		keep_dashes=input.get_flag("keep-dashes");
		alignment_file=input.get_value_or_default<std::string>("alignment-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("sequence-file", CDOD::DATATYPE_STRING, "sequence input file"));
		doc.set_option_decription(CDOD("alignment-file", CDOD::DATATYPE_STRING, "sequence alignment output file", ""));
		doc.set_option_decription(CDOD("keep-dashes", CDOD::DATATYPE_BOOL, "flag to keep dashes in sequence before alignment"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(name, false);

		InputSelector finput_selector(sequence_file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+sequence_file+"'.");
		}

		const std::string sequence=common::SequenceUtilities::read_sequence_from_stream(finput, keep_dashes);

		if(sequence.empty())
		{
			throw std::runtime_error(std::string("No sequence from file '")+sequence_file+"'");
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

		double sequence_identity=0.0;
		const std::map<common::ChainResidueAtomDescriptor, int> sequence_mapping=common::SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, sequence, false, false, &sequence_identity, alignment_file);

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
		result.sequence=sequence;
		result.sequence_identity=sequence_identity;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_SEQUENCE_ALIGNMENT_H_ */
