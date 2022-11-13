#ifndef AUXILIARIES_ATOMS_IO_H_
#define AUXILIARIES_ATOMS_IO_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <iomanip>

namespace voronota
{

namespace auxiliaries
{

class AtomsIO
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

	class PDBReader
	{
	public:
		struct Data
		{
			std::vector<AtomRecord> atom_records;
			std::vector<std::string> all_lines;
			std::vector<std::size_t> map_of_atom_records_to_all_lines;

			bool valid() const
			{
				if(atom_records.empty() || all_lines.size()<atom_records.size() || map_of_atom_records_to_all_lines.size()!=atom_records.size())
				{
					return false;
				}
				for(std::size_t i=0;i<map_of_atom_records_to_all_lines.size();i++)
				{
					if(map_of_atom_records_to_all_lines[i]>=all_lines.size())
					{
						return false;
					}
				}
				return true;
			}
		};

		static Data read_data_from_file_stream(
				std::istream& file_stream,
				const bool include_heteroatoms,
				const bool include_hydrogens,
				const bool handle_multiple_models,
				const bool store_lines)
		{
			Data data;
			int model_num=1;
			bool end_reached=false;
			while(file_stream.good() && (!end_reached || store_lines))
			{
				std::string line;
				std::getline(file_stream, line);
				if(store_lines)
				{
					data.all_lines.push_back(line);
				}
				if(!end_reached)
				{
					const std::string record_name=substring_of_columned_line(line, 1, 6);
					if(record_name=="ATOM" || record_name=="HETATM")
					{
						AtomRecord record=read_atom_record_from_line(line);
						if(check_atom_record_acceptability(record, include_heteroatoms, include_hydrogens))
						{
							if(check_atom_record_validity(record))
							{
								if(handle_multiple_models && model_num>1)
								{
									std::ostringstream chainID_output;
									chainID_output << record.chainID << model_num;
									record.chainID=chainID_output.str();
								}
								record.altLoc.clear();
								data.atom_records.push_back(record);
								if(store_lines)
								{
									data.map_of_atom_records_to_all_lines.push_back(!data.all_lines.empty() ? (data.all_lines.size()-1) : 0);
								}
							}
							else
							{
								std::cerr << "Invalid PDB atom record in line: " << line << "\n";
							}
						}
					}
					else if(record_name=="ENDMDL")
					{
						model_num++;
						if(!handle_multiple_models)
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
			return data;
		}

	private:
		static AtomRecord read_atom_record_from_line(const std::string& pdb_file_line)
		{
			AtomRecord record=AtomRecord();
			record.record_name=substring_of_columned_line(pdb_file_line, 1, 6);
			record.serial=convert_string<int>(substring_of_columned_line(pdb_file_line, 7, 11), record.serial_valid);
			record.name=substring_of_columned_line(pdb_file_line, 13, 16);
			record.altLoc=fix_undefined_string(substring_of_columned_line(pdb_file_line, 17, 17));
			record.resName=fix_undefined_string(substring_of_columned_line(pdb_file_line, 18, 20));
			record.chainID=fix_undefined_string(substring_of_columned_line(pdb_file_line, 22, 22));
			record.resSeq=convert_string<int>(substring_of_columned_line(pdb_file_line, 23, 26), record.resSeq_valid);
			record.iCode=fix_undefined_string(substring_of_columned_line(pdb_file_line, 27, 27));
			record.x=convert_string<double>(substring_of_columned_line(pdb_file_line, 31, 38), record.x_valid);
			record.y=convert_string<double>(substring_of_columned_line(pdb_file_line, 39, 46), record.y_valid);
			record.z=convert_string<double>(substring_of_columned_line(pdb_file_line, 47, 54), record.z_valid);
			record.occupancy=convert_string<double>(substring_of_columned_line(pdb_file_line, 55, 60), record.occupancy_valid);
			record.tempFactor=convert_string<double>(substring_of_columned_line(pdb_file_line, 61, 66), record.tempFactor_valid);
			record.element=fix_undefined_string(substring_of_columned_line(pdb_file_line, 77, 78));
			normalize_numbered_atom_name(record.name);
			return record;
		}
	};

	class PDBWriter
	{
	public:
		static std::string write_atom_record_in_line(const AtomRecord& atom_record)
		{
			std::string line(80, ' ');
			insert_string_to_columned_line(atom_record.record_name, 1, 6, false, line);
			if(atom_record.serial_valid)
			{
				insert_string_to_columned_line(convert_int_to_string(atom_record.serial), 7, 11, true, line);
			}
			insert_string_to_columned_line(atom_record.name, (atom_record.name.size()>3 ? 13 : 14), 16, false, line);
			insert_string_to_columned_line(atom_record.altLoc, 17, 17, false, line);
			insert_string_to_columned_line(atom_record.resName, 18, 20, true, line);
			insert_string_to_columned_line(atom_record.chainID.substr(0, 1), 22, 22, false, line);
			if(atom_record.resSeq_valid)
			{
				insert_string_to_columned_line(convert_int_to_string(atom_record.resSeq), 23, 26, true, line);
			}
			insert_string_to_columned_line(atom_record.iCode, 27, 27, false, line);
			if(atom_record.x_valid)
			{
				insert_string_to_columned_line(convert_double_to_string(atom_record.x, 3), 31, 38, true, line);
			}
			if(atom_record.y_valid)
			{
				insert_string_to_columned_line(convert_double_to_string(atom_record.y, 3), 39, 46, true, line);
			}
			if(atom_record.z_valid)
			{
				insert_string_to_columned_line(convert_double_to_string(atom_record.z, 3), 47, 54, true, line);
			}
			if(atom_record.occupancy_valid)
			{
				insert_string_to_columned_line(convert_double_to_string(atom_record.occupancy, 2), 55, 60, true, line);
			}
			if(atom_record.tempFactor_valid)
			{
				insert_string_to_columned_line(convert_double_to_string(atom_record.tempFactor, 2), 61, 66, true, line);
			}
			insert_string_to_columned_line(atom_record.element, 77, 78, true, line);
			return line;
		}

		static std::string write_temperature_factor_to_line(const std::string& line, const bool tempFactor_valid, const double tempFactor)
		{
			std::string updated_line=line;
			if(updated_line.size()<80)
			{
				updated_line.resize(80, ' ');
			}
			insert_string_to_columned_line((tempFactor_valid ? convert_double_to_string(tempFactor, 2) : std::string()), 61, 66, true, updated_line);
			return updated_line;
		}

	private:
		static std::string convert_int_to_string(const int value)
		{
			std::ostringstream output;
			output << value;
			return output.str();
		}

		static std::string convert_double_to_string(const double value, const int precision)
		{
			std::ostringstream output;
			output << std::fixed << std::setprecision(precision) << value;
			return output.str();
		}

		static bool insert_string_to_columned_line(const std::string& str, const std::size_t start, const std::size_t end, const bool shift_right, std::string& line)
		{
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
	};

	class MMCIFReader
	{
	public:
		struct Data
		{
			std::vector<AtomRecord> atom_records;
		};

		static const std::string default_atom_site_prefix()
		{
			static const std::string atom_site_prefix="_atom_site.";
			return atom_site_prefix;
		}

		static bool detect_string_format(const std::string& data)
		{
			if(!data.empty())
			{
				const std::size_t prefix_pos=data.find(default_atom_site_prefix(), data.find("loop_"));
				if(prefix_pos!=std::string::npos && prefix_pos>0 && data[prefix_pos-1]<=' ')
				{
					return true;
				}
			}
			return false;
		}

		static Data read_data_from_file_stream(
				const std::string& atom_site_prefix,
				std::istream& file_stream,
				const bool include_heteroatoms,
				const bool include_hydrogens,
				const bool handle_multiple_models)
		{
			Data data;
			while(file_stream.good())
			{
				std::string token;
				bool token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
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
						std::map<std::string, std::size_t> header_map;
						for(std::size_t i=0;i<header.size();i++)
						{
							header_map[header[i]]=i;
						}
						if(header_map.size()==header.size())
						{
							std::vector<std::string> values;
							while(token_status && !(token=="loop_" || token.rfind("_", 0)==0 || token.rfind("data_", 0)==0))
							{
								values.push_back(token);
								token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
							}
							if(!values.empty() && ((values.size()%header.size())==0))
							{
								data.atom_records.reserve(values.size()/header.size());
								const std::string first_model_id=get_value_from_table_row(header_map, values.begin(), atom_site_prefix+"pdbx_PDB_model_num");
								for(std::size_t i=0;i<values.size();i+=header.size())
								{
									const std::string current_model_id=get_value_from_table_row(header_map, (values.begin()+i), atom_site_prefix+"pdbx_PDB_model_num");
									if(handle_multiple_models || current_model_id==first_model_id)
									{
										AtomRecord record=read_atom_record_from_table_row(atom_site_prefix, header_map, (values.begin()+i));
										if(check_atom_record_acceptability(record, include_heteroatoms, include_hydrogens))
										{
											if(check_atom_record_validity(record))
											{
												record.altLoc.clear();
												if(handle_multiple_models && current_model_id!="1")
												{
													record.chainID+=current_model_id;
												}
												data.atom_records.push_back(record);
											}
											else
											{
												std::cerr << "Invalid atom record in row:";
												for(std::size_t j=0;j<header.size();j++)
												{
													std::cerr << " " << header[j] << "=" << values[i+j];
												}
												std::cerr << "\n";
											}
										}
									}
								}
								return data;
							}
							else
							{
								throw std::runtime_error("Invalid '_atom_site' loop in mmCIF input stream.");
							}
						}
						else
						{
							throw std::runtime_error("Duplicate key in '_atom_site' loop in mmCIF input stream.");
						}
					}
				}
			}
			return data;
		}

		static Data read_data_from_file_stream(std::istream& file_stream, const bool include_heteroatoms, const bool include_hydrogens, const bool handle_multiple_models)
		{
			return read_data_from_file_stream(default_atom_site_prefix(), file_stream, include_heteroatoms, include_hydrogens, handle_multiple_models);
		}

	private:
		static bool read_uncommented_token_from_mmcif_file_stream(std::istream& input, std::string& output)
		{
			output.clear();
			while(input.good())
			{
				input >> std::ws;
				const char opener=std::char_traits<char>::to_char_type(input.peek());
				if(opener=='#')
				{
					std::string comment;
					std::getline(input, comment);
				}
				else
				{
					if(opener=='"' || opener=='\'')
					{
						input.get();
						std::getline(input, output, opener);
					}
					else
					{
						input >> output;
					}

					if(input.fail())
					{
						return false;
					}
					else
					{
						return true;
					}
				}
			}
			return false;
		}

		static std::string get_value_from_table_row(
				const std::map<std::string, std::size_t>& header_map,
				const std::vector<std::string>::const_iterator& values_iter,
				const std::string& name)
		{
			std::map<std::string, std::size_t>::const_iterator it=header_map.find(name);
			if(it!=header_map.end() && it->second<header_map.size())
			{
				return (*(values_iter+it->second));
			}
			return std::string();
		}

		static std::string get_value_from_table_row(
				const std::map<std::string, std::size_t>& header_map,
				const std::vector<std::string>::const_iterator& values_iter,
				const std::string& name_primary,
				const std::string& name_alternative)
		{
			if(header_map.count(name_primary)>0)
			{
				return get_value_from_table_row(header_map, values_iter, name_primary);
			}
			else
			{
				return get_value_from_table_row(header_map, values_iter, name_alternative);
			}
		}

		static AtomRecord read_atom_record_from_table_row(const std::string& atom_site_prefix, const std::map<std::string, std::size_t>& header_map, const std::vector<std::string>::const_iterator& values_iter)
		{
			AtomRecord record=AtomRecord();
			record.record_name=get_value_from_table_row(header_map, values_iter, atom_site_prefix+"group_PDB");
			record.serial=convert_string<int>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"id"), record.serial_valid);
			record.name=get_value_from_table_row(header_map, values_iter, atom_site_prefix+"auth_atom_id", atom_site_prefix+"label_atom_id");
			record.altLoc=fix_undefined_string(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"label_alt_id"));
			record.resName=fix_undefined_string(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"auth_comp_id", atom_site_prefix+"label_comp_id"));
			record.chainID=fix_undefined_string(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"auth_asym_id", atom_site_prefix+"label_asym_id"));
			record.resSeq=convert_string<int>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"auth_seq_id", atom_site_prefix+"label_seq_id"), record.resSeq_valid);
			record.iCode=fix_undefined_string(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"pdbx_PDB_ins_code"));
			record.x=convert_string<double>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"Cartn_x"), record.x_valid);
			record.y=convert_string<double>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"Cartn_y"), record.y_valid);
			record.z=convert_string<double>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"Cartn_z"), record.z_valid);
			record.occupancy=convert_string<double>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"occupancy"), record.occupancy_valid);
			record.tempFactor=convert_string<double>(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"B_iso_or_equiv"), record.tempFactor_valid);
			record.element=fix_undefined_string(get_value_from_table_row(header_map, values_iter, atom_site_prefix+"type_symbol"));
			normalize_numbered_atom_name(record.name);
			return record;
		}
	};

	class DSSPReader
	{
	public:
		struct DSSPRecord
		{
			std::string chainID;
			int resSeq;
			std::string iCode;
			std::string resNameShort;
			std::string resSSE;
			double anglePhi;
			double anglePsi;

			bool resSeq_valid;
			bool anglePhi_valid;
			bool anglePsi_valid;
		};

		struct Data
		{
			std::vector<DSSPRecord> dssp_records;
		};

		static Data read_data_from_file_stream(std::istream& file_stream)
		{
			const std::string header_prefix="  #  RESIDUE AA STRUCTURE";
			Data data;
			bool records_started=false;
			while(file_stream.good())
			{
				std::string line;
				std::getline(file_stream, line);
				if(!line.empty())
				{
					if(!records_started)
					{
						if(line.rfind(header_prefix, 0)==0)
						{
							records_started=true;
						}
					}
					else
					{
						const std::string resNameShort=fix_undefined_string(substring_of_columned_line(line, 14, 14));
						if(!resNameShort.empty() && resNameShort!="!")
						{
							const DSSPRecord record=read_dssp_record_from_line(line);
							if(record.resSeq_valid && record.anglePhi_valid && record.anglePsi_valid)
							{
								data.dssp_records.push_back(record);
							}
							else
							{
								std::cerr << "Invalid DSSP record in line: " << line << "\n";
							}
						}
					}
				}
			}
			return data;
		}

	private:
		static DSSPRecord read_dssp_record_from_line(const std::string& dssp_file_line)
		{
			DSSPRecord record=DSSPRecord();
			record.chainID=fix_undefined_string(substring_of_columned_line(dssp_file_line, 12, 12));
			record.resSeq=convert_string<int>(substring_of_columned_line(dssp_file_line, 6, 10), record.resSeq_valid);
			record.iCode=fix_undefined_string(substring_of_columned_line(dssp_file_line, 11, 11));
			record.resNameShort=fix_undefined_string(substring_of_columned_line(dssp_file_line, 14, 14));
			record.resSSE=fix_undefined_string(substring_of_columned_line(dssp_file_line, 17, 17));
			record.anglePhi=convert_string<double>(substring_of_columned_line(dssp_file_line, 104, 109), record.anglePhi_valid);
			record.anglePsi=convert_string<double>(substring_of_columned_line(dssp_file_line, 110, 115), record.anglePsi_valid);
			return record;
		}
	};

	class HBPlusReader
	{
	public:
		struct ShortAtomDescriptor
		{
			std::string chainID;
			int resSeq;
			std::string iCode;
			std::string resName;
			std::string name;

			bool resSeq_valid;
		};

		struct HBPlusRecord
		{
			ShortAtomDescriptor first;
			ShortAtomDescriptor second;
		};

		struct Data
		{
			std::vector<HBPlusRecord> hbplus_records;
		};

		static Data read_data_from_file_stream(std::istream& file_stream)
		{
			Data data;
			int line_number=0;
			while(file_stream.good())
			{
				std::string line;
				std::getline(file_stream, line);
				line_number++;
				if(line_number>=9 && !line.empty())
				{
					const HBPlusRecord record=read_hbplus_record_from_line(line);
					if(record.first.resSeq_valid && record.second.resSeq_valid)
					{
						data.hbplus_records.push_back(record);
					}
					else
					{
						std::cerr << "Invalid HBPlus record in line: " << line << "\n";
					}
				}
			}
			return data;
		}

	private:
		static ShortAtomDescriptor read_short_atom_descriptor_from_line(const std::string& hbplus_file_line, const bool second)
		{
			const int offset=(second ? 14 : 0);
			ShortAtomDescriptor d=ShortAtomDescriptor();
			d.chainID=fix_undefined_string(fix_undefined_dash_string(substring_of_columned_line(hbplus_file_line, 1+offset, 1+offset)));
			d.resSeq=convert_string_to_resSeq(substring_of_columned_line(hbplus_file_line, 2+offset, 5+offset), d.resSeq_valid);
			d.iCode=fix_undefined_string(fix_undefined_dash_string(substring_of_columned_line(hbplus_file_line, 6+offset, 6+offset)));
			d.resName=substring_of_columned_line(hbplus_file_line, 7+offset, 9+offset);
			d.name=substring_of_columned_line(hbplus_file_line, 10+offset, 14+offset);
			return d;
		}

		static HBPlusRecord read_hbplus_record_from_line(const std::string& hbplus_file_line)
		{
			HBPlusRecord record=HBPlusRecord();
			record.first=read_short_atom_descriptor_from_line(hbplus_file_line, false);
			record.second=read_short_atom_descriptor_from_line(hbplus_file_line, true);
			return record;
		}

		static int convert_string_to_resSeq(const std::string& str, bool& valid)
		{
			std::string refined_str;
			bool number_started=false;
			for(std::size_t i=0;i<str.size();i++)
			{
				if(str[i]=='0')
				{
					if(number_started || i+1==str.size())
					{
						refined_str.push_back(str[i]);
					}
				}
				else
				{
					number_started=true;
					refined_str.push_back(str[i]);
				}
			}
			return convert_string<int>(refined_str, valid);
		}

		static std::string fix_undefined_dash_string(const std::string& str)
		{
			return (str=="-" ? std::string() : str);
		}
	};

	class XYZRReader
	{
	public:
		struct XYZRRecord
		{
			std::string atom_type;
			double x;
			double y;
			double z;
			double r;
		};

		struct Data
		{
			std::vector<XYZRRecord> xyzr_records;
			bool with_r;
		};

		static Data read_data_from_file_stream(std::istream& file_stream)
		{
			Data data;
			data.with_r=false;
			bool provided_count_available=false;
			std::size_t provided_count=0;

			file_stream >> std::ws;
			if(next_char_is_digit(file_stream))
			{
				file_stream >> provided_count;
				if(file_stream.fail() || provided_count<1)
				{
					throw std::runtime_error("Invalid number of atoms.");
				}
				provided_count_available=true;
				file_stream >> std::ws;
			}

			while(file_stream.good())
			{
				std::string line;
				std::getline(file_stream, line);
				if(!line.empty())
				{
					std::istringstream line_input(line);
					XYZRRecord record;
					line_input >> record.atom_type >> record.x >> record.y >> record.z;
					if(line_input.fail())
					{
						std::ostringstream msg;
						msg << "Invalid atom record in line: '" << line << "'.\n";
						throw std::runtime_error(msg.str());
					}
					line_input >> std::ws;
					if(data.xyzr_records.empty())
					{
						if(line_input.good() && next_char_is_digit(line_input))
						{
							data.with_r=true;
						}
					}
					if(data.with_r)
					{
						line_input >> record.r;
						if(line_input.fail())
						{
							std::ostringstream msg;
							msg << "Invalid atom radius in line: '" << line << "'.\n";
							throw std::runtime_error(msg.str());
						}
						data.with_r=true;
					}
					else
					{
						if(line_input.good())
						{
							std::ostringstream msg;
							msg << "Invalid atom record size in line: '" << line << "'.\n";
							throw std::runtime_error(msg.str());
						}
					}
					data.xyzr_records.push_back(record);
				}
			}

			if(provided_count_available && provided_count!=data.xyzr_records.size())
			{
				std::ostringstream msg;
				msg << "Mismatched promised and actual numbers of atoms: " << provided_count << " vs " << data.xyzr_records.size() << ".\n";
				throw std::runtime_error(msg.str());
			}

			return data;
		}

	private:
		static bool next_char_is_digit(std::istream& file_stream)
		{
			const int next_char=file_stream.peek();
			return (next_char>=static_cast<int>('0') && next_char<=static_cast<int>('9'));
		}
	};

	class MDLReader
	{
	public:
		struct AtomRecord
		{
			std::string atom_type;
			double x;
			double y;
			double z;
		};

		struct Data
		{
			std::string title_line;
			std::string program_line;
			std::string comment_line;
			std::vector<AtomRecord> atom_records;
		};

		static Data read_data_from_file_stream(std::istream& file_stream)
		{
			Data data;

			int line_num=0;
			int num_of_atoms=0;
			bool end_reached=false;
			while(!end_reached && file_stream.good())
			{
				std::string line;
				std::getline(file_stream, line);
				line_num++;
				if(line.rfind("M  END", 0)==0)
				{
					end_reached=true;
				}
				else if(line_num==1)
				{
					data.title_line=line;
				}
				else if(line_num==2)
				{
					data.program_line=line;
				}
				else if(line_num==3)
				{
					data.comment_line=line;
				}
				else
				{
					if(line.empty())
					{
						std::ostringstream msg;
						msg << "Empty line " << line_num << ".\n";
						throw std::runtime_error(msg.str());
					}
					else
					{
						if(line_num==4)
						{
							std::istringstream line_input(line);
							line_input >> num_of_atoms;
							if(line_input.fail() || num_of_atoms<=0)
							{
								std::ostringstream msg;
								msg << "Invalid number of atoms in counts line: '" << line << "'.\n";
								throw std::runtime_error(msg.str());
							}
						}
						else if(line_num>=5 && (line_num-5)<num_of_atoms)
						{
							std::istringstream line_input(line);
							AtomRecord record;
							line_input >> record.x >> record.y >> record.z >> record.atom_type;
							if(line_input.fail())
							{
								std::ostringstream msg;
								msg << "Invalid atom record in line: '" << line << "'.\n";
								throw std::runtime_error(msg.str());
							}
							data.atom_records.push_back(record);
						}
					}
				}
			}

			if(line_num<5)
			{
				std::ostringstream msg;
				msg << "Missing blocks in MDL file.\n";
				throw std::runtime_error(msg.str());
			}

			if(data.atom_records.empty())
			{
				std::ostringstream msg;
				msg << "No atoms read.\n";
				throw std::runtime_error(msg.str());
			}

			if(static_cast<int>(data.atom_records.size())!=num_of_atoms)
			{
				std::ostringstream msg;
				msg << "Mismatched promised and actual numbers of atoms: " << num_of_atoms << " vs " << data.atom_records.size() << ".\n";
				throw std::runtime_error(msg.str());
			}

			return data;
		}
	};

