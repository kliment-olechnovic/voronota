#ifndef COMMON_CONSTRUCTION_OF_SECONDARY_STRUCTURE_H_
#define COMMON_CONSTRUCTION_OF_SECONDARY_STRUCTURE_H_

#include "../apollota/search_for_spherical_collisions.h"

#include "construction_of_primary_structure.h"

namespace voronota
{

namespace common
{

class ConstructionOfSecondaryStructure
{
public:
	typedef ConstructionOfPrimaryStructure::Atom Atom;

	enum SecondaryStructureType
	{
		SECONDARY_STRUCTURE_TYPE_NULL,
		SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX,
		SECONDARY_STRUCTURE_TYPE_BETA_STRAND
	};

	struct ResidueDescriptor
	{
		SecondaryStructureType secondary_structure_type;
		std::pair<double, std::size_t> hbond_donated;
		std::pair<double, std::size_t> hbond_accepted;

		ResidueDescriptor() :
			secondary_structure_type(SECONDARY_STRUCTURE_TYPE_NULL),
			hbond_donated(0.0, 0),
			hbond_accepted(0.0, 0)
		{
		}
	};

	class ParametersToConstructBundleOfSecondaryStructure
	{
	public:
		double max_dist_between_CA_atoms;
		double max_hbond_energy;

		ParametersToConstructBundleOfSecondaryStructure() :
			max_dist_between_CA_atoms(9.0),
			max_hbond_energy(-500.0)
		{
		}
	};

	struct BundleOfSecondaryStructure
	{
		ParametersToConstructBundleOfSecondaryStructure parameters_of_construction;
		std::vector<ResidueDescriptor> residue_descriptors;

		bool valid(const std::vector<Atom>& atoms, const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& bundle_of_primary_structure) const
		{
			return (!residue_descriptors.empty() && residue_descriptors.size()==bundle_of_primary_structure.residues.size() && bundle_of_primary_structure.valid(atoms));
		}
	};

	static bool construct_bundle_of_secondary_structure(
			const ParametersToConstructBundleOfSecondaryStructure& parameters,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& bundle_of_primary_structure,
			BundleOfSecondaryStructure& bundle_of_secondary_structure)
	{
		bundle_of_secondary_structure=BundleOfSecondaryStructure();
		bundle_of_secondary_structure.parameters_of_construction=parameters;

		if(!bundle_of_primary_structure.valid(atoms))
		{
			return false;
		}

		bundle_of_secondary_structure.residue_descriptors.resize(bundle_of_primary_structure.residues.size());

		std::vector<ResidueMainChainDescriptor> residue_main_chain_descriptors;

		{
			std::vector<ResidueMainChainDescriptor> raw_residue_main_chain_descriptors;
			raw_residue_main_chain_descriptors.reserve(bundle_of_primary_structure.residues.size());
			for(std::size_t i=0;i<bundle_of_primary_structure.residues.size();i++)
			{
				const ConstructionOfPrimaryStructure::Residue& res=bundle_of_primary_structure.residues[i];
				if(res.residue_type==ConstructionOfPrimaryStructure::RESIDUE_TYPE_AMINO_ACID)
				{
					ResidueMainChainDescriptor rmcd;
					rmcd.residue_id=i;
					for(std::size_t j=0;j<res.atom_ids.size();j++)
					{
						const Atom& atom=atoms[res.atom_ids[j]];
						if(atom.crad.name=="CA")
						{
							rmcd.CA=std::make_pair(true, apollota::SimplePoint(atom.value));
						}
						else if(atom.crad.name=="C")
						{
							rmcd.C=std::make_pair(true, apollota::SimplePoint(atom.value));
						}
						else if(atom.crad.name=="N")
						{
							rmcd.N=std::make_pair(true, apollota::SimplePoint(atom.value));
						}
						else if(atom.crad.name=="O")
						{
							rmcd.O=std::make_pair(true, apollota::SimplePoint(atom.value));
						}
					}

					if(rmcd.almost_valid())
					{
						raw_residue_main_chain_descriptors.push_back(rmcd);
					}
				}
			}

			residue_main_chain_descriptors.reserve(raw_residue_main_chain_descriptors.size());
			for(std::size_t i=0;i+1<raw_residue_main_chain_descriptors.size();i++)
			{
				const ResidueMainChainDescriptor& rmcd_a=raw_residue_main_chain_descriptors[i];
				const ConstructionOfPrimaryStructure::Residue& r_a=bundle_of_primary_structure.residues[rmcd_a.residue_id];
				ResidueMainChainDescriptor& rmcd_b=raw_residue_main_chain_descriptors[i+1];
				const ConstructionOfPrimaryStructure::Residue& r_b=bundle_of_primary_structure.residues[rmcd_b.residue_id];
				if(r_a.distance_in_segment(r_a, r_b)==1)
				{
					if(rmcd_b.calculate_H(rmcd_a) && rmcd_b.fully_valid())
					{
						residue_main_chain_descriptors.push_back(rmcd_b);
					}
				}
			}
		}

		if(residue_main_chain_descriptors.empty())
		{
			return true;
		}

		{
			std::vector< std::vector<std::size_t> > rough_graph(residue_main_chain_descriptors.size());

			{
				std::vector<apollota::SimpleSphere> CA_anchors(residue_main_chain_descriptors.size());

				for(std::size_t i=0;i<CA_anchors.size();i++)
				{
					CA_anchors[i]=apollota::SimpleSphere(residue_main_chain_descriptors[i].CA.second, parameters.max_dist_between_CA_atoms*0.5);
				}

				apollota::BoundingSpheresHierarchy bsh(CA_anchors, parameters.max_dist_between_CA_atoms*2.5, 1);

				for(std::size_t i=0;i<CA_anchors.size();i++)
				{
					const std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, CA_anchors[i]);
					for(std::size_t j=0;j<collisions.size();j++)
					{
						if(i<collisions[j])
						{
							rough_graph[i].push_back(collisions[j]);
						}
					}
				}
			}

			for(std::size_t i=0;i<rough_graph.size();i++)
			{
				const ResidueMainChainDescriptor& rmcd_a=residue_main_chain_descriptors[i];
				ResidueDescriptor& rd_a=bundle_of_secondary_structure.residue_descriptors[rmcd_a.residue_id];
				const ConstructionOfPrimaryStructure::Residue& r_a=bundle_of_primary_structure.residues[rmcd_a.residue_id];
				for(std::size_t j=0;j<rough_graph[i].size();j++)
				{
					const ResidueMainChainDescriptor& rmcd_b=residue_main_chain_descriptors[rough_graph[i][j]];
					ResidueDescriptor& rd_b=bundle_of_secondary_structure.residue_descriptors[rmcd_b.residue_id];
					const ConstructionOfPrimaryStructure::Residue& r_b=bundle_of_primary_structure.residues[rmcd_b.residue_id];
					if(r_a.distance_in_segment(r_a, r_b)>1)
					{
						for(int e=0;e<2;e++)
						{
							const ResidueMainChainDescriptor& rmcd1=(e==0 ? rmcd_a : rmcd_b);
							const ResidueMainChainDescriptor& rmcd2=(e==0 ? rmcd_b : rmcd_a);
							ResidueDescriptor& rd1=(e==0 ? rd_a : rd_b);
							ResidueDescriptor& rd2=(e==0 ? rd_b : rd_a);
							const double energy=calculate_hbond_energy(rmcd1, rmcd2);
							if(energy<parameters.max_hbond_energy && energy<rd1.hbond_donated.first && energy<rd2.hbond_accepted.first)
							{
								if(rd1.hbond_donated.first<0.0)
								{
									bundle_of_secondary_structure.residue_descriptors[rd1.hbond_donated.second].hbond_accepted.first=0.0;
								}
								if(rd2.hbond_accepted.first<0.0)
								{
									bundle_of_secondary_structure.residue_descriptors[rd2.hbond_accepted.second].hbond_donated.first=0.0;
								}
								rd1.hbond_donated.first=energy;
								rd1.hbond_donated.second=rmcd2.residue_id;
								rd2.hbond_accepted.first=energy;
								rd2.hbond_accepted.second=rmcd1.residue_id;
							}
						}
					}
				}
			}
		}

