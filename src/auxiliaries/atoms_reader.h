#ifndef AUXILIARIES_ATOMS_READER_H_
#define AUXILIARIES_ATOMS_READER_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>

namespace auxiliaries
{

class AtomsReader
{
public:
	struct AtomRecord
	{
		std::string record_name;
		std::string serial;
		std::string name;
		std::string altLoc;
		std::string resName;
		std::string chainID;
		std::string resSeq;
		std::string iCode;
		double x;
		double y;
		double z;
		std::string element;
	};

	static std::vector<AtomRecord> read_atom_records_from_pdb_file_stream(std::istream& file_stream, const bool include_heteroatoms, const bool include_hydrogens)
	{
		std::vector<AtomRecord> records;
		while(file_stream.good())
		{
			std::string line;
			std::getline(file_stream, line);
			const std::string record_name=substring_of_columned_line(line, 1, 6);
			if(record_name=="ATOM" || record_name=="HETATM")
			{
				try
				{
					const AtomRecord record=read_atom_record_from_pdb_file_line(line);
					if(check_atom_record_acceptability(record, include_heteroatoms, include_hydrogens))
					{
						records.push_back(record);
					}
				}
				catch(const std::exception& e)
				{
					std::cerr << "Invalid atom record in line: " << line << "\n";
				}
			}
			else if(record_name=="END" || record_name=="ENDMDL")
			{
				return records;
			}
		}
		return records;
	}

	static std::vector<AtomRecord> read_atom_records_from_mmcif_file_stream(std::istream& file_stream, const bool include_heteroatoms, const bool include_hydrogens)
	{
		while(file_stream.good())
		{
			std::string token;
			bool token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
			if(token_status && token=="loop_")
			{
				std::vector<std::string> header;
				token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
				while(token_status && token.find("_atom_site.")==0)
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
						while(token_status && token.find("_")==std::string::npos)
						{
							values.push_back(token);
							token_status=read_uncommented_token_from_mmcif_file_stream(file_stream, token);
						}
						if(!values.empty() && ((values.size()%header.size())==0))
						{
							std::vector<AtomRecord> records;
							records.reserve(values.size()/header.size());
							const std::string first_model_id=get_value_from_table_row(header_map, values.begin(), "_atom_site.pdbx_PDB_model_num");
							for(std::size_t i=0;i<values.size();i+=header.size())
							{
								try
								{
									if(get_value_from_table_row(header_map, (values.begin()+i), "_atom_site.pdbx_PDB_model_num")==first_model_id)
									{
										const AtomRecord record=read_atom_record_from_table_row(header_map, (values.begin()+i));
										if(check_atom_record_acceptability(record, include_heteroatoms, include_hydrogens))
										{
											records.push_back(record);
										}
									}
								}
								catch(const std::exception& e)
								{
									std::cerr << "Invalid atom record in row:";
									for(std::size_t j=0;j<header.size();j++)
									{
										std::cerr << " " << header[j] << "=" << values[i+j];
									}
									std::cerr << "\n";
								}
							}
							return records;
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
		return std::vector<AtomRecord>();
	}

private:
	static bool check_atom_record_acceptability(const AtomRecord& record, const bool include_heteroatoms, const bool include_hydrogens)
	{
		return ((record.record_name=="ATOM" || (include_heteroatoms && record.record_name=="HETATM")) &&
				!record.name.empty() &&
				!record.resName.empty() &&
				(record.altLoc.empty() || record.altLoc=="A" || record.altLoc==".") &&
				(include_hydrogens || record.element!="H") &&
				record.resName!="HOH");
	}

	template<typename T>
	static T convert_string(const std::string& str)
	{
		std::istringstream input(str);
		input.exceptions(std::istringstream::failbit | std::istringstream::badbit);
		T value;
		input >> value;
		return value;
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

	static AtomRecord read_atom_record_from_pdb_file_line(const std::string& pdb_file_line)
	{
		AtomRecord record=AtomRecord();
		record.record_name=substring_of_columned_line(pdb_file_line, 1, 6);
		record.serial=substring_of_columned_line(pdb_file_line, 7, 11);
		record.name=substring_of_columned_line(pdb_file_line, 13, 16);
		record.altLoc=substring_of_columned_line(pdb_file_line, 17, 17);
		record.resName=substring_of_columned_line(pdb_file_line, 18, 20);
		record.chainID=substring_of_columned_line(pdb_file_line, 22, 22);
		record.resSeq=substring_of_columned_line(pdb_file_line, 23, 26);
		record.iCode=substring_of_columned_line(pdb_file_line, 27, 27);
		record.x=convert_string<double>(substring_of_columned_line(pdb_file_line, 31, 38));
		record.y=convert_string<double>(substring_of_columned_line(pdb_file_line, 39, 46));
		record.z=convert_string<double>(substring_of_columned_line(pdb_file_line, 47, 54));
		record.element=substring_of_columned_line(pdb_file_line, 77, 78);
		normalize_numbered_atom_name(record.name);
		return record;
	}

	static bool read_uncommented_token_from_mmcif_file_stream(std::istream& file_stream, std::string& token)
	{
		do
		{
			file_stream >> token;
			if(file_stream.fail())
			{
				return false;
			}
			else if(token.find("#")==std::string::npos)
			{
				return true;
			}
			else if(file_stream.good())
			{
				std::getline(file_stream, token);
			}
		}
		while(file_stream.good());
		return false;
	}

	static std::string get_value_from_table_row(const std::map<std::string, std::size_t>& header_map, const std::vector<std::string>::const_iterator& values_iter, const std::string& name)
	{
		std::map<std::string, std::size_t>::const_iterator it=header_map.find(name);
		if(it!=header_map.end() && it->second<header_map.size())
		{
			return (*(values_iter+it->second));
		}
		else
		{
			return std::string();
		}
	}

	static AtomRecord read_atom_record_from_table_row(const std::map<std::string, std::size_t>& header_map, const std::vector<std::string>::const_iterator& values_iter)
	{
		AtomRecord record=AtomRecord();
		record.record_name=get_value_from_table_row(header_map, values_iter, "_atom_site.group_PDB");
		record.serial=get_value_from_table_row(header_map, values_iter, "_atom_site.id");
		record.name=get_value_from_table_row(header_map, values_iter, "_atom_site.label_atom_id");
		record.altLoc=get_value_from_table_row(header_map, values_iter, "_atom_site.label_alt_id");
		record.resName=get_value_from_table_row(header_map, values_iter, "_atom_site.label_comp_id");
		record.chainID=get_value_from_table_row(header_map, values_iter, "_atom_site.label_asym_id");
		record.resSeq=get_value_from_table_row(header_map, values_iter, "_atom_site.label_seq_id");
		record.iCode=get_value_from_table_row(header_map, values_iter, "_atom_site.pdbx_PDB_ins_code");
		record.x=convert_string<double>(get_value_from_table_row(header_map, values_iter, "_atom_site.Cartn_x"));
		record.y=convert_string<double>(get_value_from_table_row(header_map, values_iter, "_atom_site.Cartn_y"));
		record.z=convert_string<double>(get_value_from_table_row(header_map, values_iter, "_atom_site.Cartn_z"));
		record.element=get_value_from_table_row(header_map, values_iter, "_atom_site.type_symbol");
		normalize_numbered_atom_name(record.name);
		return record;
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
};

}

#endif /* AUXILIARIES_ATOMS_READER_H_ */
