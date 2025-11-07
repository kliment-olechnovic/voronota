#ifndef VORONOTALT_MOLECULAR_FILE_READING_H_
#define VORONOTALT_MOLECULAR_FILE_READING_H_

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <cstdlib>

namespace voronotalt
{

class MolecularFileReading
{
public:
	struct AtomRecord
	{
		std::string record_name;
		int serial;
		std::string name;
		std::string altLoc;
		std::string resName;
		std::string chainID;
		int resSeq;
		std::string iCode;
		double x;
		double y;
		double z;
		double occupancy;
		double tempFactor;
		std::string element;

		bool serial_valid;
		bool resSeq_valid;
		bool x_valid;
		bool y_valid;
		bool z_valid;
		bool occupancy_valid;
		bool tempFactor_valid;
	};

	struct Parameters
	{
		bool include_heteroatoms;
		bool include_hydrogens;
		bool as_assembly;

		Parameters() noexcept : include_heteroatoms(false), include_hydrogens(false), as_assembly(false)
		{
		}

		Parameters(const bool include_heteroatoms, const bool include_hydrogens, const bool as_assembly) noexcept : include_heteroatoms(include_heteroatoms), include_hydrogens(include_hydrogens), as_assembly(as_assembly)
		{
		}
	};

	struct Data
	{
		std::vector<AtomRecord> atom_records;

		Data() noexcept
		{
		}

		void clear() noexcept
		{
			atom_records.clear();
		}

		bool valid() const noexcept
		{
			return (!atom_records.empty());
		}
	};

	class PDBReader
	{
	public:
		static bool read_data_from_file_stream(
				std::istream& file_stream,
				const Parameters& parameters,
				Data& data,
				std::ostream& error_message_output_stream) noexcept
		{
			data.clear();
			int model_num=1;
			bool end_reached=false;
			while(file_stream.good() && !end_reached)
			{
				std::string line;
				std::getline(file_stream, line);
				if(!end_reached)
				{
					const std::string record_name=substring_of_columned_line(line, 1, 6);
					if(record_name=="ATOM" || record_name=="HETATM")
					{
						AtomRecord record=read_atom_record_from_line(line);
						if(check_atom_record_acceptability(record, parameters.include_heteroatoms, parameters.include_hydrogens))
						{
							if(check_atom_record_validity(record))
							{
								if(parameters.as_assembly && model_num>1)
								{
									std::ostringstream chainID_output;
									chainID_output << record.chainID << model_num;
									record.chainID=chainID_output.str();
								}
								record.altLoc.clear();
								data.atom_records.push_back(record);
							}
							else
							{
								error_message_output_stream << "Invalid PDB atom record in line: " << line << "\n";
							}
						}
					}
					else if(record_name=="ENDMDL")
					{
						model_num++;
						if(!parameters.as_assembly)
						{
							end_reached=true;
						}
					}
					else if(record_name=="END")
					{
						end_reached=true;
					}
				}
			}
			return data.valid();
		}

	private:
		static AtomRecord read_atom_record_from_line(const std::string& pdb_file_line) noexcept
		{
			AtomRecord record=AtomRecord();
			record.record_name=substring_of_columned_line(pdb_file_line, 1, 6);
			record.serial=convert_string_to_int(substring_of_columned_line(pdb_file_line, 7, 11), record.serial_valid);
			record.name=substring_of_columned_line(pdb_file_line, 13, 16);
			record.altLoc=fix_undefined_string(substring_of_columned_line(pdb_file_line, 17, 17));
			record.resName=fix_undefined_string(substring_of_columned_line(pdb_file_line, 18, 20));
			record.chainID=fix_undefined_string(substring_of_columned_line(pdb_file_line, 22, 22));
			record.resSeq=convert_string_to_int(substring_of_columned_line(pdb_file_line, 23, 26), record.resSeq_valid);
			record.iCode=fix_undefined_string(substring_of_columned_line(pdb_file_line, 27, 27));
			record.x=convert_string_to_double(substring_of_columned_line(pdb_file_line, 31, 38), record.x_valid);
			record.y=convert_string_to_double(substring_of_columned_line(pdb_file_line, 39, 46), record.y_valid);
			record.z=convert_string_to_double(substring_of_columned_line(pdb_file_line, 47, 54), record.z_valid);
			record.occupancy=convert_string_to_double(substring_of_columned_line(pdb_file_line, 55, 60), record.occupancy_valid);
			record.tempFactor=convert_string_to_double(substring_of_columned_line(pdb_file_line, 61, 66), record.tempFactor_valid);
			record.element=fix_undefined_string(substring_of_columned_line(pdb_file_line, 77, 78));
			normalize_numbered_atom_name(record.name);
			return record;
		}
	};

