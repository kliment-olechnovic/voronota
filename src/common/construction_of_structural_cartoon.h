#ifndef COMMON_CONSTRUCTION_OF_STRUCTURAL_CARTOON_H_
#define COMMON_CONSTRUCTION_OF_STRUCTURAL_CARTOON_H_

#include "../apollota/polynomial_curves.h"

#include "../auxiliaries/opengl_printer.h"

#include "construction_of_bonding_links.h"
#include "construction_of_secondary_structure.h"

namespace voronota
{

namespace common
{

class ConstructionOfStructuralCartoon
{
public:
	typedef ConstructionOfBondingLinks::Atom Atom;

	struct Parameters
	{
		struct Spline
		{
			double k;
			int steps;

			Spline(const double spline_k, const int spline_steps) :
				k(spline_k),
				steps(spline_steps)
			{
			}
		};

		double width;
		double height;
		Spline amino_spline;
		Spline nucleic_spline;
		double amino_ss_factor;
		double amino_ss_arrow_factor;
		double nucleic_factor;
		double sidebone_width;
		int mesh_steps;
		bool factors_disabled;
		bool beta_arrows_disabled;
		bool mesh_edgy;
		bool beta_correction_disabled;
		bool factors_smoothing_enabled;

		Parameters() :
			width(0.3),
			height(0.3),
			amino_spline(0.8, 6),
			nucleic_spline(0.5, 6),
			amino_ss_factor(4.0),
			amino_ss_arrow_factor(6.0),
			nucleic_factor(3.0),
			sidebone_width(0.3),
			mesh_steps(8),
			factors_disabled(false),
			beta_arrows_disabled(false),
			mesh_edgy(false),
			beta_correction_disabled(false),
			factors_smoothing_enabled(false)
		{
		}

		static Parameters styled(const int id)
		{
			Parameters p;
			if(id==1)
			{
				p.height=0.15;
				p.mesh_edgy=true;
			}
			if(id==2)
			{
				p.width=0.4;
				p.height=0.2;
				p.amino_ss_factor=2.0;
				p.nucleic_factor=2.0;
				p.beta_correction_disabled=true;
				p.factors_smoothing_enabled=true;
			}
			return p;
		}
	};

	struct BundleOfMeshInformation
	{
		std::vector<float> global_buffer_of_vertices;
		std::vector<float> global_buffer_of_normals;
		std::vector<unsigned int> global_buffer_of_indices;
		std::vector< std::vector<unsigned int> > mapped_indices;

		void clear()
		{
			global_buffer_of_vertices.clear();
			global_buffer_of_normals.clear();
			global_buffer_of_indices.clear();
			mapped_indices.clear();
		}
	};

