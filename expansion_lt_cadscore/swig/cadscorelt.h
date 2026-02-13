#ifndef CADSCORELT_H_
#define CADSCORELT_H_

#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

#ifndef SWIG
#include "cadscorelt/cadscorelt.h"
#include "cadscorelt_cli/cadscorelt_cli.h"
#endif

struct MolecularAtomBall
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	std::string ID_atom_name;
	std::string residue_name;
	std::string element;
	double x;
	double y;
	double z;
	double r;

	MolecularAtomBall() : ID_residue_seq_number(0), x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	MolecularAtomBall(
			const std::string& ID_chain,
			const int ID_residue_seq_number,
			const std::string& ID_residue_icode,
			const std::string& ID_atom_name,
			const std::string& residue_name,
			const std::string& element,
			const double x,
			const double y,
			const double z) :
		ID_chain(ID_chain),
		ID_residue_seq_number(ID_residue_seq_number),
		ID_residue_icode(ID_residue_icode),
		ID_atom_name(ID_atom_name),
		residue_name(residue_name),
		element(element),
		x(x),
		y(y),
		z(z),
		r(0.0)
	{
	}
};

struct GlobalScore
{
	std::string target_name;
	std::string model_name;
	std::string renaming_of_model_chains;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	GlobalScore() : cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct AtomScore
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	std::string ID_atom_name;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	AtomScore() : ID_residue_seq_number(0), cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct ResidueScore
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	ResidueScore() : ID_residue_seq_number(0), cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct ChainScore
{
	std::string ID_chain;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	ChainScore() : cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct AtomAtomScore
{
	std::string ID1_chain;
	int ID1_residue_seq_number;
	std::string ID1_residue_icode;
	std::string ID1_atom_name;
	std::string ID2_chain;
	int ID2_residue_seq_number;
	std::string ID2_residue_icode;
	std::string ID2_atom_name;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	AtomAtomScore() : ID1_residue_seq_number(0), ID2_residue_seq_number(0), cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct ResidueResidueScore
{
	std::string ID1_chain;
	int ID1_residue_seq_number;
	std::string ID1_residue_icode;
	std::string ID2_chain;
	int ID2_residue_seq_number;
	std::string ID2_residue_icode;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	ResidueResidueScore() : ID1_residue_seq_number(0), ID2_residue_seq_number(0), cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct ChainChainScore
{
	std::string ID1_chain;
	std::string ID2_chain;
	double cadscore;
	double F1_of_areas;
	double target_area;
	double model_area;
	double TP_area;
	double FP_area;
	double FN_area;

	ChainChainScore() : cadscore(0.0), F1_of_areas(0.0), target_area(0.0), model_area(0.0), TP_area(0.0), FP_area(0.0), FN_area(0.0)
	{
	}
};

struct MolecularFileInput
{
	std::string input_file_path;
	bool include_heteroatoms;
	bool read_as_assembly;

	MolecularFileInput() :
		include_heteroatoms(false),
		read_as_assembly(false)
	{
	}

	MolecularFileInput(const std::string& input_file, const bool include_heteroatoms, const bool read_as_assembly) :
		input_file_path(input_file),
		include_heteroatoms(include_heteroatoms),
		read_as_assembly(read_as_assembly)
	{
	}
};

struct CADScoreParameters
{
	double probe;
	bool conflate_atom_names;
	bool remap_chains;
	bool score_atom_atom_contacts;
	bool score_residue_residue_contacts;
	bool score_chain_chain_contacts;
	bool score_atom_sas;
	bool score_residue_sas;
	bool score_chain_sas;
	bool score_atom_sites;
	bool score_residue_sites;
	bool score_chain_sites;
	bool record_local_scores;
	bool include_self_to_self_scores;
	std::string restrict_input_atoms;
	std::string subselect_contacts;
	std::string subselect_atoms;

	CADScoreParameters() :
		probe(1.4),
		conflate_atom_names(true),
		remap_chains(false),
		score_atom_atom_contacts(false),
		score_residue_residue_contacts(true),
		score_chain_chain_contacts(false),
		score_atom_sas(false),
		score_residue_sas(false),
		score_chain_sas(false),
		score_atom_sites(false),
		score_residue_sites(false),
		score_chain_sites(false),
		record_local_scores(false),
		include_self_to_self_scores(false),
		subselect_contacts("[-min-sep 1]")
	{
	}
};

class CADScore
{
public:
	CADScore(const CADScoreParameters& init_params) : need_to_reset_global_scores_(false)
	{
		params_=init_params;

		if(!init_params.restrict_input_atoms.empty())
		{
			scorable_data_construction_parameters_.filtering_expression_for_restricting_raw_input_balls=voronotalt::FilteringBySphereLabels::ExpressionForSingle(init_params.restrict_input_atoms);
			if(!scorable_data_construction_parameters_.filtering_expression_for_restricting_raw_input_balls.valid())
			{
				throw std::runtime_error("Invalid input balls filtering expression.");
			}
		}

		if(!init_params.subselect_contacts.empty())
		{
			scorable_data_construction_parameters_.filtering_expression_for_restricting_contact_descriptors=voronotalt::FilteringBySphereLabels::ExpressionForPair(init_params.subselect_contacts);
			if(!scorable_data_construction_parameters_.filtering_expression_for_restricting_contact_descriptors.valid())
			{
				throw std::runtime_error("Invalid contact descriptors subselection expression.");
			}
		}

		if(!init_params.subselect_atoms.empty())
		{
			scorable_data_construction_parameters_.filtering_expression_for_restricting_atom_descriptors=voronotalt::FilteringBySphereLabels::ExpressionForSingle(init_params.subselect_atoms);
			if(!scorable_data_construction_parameters_.filtering_expression_for_restricting_atom_descriptors.valid())
			{
				throw std::runtime_error("Invalid atom descriptors subselection expression.");
			}
		}

		scorable_data_construction_parameters_.probe=init_params.probe;
		scorable_data_construction_parameters_.record_atom_balls=false;
		scorable_data_construction_parameters_.record_sequence_alignments=false;
		scorable_data_construction_parameters_.record_graphics=false;
		scorable_data_construction_parameters_.record_atom_atom_contact_summaries=init_params.score_atom_atom_contacts;
		scorable_data_construction_parameters_.record_residue_residue_contact_summaries=(init_params.score_residue_residue_contacts || init_params.remap_chains);
		scorable_data_construction_parameters_.record_chain_chain_contact_summaries=init_params.score_chain_chain_contacts;
		scorable_data_construction_parameters_.record_atom_cell_summaries=init_params.score_atom_sas;
		scorable_data_construction_parameters_.record_residue_cell_summaries=init_params.score_residue_sas;
		scorable_data_construction_parameters_.record_chain_cell_summaries=init_params.score_chain_sas;
		scorable_data_construction_parameters_.record_atom_site_summaries=init_params.score_atom_sites;
		scorable_data_construction_parameters_.record_residue_site_summaries=init_params.score_residue_sites;
		scorable_data_construction_parameters_.record_chain_site_summaries=init_params.score_chain_sites;
		scorable_data_construction_parameters_.conflate_equivalent_atom_types=init_params.conflate_atom_names;

		scoring_result_construction_parameters_.remap_chains=init_params.remap_chains;
		scoring_result_construction_parameters_.max_permutations_to_check_exhaustively=200;
		scoring_result_construction_parameters_.record_local_scores_on_atom_level=init_params.record_local_scores;
		scoring_result_construction_parameters_.record_local_scores_on_residue_level=init_params.record_local_scores;
		scoring_result_construction_parameters_.record_local_scores_on_chain_level=init_params.record_local_scores;
		scoring_result_construction_parameters_.record_compatible_model_atom_balls=false;
	}

	CADScoreParameters get_parameters() const
	{
		return params_;
	}

	void add_target_structure_from_file_descriptor(const MolecularFileInput& input_file_info, const std::string& name)
	{
		add_structure(input_file_info, std::vector<MolecularAtomBall>(), true, false, name);
	}

	void add_target_structure_from_atoms(const std::vector<MolecularAtomBall>& input_atom_balls, const std::string& name)
	{
		add_structure(MolecularFileInput(), input_atom_balls, true, false, name);
	}

	void add_model_structure_from_file_descriptor(const MolecularFileInput& input_file_info, const std::string& name)
	{
		add_structure(input_file_info, std::vector<MolecularAtomBall>(), false, true, name);
	}

	void add_model_structure_from_atoms(const std::vector<MolecularAtomBall>& input_atom_balls, const std::string& name)
	{
		add_structure(MolecularFileInput(), input_atom_balls, false, true, name);
	}

	void add_structure_from_file_descriptor(const MolecularFileInput& input_file_info, const std::string& name)
	{
		add_structure(input_file_info, std::vector<MolecularAtomBall>(), true, true, name);
	}

	void add_structure_from_atoms(const std::vector<MolecularAtomBall>& input_atom_balls, const std::string& name)
	{
		add_structure(MolecularFileInput(), input_atom_balls, true, true, name);
	}

	const std::vector<GlobalScore>* get_all_cadscores_atom_atom_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_atom_atom_summarized_globally.empty())
		{
			throw std::runtime_error("Atom-atom contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_atom_atom_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_residue_residue_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_residue_residue_summarized_globally.empty())
		{
			throw std::runtime_error("Residue-residue contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_residue_residue_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_chain_chain_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_chain_chain_summarized_globally.empty())
		{
			throw std::runtime_error("Chain-chain contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_chain_chain_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_atom_sas_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_atom_sas_summarized_globally.empty())
		{
			throw std::runtime_error("Atom SAS global scores were not requested to compute.");
		}
		return &gsr.cadscores_atom_sas_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_residue_sas_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_residue_sas_summarized_globally.empty())
		{
			throw std::runtime_error("Residue SAS global scores were not requested to compute.");
		}
		return &gsr.cadscores_residue_sas_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_chain_sas_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_chain_sas_summarized_globally.empty())
		{
			throw std::runtime_error("Chain SAS global scores were not requested to compute.");
		}
		return &gsr.cadscores_chain_sas_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_atom_site_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_atom_site_summarized_globally.empty())
		{
			throw std::runtime_error("Atom site contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_atom_site_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_residue_site_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_residue_site_summarized_globally.empty())
		{
			throw std::runtime_error("Residue site contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_residue_site_summarized_globally;
	}

	const std::vector<GlobalScore>* get_all_cadscores_chain_site_summarized_globally()
	{
		const GlobalScoringResult& gsr=get_all_global_scores();
		if(gsr.cadscores_chain_site_summarized_globally.empty())
		{
			throw std::runtime_error("Chain site contact global scores were not requested to compute.");
		}
		return &gsr.cadscores_chain_site_summarized_globally;
	}

	const std::vector<AtomAtomScore>* get_local_cadscores_atom_atom(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom.empty())
		{
			throw std::runtime_error("Atom-atom contact local scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom;
	}

	const std::vector<ResidueResidueScore>* get_local_cadscores_atom_atom_summarized_per_residue_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_per_residue_residue.empty())
		{
			throw std::runtime_error("Atom-atom summarized per residue-residue scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_per_residue_residue;
	}

	const std::vector<ChainChainScore>* get_local_cadscores_atom_atom_summarized_per_chain_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_per_chain_chain.empty())
		{
			throw std::runtime_error("Atom-atom summarized per chain-chain scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_per_chain_chain;
	}

	const std::vector<AtomScore>* get_local_cadscores_atom_atom_summarized_per_atom(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_per_atom.empty())
		{
			throw std::runtime_error("Atom-atom summarized per atom scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_per_atom;
	}

	const std::vector<ResidueScore>* get_local_cadscores_atom_atom_summarized_per_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_per_residue.empty())
		{
			throw std::runtime_error("Atom-atom summarized per residue scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_per_residue;
	}

	const std::vector<ChainScore>* get_local_cadscores_atom_atom_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_per_chain.empty())
		{
			throw std::runtime_error("Atom-atom summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_per_chain;
	}

	const std::vector<ResidueResidueScore>* get_local_cadscores_residue_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_residue.empty())
		{
			throw std::runtime_error("Residue-residue contact local scores were not requested to compute.");
		}
		return &psr.cadscores_residue_residue;
	}

	const std::vector<ChainChainScore>* get_local_cadscores_residue_residue_summarized_per_chain_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_residue_summarized_per_chain_chain.empty())
		{
			throw std::runtime_error("Residue-residue summarized per chain-chain scores were not requested to compute.");
		}
		return &psr.cadscores_residue_residue_summarized_per_chain_chain;
	}

	const std::vector<ResidueScore>* get_local_cadscores_residue_residue_summarized_per_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_residue_summarized_per_residue.empty())
		{
			throw std::runtime_error("Residue-residue summarized per residue scores were not requested to compute.");
		}
		return &psr.cadscores_residue_residue_summarized_per_residue;
	}

	const std::vector<ChainScore>* get_local_cadscores_residue_residue_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_residue_summarized_per_chain.empty())
		{
			throw std::runtime_error("Residue-residue summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_residue_residue_summarized_per_chain;
	}

	const std::vector<ChainChainScore>* get_local_cadscores_chain_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_chain.empty())
		{
			throw std::runtime_error("Chain-chain contact local scores were not requested to compute.");
		}
		return &psr.cadscores_chain_chain;
	}

	const std::vector<ChainScore>* get_local_cadscores_chain_chain_contact_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_chain_contact_summarized_per_chain.empty())
		{
			throw std::runtime_error("Chain-chain summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_chain_chain_contact_summarized_per_chain;
	}

	const std::vector<AtomScore>* get_local_cadscores_atom_sas(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_sas.empty())
		{
			throw std::runtime_error("Atom SAS local scores were not requested to compute.");
		}
		return &psr.cadscores_atom_sas;
	}

	const std::vector<ResidueScore>* get_local_cadscores_atom_sas_summarized_per_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_sas_summarized_per_residue.empty())
		{
			throw std::runtime_error("Atom SAS summarized per residue scores were not requested to compute.");
		}
		return &psr.cadscores_atom_sas_summarized_per_residue;
	}

