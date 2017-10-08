#ifndef COMMON_CONSTRUCTION_OF_ATOMIC_REPRESENTATIONS_H_
#define COMMON_CONSTRUCTION_OF_ATOMIC_REPRESENTATIONS_H_

#include "../apollota/search_for_spherical_collisions.h"

#include "construction_of_atomic_balls.h"

namespace common
{

class ConstructionOfAtomicRepresentations
{
public:
	enum ResidueType
	{
		RESIDUE_TYPE_AMINO_ACID,
		RESIDUE_TYPE_NUCLEOTIDE,
		RESIDUE_TYPE_OTHER
	};

	static bool recognize_residue_types(const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atoms, std::vector<ResidueType>& residue_types)
	{
		residue_types.clear();

		if(atoms.empty())
		{
			return false;
		}

		residue_types.resize(atoms.size(), RESIDUE_TYPE_OTHER);

		std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> > map_of_resudues;
		for(std::size_t i=0;i<atoms.size();i++)
		{
			map_of_resudues[atoms[i].crad.without_atom()].push_back(i);
		}

		for(std::map< ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=map_of_resudues.begin();it!=map_of_resudues.end();++it)
		{
			const std::vector<std::size_t>& ids=it->second;

			std::set<std::string> names;

			for(std::vector<std::size_t>::const_iterator jt=ids.begin();jt!=ids.end();++jt)
			{
				const std::size_t id=(*jt);
				names.insert(atoms[id].crad.name);
			}

			ResidueType residue_type=RESIDUE_TYPE_OTHER;

			if(names.count("CA")+names.count("C")+names.count("N")+names.count("O")==4)
			{
				residue_type=RESIDUE_TYPE_AMINO_ACID;
			}
			else if(names.count("C2")+names.count("C3'")+names.count("P")==3)
			{
				residue_type=RESIDUE_TYPE_NUCLEOTIDE;
			}

			for(std::vector<std::size_t>::const_iterator jt=ids.begin();jt!=ids.end();++jt)
			{
				const std::size_t id=(*jt);
				residue_types[id]=residue_type;
			}
		}

		return true;
	}

	static bool construct_atomic_links(
			const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atoms,
			std::vector< std::set<std::size_t> >& links,
			const double ball_collision_radius=0.8,
			const double bsh_initial_radius=4.0)
	{
		links.clear();

		if(atoms.empty())
		{
			return false;
		}

		links.resize(atoms.size());

		std::vector<apollota::SimpleSphere> spheres(atoms.size());
		for(std::size_t i=0;i<atoms.size();i++)
		{
			spheres[i]=apollota::SimpleSphere(atoms[i].value, ball_collision_radius);
		}

		apollota::BoundingSpheresHierarchy bsh(spheres, bsh_initial_radius, 1);
		for(std::size_t i=0;i<atoms.size();i++)
		{
			const std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
			for(std::size_t j=0;j<collisions.size();j++)
			{
				const std::size_t collision_id=collisions[j];
				if(i!=collision_id)
				{
					links[i].insert(collision_id);
				}
			}
		}

		return true;
	}

	static bool construct_residue_trace_links(
			const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atoms,
			std::map< std::size_t, std::set<std::size_t> >& links_for_protein,
			std::map< std::size_t, std::set<std::size_t> >& links_for_nucleic_acid)
	{
		links_for_protein.clear();
		links_for_nucleic_acid.clear();

		if(atoms.empty())
		{
			return false;
		}

		std::vector<ResidueType> residue_types;

		if(!recognize_residue_types(atoms, residue_types) || residue_types.size()!=atoms.size())
		{
			return false;
		}

		std::vector<std::size_t> ids_protein;
		std::vector<std::size_t> ids_nucleic_acid;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			if(residue_types[i]==RESIDUE_TYPE_AMINO_ACID && atoms[i].crad.name=="CA")
			{
				ids_protein.push_back(i);
			}
			else if(residue_types[i]==RESIDUE_TYPE_NUCLEOTIDE && atoms[i].crad.name=="C3'")
			{
				ids_nucleic_acid.push_back(i);
			}
		}

		construct_residue_trace_links(atoms, ids_protein, 4.0, links_for_protein);
		construct_residue_trace_links(atoms, ids_nucleic_acid, 8.0, links_for_nucleic_acid);

		return true;
	}

private:
	void construct_residue_trace_links(
			const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atoms,
			const std::vector<std::size_t>& ids,
			const double max_distance,
			std::map< std::size_t, std::set<std::size_t> >& links)
	{
		for(std::vector<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			const std::size_t id=(*it);
			std::set<std::size_t>& neighbors=links[id];

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
	}
};

}

#endif /* COMMON_CONSTRUCTION_OF_ATOMIC_REPRESENTATIONS_H_ */
