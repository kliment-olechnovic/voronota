#ifndef DUKTAPER_OPERATORS_VOROMQA_DARK_LOCAL_H_
#define DUKTAPER_OPERATORS_VOROMQA_DARK_LOCAL_H_

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkLocal : public scripting::OperatorBase<VoroMQADarkLocal>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		double global_score;
		int number_of_residues;

		Result() : global_score(0.0), number_of_residues(0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("global_score")=global_score;
			heterostorage.variant_object.value("number_of_residues")=number_of_residues;
		}
	};

	std::string selection_expresion_for_atoms;
	std::string adjunct_atom_quality_scores;
	std::string global_adj_prefix;

	VoroMQADarkLocal()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("atoms", "[]");
		adjunct_atom_quality_scores=input.get_value_or_default<std::string>("adj-atom-quality", "vd1");
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_dark_local");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms", CDOD::DATATYPE_STRING, "selection expression for atoms", "[]"));
		doc.set_option_decription(CDOD("adj-atom-quality", CDOD::DATATYPE_STRING, "name of input adjunct with atom quality scores", "vd1"));
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_dark_local"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_adjunct_availability(adjunct_atom_quality_scores);

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(scripting::SelectionManager::Query(selection_expresion_for_atoms, false));

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::set<std::size_t> atom_ids_with_adjuncts=data_manager.selection_manager().select_atoms(
				scripting::SelectionManager::Query(atom_ids, (std::string("[")+"--adjuncts "+adjunct_atom_quality_scores+"]"), false));

		if(atom_ids_with_adjuncts.empty())
		{
			throw std::runtime_error(std::string("No atoms with scores."));
		}

		std::map<common::ChainResidueAtomDescriptor, double> residue_scores;

		for(std::set<std::size_t>::const_iterator it=atom_ids_with_adjuncts.begin();it!=atom_ids_with_adjuncts.end();++it)
		{
			const scripting::Atom& atom=data_manager.atoms()[*it];
			residue_scores[atom.crad.without_atom()]=atom.value.props.adjuncts.find(adjunct_atom_quality_scores)->second;
		}

		double sum_of_residue_scores=0.0;
		for(std::map<common::ChainResidueAtomDescriptor, double>::const_iterator it=residue_scores.begin();it!=residue_scores.end();++it)
		{
			sum_of_residue_scores+=it->second;
		}

		Result result;

		result.global_score=sum_of_residue_scores/static_cast<double>(residue_scores.size());
		result.number_of_residues=static_cast<int>(residue_scores.size());

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_residues_count"]=residue_scores.size();
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_quality_score"]=result.global_score;
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_VOROMQA_DARK_LOCAL_H_ */