	class MMCIFReader
	{
	public:
		static bool read_data_from_file_stream(
				std::istream& file_stream,
				const Parameters& parameters,
				Data& data,
				std::ostream& error_message_output_stream) noexcept
		{
			static const std::string atom_site_prefix="_atom_site.";
			data.clear();
			while(file_stream.good())
			{
				std::string token;
				std::getline(file_stream, token);
				bool token_status=true;
				if(token_status && token=="loop_")
				{
					std::vector<std::string> header;
					token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
					while(token_status && token.rfind(atom_site_prefix, 0)==0)
					{
						header.push_back(token);
						token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
					}
					if(!header.empty())
					{
						const std::vector< std::pair<bool, std::size_t> > map_of_value_positions=AtomSiteMapping::map_value_names_to_header_positions(header, atom_site_prefix);
						std::string first_model_id;
						bool first_model_id_ready=false;
						std::vector<std::string> values;
						values.reserve(header.size());
						while(token_status && !(token.rfind("_", 0)==0 || token.rfind("data_", 0)==0 || token=="loop_"))
						{
							values.clear();
							while(token_status && values.size()<header.size())
							{
								values.push_back(token);
								token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
							}
							if(values.size()==header.size())
							{
								if(!first_model_id_ready)
								{
									first_model_id=get_value_from_table_row(map_of_value_positions, values.begin(), AtomSiteMapping::atom_site__pdbx_PDB_model_num);
									first_model_id_ready=true;
								}
								read_and_store_atom_record_from_table_row(header, map_of_value_positions, values, parameters, first_model_id, data.atom_records, error_message_output_stream);
							}
							else
							{
								error_message_output_stream << "Invalid '_atom_site' loop in mmCIF input stream.\n";
								data.clear();
								return false;
							}
						}
						return data.valid();
					}
				}
			}
			return data.valid();
		}

	private:
		class AtomSiteMapping
		{
		public:
			enum ValueName
			{
				atom_site__pdbx_PDB_model_num,
				atom_site__group_PDB,
				atom_site__id,
				atom_site__auth_atom_id,
				atom_site__label_atom_id,
				atom_site__label_alt_id,
				atom_site__auth_comp_id,
				atom_site__label_comp_id,
				atom_site__auth_asym_id,
				atom_site__label_asym_id,
				atom_site__auth_seq_id,
				atom_site__label_seq_id,
				atom_site__pdbx_PDB_ins_code,
				atom_site__Cartn_x,
				atom_site__Cartn_y,
				atom_site__Cartn_z,
				atom_site__occupancy,
				atom_site__B_iso_or_equiv,
				atom_site__type_symbol,
				atom_site__undefined
			};

