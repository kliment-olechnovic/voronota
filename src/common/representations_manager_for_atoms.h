#ifndef COMMON_REPRESENTATIONS_MANAGER_FOR_ATOMS_H_
#define COMMON_REPRESENTATIONS_MANAGER_FOR_ATOMS_H_

#include "../apollota/search_for_spherical_collisions.h"

#include "construction_of_atomic_balls.h"

namespace common
{

class RepresentationsManagerForAtoms
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;

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

	enum ResidueType
	{
		RESIDUE_TYPE_AMINO_ACID,
		RESIDUE_TYPE_NUCLEOTIDE,
		RESIDUE_TYPE_OTHER
	};

	RepresentationsManagerForAtoms() : atoms_ptr_(0)
	{
	}

	RepresentationsManagerForAtoms(const std::vector<Atom>* atoms_ptr) : atoms_ptr_(0)
	{
		set_atoms(atoms_ptr);
	}

	const std::vector<Atom>& atoms() const
	{
		static const std::vector<Atom> empty_atoms;
		if(atoms_ptr_==0)
		{
			return empty_atoms;
		}
		else
		{
			return (*atoms_ptr_);
		}
	}

	void set_atoms(const std::vector<Atom>* atoms_ptr)
	{
		atoms_ptr_=atoms_ptr;
		collect_residues_information(atoms(), residues_, residue_types_, map_of_atoms_to_residues_, map_of_chains_to_residues_);
		construct_atomic_bonds_directed_links(atoms(), bonds_links_, map_of_atoms_to_bonds_links_);
		construct_residue_trace_directed_links(RESIDUE_TYPE_AMINO_ACID, "CA", 4.0, atoms(), map_of_atoms_to_residues_, residue_types_, protein_residue_trace_links_, map_of_atoms_to_protein_residue_trace_links_);
		construct_residue_trace_directed_links(RESIDUE_TYPE_NUCLEOTIDE, "C3'", 8.0, atoms(), map_of_atoms_to_residues_, residue_types_, na_residue_trace_links_, map_of_atoms_to_na_residue_trace_links_);
	}

private:
	static void collect_residues_information(
			const std::vector<Atom>& atoms,
			std::vector< std::vector<std::size_t> >& residues,
			std::vector<ResidueType>& residue_types,
			std::vector<std::size_t>& atoms_to_residues,
			std::map< std::string, std::vector<std::size_t> >& chains_to_residues)
	{
		residues.clear();
		residue_types.clear();
		atoms_to_residues.clear();
		chains_to_residues.clear();

		if(atoms.empty())
		{
			return;
		}

		atoms_to_residues.resize(atoms.size());

		std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_residues;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			map_of_residues[atoms[i].crad.without_atom()].push_back(i);
		}

		for(std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_residues.begin();it!=map_of_residues.end();++it)
		{
			const std::vector<std::size_t>& ids=it->second;

			std::set<std::string> names;
			for(std::vector<std::size_t>::const_iterator jt=ids.begin();jt!=ids.end();++jt)
			{
				names.insert(atoms[*jt].crad.name);
				atoms_to_residues[*jt]=residues.size();
			}

			chains_to_residues[it->first.chainID].push_back(residues.size());

			ResidueType residue_type=RESIDUE_TYPE_OTHER;
			if(names.count("CA")+names.count("C")+names.count("N")+names.count("O")==4)
			{
				residue_type=RESIDUE_TYPE_AMINO_ACID;
			}
			else if(names.count("C2")+names.count("C3'")+names.count("P")==3)
			{
				residue_type=RESIDUE_TYPE_NUCLEOTIDE;
			}

			residues.push_back(ids);
			residue_types.push_back(residue_type);
		}
	}

	static void construct_atomic_bonds_directed_links(
			const std::vector<Atom>& atoms,
			std::vector<DirectedLink>& links,
			std::vector< std::vector<std::size_t> >& map_of_links,
			const double ball_collision_radius=0.8,
			const double bsh_initial_radius=4.0)
	{
		links.clear();
		map_of_links.clear();

		if(atoms.empty())
		{
			return;
		}

		map_of_links.resize(atoms.size());

		std::vector<apollota::SimpleSphere> spheres(atoms.size());

		for(std::size_t i=0;i<atoms.size();i++)
		{
			spheres[i]=apollota::SimpleSphere(atoms[i].value, ball_collision_radius);
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
			const ResidueType target_residue_type,
			const std::string& target_atom_name,
			const double max_distance,
			const std::vector<Atom>& atoms,
			const std::vector<std::size_t>& atoms_to_residues,
			const std::vector<ResidueType>& residue_types,
			std::vector<DirectedLink>& links,
			std::vector< std::vector<std::size_t> >& map_of_links)
	{
		links.clear();
		map_of_links.clear();

		if(atoms.empty() || atoms.size()!=atoms_to_residues.size())
		{
			return;
		}

		map_of_links.resize(atoms.size());

		std::vector<std::size_t> ids;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			if(atoms_to_residues[i]<residue_types.size() && residue_types[atoms_to_residues[i]]==target_residue_type && atoms[i].crad.name==target_atom_name)
			{
				ids.push_back(i);
			}
		}

		if(ids.empty())
		{
			return;
		}

		std::map< std::size_t, std::set<std::size_t> > map_of_neighbors;

		for(std::vector<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			std::set<std::size_t>& neighbors=map_of_neighbors[id];

			std::vector<std::size_t>::const_iterator jt_forward=it;
			jt_forward++;
			std::vector<std::size_t>::const_reverse_iterator jt_backward(it);

			while((jt_forward!=ids.end() || jt_backward!=ids.rend()) && neighbors.size()<2)
			{
				if(id!=(*jt_forward) && atoms[id].crad.chainID==atoms[*jt_forward].crad.chainID && apollota::distance_from_point_to_point(atoms[id].value, atoms[*jt_forward].value)<max_distance && neighbors.size()<2)
				{
					neighbors.insert(*jt_forward);
				}
				if(id!=(*jt_backward) && atoms[id].crad.chainID==atoms[*jt_backward].crad.chainID && apollota::distance_from_point_to_point(atoms[id].value, atoms[*jt_backward].value)<max_distance && neighbors.size()<2)
				{
					neighbors.insert(*jt_backward);
				}
				jt_forward++;
				jt_backward++;
			}
		}

		for(std::map< std::size_t, std::set<std::size_t> >::const_iterator it=map_of_neighbors.begin();it!=map_of_neighbors.end();++it)
		{
			for(std::set<std::size_t>::const_iterator jt=it->second.begin();jt!=it->second.end();++jt)
			{
				map_of_links[it->first].push_back(links.size());
				links.push_back(DirectedLink(it->first, *jt));
			}
		}
	}

	const std::vector<Atom>* atoms_ptr_;
	std::vector< std::vector<std::size_t> > residues_;
	std::vector<ResidueType> residue_types_;
	std::vector<std::size_t> map_of_atoms_to_residues_;
	std::map< std::string, std::vector<std::size_t> > map_of_chains_to_residues_;
	std::vector<DirectedLink> bonds_links_;
	std::vector< std::vector<std::size_t> > map_of_atoms_to_bonds_links_;
	std::vector<DirectedLink> protein_residue_trace_links_;
	std::vector< std::vector<std::size_t> > map_of_atoms_to_protein_residue_trace_links_;
	std::vector<DirectedLink> na_residue_trace_links_;
	std::vector< std::vector<std::size_t> > map_of_atoms_to_na_residue_trace_links_;
};

}

#endif /* COMMON_REPRESENTATIONS_MANAGER_FOR_ATOMS_H_ */