	static bool construct_bundle_of_mesh_information(
			const Parameters& parameters,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure,
			const ConstructionOfSecondaryStructure::BundleOfSecondaryStructure& secondary_structure,
			const ConstructionOfBondingLinks::BundleOfBondingLinks& bonding_links,
			BundleOfMeshInformation& bundle)
	{
		bundle.clear();

		if(!bonding_links.valid(atoms, primary_structure) || !secondary_structure.valid(atoms, primary_structure))
		{
			return false;
		}

		std::vector< std::vector<std::size_t> > map_of_atoms_to_ribbon_segments(atoms.size());
		std::vector<RibbonSegment> ribbon_segments;

		for(std::size_t i=0;i<bonding_links.continuous_chains_of_residue_trace.size();i++)
		{
			const std::vector<std::size_t>& chain=bonding_links.continuous_chains_of_residue_trace[i];
			if(chain.size()>3)
			{
				const std::size_t atom_id=bonding_links.continuous_chains_of_residue_trace[i].front();
				const std::size_t residue_id=primary_structure.map_of_atoms_to_residues[atom_id];
				const ConstructionOfPrimaryStructure::ResidueType residue_type=primary_structure.residues[residue_id].residue_type;
				if(residue_type==ConstructionOfPrimaryStructure::RESIDUE_TYPE_AMINO_ACID)
				{
					add_ribbon_segments<ProteinResidueOrientation>(parameters, parameters.amino_spline, atoms, primary_structure, secondary_structure, chain, ribbon_segments, map_of_atoms_to_ribbon_segments);
				}
				else if(residue_type==ConstructionOfPrimaryStructure::RESIDUE_TYPE_NUCLEOTIDE)
				{
					add_ribbon_segments<NucleicAcidResidueOrientation>(parameters, parameters.nucleic_spline, atoms, primary_structure, secondary_structure, chain, ribbon_segments, map_of_atoms_to_ribbon_segments);
				}
			}
		}

		if(ribbon_segments.empty())
		{
			return false;
		}

		bundle.mapped_indices.resize(map_of_atoms_to_ribbon_segments.size());

		{
			std::vector<float> buffer_of_vertices;
			std::vector<float> buffer_of_normals;
			std::vector<unsigned int> buffer_of_indices;

			const double pi=3.14159265358979323846;

			for(std::size_t atom_id=0;atom_id<map_of_atoms_to_ribbon_segments.size();atom_id++)
			{
				if(map_of_atoms_to_ribbon_segments[atom_id].size()==1)
				{
					const RibbonSegment& rs=ribbon_segments[map_of_atoms_to_ribbon_segments[atom_id].front()];
					auxiliaries::OpenGLPrinter opengl_printer;

					if(rs.spine.size()>1)
					{
						const int mesh_steps=(parameters.mesh_edgy ? 4 : parameters.mesh_steps);
						const double tstep=2.0*pi/static_cast<double>(mesh_steps);
						std::vector<apollota::SimplePoint> ps[2];
						std::vector<apollota::SimplePoint> ns[2];
						std::vector<apollota::SimplePoint> vertices;
						std::vector<apollota::SimplePoint> normals;
						for(std::size_t i=0;i+1<rs.spine.size();i++)
						{
							RibbonSegment::Vertebra v[2]={rs.spine[i], rs.spine[i+1]};

							for(int j=0;j<2;j++)
							{
								v[j].right=v[j].right*parameters.width;
								v[j].up=v[j].up*parameters.height;
								const double ellipse_a=v[j].right.module();
								const double ellipse_b=v[j].up.module();
								const double ellipse_c=(ellipse_a==ellipse_b ? 0.0 : sqrt(fabs(ellipse_a*ellipse_a-ellipse_b*ellipse_b)));
								ps[j].clear();
								ns[j].clear();
								for(int t=0;t<mesh_steps;t++)
								{
									const double tval=tstep*0.5+static_cast<double>(t)*tstep;
									ps[j].push_back((v[j].right*cos(tval))+(v[j].up*sin(tval)));
									if(ellipse_c==0.0)
									{
										ns[j].push_back(ps[j].back().unit());
									}
									else
									{
										ns[j].push_back(((ps[j].back()-(v[j].right.unit()*ellipse_c)).unit()+(ps[j].back()-(v[j].right.unit()*(0.0-ellipse_c))).unit()).unit());
									}
								}
							}

							if(!parameters.mesh_edgy)
							{
								vertices.clear();
								normals.clear();
								for(std::size_t t=0;t<ps[0].size();t++)
								{
									vertices.push_back(v[0].center+ps[0][t]);
									vertices.push_back(v[1].center+ps[1][t]);
									normals.push_back(ns[0][t]);
									normals.push_back(ns[1][t]);
								}
								vertices.push_back(vertices[0]);
								vertices.push_back(vertices[1]);
								normals.push_back(normals[0]);
								normals.push_back(normals[1]);
								opengl_printer.add_triangle_strip(vertices, normals);
							}
							else
							{
								for(std::size_t t1=0;t1<ps[0].size();t1++)
								{
									const std::size_t t2=(t1+1<ps[0].size() ? t1+1 : std::size_t(0));
									const apollota::SimplePoint n0=(ps[0][t1].unit()+ps[0][t2].unit()).unit();
									const apollota::SimplePoint n1=(ps[1][t1].unit()+ps[1][t2].unit()).unit();

									vertices.clear();
									normals.clear();
									vertices.push_back(v[0].center+ps[0][t1]);
									normals.push_back(n0);
									vertices.push_back(v[1].center+ps[1][t1]);
									normals.push_back(n1);
									vertices.push_back(v[0].center+ps[0][t2]);
									normals.push_back(n0);
									vertices.push_back(v[1].center+ps[1][t2]);
									normals.push_back(n1);
									opengl_printer.add_triangle_strip(vertices, normals);
								}
							}
						}
					}

					if(rs.sidebone.size()==2)
					{
						const int mesh_steps=6;
						const double tstep=2.0*pi/static_cast<double>(mesh_steps);
						const apollota::SimplePoint na=apollota::any_normal_of_vector<apollota::SimplePoint>(rs.sidebone[1]-rs.sidebone[0])*parameters.sidebone_width;
						const apollota::SimplePoint nb=((rs.sidebone[1]-rs.sidebone[0])&na).unit()*parameters.sidebone_width;
						std::vector<apollota::SimplePoint> ps;
						for(int t=0;t<mesh_steps;t++)
						{
							const double tval=tstep*0.5+static_cast<double>(t)*tstep;
							ps.push_back((na*cos(tval))+(nb*sin(tval)));
						}
						{
							std::vector<apollota::SimplePoint> vertices;
							std::vector<apollota::SimplePoint> normals;
							for(std::size_t t=0;t<ps.size();t++)
							{
								vertices.push_back(rs.sidebone[0]+ps[t]);
								vertices.push_back(rs.sidebone[1]+ps[t]);
								normals.push_back(ps[t].unit());
								normals.push_back(ps[t].unit());
							}
							vertices.push_back(vertices[0]);
							vertices.push_back(vertices[1]);
							normals.push_back(normals[0]);
							normals.push_back(normals[1]);
							opengl_printer.add_triangle_strip(vertices, normals);
						}
						{
							std::vector<apollota::SimplePoint> vertices;
							for(std::size_t t=0;t<ps.size();t++)
							{
								vertices.push_back(rs.sidebone[1]+ps[t]);
							}
							opengl_printer.add_triangle_fan(rs.sidebone[1], vertices, (rs.sidebone[1]-rs.sidebone[0]).unit());
						}
					}

					if(opengl_printer.write_to_low_level_triangle_buffers(buffer_of_vertices, buffer_of_normals, buffer_of_indices, true))
					{
						for(std::size_t j=0;j<buffer_of_indices.size();j++)
						{
							buffer_of_indices[j]+=bundle.global_buffer_of_vertices.size()/3;
						}
						bundle.global_buffer_of_vertices.insert(bundle.global_buffer_of_vertices.end(), buffer_of_vertices.begin(), buffer_of_vertices.end());
						bundle.global_buffer_of_normals.insert(bundle.global_buffer_of_normals.end(), buffer_of_normals.begin(), buffer_of_normals.end());
						bundle.global_buffer_of_indices.insert(bundle.global_buffer_of_indices.end(), buffer_of_indices.begin(), buffer_of_indices.end());
						bundle.mapped_indices[atom_id]=buffer_of_indices;
					}
				}
			}
		}

		return (!bundle.global_buffer_of_indices.empty());
	}

private:
	struct RibbonSegment
	{
		struct Vertebra
		{
			apollota::SimplePoint center;
			apollota::SimplePoint up;
			apollota::SimplePoint right;
		};

