#ifndef COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_
#define COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_

#include <limits>

#include "../apollota/basic_operations_on_points.h"

#include "../auxiliaries/residue_letters_coding.h"

#include "construction_of_atomic_balls.h"

namespace voronota
{

namespace common
{

class ConstructionOfPrimaryStructure
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;

	enum ResidueType
	{
		RESIDUE_TYPE_OTHER,
		RESIDUE_TYPE_AMINO_ACID,
		RESIDUE_TYPE_NUCLEOTIDE
	};

	struct Residue
	{
		ResidueType residue_type;
		int segment_id;
		int position_in_segment;
		ChainResidueAtomDescriptor chain_residue_descriptor;
		std::vector<std::size_t> atom_ids;
		std::string short_name;

		Residue() :
			residue_type(RESIDUE_TYPE_OTHER),
			segment_id(0),
			position_in_segment(0)
		{
		}

		static int distance_in_segment(const Residue& a, const Residue& b)
		{
			if(a.segment_id==b.segment_id)
			{
				return (b.position_in_segment-a.position_in_segment);
			}
			else
			{
				return std::numeric_limits<int>::max();
			}
		}
	};

	struct Chain
	{
		ResidueType prevalent_residue_type;
		std::string name;
		std::vector<std::size_t> residue_ids;
	};

	struct BundleOfPrimaryStructure
	{
		std::vector<Residue> residues;
		std::vector<std::size_t> map_of_atoms_to_residues;
		std::vector<Chain> chains;
		std::vector<std::size_t> map_of_residues_to_chains;

		bool valid(const std::vector<Atom>& atoms) const
		{
			return (!residues.empty() && map_of_atoms_to_residues.size()==atoms.size() && !chains.empty() && map_of_residues_to_chains.size()==residues.size());
		}
	};

	static bool construct_bundle_of_primary_structure(const std::vector<Atom>& atoms, BundleOfPrimaryStructure& bundle)
	{
		bundle=BundleOfPrimaryStructure();

		if(atoms.empty())
		{
			return false;
		}

		{
			std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residues;

			for(std::size_t i=0;i<atoms.size();i++)
			{
				map_of_residues[atoms[i].crad.without_atom().without_alt_loc()].push_back(i);
			}

			bundle.residues.reserve(map_of_residues.size());

			bundle.map_of_atoms_to_residues.resize(atoms.size());

			for(std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
			{
				Residue residue;
				residue.chain_residue_descriptor=it->first;
				residue.atom_ids=it->second;

				std::set<std::string> names;
				for(std::vector<std::size_t>::const_iterator jt=residue.atom_ids.begin();jt!=residue.atom_ids.end();++jt)
				{
					names.insert(atoms[*jt].crad.name);
					bundle.map_of_atoms_to_residues[*jt]=bundle.residues.size();
				}

				if(names.count("CA")+names.count("C")+names.count("N")+names.count("O")==4)
				{
					residue.residue_type=RESIDUE_TYPE_AMINO_ACID;
				}
				else if(names.count("C2")+names.count("C3'")+names.count("O3'")+names.count("O5'")==4)
				{
					residue.residue_type=RESIDUE_TYPE_NUCLEOTIDE;
				}

				residue.short_name=auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(residue.chain_residue_descriptor.resName);
				if(residue.short_name=="X")
				{
					residue.short_name=residue.chain_residue_descriptor.resName;
				}

				bundle.residues.push_back(residue);
			}
		}

		{
			int current_segment_id=0;
			int current_position_in_segment=0;
			for(std::size_t i=1;i<bundle.residues.size();i++)
			{
				const Residue& a=bundle.residues[i-1];
				const Residue& b=bundle.residues[i];
				if(a.residue_type!=b.residue_type
						|| a.chain_residue_descriptor.chainID!=b.chain_residue_descriptor.chainID
						|| !check_for_polymeric_bond_between_residues(atoms, a, b))
				{
					current_segment_id++;
					current_position_in_segment=0;
				}
				else
				{
					current_position_in_segment++;
				}
				bundle.residues[i].segment_id=current_segment_id;
				bundle.residues[i].position_in_segment=current_position_in_segment;
			}
		}

		{
			std::map< std::string, std::vector<std::size_t> > map_of_chains;

			for(std::size_t i=0;i<bundle.residues.size();i++)
			{
				map_of_chains[bundle.residues[i].chain_residue_descriptor.chainID].push_back(i);
			}

			bundle.chains.reserve(map_of_chains.size());

			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_chains.begin();it!=map_of_chains.end();++it)
			{
				Chain chain;
				chain.name=it->first;
				chain.residue_ids=it->second;
				chain.prevalent_residue_type=RESIDUE_TYPE_OTHER;
				bundle.chains.push_back(chain);
			}

			bundle.map_of_residues_to_chains.resize(bundle.residues.size());

			for(std::size_t i=0;i<bundle.chains.size();i++)
			{
				Chain& chain=bundle.chains[i];
				if(!chain.residue_ids.empty())
				{
					std::map<ResidueType, int> map_of_residue_type_counts;
					for(std::size_t j=0;j<chain.residue_ids.size();j++)
					{
						bundle.map_of_residues_to_chains[chain.residue_ids[j]]=i;
						map_of_residue_type_counts[bundle.residues[chain.residue_ids[j]].residue_type]++;
					}
					int selected_count=0;
					for(std::map<ResidueType, int>::const_iterator it=map_of_residue_type_counts.begin();it!=map_of_residue_type_counts.end();++it)
					{
						if((it->second)>selected_count)
						{
							chain.prevalent_residue_type=it->first;
							selected_count=it->second;
						}
					}
				}
			}
		}

		return true;
	}

