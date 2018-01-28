#ifndef COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_
#define COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_

#include <limits>

#include "../apollota/basic_operations_on_points.h"

#include "construction_of_atomic_balls.h"

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
		common::ChainResidueAtomDescriptor chain_residue_descriptor;
		std::vector<std::size_t> atom_ids;

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
		ResidueType residue_type;
		std::string name;
		std::vector<std::size_t> residue_ids;
	};

	struct BundleOfPrimaryStructure
	{
		std::vector<Residue> residues;
		std::vector<std::size_t> map_of_atoms_to_residues;
		std::vector<Chain> chains;
		std::vector<std::size_t> map_of_residues_to_chains;
	};

	static bool construct_bundle_of_primary_structure(const std::vector<Atom>& atoms, BundleOfPrimaryStructure& bundle)
	{
		bundle=BundleOfPrimaryStructure();

		if(atoms.empty())
		{
			return false;
		}

		{
			std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residues;

			for(std::size_t i=0;i<atoms.size();i++)
			{
				map_of_residues[atoms[i].crad.without_atom()].push_back(i);
			}

			bundle.residues.reserve(map_of_residues.size());

			bundle.map_of_atoms_to_residues.resize(atoms.size());

			for(std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
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
				else if(names.count("C2")+names.count("C3'")+names.count("O3'")+names.count("P")==3)
				{
					residue.residue_type=RESIDUE_TYPE_NUCLEOTIDE;
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

			bundle.map_of_residues_to_chains.resize(bundle.residues.size());

			for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_chains.begin();it!=map_of_chains.end();++it)
			{
				Chain chain;
				chain.name=it->first;
				chain.residue_ids=it->second;
				chain.residue_type=bundle.residues[chain.residue_ids.front()].residue_type;
				bundle.chains.push_back(chain);
			}
		}

		return true;
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

#endif /* COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_ */

