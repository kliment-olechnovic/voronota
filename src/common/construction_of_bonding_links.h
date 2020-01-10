#ifndef COMMON_CONSTRUCTION_OF_BONDING_LINKS_H_
#define COMMON_CONSTRUCTION_OF_BONDING_LINKS_H_

#include "../apollota/search_for_spherical_collisions.h"

#include "construction_of_primary_structure.h"

namespace voronota
{

namespace common
{

class ConstructionOfBondingLinks
{
public:
	typedef ConstructionOfPrimaryStructure::Atom Atom;

	struct DirectedLink
	{
		std::size_t a;
		std::size_t b;

		DirectedLink() : a(0), b(0)
		{
		}

		DirectedLink(const std::size_t a, const std::size_t b) : a(a), b(b)
		{
		}
	};

	struct ParametersToConstructBundleOfBondingLinks
	{
		double ball_collision_radius;
		double bsh_initial_radius;

		ParametersToConstructBundleOfBondingLinks() :
			ball_collision_radius(0.9),
			bsh_initial_radius(4.0)
		{
		}
	};

	struct BundleOfBondingLinks
	{
		ParametersToConstructBundleOfBondingLinks parameters_of_construction;
		std::vector<DirectedLink> bonds_links;
		std::vector< std::vector<std::size_t> > map_of_atoms_to_bonds_links;
		std::vector<DirectedLink> residue_trace_links;
		std::vector< std::vector<std::size_t> > map_of_atoms_to_residue_trace_links;
		std::vector< std::vector<std::size_t> > continuous_chains_of_residue_trace;

		bool valid(const std::vector<Atom>& atoms, const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure) const
		{
			return (!atoms.empty()
					&& primary_structure.valid(atoms)
					&& map_of_atoms_to_bonds_links.size()==atoms.size()
					&& map_of_atoms_to_residue_trace_links.size()==atoms.size());
		}

		void swap(BundleOfBondingLinks& b)
		{
			parameters_of_construction=b.parameters_of_construction;
			bonds_links.swap(b.bonds_links);
			map_of_atoms_to_bonds_links.swap(b.map_of_atoms_to_bonds_links);
			residue_trace_links.swap(b.residue_trace_links);
			map_of_atoms_to_residue_trace_links.swap(b.map_of_atoms_to_residue_trace_links);
			continuous_chains_of_residue_trace.swap(b.continuous_chains_of_residue_trace);
		}
	};

	static bool construct_bundle_of_bonding_links(
			const ParametersToConstructBundleOfBondingLinks& parameters,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure,
			BundleOfBondingLinks& bundle_of_bonding_links)
	{
		if(atoms.empty() || !primary_structure.valid(atoms))
		{
			return false;
		}

		bundle_of_bonding_links.parameters_of_construction=parameters;
		construct_atomic_bonds_directed_links(atoms, bundle_of_bonding_links.bonds_links, bundle_of_bonding_links.map_of_atoms_to_bonds_links, parameters.ball_collision_radius, parameters.bsh_initial_radius);
		construct_residue_trace_directed_links(atoms, primary_structure, bundle_of_bonding_links.residue_trace_links, bundle_of_bonding_links.map_of_atoms_to_residue_trace_links, bundle_of_bonding_links.continuous_chains_of_residue_trace);

		return bundle_of_bonding_links.valid(atoms, primary_structure);
	}

	static BundleOfBondingLinks construct_bundle_of_bonding_links(
			const ParametersToConstructBundleOfBondingLinks& parameters,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure)
	{
		BundleOfBondingLinks bundle_of_bonding_links;
		if(construct_bundle_of_bonding_links(parameters, atoms, primary_structure, bundle_of_bonding_links))
		{
			return bundle_of_bonding_links;
		}
		return BundleOfBondingLinks();
	}

private:
	typedef ConstructionOfPrimaryStructure COPS;

	static void construct_atomic_bonds_directed_links(
			const std::vector<Atom>& atoms,
			std::vector<DirectedLink>& links,
			std::vector< std::vector<std::size_t> >& map_of_links,
			const double ball_collision_radius,
			const double bsh_initial_radius)
	{
		map_of_links.resize(atoms.size());

		std::vector<apollota::SimpleSphere> spheres(atoms.size());

		for(std::size_t i=0;i<atoms.size();i++)
		{
			spheres[i]=apollota::SimpleSphere(atoms[i].value, ball_collision_radius);
			const std::string& name=atoms[i].crad.name;
			if(
				name.find_first_of("SPZMF", 0)==0 ||
				name.rfind("CL", 0)==0
				)
			{
				spheres[i].r+=0.2;
			}
		}

		apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);

		std::vector< std::set<std::size_t> > map_of_neighbors(atoms.size());