		std::size_t atom_id;
		Vertebra core;
		std::vector<Vertebra> spine;
		std::vector<apollota::SimplePoint> sidebone;
	};

	class ProteinResidueOrientation
	{
	public:
		apollota::SimplePoint CA;
		apollota::SimplePoint C;
		apollota::SimplePoint N;
		apollota::SimplePoint O;
		apollota::SimplePoint up;
		apollota::SimplePoint right;
		bool CA_flag;
		bool C_flag;
		bool N_flag;
		bool O_flag;

		ProteinResidueOrientation() : CA_flag(false), C_flag(false), N_flag(false), O_flag(false)
		{
		}

		static void orient_sequence(std::vector<ProteinResidueOrientation>& seq)
		{
			if(seq.size()>1)
			{
				seq[0].orient(seq[1].CA-seq[0].CA);
				for(std::size_t i=1;i<seq.size();i++)
				{
					seq[i].orient(seq[i-1]);
				}
				for(std::size_t i=1;i<seq.size();i++)
				{
					seq[i].reorient(seq[i-1]);
				}
			}
		}

		void fill(const ConstructionOfAtomicBalls::AtomicBall& atom)
		{
			if(atom.crad.name=="CA")
			{
				CA=apollota::SimplePoint(atom.value);
				CA_flag=true;
			}
			else if(atom.crad.name=="C")
			{
				C=apollota::SimplePoint(atom.value);
				C_flag=true;
			}
			else if(atom.crad.name=="N")
			{
				N=apollota::SimplePoint(atom.value);
				N_flag=true;
			}
			else if(atom.crad.name=="O")
			{
				O=apollota::SimplePoint(atom.value);
				O_flag=true;
			}
		}