	const std::vector<ChainScore>* get_local_cadscores_atom_sas_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_sas_summarized_per_chain.empty())
		{
			throw std::runtime_error("Atom SAS summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_atom_sas_summarized_per_chain;
	}

	const std::vector<ResidueScore>* get_local_cadscores_residue_sas(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_sas.empty())
		{
			throw std::runtime_error("Residue SAS local scores were not requested to compute.");
		}
		return &psr.cadscores_residue_sas;
	}

	const std::vector<ChainScore>* get_local_cadscores_residue_sas_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_sas_summarized_per_chain.empty())
		{
			throw std::runtime_error("Residue SAS summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_residue_sas_summarized_per_chain;
	}

	const std::vector<ChainScore>* get_local_cadscores_chain_sas(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_sas.empty())
		{
			throw std::runtime_error("Chain SAS local scores were not requested to compute.");
		}
		return &psr.cadscores_chain_sas;
	}

	const std::vector<AtomScore>* get_local_cadscores_atom_site(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_site.empty())
		{
			throw std::runtime_error("Atom site local scores were not requested to compute.");
		}
		return &psr.cadscores_atom_site;
	}

	const std::vector<ResidueScore>* get_local_cadscores_atom_site_summarized_per_residue(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_site_summarized_per_residue.empty())
		{
			throw std::runtime_error("Atom site summarized per residue scores were not requested to compute.");
		}
		return &psr.cadscores_atom_site_summarized_per_residue;
	}

	const std::vector<ChainScore>* get_local_cadscores_atom_site_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_site_summarized_per_chain.empty())
		{
			throw std::runtime_error("Atom site summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_atom_site_summarized_per_chain;
	}

	const std::vector<ResidueScore>* get_local_cadscores_residue_site(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_site.empty())
		{
			throw std::runtime_error("Residue site local scores were not requested to compute.");
		}
		return &psr.cadscores_residue_site;
	}

	const std::vector<ChainScore>* get_local_cadscores_residue_site_summarized_per_chain(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_site_summarized_per_chain.empty())
		{
			throw std::runtime_error("Residue site summarized per chain scores were not requested to compute.");
		}
		return &psr.cadscores_residue_site_summarized_per_chain;
	}

	const std::vector<ChainScore>* get_local_cadscores_chain_site(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_site.empty())
		{
			throw std::runtime_error("Chain site local scores were not requested to compute.");
		}
		return &psr.cadscores_chain_site;
	}

	const GlobalScore* get_cadscores_atom_atom_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_atom_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Atom-atom contact global score was not requested to compute.");
		}
		return &psr.cadscores_atom_atom_summarized_globally;
	}

	const GlobalScore* get_cadscores_residue_residue_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_residue_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Residue-residue contact global score was not requested to compute.");
		}
		return &psr.cadscores_residue_residue_summarized_globally;
	}

	const GlobalScore* get_cadscores_chain_chain_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_chain_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Chain-chain contact global score was not requested to compute.");
		}
		return &psr.cadscores_chain_chain_summarized_globally;
	}

	const GlobalScore* get_cadscores_atom_sas_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_sas_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Atom SAS global score was not requested to compute.");
		}
		return &psr.cadscores_atom_sas_summarized_globally;
	}

	const GlobalScore* get_cadscores_residue_sas_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_sas_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Residue SAS global score was not requested to compute.");
		}
		return &psr.cadscores_residue_sas_summarized_globally;
	}

	const GlobalScore* get_cadscores_chain_sas_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_sas_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Chain SAS global score was not requested to compute.");
		}
		return &psr.cadscores_chain_sas_summarized_globally;
	}

	const GlobalScore* get_cadscores_atom_site_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_atom_site_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Atom site global score was not requested to compute.");
		}
		return &psr.cadscores_atom_site_summarized_globally;
	}

	const GlobalScore* get_cadscores_residue_site_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_residue_site_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Residue site global score was not requested to compute.");
		}
		return &psr.cadscores_residue_site_summarized_globally;
	}

	const GlobalScore* get_cadscores_chain_site_summarized_globally(const std::string& target_name, const std::string& model_name)
	{
		const PairScoringResult& psr=get_pair_scores(target_name, model_name);
		if(psr.cadscores_chain_site_summarized_globally.target_area<=0.0)
		{
			throw std::runtime_error("Chain site global score was not requested to compute.");
		}
		return &psr.cadscores_chain_site_summarized_globally;
	}