	static BundleOfPrimaryStructure construct_bundle_of_primary_structure(const std::vector<Atom>& atoms)
	{
		BundleOfPrimaryStructure bundle;
		if(construct_bundle_of_primary_structure(atoms, bundle))
		{
			return bundle;
		}
		return BundleOfPrimaryStructure();
	}

	static double estimate_uniqueness_of_chains(const BundleOfPrimaryStructure& bundle, const double chains_similarity_threshold)
	{
		if(bundle.chains.empty())
		{
			return 0.0;
		}

		std::size_t number_of_repeats=0;
		for(std::size_t i=1;i<bundle.chains.size();i++)
		{
			double best_similarity=0.0;
			for(std::size_t j=0;j<i;j++)
			{
				std::map<ChainResidueAtomDescriptor, int> map_of_counts;
				for(std::size_t e=0;e<bundle.chains[i].residue_ids.size();e++)
				{
					ChainResidueAtomDescriptor crd=bundle.residues[bundle.chains[i].residue_ids[e]].chain_residue_descriptor;
					crd.chainID="";
					map_of_counts[crd]++;
				}
				for(std::size_t e=0;e<bundle.chains[j].residue_ids.size();e++)
				{
					ChainResidueAtomDescriptor crd=bundle.residues[bundle.chains[j].residue_ids[e]].chain_residue_descriptor;
					crd.chainID="";
					map_of_counts[crd]++;
				}

				int number_of_matches=0;
				for(std::map<ChainResidueAtomDescriptor, int>::const_iterator it=map_of_counts.begin();it!=map_of_counts.end();++it)
				{
					if(it->second>1)
					{
						number_of_matches++;
					}
				}

				const double similarity=static_cast<double>(number_of_matches)/static_cast<double>(map_of_counts.size());
				if(similarity>best_similarity)
				{
					best_similarity=similarity;
				}
			}
			if(best_similarity>chains_similarity_threshold)
			{
				number_of_repeats++;
			}
		}

		return (static_cast<double>(bundle.chains.size()-number_of_repeats)/static_cast<double>(bundle.chains.size()));
	}

	static std::vector< std::vector<std::size_t> > find_residue_ids_of_motif(const BundleOfPrimaryStructure& bundle, const std::string& motif)
	{
		std::vector< std::vector<std::size_t> > result;

		if(motif.empty())
		{
			return result;
		}

		std::vector<std::string> motif_sequence(motif.size());
		for(std::size_t i=0;i<motif.size();i++)
		{
			motif_sequence[i]=motif.substr(i, 1);
		}

		for(std::size_t i=0;i<bundle.chains.size();i++)
		{
			const std::vector<std::size_t>& residue_ids=bundle.chains[i].residue_ids;
			std::size_t j=0;
			while((j+motif_sequence.size())<=residue_ids.size())
			{
				bool match=true;
				for(std::size_t l=0;match && l<motif_sequence.size();l++)
				{
					match=match && (bundle.residues[residue_ids[j+l]].short_name==motif_sequence[l] || motif_sequence[l]=="." || motif_sequence[l]=="?");
				}
				if(match)
				{
					result.push_back(std::vector<std::size_t>(motif_sequence.size(), 0));
					for(std::size_t l=0;l<motif_sequence.size();l++)
					{
						result.back()[l]=residue_ids[j+l];
					}
					j+=motif_sequence.size();
				}
				else
				{
					j++;
				}
			}
		}

		return result;
	}

	static bool collect_atom_ids_from_residue_ids(const BundleOfPrimaryStructure& bundle, const std::vector<std::size_t>& residue_ids, std::set<std::size_t>& atom_ids)
	{
		bool inserted=false;
		for(std::size_t j=0;j<residue_ids.size();j++)
		{
			if(residue_ids[j]<bundle.residues.size())
			{
				const std::vector<std::size_t>& residue_atom_ids=bundle.residues[residue_ids[j]].atom_ids;
				atom_ids.insert(residue_atom_ids.begin(), residue_atom_ids.end());
				inserted=true;
			}
		}
		return inserted;
	}

