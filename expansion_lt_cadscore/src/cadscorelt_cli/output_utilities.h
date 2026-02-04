#ifndef CADSCORELT_CLI_OUTPUT_UTILITIES_H_
#define CADSCORELT_CLI_OUTPUT_UTILITIES_H_

#include "../cadscorelt/core.h"

namespace cadscorelt
{

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
			output+=std::to_string(cadd.score());
			if(level_of_details>=1)
			{
				output+="\t";
				output+=std::to_string(cadd.score_F1());
				if(level_of_details>=2)
				{
					output+="\t";
					output+=std::to_string(cadd.target_area_sum);
					output+="\t";
					output+=std::to_string(cadd.model_target_area_sum);
					output+="\t";
					output+=std::to_string(cadd.confusion_TP);
					output+="\t";
					output+=std::to_string(cadd.confusion_FP);
					output+="\t";
					output+=std::to_string(cadd.confusion_FN);
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

		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
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
			insert_string_to_columned_line(convert_double_to_string((cadd.target_area_sum>0.0 ? 1.0 : 0.0), 2), 55, 60, true, line);
			insert_string_to_columned_line(convert_double_to_string((cadd.target_area_sum>0.0 ? cadd.score() : 0.0), 2), 61, 66, true, line);
			return line;
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
		template<class MapContainer>
		static void print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds, std::string& output) noexcept
		{
			if(!atom_balls.empty())
			{
				output="data_minimal";
				output+=R"(
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
		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
		{
			std::string line="ATOM ";
			line+=std::to_string(serial);
			line+=" ? ";
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
			line+=convert_double_to_string((cadd.target_area_sum>0.0 ? 1.0 : 0.0), 2);
			line+=" ";
			line+=convert_double_to_string((cadd.target_area_sum>0.0 ? cadd.score() : 0.0), 2);
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

}

#endif /* CADSCORELT_CLI_OUTPUT_UTILITIES_H_ */