		const std::vector<ConstructionOfPrimaryStructure::Residue>& rs=bundle_of_primary_structure.residues;
		std::vector<ResidueDescriptor>& rds=bundle_of_secondary_structure.residue_descriptors;

		{
			int periods[3]={4,3,5};
			for(int t=0;t<3;t++)
			{
				const int period=periods[t];
				for(std::size_t i=0;i<rds.size();i++)
				{
					if(check_hbond(rs, rds, i, -1, i, period-1) && check_hbond(rs, rds, i, 0, i, period) && check_hbond(rs, rds, i, 1, i, period+1))
					{
						for(std::size_t j=0;j<=static_cast<std::size_t>(period) && (i+j)<rds.size();j++)
						{
							rds[i+j].secondary_structure_type=SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX;
						}
					}
				}
			}
		}

		{
			std::vector< std::pair<int, std::size_t> > bridges(rds.size(), std::pair<int, std::size_t>(0, 0));

			for(std::size_t i=0;i<rds.size();i++)
			{
				std::pair<int, std::size_t> bridge(0, 0);

				if(bridge.first==0 && i>0 && rds[i-1].hbond_accepted.first<0)
				{
					const std::size_t j=rds[i-1].hbond_accepted.second;
					if(check_hbond(rs, rds, i, -1, j, 0) && check_hbond(rs, rds, j, 0, i, 1))
					{
						bridge.first=1;
						bridge.second=j;
					}
				}

				if(bridge.first==0 && rds[i].hbond_accepted.first<0 && rds[i].hbond_accepted.second>0)
				{
					const std::size_t j=rds[i].hbond_accepted.second-1;
					if(check_hbond(rs, rds, j, -1, i, 0) && check_hbond(rs, rds, i, 0, j, 1))
					{
						bridge.first=1;
						bridge.second=j;
					}
				}

				if(bridge.first==0 && rds[i].hbond_accepted.first<0)
				{
					const std::size_t j=rds[i].hbond_accepted.second;
					if(check_hbond(rs, rds, i, 0, j, 0) && check_hbond(rs, rds, j, 0, i, 0))
					{
						bridge.first=2;
						bridge.second=j;
					}
				}

				if(bridge.first==0 && i>0 && rds[i-1].hbond_accepted.first<0 && rds[i-1].hbond_accepted.second>0)
				{
					const std::size_t j=rds[i-1].hbond_accepted.second-1;
					if(check_hbond(rs, rds, i, -1, j, 1) && check_hbond(rs, rds, j, -1, i, 1))
					{
						bridge.first=2;
						bridge.second=j;
					}
				}

				if(bridge.first>0 && abs(static_cast<int>(i)-static_cast<int>(bridge.second))>=3)
				{
					bridges[i]=bridge;
				}
			}

			for(std::size_t i=0;i<rds.size();i++)
			{
				if(bridges[i].first>0)
				{
					rds[i].secondary_structure_type=SECONDARY_STRUCTURE_TYPE_BETA_STRAND;
				}
			}
		}