		for(std::size_t i=0;i<atoms.size();i++)
		{
			const std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
			for(std::size_t j=0;j<collisions.size();j++)
			{
				const std::size_t collision_id=collisions[j];
				if(i!=collision_id)
				{
					map_of_neighbors[i].insert(collision_id);
				}
			}
		}

		for(std::size_t i=0;i<map_of_neighbors.size();i++)
		{
			for(std::set<std::size_t>::const_iterator it=map_of_neighbors[i].begin();it!=map_of_neighbors[i].end();++it)
			{
				map_of_links[i].push_back(links.size());
				links.push_back(DirectedLink(i, *it));
			}
		}
	}

	static void construct_residue_trace_directed_links(
			const std::vector<Atom>& atoms,
			const COPS::BundleOfPrimaryStructure& primary_structure,
			std::vector<DirectedLink>& residue_trace_links,
			std::vector< std::vector<std::size_t> >& map_of_atoms_to_residue_trace_links,
			std::vector< std::vector<std::size_t> >& continuous_chains_of_residue_trace)
	{
		map_of_atoms_to_residue_trace_links.resize(atoms.size());

		construct_residue_trace_directed_links(COPS::RESIDUE_TYPE_AMINO_ACID, atoms, primary_structure, residue_trace_links, map_of_atoms_to_residue_trace_links, continuous_chains_of_residue_trace);
		construct_residue_trace_directed_links(COPS::RESIDUE_TYPE_NUCLEOTIDE, atoms, primary_structure, residue_trace_links, map_of_atoms_to_residue_trace_links, continuous_chains_of_residue_trace);
	}

	static void construct_residue_trace_directed_links(
			const COPS::ResidueType target_residue_type,
			const std::vector<Atom>& atoms,
			const COPS::BundleOfPrimaryStructure& primary_structure,
			std::vector<DirectedLink>& residue_trace_links,
			std::vector< std::vector<std::size_t> >& map_of_atoms_to_residue_trace_links,
			std::vector< std::vector<std::size_t> >& continuous_chains_of_residue_trace)
	{
		const std::string target_atom_name=(target_residue_type==COPS::RESIDUE_TYPE_AMINO_ACID ? "CA" : (target_residue_type==COPS::RESIDUE_TYPE_NUCLEOTIDE ? "C3'" : "X"));

		std::vector< std::vector<std::size_t> > target_atom_ids(primary_structure.residues.size());

		int found_target_atoms=0;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			const std::size_t residue_id=primary_structure.map_of_atoms_to_residues[i];
			if(residue_id<primary_structure.residues.size()
					&& primary_structure.residues[residue_id].residue_type==target_residue_type
					&& atoms[i].crad.name==target_atom_name)
			{
				target_atom_ids[residue_id].push_back(i);
				found_target_atoms++;
			}
		}

		if(found_target_atoms==0)
		{
			return;
		}

		int found_links=0;

		for(std::size_t i=0;(i+1)<primary_structure.residues.size();i++)
		{
			if(!target_atom_ids[i].empty() && !target_atom_ids[i+1].empty()
					&& primary_structure.residues[i].segment_id==primary_structure.residues[i+1].segment_id
					&& COPS::Residue::distance_in_segment(primary_structure.residues[i], primary_structure.residues[i+1])==1)
			{
				const std::size_t a=target_atom_ids[i][0];
				const std::size_t b=target_atom_ids[i+1][0];
				map_of_atoms_to_residue_trace_links[a].push_back(residue_trace_links.size());
				residue_trace_links.push_back(DirectedLink(a, b));
				map_of_atoms_to_residue_trace_links[b].push_back(residue_trace_links.size());
				residue_trace_links.push_back(DirectedLink(b, a));
				found_links++;
			}
		}

		if(found_links==0)
		{
			return;
		}

		std::vector< std::vector<std::size_t> > strands;
		for(std::size_t i=0;i<primary_structure.residues.size();i++)
		{
			if(!target_atom_ids[i].empty())
			{
				const std::size_t id=target_atom_ids[i][0];
				if(strands.empty())
				{
					strands.push_back(std::vector<std::size_t>(1, id));
				}
				else
				{
					const std::size_t prev_id=strands.back().back();
					const std::size_t prev_residue_id=primary_structure.map_of_atoms_to_residues[prev_id];
					if(primary_structure.residues[i].segment_id==primary_structure.residues[prev_residue_id].segment_id
							&& COPS::Residue::distance_in_segment(primary_structure.residues[prev_residue_id], primary_structure.residues[i])==1)
					{
						strands.back().push_back(id);
					}
					else
					{
						strands.push_back(std::vector<std::size_t>(1, id));
					}
				}
			}
		}

		if(!strands.empty())
		{
			continuous_chains_of_residue_trace.insert(continuous_chains_of_residue_trace.end(), strands.begin(), strands.end());
		}
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_BONDING_LINKS_H_ */

