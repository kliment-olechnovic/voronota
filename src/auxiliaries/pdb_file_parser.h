#ifndef AUXILIARIES_PDB_FILE_PARSER_H_
#define AUXILIARIES_PDB_FILE_PARSER_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace auxiliaries
{

class PDBFileParser
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

	static std::vector<AtomRecord> read_atom_records_from_pdb_file_stream(std::istream& pdb_file_stream, const bool include_heteroatoms)
	{
		std::vector<AtomRecord> records;
		while(pdb_file_stream.good())
		{
			std::string line;
			std::getline(pdb_file_stream, line);
			const std::string record_name=substring_of_columned_line(line, 1, 6);
			if(record_name=="ATOM" || (include_heteroatoms && record_name=="HETATM"))
			{
				try
				{
					const AtomRecord record=read_atom_record_from_pdb_file_line(line);
					if(
							(record.altLoc.empty() || record.altLoc=="A") &&
							record.resName!="HOH" &&
							record.element!="H" &&
							record.name.find("H")!=0 &&
							record.name.find("1H")!=0 &&
							record.name.find("2H")!=0 &&
							record.name.find("3H")!=0 &&
							record.name.find("4H")!=0
						)
					{
						records.push_back(record);
					}
				}
				catch(const std::exception& e)
				{
					std::cerr << "Invalid atom record in line: " << line << ".\n";
				}
			}
			else if(record_name=="END" || record_name=="ENDMDL")
			{
				return records;
			}
		}
		return records;
	}

private:
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

	template<typename T>
	static T convert_string(const std::string& str)
	{
		std::istringstream input(str);
		input.exceptions(std::istringstream::failbit | std::istringstream::badbit);
		T value;
		input >> value;
		return value;
	}

	template<typename T>
	static T convert_string_or_return_default_value(const std::string& str, const T default_value)
	{
		std::istringstream input(str);
		T value;
		input >> value;
		if(!input.fail())
		{
			return value;
		}
		else
		{
			return default_value;
		}
	}

	static AtomRecord read_atom_record_from_pdb_file_line(const std::string& pdb_file_line)
	{
		AtomRecord record=AtomRecord();

		record.record_name=substring_of_columned_line(pdb_file_line, 1, 6);
		if(record.record_name.empty())
		{
			throw std::runtime_error("Missing record name.");
		}

		record.serial=substring_of_columned_line(pdb_file_line, 7, 11);

		record.name=substring_of_columned_line(pdb_file_line, 13, 16);
		if(record.name.empty())
		{
			throw std::runtime_error("Missing atom name.");
		}

		record.altLoc=substring_of_columned_line(pdb_file_line, 17, 17);

		record.resName=substring_of_columned_line(pdb_file_line, 18, 20);
		if(record.resName.empty())
		{
			throw std::runtime_error("Missing residue name.");
		}

		record.chainID=substring_of_columned_line(pdb_file_line, 22, 22);
		if(record.chainID.empty())
		{
			record.chainID="?";
		}

		record.resSeq=substring_of_columned_line(pdb_file_line, 23, 26);

		record.iCode=substring_of_columned_line(pdb_file_line, 27, 27);

		record.x=convert_string<double>(substring_of_columned_line(pdb_file_line, 31, 38));
		record.y=convert_string<double>(substring_of_columned_line(pdb_file_line, 39, 46));
		record.z=convert_string<double>(substring_of_columned_line(pdb_file_line, 47, 54));

		record.element=substring_of_columned_line(pdb_file_line, 77, 78);

		return record;
	}
};

}

#endif /* AUXILIARIES_PDB_FILE_PARSER_H_ */