		return true;
	}

	static BundleOfSecondaryStructure construct_bundle_of_secondary_structure(
			const ParametersToConstructBundleOfSecondaryStructure& parameters,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& bundle_of_primary_structure)
	{
		BundleOfSecondaryStructure bundle_of_secondary_structure;
		if(construct_bundle_of_secondary_structure(parameters, atoms, bundle_of_primary_structure, bundle_of_secondary_structure))
		{
			return bundle_of_secondary_structure;
		}
		return BundleOfSecondaryStructure();
	}

	static BundleOfSecondaryStructure construct_bundle_of_secondary_structure(
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& bundle_of_primary_structure)
	{
		return construct_bundle_of_secondary_structure(ParametersToConstructBundleOfSecondaryStructure(), atoms, bundle_of_primary_structure);
	}

private:
	struct ResidueMainChainDescriptor
	{
		std::size_t residue_id;
		std::pair<bool, apollota::SimplePoint> CA;
		std::pair<bool, apollota::SimplePoint> C;
		std::pair<bool, apollota::SimplePoint> N;
		std::pair<bool, apollota::SimplePoint> O;
		std::pair<bool, apollota::SimplePoint> H;

		ResidueMainChainDescriptor() :
			residue_id(0),
			CA(false, apollota::SimplePoint()),
			C(false, apollota::SimplePoint()),
			N(false, apollota::SimplePoint()),
			O(false, apollota::SimplePoint()),
			H(false, apollota::SimplePoint())
		{
		}

		bool almost_valid() const
		{
			return (CA.first && C.first && N.first && O.first);
		}

		bool fully_valid() const
		{
			return (almost_valid() && H.first);
		}

		bool calculate_H(const ResidueMainChainDescriptor& prev)
		{
			if(almost_valid() && prev.almost_valid())
			{
				const apollota::SimplePoint CO=(prev.C.second-prev.O.second);
				if(CO.module()>0.0)
				{
					H=std::make_pair(true, N.second+CO.unit());
					return true;
				}
			}
			return false;
		}
	};

	static double calculate_hbond_energy(const ResidueMainChainDescriptor& donor, const ResidueMainChainDescriptor& acceptor)
	{
		const apollota::SimplePoint& N=donor.N.second;
		const apollota::SimplePoint& H=donor.H.second;
		const apollota::SimplePoint& C=acceptor.C.second;
		const apollota::SimplePoint& O=acceptor.O.second;

		const double dist_ON=apollota::distance_from_point_to_point(O, N);
		const double dist_CH=apollota::distance_from_point_to_point(C, H);
		const double dist_OH=apollota::distance_from_point_to_point(O, H);
		const double dist_CN=apollota::distance_from_point_to_point(C, N);

		const double w=27888.0;
		const double energy=(w/dist_ON+w/dist_CH-w/dist_OH-w/dist_CN);

		return energy;
	}

	static bool check_hbond(
			const std::vector<ConstructionOfPrimaryStructure::Residue>& rs,
			const std::vector<ResidueDescriptor>& rds,
			const std::size_t i,
			const int di,
			const std::size_t j,
			const int dj)
	{
		if(!(rs.size()==rds.size() && i<rs.size() && j<rs.size()))
		{
			return false;
		}

		if(di==0 && dj==0)
		{
			return (rds[i].hbond_accepted.first<0.0 && rds[i].hbond_accepted.second==j);
		}

		if(static_cast<int>(i)+di<0 || static_cast<int>(j)+dj<0)
		{
			return false;
		}

		const std::size_t ti=static_cast<std::size_t>(static_cast<int>(i)+di);
		const std::size_t tj=static_cast<std::size_t>(static_cast<int>(j)+dj);

		if(!(ti<rs.size() && tj<rs.size()))
		{
			return false;
		}

		if(!(rs[i].distance_in_segment(rs[i], rs[ti])==di && rs[j].distance_in_segment(rs[j], rs[tj])==dj))
		{
			return false;
		}

		return (rds[ti].hbond_accepted.first<0.0 && rds[ti].hbond_accepted.second==tj);
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_SECONDARY_STRUCTURE_H_ */
