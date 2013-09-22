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

	static std::vector<AtomRecord> read_atom_records_from_pdb_file_stream(std::istream& file_stream, const bool include_heteroatoms)
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
					if(check_atom_record_acceptability(record, include_heteroatoms))
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

	static std::vector<AtomRecord> read_atom_records_from_mmcif_file_stream(std::istream& file_stream, const bool include_heteroatoms)
	{
		std::vector<AtomRecord> records;
		std::vector< std::vector<std::string> > table=read_atom_site_table_from_mmcif_file_stream(file_stream);
		if(table.size()>1)
		{
			std::map<std::string, std::size_t> header_map;
			for(std::size_t i=0;i<table[0].size();i++)
			{
				header_map[table[0][i]]=i;
			}
			if(header_map.size()==table[0].size())
			{
				const std::string first_model_id=get_value_from_table_row(header_map, table[1], "_atom_site.pdbx_PDB_model_num");
				for(std::size_t i=1;i<table.size();i++)
				{
					try
					{
						if(get_value_from_table_row(header_map, table[i], "_atom_site.pdbx_PDB_model_num")==first_model_id)
						{
							const AtomRecord record=read_atom_record_from_table_row(header_map, table[i]);
							if(check_atom_record_acceptability(record, include_heteroatoms))
							{
								records.push_back(record);
							}
						}
					}
					catch(const std::exception& e)
					{
						std::cerr << "Invalid atom record in row:";
						for(std::size_t j=0;j<table[0].size() && j<table[i].size();j++)
						{
							std::cerr << " " << table[0][j] << "=" << table[i][j];
						}
						std::cerr << "\n";
					}
				}
			}
		}
		return records;
	}

private:
	static bool check_atom_record_acceptability(const AtomRecord& record, const bool include_heteroatoms)
	{
		return ((record.record_name=="ATOM" || (include_heteroatoms && record.record_name=="HETATM")) &&
				!record.name.empty() &&
				!record.resName.empty() &&
				(record.altLoc.empty() || record.altLoc=="A" || record.altLoc==".") &&
				record.resName!="HOH" &&
				record.element!="H" &&
				record.name.find("H")!=0 &&
				record.name.find("1H")!=0 &&
				record.name.find("2H")!=0 &&
				record.name.find("3H")!=0 &&
				record.name.find("4H")!=0);
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
		return record;
	}

	static std::vector< std::vector<std::string> > read_atom_site_table_from_mmcif_file_stream(std::istream& file_stream)
	{
		while(file_stream.good())
		{
			std::string token;
			file_stream >> token;
			if(!file_stream.fail() && token=="loop_")
			{
				std::vector<std::string> header;
				file_stream >> token;
				while(!file_stream.fail() && token.find("_atom_site.")==0)
				{
					header.push_back(token);
					file_stream >> token;
				}
				if(!header.empty())
				{
					std::vector<std::string> values;
					while(!file_stream.fail() && token.find("_")!=0 && token.find("loop_")!=0 && token.find("#")!=0)
					{
						values.push_back(token);
						file_stream >> token;
					}
					if(values.size()%header.size()==0)
					{
						std::vector< std::vector<std::string> > table;
						table.reserve(values.size()/header.size()+1);
						table.push_back(header);
						for(std::size_t i=0;i<values.size();i++)
						{
							if(i%header.size()==0)
							{
								table.push_back(std::vector<std::string>());
								table.back().reserve(header.size());
							}
							table.back().push_back(values[i]);
						}
						return table;
					}
				}
			}
		}
		return std::vector< std::vector<std::string> >();
	}

	static std::string get_value_from_table_row(const std::map<std::string, std::size_t> header_map, const std::vector<std::string>& row, const std::string& name)
	{
		std::map<std::string, std::size_t>::const_iterator it=header_map.find(name);
		if(it!=header_map.end() && it->second<row.size())
		{
			return row[it->second];
		}
		else
		{
			return std::string();
		}
	}

	static AtomRecord read_atom_record_from_table_row(const std::map<std::string, std::size_t> header_map, const std::vector<std::string>& row)
	{
		AtomRecord record=AtomRecord();
		record.record_name=get_value_from_table_row(header_map, row, "_atom_site.group_PDB");
		record.serial=get_value_from_table_row(header_map, row, "_atom_site.id");
		record.name=get_value_from_table_row(header_map, row, "_atom_site.label_atom_id");
		record.altLoc=get_value_from_table_row(header_map, row, "_atom_site.label_alt_id");
		record.resName=get_value_from_table_row(header_map, row, "_atom_site.label_comp_id");
		record.chainID=get_value_from_table_row(header_map, row, "_atom_site.label_asym_id");
		record.resSeq=get_value_from_table_row(header_map, row, "_atom_site.label_seq_id");
		record.iCode=get_value_from_table_row(header_map, row, "_atom_site.pdbx_PDB_ins_code");
		record.x=convert_string<double>(get_value_from_table_row(header_map, row, "_atom_site.Cartn_x"));
		record.y=convert_string<double>(get_value_from_table_row(header_map, row, "_atom_site.Cartn_y"));
		record.z=convert_string<double>(get_value_from_table_row(header_map, row, "_atom_site.Cartn_z"));
		record.element=get_value_from_table_row(header_map, row, "_atom_site.type_symbol");
		return record;
	}
};

}

#endif /* AUXILIARIES_ATOMS_READER_H_ */
