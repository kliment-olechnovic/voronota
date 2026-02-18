#ifndef CADSCORELT_CLI_OUTPUT_UTILITIES_H_
#define CADSCORELT_CLI_OUTPUT_UTILITIES_H_

#include "../cadscorelt/core.h"

namespace cadscorelt
{

std::string to_string_compact(const double v)
{
	if(v==0.0)
	{
		return std::string("0");
	}
	else if(v==-1.0)
	{
		return std::string("-1");
	}
	else if(v==1.0)
	{
		return std::string("1");
	}
	return std::to_string(v);
}

class PrintingUtilites
{
public:
	static void print(const IDChain& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		if(header)
		{
			output+=header_prefix;
			output+="chain";
		}
		else
		{
			output+=id.chain_name;
		}
	}

	static void print(const IDResidue& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_chain, header, header_prefix, output);
		output+="\t";
		if(header)
		{
			output+=header_prefix;
			output+="rnum\t";
			output+=header_prefix;
			output+="icode\t";
			output+=header_prefix;
			output+="rname";
		}
		else
		{
			output+=std::to_string(id.residue_seq_number);
			output+="\t";
			output+=id.residue_icode.empty() ? std::string(".") : id.residue_icode;
			output+="\t";
			output+=id.residue_name.empty() ? std::string(".") : id.residue_name;
		}
	}

	static void print(const IDAtom& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_residue, header, header_prefix, output);
		output+="\t";
		if(header)
		{
			output+=header_prefix;
			output+="aname";
		}
		else
		{
			output+=id.atom_name;
		}
	}

	static void print(const IDChainChain& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const IDResidueResidue& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const IDAtomAtom& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const int level_of_details, const CADDescriptor& cadd, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		if(header)
		{
			output+=header_prefix;
			output+="cadscore";
			if(level_of_details>=1)
			{
				output+="\t";
				output+=header_prefix;
				output+="F1_of_areas";
				if(level_of_details>=2)
				{
					output+="\t";
					output+=header_prefix;
					output+="target_area\t";
					output+=header_prefix;
					output+="model_area\t";
					output+=header_prefix;
					output+="TP_area\t";
					output+=header_prefix;
					output+="FP_area\t";
					output+=header_prefix;
					output+="FN_area";
				}
			}
		}
		else
		{
			output+=to_string_compact(cadd.score());
			if(level_of_details>=1)
			{
				output+="\t";
				output+=to_string_compact(cadd.score_F1());
				if(level_of_details>=2)
				{
					output+="\t";
					output+=to_string_compact(cadd.target_area_sum);
					output+="\t";
					output+=to_string_compact(cadd.model_target_area_sum);
					output+="\t";
					output+=to_string_compact(cadd.confusion_TP);
					output+="\t";
					output+=to_string_compact(cadd.confusion_FP);
					output+="\t";
					output+=to_string_compact(cadd.confusion_FN);
				}
			}
		}
	}

	template<class MapContainer>
	static void print(const int level_of_details, const MapContainer& container, std::string& output) noexcept
	{
		typedef typename MapContainer::key_type ID;
		typedef typename MapContainer::mapped_type Value;
		print(ID(), true, std::string(), output);
		output+="\t";
		print(level_of_details, Value(), true, std::string(), output);
		output+="\n";
		for(typename MapContainer::const_iterator it=container.begin();it!=container.end();++it)
		{
			print(it->first, false, std::string(), output);
			output+="\t";
			print(level_of_details, it->second, false, std::string(), output);
			output+="\n";
		}
	}

	template<class MapContainer>
	static std::string print(const int level_of_details, const MapContainer& container) noexcept
	{
		std::string output;
		print(level_of_details, container, output);
		return output;
	}
};

class MolecularFileWritingUtilities
{
public:
	class PDB
	{
	public:
		static bool check_compatability_with_pdb_format(const std::vector<AtomBall>& atom_balls) noexcept
		{
			if(atom_balls.size()>99999)
			{
				return false;
			}
			for(std::size_t i=0;i<atom_balls.size();i++)
			{
				const AtomBall& ab=atom_balls[i];
				if(ab.id_atom.id_residue.id_chain.chain_name.size()>1)
				{
					return false;
				}
			}
			return true;
		}