private:
	struct GlobalScoringResult
	{
		std::vector<GlobalScore> cadscores_atom_atom_summarized_globally;
		std::vector<GlobalScore> cadscores_residue_residue_summarized_globally;
		std::vector<GlobalScore> cadscores_chain_chain_summarized_globally;
		std::vector<GlobalScore> cadscores_atom_sas_summarized_globally;
		std::vector<GlobalScore> cadscores_residue_sas_summarized_globally;
		std::vector<GlobalScore> cadscores_chain_sas_summarized_globally;
		std::vector<GlobalScore> cadscores_atom_site_summarized_globally;
		std::vector<GlobalScore> cadscores_residue_site_summarized_globally;
		std::vector<GlobalScore> cadscores_chain_site_summarized_globally;

		static void sort_vector_of_global_scores_by_cadscore(std::vector<GlobalScore>& v)
		{
			std::sort(v.begin(), v.end(),
			[](const GlobalScore& a, const GlobalScore& b)
			{
				return a.cadscore > b.cadscore;
			});
		}

		void clear()
		{
			cadscores_atom_atom_summarized_globally.clear();
			cadscores_residue_residue_summarized_globally.clear();
			cadscores_chain_chain_summarized_globally.clear();
			cadscores_atom_sas_summarized_globally.clear();
			cadscores_residue_sas_summarized_globally.clear();
			cadscores_chain_sas_summarized_globally.clear();
			cadscores_atom_site_summarized_globally.clear();
			cadscores_residue_site_summarized_globally.clear();
			cadscores_chain_site_summarized_globally.clear();
		}

		void sort_by_cadscore()
		{
			sort_vector_of_global_scores_by_cadscore(cadscores_atom_atom_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_residue_residue_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_chain_chain_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_atom_sas_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_residue_sas_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_chain_sas_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_atom_site_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_residue_site_summarized_globally);
			sort_vector_of_global_scores_by_cadscore(cadscores_chain_site_summarized_globally);
		}
	};

	struct PairScoringResult
	{
		std::vector<AtomAtomScore> cadscores_atom_atom;
		std::vector<ResidueResidueScore> cadscores_atom_atom_summarized_per_residue_residue;
		std::vector<ChainChainScore> cadscores_atom_atom_summarized_per_chain_chain;
		std::vector<AtomScore> cadscores_atom_atom_summarized_per_atom;
		std::vector<ResidueScore> cadscores_atom_atom_summarized_per_residue;
		std::vector<ChainScore> cadscores_atom_atom_summarized_per_chain;
		GlobalScore cadscores_atom_atom_summarized_globally;

		std::vector<ResidueResidueScore> cadscores_residue_residue;
		std::vector<ChainChainScore> cadscores_residue_residue_summarized_per_chain_chain;
		std::vector<ResidueScore> cadscores_residue_residue_summarized_per_residue;
		std::vector<ChainScore> cadscores_residue_residue_summarized_per_chain;
		GlobalScore cadscores_residue_residue_summarized_globally;

		std::vector<ChainChainScore> cadscores_chain_chain;
		std::vector<ChainScore> cadscores_chain_chain_contact_summarized_per_chain;
		GlobalScore cadscores_chain_chain_summarized_globally;

		std::vector<AtomScore> cadscores_atom_sas;
		std::vector<ResidueScore> cadscores_atom_sas_summarized_per_residue;
		std::vector<ChainScore> cadscores_atom_sas_summarized_per_chain;
		GlobalScore cadscores_atom_sas_summarized_globally;

		std::vector<ResidueScore> cadscores_residue_sas;
		std::vector<ChainScore> cadscores_residue_sas_summarized_per_chain;
		GlobalScore cadscores_residue_sas_summarized_globally;

		std::vector<ChainScore> cadscores_chain_sas;
		GlobalScore cadscores_chain_sas_summarized_globally;

		std::vector<AtomScore> cadscores_atom_site;
		std::vector<ResidueScore> cadscores_atom_site_summarized_per_residue;
		std::vector<ChainScore> cadscores_atom_site_summarized_per_chain;
		GlobalScore cadscores_atom_site_summarized_globally;

		std::vector<ResidueScore> cadscores_residue_site;
		std::vector<ChainScore> cadscores_residue_site_summarized_per_chain;
		GlobalScore cadscores_residue_site_summarized_globally;

		std::vector<ChainScore> cadscores_chain_site;
		GlobalScore cadscores_chain_site_summarized_globally;

		std::string renaming_of_model_chains;
	};

	template<class ScoreRecord>
	static void record_id_atom(const cadscorelt::IDAtom& id, ScoreRecord& s)
	{
		s.ID_chain=id.id_residue.id_chain.chain_name;
		s.ID_residue_seq_number=id.id_residue.residue_seq_number;
		s.ID_residue_icode=id.id_residue.residue_icode;
		s.ID_atom_name=id.atom_name;
	}

	template<class ScoreRecord>
	static void record_id_residue(const cadscorelt::IDResidue& id, ScoreRecord& s)
	{
		s.ID_chain=id.id_chain.chain_name;
		s.ID_residue_seq_number=id.residue_seq_number;
		s.ID_residue_icode=id.residue_icode;
	}

	template<class ScoreRecord>
	static void record_id_chain(const cadscorelt::IDChain& id, ScoreRecord& s)
	{
		s.ID_chain=id.chain_name;
	}

	template<class ScoreRecord>
	static void record_id_atom_atom(const cadscorelt::IDAtomAtom& id, ScoreRecord& s)
	{
		s.ID1_chain=id.id_a.id_residue.id_chain.chain_name;
		s.ID1_residue_seq_number=id.id_a.id_residue.residue_seq_number;
		s.ID1_residue_icode=id.id_a.id_residue.residue_icode;
		s.ID1_atom_name=id.id_a.atom_name;
		s.ID2_chain=id.id_b.id_residue.id_chain.chain_name;
		s.ID2_residue_seq_number=id.id_b.id_residue.residue_seq_number;
		s.ID2_residue_icode=id.id_b.id_residue.residue_icode;
		s.ID2_atom_name=id.id_b.atom_name;
	}

	template<class ScoreRecord>
	static void record_id_residue_residue(const cadscorelt::IDResidueResidue& id, ScoreRecord& s)
	{
		s.ID1_chain=id.id_a.id_chain.chain_name;
		s.ID1_residue_seq_number=id.id_a.residue_seq_number;
		s.ID1_residue_icode=id.id_a.residue_icode;
		s.ID2_chain=id.id_b.id_chain.chain_name;
		s.ID2_residue_seq_number=id.id_b.residue_seq_number;
		s.ID2_residue_icode=id.id_b.residue_icode;
	}

	template<class ScoreRecord>
	static void record_id_chain_chain(const cadscorelt::IDChainChain& id, ScoreRecord& s)
	{
		s.ID1_chain=id.id_a.chain_name;
		s.ID2_chain=id.id_b.chain_name;
	}

	template<class ScoreRecord>
	static void record_cad_descriptor(const cadscorelt::CADDescriptor& cadd, ScoreRecord& s)
	{
		s.cadscore=cadd.score();
		s.F1_of_areas=cadd.score_F1();
		s.target_area=cadd.target_area_sum;
		s.model_area=cadd.model_target_area_sum;
		s.TP_area=cadd.confusion_TP;
		s.FP_area=cadd.confusion_FP;
		s.FN_area=cadd.confusion_FN;
	}

	static void record_global_cad_descriptor(const std::string& target_name, const std::string& model_name, const std::string& renaming_of_model_chains, const cadscorelt::CADDescriptor& cadd, GlobalScore& s)
	{
		s.target_name=target_name;
		s.model_name=model_name;
		s.renaming_of_model_chains=renaming_of_model_chains;
		record_cad_descriptor(cadd, s);
	}

	static void record_vector(const std::map<cadscorelt::IDAtom, cadscorelt::CADDescriptor>& source, std::vector<AtomScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDAtom, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_atom(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	static void record_vector(const std::map<cadscorelt::IDResidue, cadscorelt::CADDescriptor>& source, std::vector<ResidueScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDResidue, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_residue(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	static void record_vector(const std::map<cadscorelt::IDChain, cadscorelt::CADDescriptor>& source, std::vector<ChainScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDChain, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_chain(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	static void record_vector(const std::map<cadscorelt::IDAtomAtom, cadscorelt::CADDescriptor>& source, std::vector<AtomAtomScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDAtomAtom, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_atom_atom(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	static void record_vector(const std::map<cadscorelt::IDResidueResidue, cadscorelt::CADDescriptor>& source, std::vector<ResidueResidueScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDResidueResidue, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_residue_residue(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	static void record_vector(const std::map<cadscorelt::IDChainChain, cadscorelt::CADDescriptor>& source, std::vector<ChainChainScore>& dest)
	{
		if(source.empty())
		{
			dest.clear();
		}
		else
		{
			dest.resize(source.size());
			std::size_t i=0;
			for(std::map<cadscorelt::IDChainChain, cadscorelt::CADDescriptor>::const_iterator it=source.begin();it!=source.end();++it)
			{
				record_id_chain_chain(it->first, dest[i]);
				record_cad_descriptor(it->second, dest[i]);
				i++;
			}
		}
	}

	void add_structure(const MolecularFileInput& input_file_info, const std::vector<MolecularAtomBall>& input_atom_balls, const bool as_target, const bool as_model, const std::string& name)
	{
		if(!as_target && !as_model)
		{
			throw std::runtime_error(std::string("Input type (target or model) not specified for input name '")+name+"'.");
		}
		if(name.empty())
		{
			throw std::runtime_error(std::string("Empty name provided."));
		}
		if(scorable_objects_.count(name)>0)
		{
			throw std::runtime_error(std::string("Cannot add structure, because a structure named '")+name+"' already exists.");
		}
		if(input_file_info.input_file_path.empty() && input_atom_balls.empty())
		{
			throw std::runtime_error(std::string("No input for structure with name '")+name+"'.");
		}
		if(!input_file_info.input_file_path.empty() && !input_atom_balls.empty())
		{
			throw std::runtime_error(std::string("Conflicting input sources for structure with name '")+name+"'.");
		}

		std::map<std::string, cadscorelt::ScorableData>::iterator it=scorable_objects_.emplace_hint(scorable_objects_.end(), name, cadscorelt::ScorableData());

		cadscorelt::ScorableData& sd=it->second;
		std::ostringstream local_error_stream;
		if(!input_file_info.input_file_path.empty())
		{
			sd.construct(scorable_data_construction_parameters_, cadscorelt::MolecularFileInput(input_file_info.input_file_path, input_file_info.include_heteroatoms, input_file_info.read_as_assembly), local_error_stream);
		}
		else if(!input_atom_balls.empty())
		{
			std::vector<cadscorelt::AtomBall> atom_balls(input_atom_balls.size());
			for(std::size_t i=0;i<atom_balls.size();i++)
			{
				const MolecularAtomBall& iab=input_atom_balls[i];
				cadscorelt::AtomBall& ab=atom_balls[i];
				ab.id_atom.id_residue.id_chain.chain_name=iab.ID_chain;
				ab.id_atom.id_residue.residue_seq_number=iab.ID_residue_seq_number;
				ab.id_atom.id_residue.residue_icode=iab.ID_residue_icode;
				ab.id_atom.atom_name=iab.ID_atom_name;
				ab.residue_name=iab.residue_name;
				ab.element=iab.element;
				ab.x=iab.x;
				ab.y=iab.y;
				ab.z=iab.z;
				ab.r=iab.r;
			}
			sd.construct(scorable_data_construction_parameters_, atom_balls, local_error_stream);
		}

		std::string output_error_message=local_error_stream.str();
		if(!sd.valid() && output_error_message.empty())
		{
			output_error_message="unrecognized error";
		}
		if(!output_error_message.empty())
		{
			scorable_objects_.erase(it);
			throw std::runtime_error(std::string("Failed to process and add structure with name '")+name+"' due to error: "+output_error_message);
		}

		if(as_target)
		{
			target_names_.insert(name);
		}
		if(as_model)
		{
			model_names_.insert(name);
		}

		need_to_reset_global_scores_=true;
	}

	std::map< std::pair<std::string, std::string>, PairScoringResult >::const_iterator calculate_pair_scores(const std::string& target_name, const std::string& model_name)
	{
		const std::pair<std::string, std::string> pair(target_name, model_name);
		std::map< std::pair<std::string, std::string>, PairScoringResult >::iterator pair_it=full_scoring_results_.find(pair);
		if(pair_it!=full_scoring_results_.end())
		{
			return pair_it;
		}
		std::map<std::string, cadscorelt::ScorableData>::const_iterator target_it=scorable_objects_.find(target_name);
		if(target_it==scorable_objects_.end())
		{
			throw std::runtime_error(std::string("No target named '")+target_name+"'.");
		}
		std::map<std::string, cadscorelt::ScorableData>::const_iterator model_it=scorable_objects_.find(model_name);
		if(model_it==scorable_objects_.end())
		{
			throw std::runtime_error(std::string("No model named '")+model_name+"'.");
		}

		cadscorelt::ScoringResult sr;
		std::ostringstream local_err_stream;
		sr.construct(scoring_result_construction_parameters_, target_it->second, model_it->second, local_err_stream);
		std::string output_error_message=local_err_stream.str();
		if(!sr.valid() && output_error_message.empty())
		{
			output_error_message="unrecognized error";
		}
		if(!output_error_message.empty())
		{
			throw std::runtime_error(std::string("Failed to compute CAD-score for target '")+target_name+"' and model '"+model_name+"' due to error: "+output_error_message);
		}

		pair_it=full_scoring_results_.emplace_hint(full_scoring_results_.end(), pair, PairScoringResult());

		PairScoringResult& psr=pair_it->second;

		if(sr.params.chain_renaming_map.empty())
		{
			psr.renaming_of_model_chains=".";
		}
		else
		{
			std::string& summary=psr.renaming_of_model_chains;
			for(std::map<std::string, std::string>::const_iterator mit=sr.params.chain_renaming_map.begin();mit!=sr.params.chain_renaming_map.end();++mit)
			{
				summary+=(mit!=sr.params.chain_renaming_map.begin() ? ";" : "");
				summary+=mit->first;
				summary+="=";
				summary+=mit->second;
			}
		}

		record_vector(sr.cadscores_atom_atom, psr.cadscores_atom_atom);
		record_vector(sr.cadscores_atom_atom_summarized_per_residue_residue, psr.cadscores_atom_atom_summarized_per_residue_residue);
		record_vector(sr.cadscores_atom_atom_summarized_per_chain_chain, psr.cadscores_atom_atom_summarized_per_chain_chain);
		record_vector(sr.cadscores_atom_atom_summarized_per_atom, psr.cadscores_atom_atom_summarized_per_atom);
		record_vector(sr.cadscores_atom_atom_summarized_per_residue, psr.cadscores_atom_atom_summarized_per_residue);
		record_vector(sr.cadscores_atom_atom_summarized_per_chain, psr.cadscores_atom_atom_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_atom_atom_summarized_globally, psr.cadscores_atom_atom_summarized_globally);

		record_vector(sr.cadscores_residue_residue, psr.cadscores_residue_residue);
		record_vector(sr.cadscores_residue_residue_summarized_per_chain_chain, psr.cadscores_residue_residue_summarized_per_chain_chain);
		record_vector(sr.cadscores_residue_residue_summarized_per_residue, psr.cadscores_residue_residue_summarized_per_residue);
		record_vector(sr.cadscores_residue_residue_summarized_per_chain, psr.cadscores_residue_residue_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_residue_residue_summarized_globally, psr.cadscores_residue_residue_summarized_globally);

		record_vector(sr.cadscores_chain_chain, psr.cadscores_chain_chain);
		record_vector(sr.cadscores_chain_chain_contact_summarized_per_chain, psr.cadscores_chain_chain_contact_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_chain_chain_summarized_globally, psr.cadscores_chain_chain_summarized_globally);

		record_vector(sr.cadscores_atom_sas, psr.cadscores_atom_sas);
		record_vector(sr.cadscores_atom_sas_summarized_per_residue, psr.cadscores_atom_sas_summarized_per_residue);
		record_vector(sr.cadscores_atom_sas_summarized_per_chain, psr.cadscores_atom_sas_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_atom_sas_summarized_globally, psr.cadscores_atom_sas_summarized_globally);

		record_vector(sr.cadscores_residue_sas, psr.cadscores_residue_sas);
		record_vector(sr.cadscores_residue_sas_summarized_per_chain, psr.cadscores_residue_sas_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_residue_sas_summarized_globally, psr.cadscores_residue_sas_summarized_globally);

		record_vector(sr.cadscores_chain_sas, psr.cadscores_chain_sas);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_chain_sas_summarized_globally, psr.cadscores_chain_sas_summarized_globally);

		record_vector(sr.cadscores_atom_site, psr.cadscores_atom_site);
		record_vector(sr.cadscores_atom_site_summarized_per_residue, psr.cadscores_atom_site_summarized_per_residue);
		record_vector(sr.cadscores_atom_site_summarized_per_chain, psr.cadscores_atom_site_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_atom_site_summarized_globally, psr.cadscores_atom_site_summarized_globally);

		record_vector(sr.cadscores_residue_site, psr.cadscores_residue_site);
		record_vector(sr.cadscores_residue_site_summarized_per_chain, psr.cadscores_residue_site_summarized_per_chain);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_residue_site_summarized_globally, psr.cadscores_residue_site_summarized_globally);

		record_vector(sr.cadscores_chain_site, psr.cadscores_chain_site);
		record_global_cad_descriptor(target_name, model_name, psr.renaming_of_model_chains, sr.cadscores_chain_site_summarized_globally, psr.cadscores_chain_site_summarized_globally);

		return pair_it;
	}

	const PairScoringResult& get_pair_scores(const std::string& target_name, const std::string& model_name)
	{
		std::map< std::pair<std::string, std::string>, PairScoringResult >::const_iterator it=full_scoring_results_.find(std::pair<std::string, std::string>(target_name, model_name));
		if(it==full_scoring_results_.end())
		{
			std::set<std::string>::const_iterator a1=target_names_.find(target_name);
			if(a1==target_names_.end())
			{
				throw std::runtime_error(std::string("No target named '")+target_name+"'.");
			}
			else
			{
				std::set<std::string>::const_iterator a2=model_names_.find(model_name);
				if(a2==model_names_.end())
				{
					throw std::runtime_error(std::string("No model named '")+model_name+"'.");
				}
				else
				{
					it=calculate_pair_scores(target_name, model_name);
				}
			}
			if(it==full_scoring_results_.end())
			{
				throw std::runtime_error(std::string("No scores computed for target '")+target_name+"' and model '"+model_name+"'.");
			}
		}
		return it->second;
	}

	const GlobalScoringResult& calculate_all_global_scores()
	{
		global_scoring_result_.clear();
		std::size_t num_of_pair_scores=0;
		for(std::set<std::string>::const_iterator it1=target_names_.begin();it1!=target_names_.end();++it1)
		{
			for(std::set<std::string>::const_iterator it2=model_names_.begin();it2!=model_names_.end();++it2)
			{
				if(params_.include_self_to_self_scores || (*it1)!=(*it2))
				{
					calculate_pair_scores(*it1, *it2);
					num_of_pair_scores++;
				}
			}
		}
		if(num_of_pair_scores==0)
		{
			throw std::runtime_error(std::string("No valid pairs of structures to calculate scores."));
		}
		for(std::map< std::pair<std::string, std::string>, PairScoringResult >::const_iterator it=full_scoring_results_.begin();it!=full_scoring_results_.end();++it)
		{
			if(params_.include_self_to_self_scores || (it->first.first)!=(it->first.second))
			{
				const PairScoringResult& psr=it->second;
				if(params_.score_atom_atom_contacts)
				{
					global_scoring_result_.cadscores_atom_atom_summarized_globally.push_back(psr.cadscores_atom_atom_summarized_globally);
				}
				if(params_.score_residue_residue_contacts)
				{
					global_scoring_result_.cadscores_residue_residue_summarized_globally.push_back(psr.cadscores_residue_residue_summarized_globally);
				}
				if(params_.score_chain_chain_contacts)
				{
					global_scoring_result_.cadscores_chain_chain_summarized_globally.push_back(psr.cadscores_chain_chain_summarized_globally);
				}
				if(params_.score_atom_sas)
				{
					global_scoring_result_.cadscores_atom_sas_summarized_globally.push_back(psr.cadscores_atom_sas_summarized_globally);
				}
				if(params_.score_residue_sas)
				{
					global_scoring_result_.cadscores_residue_sas_summarized_globally.push_back(psr.cadscores_residue_sas_summarized_globally);
				}
				if(params_.score_chain_sas)
				{
					global_scoring_result_.cadscores_chain_sas_summarized_globally.push_back(psr.cadscores_chain_sas_summarized_globally);
				}
				if(params_.score_atom_sites)
				{
					global_scoring_result_.cadscores_atom_site_summarized_globally.push_back(psr.cadscores_atom_site_summarized_globally);
				}
				if(params_.score_residue_sites)
				{
					global_scoring_result_.cadscores_residue_site_summarized_globally.push_back(psr.cadscores_residue_site_summarized_globally);
				}
				if(params_.score_chain_sites)
				{
					global_scoring_result_.cadscores_chain_site_summarized_globally.push_back(psr.cadscores_chain_site_summarized_globally);
				}
			}
		}
		global_scoring_result_.sort_by_cadscore();
		return global_scoring_result_;
	}

	const GlobalScoringResult& get_all_global_scores()
	{
		if(target_names_.empty())
		{
			throw std::runtime_error(std::string("No targets available."));
		}
		if(model_names_.empty())
		{
			throw std::runtime_error(std::string("No models available."));
		}
		if(need_to_reset_global_scores_)
		{
			calculate_all_global_scores();
			need_to_reset_global_scores_=false;
		}
		return global_scoring_result_;
	}

	bool need_to_reset_global_scores_;
	CADScoreParameters params_;
	cadscorelt::ScorableData::ConstructionParameters scorable_data_construction_parameters_;
	cadscorelt::ScoringResult::ConstructionParameters scoring_result_construction_parameters_;
	std::set<std::string> target_names_;
	std::set<std::string> model_names_;
	std::map<std::string, cadscorelt::ScorableData> scorable_objects_;
	std::map< std::pair<std::string, std::string>, PairScoringResult > full_scoring_results_;
	GlobalScoringResult global_scoring_result_;
};

void enable_considering_residue_names()
{
	cadscorelt::IDResidue::consider_residue_names()=true;
}

void disable_considering_residue_names()
{
	cadscorelt::IDResidue::consider_residue_names()=false;
}

void reset_atom_names_conflation_rules()
{
	cadscorelt::ConflationOfAtomNames::reset_conflation_rules();
}

void add_atom_names_conflation_rule(const std::string& residue_name, const std::string& atom_name, const std::string& conflated_atom_name)
{
	if(cadscorelt::ConflationOfAtomNames::add_conflation_rule(residue_name, atom_name, conflated_atom_name))
	{
		throw std::runtime_error(std::string("Failed to add atom names conflation rule for residue '")+residue_name+"' atom '"+atom_name+"' -> '"+conflated_atom_name+"'.");
	}
}

void add_atom_names_conflation_rules_from_file(const std::string& configuration_file_path)
{
	std::string input_data;

	if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(configuration_file_path, input_data))
	{
		throw std::runtime_error(std::string("Failed to open file '")+configuration_file_path+"'");
	}

	if(input_data.empty())
	{
		throw std::runtime_error(std::string("No data in file '")+configuration_file_path+"'");
	}

	if(cadscorelt::ConflationOfAtomNames::add_conflation_rules(input_data))
	{
		throw std::runtime_error(std::string("Invalid atom names conflation configuration file '")+configuration_file_path+"'");
	}
}

void reset_molecular_radii_assignment_rules()
{
	voronotalt::MolecularRadiiAssignment::clear_radius_value_rules();
}

void configure_molecular_radii_assignment_rules(const std::string& configuration_file_path)
{
	std::string input_data;

	if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(configuration_file_path, input_data))
	{
		throw std::runtime_error(std::string("Failed to open file '")+configuration_file_path+"'");
	}

	if(input_data.empty())
	{
		throw std::runtime_error(std::string("No data in file '")+configuration_file_path+"'");
	}

	if(!voronotalt::MolecularRadiiAssignment::set_radius_value_rules(input_data))
	{
		throw std::runtime_error(std::string("Invalid atom radii configuration file '")+configuration_file_path+"'");
	}
}

#endif /* CADSCORELT_H_ */