private:
	static bool check_atom_record_validity(const AtomRecord& record)
	{
		return (record.serial_valid &&
				record.resSeq_valid &&
				record.x_valid &&
				record.y_valid &&
				record.z_valid &&
				!record.record_name.empty() &&
				!record.name.empty());
	}

	static bool check_atom_record_acceptability(const AtomRecord& record, const bool include_heteroatoms, const bool include_hydrogens)
	{
		return ((record.record_name=="ATOM" || (include_heteroatoms && record.record_name=="HETATM")) &&
				(record.altLoc.empty() || record.altLoc=="A" || record.altLoc=="1" || record.altLoc==".") &&
				(include_hydrogens || record.name.find("H")!=0) &&
				(include_hydrogens || (record.element!="H" && record.element!="D")) &&
				record.resName!="HOH");
	}

	template<typename T>
	static T convert_string(const std::string& str, bool& valid)
	{
		T value=T();
		if(str.empty())
		{
			valid=false;
		}
		else
		{
			std::istringstream input(str);
			input >> value;
			valid=!input.fail();
		}
		return value;
	}

	static std::string fix_undefined_string(const std::string& str)
	{
		return ((str=="." || str=="?") ? std::string() : str);
	}

	static void normalize_numbered_atom_name(std::string& name)
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

	static std::string substring_of_columned_line(const std::string& line, const int start, const int end)
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

}

#endif /* AUXILIARIES_ATOMS_IO_H_ */