		static void print(const std::vector<AtomBall>& atom_balls, std::string& output) noexcept
		{
			if(check_compatability_with_pdb_format(atom_balls))
			{
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), 1.0, 0.0);
					output+="\n";
				}
			}
		}

		static std::string print(const std::vector<AtomBall>& atom_balls) noexcept
		{
			std::string output;
			print(atom_balls, output);
			return output;
		}

		template<class MapContainer>
		static void print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds, std::string& output) noexcept
		{
			if(check_compatability_with_pdb_format(atom_balls))
			{
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), map_of_cadds);
					output+="\n";
				}
			}
		}

		template<class MapContainer>
		static std::string print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds) noexcept
		{
			std::string output;
			print(atom_balls, map_of_cadds, output);
			return output;
		}

	private:
		static bool insert_string_to_columned_line(const std::string& str, const std::size_t start, const std::size_t end, const bool shift_right, std::string& line) noexcept
		{
			if(str.empty())
			{
				return true;
			}
			if(start>=1 && start<=end && end<=line.size())
			{
				const std::size_t interval_length=(end-start)+1;
				if(str.size()<=interval_length)
				{
					const std::string addition(interval_length-str.size(), ' ');
					line.replace(start-1, interval_length, (shift_right ? addition+str : str+addition));
					return true;
				}
			}
			return false;
		}

		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const double oc, const double tf) noexcept
		{
			std::string line(80, ' ');
			insert_string_to_columned_line("ATOM", 1, 6, false, line);
			insert_string_to_columned_line((serial>0 ? std::to_string(serial) : std::string()), 7, 11, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.atom_name, (atom_ball.id_atom.atom_name.size()>3 ? 13 : 14), 16, false, line);
			insert_string_to_columned_line(atom_ball.residue_name, 18, 20, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.id_residue.id_chain.chain_name.substr(0, 1), 22, 22, false, line);
			insert_string_to_columned_line(std::to_string(atom_ball.id_atom.id_residue.residue_seq_number), 23, 26, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.id_residue.residue_icode, 27, 27, false, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.x, 3), 31, 38, true, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.y, 3), 39, 46, true, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.z, 3), 47, 54, true, line);
			insert_string_to_columned_line(convert_double_to_string(oc, 2), 55, 60, true, line);
			insert_string_to_columned_line(convert_double_to_string(tf, 2), 61, 66, true, line);
			insert_string_to_columned_line(atom_ball.element, 77, 78, true, line);
			return line;
		}

		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
		{
			return print_atom_line(atom_ball, serial, (cadd.target_area_sum>0.0 ? 1.0 : 0.0), (cadd.target_area_sum>0.0 ? cadd.score() : 0.0));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDAtom, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDAtom, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom);
			if(it==map_of_cadds.end() && !atom_ball.conflated_atom_name.empty())
			{
				IDAtom conflated_id_atom=atom_ball.id_atom;
				conflated_id_atom.atom_name=atom_ball.conflated_atom_name;
				it=map_of_cadds.find(conflated_id_atom);
			}
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDResidue, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDResidue, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDChain, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDChain, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue.id_chain);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}
	};

	class MMCIF
	{
	public:
		static const std::string& minimal_header() noexcept
		{
			static std::string header=R"(data_minimal
loop_
_atom_site.group_PDB
_atom_site.id
_atom_site.type_symbol
_atom_site.label_atom_id
_atom_site.label_alt_id
_atom_site.label_comp_id
_atom_site.label_asym_id
_atom_site.label_entity_id
_atom_site.label_seq_id
_atom_site.pdbx_PDB_ins_code
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.occupancy
_atom_site.B_iso_or_equiv
_atom_site.pdbx_formal_charge
_atom_site.auth_seq_id
_atom_site.auth_comp_id
_atom_site.auth_asym_id
_atom_site.auth_atom_id
_atom_site.pdbx_PDB_model_num
)";
			return header;
		}

		static void print(const std::vector<AtomBall>& atom_balls, std::string& output) noexcept
		{
			if(!atom_balls.empty())
			{
				output=minimal_header();
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), 1.0, 0.0);
					output+="\n";
				}
				output+="#\n";
			}
		}

		static std::string print(const std::vector<AtomBall>& atom_balls) noexcept
		{
			std::string output;
			print(atom_balls, output);
			return output;
		}

		template<class MapContainer>
		static void print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds, std::string& output) noexcept
		{
			if(!atom_balls.empty())
			{
				output=minimal_header();
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), map_of_cadds);
					output+="\n";
				}
				output+="#\n";
			}
		}

		template<class MapContainer>
		static std::string print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds) noexcept
		{
			std::string output;
			print(atom_balls, map_of_cadds, output);
			return output;
		}

	private:
		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const double oc, const double tf) noexcept
		{
			std::string line="ATOM ";
			line+=std::to_string(serial);
			line+=" ";
			line+=(atom_ball.element.empty() ? std::string("?") : atom_ball.element);
			line+=" ";
			line+=atom_ball.id_atom.atom_name;
			line+=" . ";
			line+=atom_ball.residue_name;
			line+=" ";
			line+=atom_ball.id_atom.id_residue.id_chain.chain_name;
			line+=" 1 ";
			line+=std::to_string(atom_ball.id_atom.id_residue.residue_seq_number);
			line+=" ";
			line+=(atom_ball.id_atom.id_residue.residue_icode.empty() ? std::string("?") : atom_ball.id_atom.id_residue.residue_icode);
			line+=" ";
			line+=convert_double_to_string(atom_ball.x, 3);
			line+=" ";
			line+=convert_double_to_string(atom_ball.y, 3);
			line+=" ";
			line+=convert_double_to_string(atom_ball.z, 3);
			line+=" ";
			line+=convert_double_to_string(oc, 2);
			line+=" ";
			line+=convert_double_to_string(tf, 2);
			line+=" ? ";
			line+=std::to_string(atom_ball.id_atom.id_residue.residue_seq_number);
			line+=" ";
			line+=atom_ball.residue_name;
			line+=" ";
			line+=atom_ball.id_atom.id_residue.id_chain.chain_name;
			line+=" ";
			line+=atom_ball.id_atom.atom_name;
			line+=" 1";
			return line;
		}

		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
		{
			return print_atom_line(atom_ball, serial, (cadd.target_area_sum>0.0 ? 1.0 : 0.0), (cadd.target_area_sum>0.0 ? cadd.score() : 0.0));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDAtom, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDAtom, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom);
			if(it==map_of_cadds.end() && !atom_ball.conflated_atom_name.empty())
			{
				IDAtom conflated_id_atom=atom_ball.id_atom;
				conflated_id_atom.atom_name=atom_ball.conflated_atom_name;
				it=map_of_cadds.find(conflated_id_atom);
			}
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDResidue, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDResidue, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDChain, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDChain, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue.id_chain);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}
	};