			static std::vector< std::pair<bool, std::size_t> > map_value_names_to_header_positions(const std::vector<std::string>& header, const std::string& atom_site_prefix) noexcept
			{
				std::vector< std::pair<bool, std::size_t> > result(static_cast<int>(atom_site__undefined)+1, std::pair<bool, std::size_t>(false, 0));
				std::map<std::string, std::size_t> header_map;
				for(std::size_t i=0;i<header.size();i++)
				{
					header_map[header[i]]=i;
				}
				result[static_cast<int>(atom_site__pdbx_PDB_model_num)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"pdbx_PDB_model_num");
				result[static_cast<int>(atom_site__group_PDB)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"group_PDB");
				result[static_cast<int>(atom_site__id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"id");
				result[static_cast<int>(atom_site__auth_atom_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"auth_atom_id");
				result[static_cast<int>(atom_site__label_atom_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"label_atom_id");
				result[static_cast<int>(atom_site__label_alt_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"label_alt_id");
				result[static_cast<int>(atom_site__auth_comp_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"auth_comp_id");
				result[static_cast<int>(atom_site__label_comp_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"label_comp_id");
				result[static_cast<int>(atom_site__auth_asym_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"auth_asym_id");
				result[static_cast<int>(atom_site__label_asym_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"label_asym_id");
				result[static_cast<int>(atom_site__auth_seq_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"auth_seq_id");
				result[static_cast<int>(atom_site__label_seq_id)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"label_seq_id");
				result[static_cast<int>(atom_site__pdbx_PDB_ins_code)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"pdbx_PDB_ins_code");
				result[static_cast<int>(atom_site__Cartn_x)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"Cartn_x");
				result[static_cast<int>(atom_site__Cartn_y)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"Cartn_y");
				result[static_cast<int>(atom_site__Cartn_z)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"Cartn_z");
				result[static_cast<int>(atom_site__occupancy)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"occupancy");
				result[static_cast<int>(atom_site__B_iso_or_equiv)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"B_iso_or_equiv");
				result[static_cast<int>(atom_site__type_symbol)]=get_position_by_name_from_header_map(header_map, atom_site_prefix+"type_symbol");
				return result;
			}

		private:
			static std::pair<bool, std::size_t> get_position_by_name_from_header_map(const std::map<std::string, std::size_t>& header_map, const std::string& name) noexcept
			{
				std::map<std::string, std::size_t>::const_iterator it=header_map.find(name);
				return (it==header_map.end() ? std::pair<bool, std::size_t>(false, 0) : std::pair<bool, std::size_t>(true, it->second));
			}
		};

		static bool read_uncommented_token_from_mmcif_file_stream(std::istream& in, std::string& out) noexcept
		{
			out.clear();

			std::streambuf* sb=in.rdbuf();
			if(sb==NULL)
			{
				in.setstate(std::ios::failbit);
				return false;
			}

			const int EOFc=std::char_traits<char>::eof();
			int c=sb->sgetc();

			bool rescan=true;
			while(rescan)
			{
				while(c!=EOFc && c<=32)
				{
					c=sb->snextc();
				}

				if(c==EOFc)
				{
					in.setstate(std::ios::eofbit | std::ios::failbit);
					return false;
				}

				if(c=='#')
				{
					do
					{
						c=sb->snextc();
					}
					while(c!=EOFc && c!='\n');

					if(c==EOFc)
					{
						in.setstate(std::ios::eofbit | std::ios::failbit);
						return false;
					}

					c=sb->snextc();
					rescan=true;
				}
				else
				{
					rescan=false;
				}
			}

			if(c=='\'' || c=='"')
			{
				const int quote=c;
				c=sb->snextc();

				while(c!=EOFc && c!=quote)
				{
					out.push_back(std::char_traits<char>::to_char_type(c));
					c=sb->snextc();
				}

				if(c==quote)
				{
					sb->snextc();
					if(out.empty() && in.good()==false)
					{
						in.clear();
					}
					return true;
				}
				else
				{
					in.setstate(std::ios::eofbit | std::ios::failbit);
					return false;
				}
			}
			else
			{
				while(c!=EOFc && c>32)
				{
					out.push_back(std::char_traits<char>::to_char_type(c));
					c=sb->snextc();
				}

				if(!out.empty())
				{
					return true;
				}
				else
				{
					if(c==EOFc)
					{
						in.setstate(std::ios::eofbit | std::ios::failbit);
					}
					else
					{
						in.setstate(std::ios::failbit);
					}
					return false;
				}
			}

			return false;
		}

		static std::string get_value_from_table_row(
				const std::vector< std::pair<bool, std::size_t> >& map_to_header_positions,
				const std::vector<std::string>::const_iterator& values_iter,
				const AtomSiteMapping::ValueName name) noexcept
		{
			const std::pair<bool, std::size_t>& pos=map_to_header_positions[static_cast<int>(name)];
			return (pos.first ? (*(values_iter+pos.second)) : std::string());
		}

		static std::string get_value_from_table_row(
				const std::vector< std::pair<bool, std::size_t> >& map_to_header_positions,
				const std::vector<std::string>::const_iterator& values_iter,
				const AtomSiteMapping::ValueName name_primary,
				const AtomSiteMapping::ValueName name_alternative) noexcept
		{
			const std::pair<bool, std::size_t>& pos_primary=map_to_header_positions[static_cast<int>(name_primary)];
			if(pos_primary.first)
			{
				return (*(values_iter+pos_primary.second));
			}
			const std::pair<bool, std::size_t>& pos_alternative=map_to_header_positions[static_cast<int>(name_alternative)];
			return (pos_alternative.first ? (*(values_iter+pos_alternative.second)) : std::string());
		}

		static AtomRecord read_atom_record_from_table_row(const std::vector< std::pair<bool, std::size_t> >& map_to_header_positions, const std::vector<std::string>::const_iterator& values_iter) noexcept
		{
			AtomRecord record=AtomRecord();
			record.record_name=get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__group_PDB);
			record.serial=convert_string_to_int(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__id), record.serial_valid);
			record.name=get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__auth_atom_id, AtomSiteMapping::atom_site__label_atom_id);
			record.altLoc=fix_undefined_string(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__label_alt_id));
			record.resName=fix_undefined_string(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__auth_comp_id, AtomSiteMapping::atom_site__label_comp_id));
			record.chainID=fix_undefined_string(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__auth_asym_id, AtomSiteMapping::atom_site__label_asym_id));
			record.resSeq=convert_string_to_int(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__auth_seq_id, AtomSiteMapping::atom_site__label_seq_id), record.resSeq_valid);
			record.iCode=fix_undefined_string(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__pdbx_PDB_ins_code));
			record.x=convert_string_to_double(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__Cartn_x), record.x_valid);
			record.y=convert_string_to_double(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__Cartn_y), record.y_valid);
			record.z=convert_string_to_double(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__Cartn_z), record.z_valid);
			record.occupancy=convert_string_to_double(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__occupancy), record.occupancy_valid);
			record.tempFactor=convert_string_to_double(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__B_iso_or_equiv), record.tempFactor_valid);
			record.element=fix_undefined_string(get_value_from_table_row(map_to_header_positions, values_iter, AtomSiteMapping::atom_site__type_symbol));
			normalize_numbered_atom_name(record.name);
			return record;
		}

		static void read_and_store_atom_record_from_table_row(
				const std::vector<std::string>& header,
				const std::vector< std::pair<bool, std::size_t> >& map_to_header_positions,
				const std::vector<std::string>& values,
				const Parameters& parameters,
				const std::string& first_model_id,
				std::vector<AtomRecord>& atom_records,
				std::ostream& error_message_output_stream)
		{
			const std::string current_model_id=get_value_from_table_row(map_to_header_positions, values.begin(), AtomSiteMapping::atom_site__pdbx_PDB_model_num);
			if(parameters.as_assembly || current_model_id==first_model_id)
			{
				AtomRecord record=read_atom_record_from_table_row(map_to_header_positions, values.begin());
				if(check_atom_record_acceptability(record, parameters.include_heteroatoms, parameters.include_hydrogens))
				{
					if(check_atom_record_validity(record))
					{
						record.altLoc.clear();
						if(parameters.as_assembly && current_model_id!="1")
						{
							record.chainID+=current_model_id;
						}
						atom_records.push_back(record);
					}
					else
					{
						error_message_output_stream << "Invalid atom record in row:";
						for(std::size_t j=0;j<header.size();j++)
						{
							error_message_output_stream << " " << header[j] << "=" << values[j];
						}
						error_message_output_stream << "\n";
					}
				}
			}
		}
	};