		bool valid() const
		{
			return (CA_flag && C_flag && N_flag && O_flag);
		}

		void orient(const apollota::SimplePoint& forward)
		{
			if(valid())
			{
				right=(O-C).unit();
				up=(forward&right).unit();
			}
		}

		void orient(const ProteinResidueOrientation& prev_ro)
		{
			if(valid() && prev_ro.valid())
			{
				const apollota::SimplePoint forward=CA-prev_ro.CA;
				apollota::SimplePoint v0=(prev_ro.O-prev_ro.C).unit();
				apollota::SimplePoint v1=(O-C).unit();
				if((v0*v1)<0.0)
				{
					v0=v0.inverted();
				}
				right=((v0*0.5)+(v1*0.5)).unit();
				up=(forward&right).unit();
			}
		}

		void reorient(const ProteinResidueOrientation& reference)
		{
			if(valid())
			{
				const double v_plus=(right*reference.right);
				const double v_minus=(right.inverted()*reference.right);
				if(v_minus>v_plus)
				{
					right=right.inverted();
					up=up.inverted();
				}
			}
		}

		RibbonSegment::Vertebra get_core_vertebra() const
		{
			RibbonSegment::Vertebra core;
			core.center=CA;
			core.up=up;
			core.right=right;
			return core;
		}

		std::vector<apollota::SimplePoint> get_sidebone() const
		{
			return std::vector<apollota::SimplePoint>();
		}
	};

	class NucleicAcidResidueOrientation
	{
	public:
		apollota::SimplePoint C1m;
		apollota::SimplePoint C3m;
		apollota::SimplePoint C4;
		apollota::SimplePoint up;
		apollota::SimplePoint right;
		bool C1m_flag;
		bool C3m_flag;
		bool C4_flag;

		NucleicAcidResidueOrientation() : C1m_flag(false), C3m_flag(false), C4_flag(false)
		{
		}

		static void orient_sequence(std::vector<NucleicAcidResidueOrientation>& seq)
		{
			if(seq.size()>1)
			{
				seq[0].orient(seq[1].C3m-seq[0].C3m);
				for(std::size_t i=1;i<seq.size();i++)
				{
					seq[i].orient(seq[i].C3m-seq[i-1].C3m);
				}
				for(std::size_t i=1;i<seq.size();i++)
				{
					seq[i].reorient(seq[i-1]);
				}
			}
		}

		bool valid() const
		{
			return (C1m_flag && C3m_flag && C4_flag);
		}

		void fill(const ConstructionOfAtomicBalls::AtomicBall& atom)
		{
			if(atom.crad.name=="C1'")
			{
				C1m=apollota::SimplePoint(atom.value);
				C1m_flag=true;
			}
			else if(atom.crad.name=="C3'")
			{
				C3m=apollota::SimplePoint(atom.value);
				C3m_flag=true;
			}
			else if(atom.crad.name=="C4")
			{
				C4=apollota::SimplePoint(atom.value);
				C4_flag=true;
			}
		}

