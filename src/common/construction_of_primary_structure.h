#ifndef COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_
#define COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_

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
		common::ChainResidueAtomDescriptor chain_residue_descriptor;
		std::vector<std::size_t> atom_ids;

		Residue() : residue_type(RESIDUE_TYPE_OTHER)
		{
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

		BundleOfPrimaryStructure()
		{
		}

		bool valid(const std::vector<Atom>& atoms) const
		{
			return (!map_of_atoms_to_residues.empty() &&
					map_of_atoms_to_residues.size()==atoms.size() &&
					map_of_residues_to_chains.size()==residues.size());
		}
	};

	static bool construct_bundle_of_primary_structure(const std::vector<Atom>& atoms, BundleOfPrimaryStructure& bundle)
	{
		bundle=BundleOfPrimaryStructure();

		if(atoms.empty())
		{
			return false;
		}

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
			else if(names.count("C2")+names.count("C3'")+names.count("P")==3)
			{
				residue.residue_type=RESIDUE_TYPE_NUCLEOTIDE;
			}

			bundle.residues.push_back(residue);
		}

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

		return true;
	}
};

}

#endif /* COMMON_CONSTRUCTION_OF_PRIMARY_STRUCTURE_H_ */