	static std::set<std::size_t> collect_atom_ids_from_residue_ids(const BundleOfPrimaryStructure& bundle, const std::vector<std::size_t>& residue_ids)
	{
		std::set<std::size_t> atom_ids;
		collect_atom_ids_from_residue_ids(bundle, residue_ids, atom_ids);
		return atom_ids;
	}

	static std::set<std::size_t> collect_atom_ids_from_residue_ids(const BundleOfPrimaryStructure& bundle, const std::vector< std::vector<std::size_t> >& residue_ids)
	{
		std::set<std::size_t> atom_ids;
		for(std::size_t i=0;i<residue_ids.size();i++)
		{
			collect_atom_ids_from_residue_ids(bundle, residue_ids[i], atom_ids);
		}
		return atom_ids;
	}

	static std::map<std::string, std::string> collect_chain_sequences_from_atom_ids(const BundleOfPrimaryStructure& bundle, const std::set<std::size_t>& atom_ids, const char gap_filler_letter, const bool fill_middle_gaps, const bool fill_start_gaps)
	{
		std::map< std::string, std::map<int, std::string> > needed_chains_and_residues_info;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const std::size_t atom_id=(*it);
			if(atom_id<bundle.map_of_atoms_to_residues.size())
			{
				const Residue& residue=bundle.residues[bundle.map_of_atoms_to_residues[atom_id]];
				if(residue.residue_type==RESIDUE_TYPE_AMINO_ACID || residue.residue_type==RESIDUE_TYPE_NUCLEOTIDE)
				{
					needed_chains_and_residues_info[residue.chain_residue_descriptor.chainID][residue.chain_residue_descriptor.resSeq]=residue.short_name;
				}
			}
		}

		std::map<std::string, std::string> chain_sequences;
		for(std::map< std::string, std::map<int, std::string> >::const_iterator it=needed_chains_and_residues_info.begin();it!=needed_chains_and_residues_info.end();++it)
		{
			std::string& sequence=chain_sequences[it->first];
			const std::map<int, std::string>& residues_info=it->second;
			for(std::map<int, std::string>::const_iterator jt=residues_info.begin();jt!=residues_info.end();++jt)
			{
				const int num=jt->first;
				const std::string& letter=jt->second;
				if(jt==residues_info.begin() && fill_start_gaps && num>1)
				{
					sequence+=std::string(static_cast<std::size_t>(num-1), gap_filler_letter);
				}
				if(jt!=residues_info.begin() && fill_middle_gaps)
				{
					std::map<int, std::string>::const_iterator prev_jt=jt;
					--prev_jt;
					const int prev_num=prev_jt->first;
					if(num>(prev_num+1))
					{
						sequence+=std::string(static_cast<std::size_t>(num-(prev_num+1)), gap_filler_letter);
					}
				}
				sequence+=(letter.size()==1 ? letter : std::string("X"));
			}
		}
		return chain_sequences;
	}

private:
	static bool check_for_polymeric_bond_between_residues(const std::vector<Atom>& atoms, const Residue& a, const Residue& b)
	{
		if(a.residue_type==b.residue_type)
		{
			if(a.residue_type==RESIDUE_TYPE_AMINO_ACID)
			{
				const std::size_t a_C_id=get_atom_id_by_residue_and_name(atoms, a, "C");
				const std::size_t b_N_id=get_atom_id_by_residue_and_name(atoms, b, "N");
				if(a_C_id<atoms.size() && b_N_id<atoms.size())
				{
					return (apollota::distance_from_point_to_point(atoms[a_C_id].value, atoms[b_N_id].value)<2.0);
				}
			}
			else if(a.residue_type==RESIDUE_TYPE_NUCLEOTIDE)
			{
				const std::size_t a_O3_id=get_atom_id_by_residue_and_name(atoms, a, "O3'");
				const std::size_t b_P_id=get_atom_id_by_residue_and_name(atoms, b, "P");
				if(a_O3_id<atoms.size() && b_P_id<atoms.size())
				{
					return (apollota::distance_from_point_to_point(atoms[a_O3_id].value, atoms[b_P_id].value)<2.0);
				}
			}
		}
		return false;
	}

	static std::size_t get_atom_id_by_residue_and_name(const std::vector<Atom>& atoms, const Residue& residue, const std::string& name)
	{
		for(std::size_t i=0;i<residue.atom_ids.size();i++)
		{
			const std::size_t id=residue.atom_ids[i];
			if(id<atoms.size() && atoms[id].crad.name==name)
			{
				return id;
			}
		}
		return atoms.size();
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_ */