private:
	static std::string convert_double_to_string(const double value, const int precision)
	{
		std::ostringstream output;
		output << std::fixed << std::setprecision(precision) << value;
		return output.str();
	}
};

class ContactMapPlottingUtilities
{
public:
	static std::string print(const std::map<IDAtomAtom, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDAtomAtom, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDAtomAtom& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.id_residue.id_chain.chain_name, id.id_a.id_residue.residue_seq_number, id.id_a.id_residue.residue_icode, id.id_a.atom_name, id.id_b.id_residue.id_chain.chain_name, id.id_b.id_residue.residue_seq_number, id.id_b.id_residue.residue_icode, id.id_b.atom_name, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

	static std::string print(const std::map<IDResidueResidue, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDResidueResidue, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDResidueResidue& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.id_chain.chain_name, id.id_a.residue_seq_number, id.id_a.residue_icode, id.id_b.id_chain.chain_name, id.id_b.residue_seq_number, id.id_b.residue_icode, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

	static std::string print(const std::map<IDChainChain, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDChainChain, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDChainChain& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.chain_name, id.id_b.chain_name, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

private:
	static voronotalt::ContactPlotter::ConfigFlags generate_config_flags() noexcept
	{
		voronotalt::ContactPlotter::ConfigFlags cf;
		cf.colored=true;
		cf.xlabeled=true;
		cf.ylabeled=true;
		cf.compact=false;
		cf.dark=true;
		cf.valid=true;
		return cf;
	}

	static unsigned int generate_color(const double max_value, const CADDescriptor& cadd) noexcept
	{
		const unsigned int red_value=static_cast<unsigned int>(std::min(cadd.target_area_sum/max_value, 1.0)*255.0);
		const unsigned int green_value=static_cast<unsigned int>(std::min(cadd.model_area_sum/max_value, 1.0)*255.0);
		const unsigned int color=(red_value << 16)+(green_value << 8);
		return color;
	}
};

class GraphicsPrintingUtilities
{
public:
	struct OutputFormat
	{
		enum ID
		{
			pymol,
			chimera
		};
	};

	template<class Container>
	static std::string print_contacts_graphics(const ScorableData& sd, const Container& map_of_cadds, const OutputFormat::ID output_format, const std::string& title) noexcept
	{
		if(sd.rt_result_graphics.contacts_graphics.empty() || sd.atom_balls.empty())
		{
			return std::string();
		}
		voronotalt::GraphicsWriter graphics_writer(true);
		const unsigned int base_color=0xFFFF00;
		const std::string category_name="contacts";
		const std::string group_name="faces";
		graphics_writer.add_color(category_name, "", base_color);
		for(std::size_t i=0;i<sd.rt_result_graphics.contacts_graphics.size() && i<sd.rt_result.contacts_summaries.size();i++)
		{
			typename Container::const_iterator it=find_in_map_of_cadds(sd, sd.rt_result.contacts_summaries[i], map_of_cadds);
			if(it!=map_of_cadds.end())
			{
				graphics_writer.add_color(category_name, group_name, color_from_cadd(it->second, base_color));
				const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=sd.rt_result_graphics.contacts_graphics[i];
				graphics_writer.add_triangle_fan(category_name, group_name, pair_graphics.outer_points, pair_graphics.barycenter, pair_graphics.plane_normal);
			}
		}
		if(output_format==OutputFormat::chimera)
		{
			return graphics_writer.write_to_string_for_chimera(title);
		}
		else
		{
			return graphics_writer.write_to_string_for_pymol(title);
		}
	}

	template<class Container>
	static std::string print_site_graphics(const ScorableData& sd, const Container& map_of_cadds, const OutputFormat::ID output_format, const std::string& title) noexcept
	{
		if(sd.rt_result_graphics.contacts_graphics.empty() || sd.atom_balls.empty())
		{
			return std::string();
		}
		voronotalt::GraphicsWriter graphics_writer(true);
		const unsigned int base_color=0xFFFF00;
		const std::string category_name="contacts";
		const std::string group_name="faces";
		graphics_writer.add_color(category_name, "", base_color);
		for(std::size_t i=0;i<sd.rt_result_graphics.contacts_graphics.size() && i<sd.rt_result.contacts_summaries.size();i++)
		{
			const std::size_t ball_ids[2]={sd.rt_result.contacts_summaries[i].id_a, sd.rt_result.contacts_summaries[i].id_b};
			bool found_statuses[2]={false, false};
			double score_values[2]={-1.0, -1.0};
			for(int t=0;t<2;t++)
			{
				if(ball_ids[t]<sd.atom_balls.size())
				{
					const AtomBall& ball=sd.atom_balls[ball_ids[t]];
					typename Container::const_iterator it=find_in_map_of_cadds(ball, map_of_cadds);
					if(it!=map_of_cadds.end())
					{
						found_statuses[t]=true;
						score_values[t]=it->second.score();
					}
				}
			}
			if(found_statuses[0] || found_statuses[1])
			{
				const double combined_score_value=((score_values[0]>=0.0 && score_values[1]>=0.0) ? (score_values[0]+score_values[1])/2.0 : std::max(score_values[0], score_values[1]));
				graphics_writer.add_color(category_name, group_name, (combined_score_value>=0.0 ? color_from_value(combined_score_value) : base_color));
				const voronotalt::RadicalTessellationContactConstruction::ContactDescriptorGraphics& pair_graphics=sd.rt_result_graphics.contacts_graphics[i];
				graphics_writer.add_triangle_fan(category_name, group_name, pair_graphics.outer_points, pair_graphics.barycenter, pair_graphics.plane_normal);
			}
		}
		if(output_format==OutputFormat::chimera)
		{
			return graphics_writer.write_to_string_for_chimera(title);
		}
		else
		{
			return graphics_writer.write_to_string_for_pymol(title);
		}
	}

	template<class Container>
	static std::string print_sas_graphics(const ScorableData& sd, const Container& map_of_cadds, const OutputFormat::ID output_format, const std::string& title) noexcept
	{
		if(sd.rt_result_graphics.sas_graphics.empty() || sd.atom_balls.empty())
		{
			return std::string();
		}
		voronotalt::GraphicsWriter graphics_writer(true);
		const unsigned int base_color=0xFFFF00;
		const std::string category_name="sas";
		const std::string group_name="patches";
		graphics_writer.add_color(category_name, "", base_color);
		for(std::size_t i=0;i<sd.rt_result_graphics.sas_graphics.size();i++)
		{
			const voronotalt::SubdividedIcosahedronCut::GraphicsBundle& gb=sd.rt_result_graphics.sas_graphics[i];
			const std::size_t ball_id=sd.rt_result.cells_summaries[i].id;
			if(ball_id<sd.atom_balls.size())
			{
				const AtomBall& ball=sd.atom_balls[ball_id];
				typename Container::const_iterator it=find_in_map_of_cadds(ball, map_of_cadds);
				if(it!=map_of_cadds.end())
				{
					graphics_writer.add_color(category_name, group_name, color_from_cadd(it->second, base_color));
					for(std::size_t j=0;j<gb.triples.size();j++)
					{
						const voronotalt::SubdividedIcosahedron::Triple& t=gb.triples[j];
						graphics_writer.add_triangle_on_sphere(category_name, group_name, voronotalt::SimplePoint(ball.x, ball.y, ball.z), gb.vertices[t.ids[0]], gb.vertices[t.ids[1]], gb.vertices[t.ids[2]]);
					}
				}
			}
		}
		if(output_format==OutputFormat::chimera)
		{
			return graphics_writer.write_to_string_for_chimera(title);
		}
		else
		{
			return graphics_writer.write_to_string_for_pymol(title);
		}
	}

private:
	static std::map<IDAtomAtom, CADDescriptor>::const_iterator find_in_map_of_cadds(const ScorableData& sd, const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary, const std::map<IDAtomAtom, CADDescriptor>& map_of_cadds) noexcept
	{
		std::map<IDAtomAtom, CADDescriptor>::const_iterator it=map_of_cadds.end();
		if(pair_summary.id_a<sd.atom_balls.size() && pair_summary.id_b<sd.atom_balls.size())
		{
			const AtomBall& ball_a=sd.atom_balls[pair_summary.id_a];
			const AtomBall& ball_b=sd.atom_balls[pair_summary.id_b];
			it=map_of_cadds.find(IDAtomAtom(ball_a.id_atom, ball_b.id_atom));
			if(it==map_of_cadds.end() && (!ball_a.conflated_atom_name.empty() || !ball_b.conflated_atom_name.empty()))
			{
				IDAtom conflated_id_a=ball_a.id_atom;
				IDAtom conflated_id_b=ball_b.id_atom;
				if(!ball_a.conflated_atom_name.empty())
				{
					conflated_id_a.atom_name=ball_a.conflated_atom_name;
				}
				if(!ball_b.conflated_atom_name.empty())
				{
					conflated_id_b.atom_name=ball_b.conflated_atom_name;
				}
				it=map_of_cadds.find(IDAtomAtom(conflated_id_a, conflated_id_b));
			}
		}
		return it;
	}

	static std::map<IDResidueResidue, CADDescriptor>::const_iterator find_in_map_of_cadds(const ScorableData& sd, const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary, const std::map<IDResidueResidue, CADDescriptor>& map_of_cadds)
	{
		std::map<IDResidueResidue, CADDescriptor>::const_iterator it=map_of_cadds.end();
		if(pair_summary.id_a<sd.atom_balls.size() && pair_summary.id_b<sd.atom_balls.size())
		{
			const AtomBall& ball_a=sd.atom_balls[pair_summary.id_a];
			const AtomBall& ball_b=sd.atom_balls[pair_summary.id_b];
			it=map_of_cadds.find(IDResidueResidue(ball_a.id_atom.id_residue, ball_b.id_atom.id_residue));
		}
		return it;
	}

	static std::map<IDChainChain, CADDescriptor>::const_iterator find_in_map_of_cadds(const ScorableData& sd, const voronotalt::RadicalTessellation::ContactDescriptorSummary& pair_summary, const std::map<IDChainChain, CADDescriptor>& map_of_cadds) noexcept
	{
		std::map<IDChainChain, CADDescriptor>::const_iterator it=map_of_cadds.end();
		if(pair_summary.id_a<sd.atom_balls.size() && pair_summary.id_b<sd.atom_balls.size())
		{
			const AtomBall& ball_a=sd.atom_balls[pair_summary.id_a];
			const AtomBall& ball_b=sd.atom_balls[pair_summary.id_b];
			it=map_of_cadds.find(IDChainChain(ball_a.id_atom.id_residue.id_chain, ball_b.id_atom.id_residue.id_chain));
		}
		return it;
	}

	static std::map<IDAtom, CADDescriptor>::const_iterator find_in_map_of_cadds(const AtomBall& ball, const std::map<IDAtom, CADDescriptor>& map_of_cadds) noexcept
	{
		std::map<IDAtom, CADDescriptor>::const_iterator it=map_of_cadds.find(ball.id_atom);
		if(it==map_of_cadds.end() && !ball.conflated_atom_name.empty())
		{
			IDAtom conflated_id=ball.id_atom;
			conflated_id.atom_name=ball.conflated_atom_name;
			it=map_of_cadds.find(conflated_id);
		}
		return it;
	}

	static std::map<IDResidue, CADDescriptor>::const_iterator find_in_map_of_cadds(const AtomBall& ball, const std::map<IDResidue, CADDescriptor>& map_of_cadds) noexcept
	{
		return map_of_cadds.find(ball.id_atom.id_residue);
	}

	static std::map<IDChain, CADDescriptor>::const_iterator find_in_map_of_cadds(const AtomBall& ball, const std::map<IDChain, CADDescriptor>& map_of_cadds) noexcept
	{
		return map_of_cadds.find(ball.id_atom.id_residue.id_chain);
	}

	static unsigned int color_from_value(const double value) noexcept
	{
		double r=0.0;
		double g=0.0;
		double b=0.0;
		if(value<=0.0)
		{
			r=1.0;
		}
		else if(value>=1.0)
		{
			b=1.0;
		}
		else if(value<=0.5)
		{
			r=1.0;
			b=(value/0.5);
			g=b;
		}
		else
		{
			b=1.0;
			r=(1.0-((value-0.5)/0.5));
			g=r;
		}
		const double scale=static_cast<double>(0xFF);
		return ((static_cast<unsigned int>(r*scale) << 16)+(static_cast<unsigned int>(g*scale) << 8)+static_cast<unsigned int>(b*scale));
	}

	static unsigned int color_from_cadd(const CADDescriptor& cadd, const unsigned int base_color) noexcept
	{
		const double value=cadd.score();
		if(cadd.target_area_sum<=0.0 || value<0.0)
		{
			return base_color;
		}
		return color_from_value(value);
	}
};

}

#endif /* CADSCORELT_CLI_OUTPUT_UTILITIES_H_ */