		void orient(const apollota::SimplePoint& forward)
		{
			if(valid())
			{
				right=(forward&(C4-C1m)).unit();
				up=(forward&right).unit();
			}
		}

		void reorient(const NucleicAcidResidueOrientation& reference)
		{
			if(valid())
			{
				const double v_plus=(right*reference.right);
				const double v_minus=(right.inverted()*reference.right);
				if(v_minus>v_plus)
				{
					right=right.inverted();
					up=up.inverted();
				}
			}
		}

		RibbonSegment::Vertebra get_core_vertebra() const
		{
			RibbonSegment::Vertebra core;
			core.center=C3m;
			core.up=up;
			core.right=right;
			return core;
		}

		std::vector<apollota::SimplePoint> get_sidebone() const
		{
			std::vector<apollota::SimplePoint> s(2);
			s[0]=C3m;
			s[1]=s[0]+((C4-s[0]).unit()*5.0);
			return s;
		}
	};

	static void smooth_ribbon_segments(const Parameters::Spline& parameters, std::vector<RibbonSegment>& ribbon_segments)
	{
		std::vector<apollota::SimplePoint> strip_center;
		std::vector<apollota::SimplePoint> strip_up;
		std::vector<apollota::SimplePoint> strip_right;
		for(std::size_t i=0;i+1<ribbon_segments.size();i++)
		{
			strip_center.clear();
			strip_up.clear();
			strip_right.clear();
			const RibbonSegment::Vertebra& c0=ribbon_segments[i].core;
			const RibbonSegment::Vertebra& c1=ribbon_segments[i+1].core;
			if(i==0)
			{
				const RibbonSegment::Vertebra& c2=ribbon_segments[i+2].core;
				strip_center=apollota::interpolate_using_cubic_hermite_spline(c0.center+(c0.center-c1.center), c0.center, c1.center, c2.center, parameters.k, parameters.steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(c0.up+(c0.up-c1.up), c0.up, c1.up, c2.up, parameters.k, parameters.steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(c0.right+(c0.right-c1.right), c0.right, c1.right, c2.right, parameters.k, parameters.steps);
			}
			else if(i+2==ribbon_segments.size())
			{
				const RibbonSegment::Vertebra& cM=ribbon_segments[i-1].core;
				strip_center=apollota::interpolate_using_cubic_hermite_spline(cM.center, c0.center, c1.center, c1.center+(c1.center-c0.center), parameters.k, parameters.steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(cM.up, c0.up, c1.up, c1.up+(c1.up-c0.up), parameters.k, parameters.steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(cM.right, c0.right, c1.right, c1.right+(c1.right-c0.right), parameters.k, parameters.steps);
			}
			else
			{
				const RibbonSegment::Vertebra& cM=ribbon_segments[i-1].core;
				const RibbonSegment::Vertebra& c2=ribbon_segments[i+2].core;
				strip_center=apollota::interpolate_using_cubic_hermite_spline(cM.center, c0.center, c1.center, c2.center, parameters.k, parameters.steps);
				strip_up=apollota::interpolate_using_cubic_hermite_spline(cM.up, c0.up, c1.up, c2.up, parameters.k, parameters.steps);
				strip_right=apollota::interpolate_using_cubic_hermite_spline(cM.right, c0.right, c1.right, c2.right, parameters.k, parameters.steps);
			}
			if(!strip_center.empty() && strip_center.size()==strip_up.size() && strip_center.size()==strip_right.size())
			{
				RibbonSegment::Vertebra vertebra;
				for(std::size_t j=(i==0 ? 0 : 1);j<strip_center.size();j++)
				{
					vertebra.center=strip_center[j];
					vertebra.up=strip_up[j];
					vertebra.right=strip_right[j];
					if(j<=strip_center.size()/2)
					{
						ribbon_segments[i].spine.push_back(vertebra);
					}
					if(j>=strip_center.size()/2)
					{
						ribbon_segments[i+1].spine.push_back(vertebra);
					}
				}
			}
		}
	}

	template<typename ResidueOrientation>
	static bool add_ribbon_segments(
			const Parameters& parameters,
			const Parameters::Spline& parameters_spline,
			const std::vector<Atom>& atoms,
			const ConstructionOfPrimaryStructure::BundleOfPrimaryStructure& primary_structure,
			const ConstructionOfSecondaryStructure::BundleOfSecondaryStructure& secondary_structure,
			const std::vector<std::size_t>& chain,
			std::vector<RibbonSegment>& all_ribbon_segments,
			std::vector< std::vector<std::size_t> >& map_of_atoms_to_ribbon_segments)
	{
		if(chain.size()<4 || map_of_atoms_to_ribbon_segments.size()!=atoms.size())
		{
			return false;
		}

		std::vector<ResidueOrientation> residue_orientations(chain.size());

		for(std::size_t i=0;i<chain.size();i++)
		{
			const std::size_t residue_id=primary_structure.map_of_atoms_to_residues[chain[i]];
			const std::vector<std::size_t>& residue_atom_ids=primary_structure.residues[residue_id].atom_ids;
			for(std::size_t j=0;j<residue_atom_ids.size();j++)
			{
				residue_orientations[i].fill(atoms.at(residue_atom_ids[j]));
			}
		}

		ResidueOrientation::orient_sequence(residue_orientations);

		std::vector<RibbonSegment> ribbon_segments(residue_orientations.size());

		for(std::size_t i=0;i<residue_orientations.size();i++)
		{
			ribbon_segments[i].core=residue_orientations[i].get_core_vertebra();
			ribbon_segments[i].sidebone=residue_orientations[i].get_sidebone();
		}

		std::vector<bool> statuses_alpha(chain.size(), false);
		std::vector<bool> statuses_beta(chain.size(), false);
		std::vector<bool> statuses_loop(chain.size(), false);
		std::vector<bool> statuses_na(chain.size(), false);
		for(std::size_t i=0;i<chain.size();i++)
		{
			const std::size_t residue_id=primary_structure.map_of_atoms_to_residues[chain[i]];
			const ConstructionOfSecondaryStructure::SecondaryStructureType residue_ss_type=secondary_structure.residue_descriptors[residue_id].secondary_structure_type;
			const ConstructionOfPrimaryStructure::ResidueType residue_type=primary_structure.residues[residue_id].residue_type;
			statuses_alpha[i]=(residue_ss_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_ALPHA_HELIX);
			statuses_beta[i]=(residue_ss_type==ConstructionOfSecondaryStructure::SECONDARY_STRUCTURE_TYPE_BETA_STRAND);
			statuses_loop[i]=!(statuses_alpha[i] || statuses_beta[i]);
			statuses_na[i]=(residue_type==ConstructionOfPrimaryStructure::RESIDUE_TYPE_NUCLEOTIDE);
		}

		if(!parameters.beta_correction_disabled)
		{
			std::vector<RibbonSegment> ribbon_segments_raw=ribbon_segments;
			for(std::size_t i=1;(i+1)<chain.size();i++)
			{
				if(statuses_beta[i] || statuses_beta[i-1] || statuses_beta[i+1])
				{
					ribbon_segments[i].core.center=(ribbon_segments_raw[i-1].core.center+(ribbon_segments_raw[i].core.center*2.0)+ribbon_segments_raw[i+1].core.center)*0.25;
					ribbon_segments[i].core.up=(ribbon_segments_raw[i-1].core.up+(ribbon_segments_raw[i].core.up*2.0)+ribbon_segments_raw[i+1].core.up)*0.25;
					ribbon_segments[i].core.right=(ribbon_segments_raw[i-1].core.right+(ribbon_segments_raw[i].core.right*2.0)+ribbon_segments_raw[i+1].core.right)*0.25;
				}
			}
		}

		if(parameters.factors_smoothing_enabled)
		{
			for(std::size_t i=0;i<chain.size();i++)
			{
				double w=1.0;
				if(!parameters.factors_disabled)
				{
					if(statuses_na[i])
					{
						w=parameters.nucleic_factor;
					}
					else if(!statuses_loop[i])
					{
						w=parameters.amino_ss_factor;
					}
				}
				ribbon_segments[i].core.right=ribbon_segments[i].core.right.unit()*w;
			}
		}

		smooth_ribbon_segments(parameters_spline, ribbon_segments);

		for(std::size_t i=0;i<chain.size();i++)
		{
			std::vector<RibbonSegment::Vertebra>& spine=ribbon_segments[i].spine;
			const std::size_t N=spine.size();

			std::vector<double> ls(N, 0.0);
			for(std::size_t j=1;j<N;j++)
			{
				ls[j]=ls[j-1]+(spine[j].center-spine[j-1].center).module();
			}
			for(std::size_t j=1;j<N;j++)
			{
				ls[j]=ls[j]/ls[N-1];
			}

			if(!parameters.factors_smoothing_enabled)
			{
				for(std::size_t j=0;j<N;j++)
				{
					double w=1.0;
					if(!parameters.factors_disabled)
					{
						if(statuses_na[i])
						{
							w=parameters.nucleic_factor;
						}
						else if(!statuses_loop[i])
						{
							const bool mask[2]={!(i==0 || statuses_loop[i-1]), !(i+1==chain.size() || statuses_loop[i+1])};
							if(mask[0] && mask[1])
							{
								w=parameters.amino_ss_factor;
							}
							else if(!mask[0] && mask[1])
							{
								w=(j<(N/3) ? 1.0 : parameters.amino_ss_factor);
							}
							else if(mask[0] && !mask[1])
							{
								if(statuses_beta[i] && !parameters.beta_arrows_disabled)
								{
									if(j<1)
									{
										w=parameters.amino_ss_factor;
									}
									else if(j<=(N/3*2))
									{
										w=1.0+(parameters.amino_ss_arrow_factor-1.0)*((ls[N/3*2]-ls[j])/(ls[N/3*2]-ls[1]));
									}
									else
									{
										w=1.0;
									}
								}
								else
								{
									w=(j>(N/3*2) ? 1.0 : parameters.amino_ss_factor);
								}
							}
							else if(!mask[0] && !mask[1])
							{
								w=(j<(N/4) ? 1.0 : (j>(N/4*3) ? 1.0 : parameters.amino_ss_factor));
							}
						}
					}
					spine[j].right=spine[j].right.unit()*w;
				}
			}

			for(std::size_t j=0;j<N;j++)
			{
				apollota::SimplePoint f1;
				apollota::SimplePoint f2;

				if(j>0)
				{
					f1=(spine[j].center-spine[j-1].center);
				}
				else if(i>0 && ribbon_segments[i-1].spine.size()>1)
				{
					f1=(spine[j].center-ribbon_segments[i-1].spine[ribbon_segments[i-1].spine.size()-2].center);
				}

				if(j+1<N)
				{
					f2=(spine[j+1].center-spine[j].center);
				}
				else if(i+1<ribbon_segments.size() && ribbon_segments[i+1].spine.size()>2)
				{
					f2=(ribbon_segments[i+1].spine[1].center-spine[j].center);
				}

				const bool f1_status=f1.module()>0.0;
				const bool f2_status=f2.module()>0.0;

				if(f1_status || f2_status)
				{
					if(!f2_status)
					{
						f2=f1;
					}
					else if(!f1_status)
					{
						f1=f2;
					}

					spine[j].up=((f1&spine[j].right).unit()+(f2&spine[j].right).unit()).unit();
				}
			}
		}

		for(std::size_t i=0;i<chain.size();i++)
		{
			map_of_atoms_to_ribbon_segments[chain[i]].push_back(all_ribbon_segments.size());
			all_ribbon_segments.push_back(ribbon_segments[i]);
		}

		return true;
	}
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_STRUCTURAL_CARTOON_H_ */