private:
	static bool check_atom_record_validity(const AtomRecord& record) noexcept
	{
		return (record.serial_valid &&
				record.resSeq_valid &&
				record.x_valid &&
				record.y_valid &&
				record.z_valid &&
				!record.record_name.empty() &&
				!record.name.empty());
	}

	static bool check_atom_record_acceptability(const AtomRecord& record, const bool include_heteroatoms, const bool include_hydrogens) noexcept
	{
		return ((record.record_name=="ATOM" || (include_heteroatoms && record.record_name=="HETATM")) &&
				(record.altLoc.empty() || record.altLoc=="A" || record.altLoc=="1" || record.altLoc==".") &&
				(include_hydrogens || record.name.find("H")!=0) &&
				(include_hydrogens || (record.element!="H" && record.element!="D")) &&
				record.resName!="HOH");
	}

	static int convert_string_to_int(const std::string& str, bool& valid) noexcept
	{
		int value=0;
		if(str.empty())
		{
			valid=false;
		}
		else
		{
			value=std::atoi(str.c_str());
			valid=!(value==0 && str!="0");
		}
		return value;
	}

	static double convert_string_to_double(const std::string& str, bool& valid) noexcept
	{
		double value=0.0;
		if(str.empty())
		{
			valid=false;
		}
		else
		{
			char* str_next=0;
			value=std::strtod(str.c_str(), &str_next);
			valid=(str!=str_next);
		}
		return value;
	}

	static std::string fix_undefined_string(const std::string& str) noexcept
	{
		return ((str=="." || str=="?") ? std::string() : str);
	}

	static void normalize_numbered_atom_name(std::string& name) noexcept
	{
		if(name.find_first_of("123456789")==0)
		{
			std::size_t first_letter_pos=name.find_first_not_of("0123456789");
			if(first_letter_pos!=std::string::npos)
			{
				name=(name.substr(first_letter_pos)+name.substr(0, first_letter_pos));
			}
		}
	}

	static std::string substring_of_columned_line(const std::string& line, const int start, const int end) noexcept
	{
		std::string extraction;
		int line_length=static_cast<int>(line.size());
		for(int i=start-1;i<end && i<line_length && i>=0;i++)
		{
			if(line[i]!=32) { extraction.push_back(line[i]); }
		}
		return extraction;
	}
};

}

#endif /* VORONOTALT_MOLECULAR_FILE_READING_H_ */
