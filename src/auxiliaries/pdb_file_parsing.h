#ifndef PDB_FILE_PARSING_H_
#define PDB_FILE_PARSING_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

namespace auxiliaries
{

class PDBFileParsing
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
		double tempFactor;
		std::string element;
		std::string charge;

		AtomRecord(const std::string& pdb_file_line) :
			record_name(substring_of_columned_line(pdb_file_line, 1, 6)),
			serial(convert_string<int>(substring_of_columned_line(pdb_file_line, 7, 11))),
			name(substring_of_columned_line(pdb_file_line, 13, 16)),
			altLoc(substring_of_columned_line(pdb_file_line, 17, 17)),
			resName(substring_of_columned_line(pdb_file_line, 18, 20)),
			chainID(substring_of_columned_line(pdb_file_line, 22, 22)),
			resSeq(convert_string<int>(substring_of_columned_line(pdb_file_line, 23, 26))),
			iCode(substring_of_columned_line(pdb_file_line, 27, 27)),
			x(convert_string<double>(substring_of_columned_line(pdb_file_line, 31, 38))),
			y(convert_string<double>(substring_of_columned_line(pdb_file_line, 39, 46))),
			z(convert_string<double>(substring_of_columned_line(pdb_file_line, 47, 54))),
			tempFactor(safe_convert_string<double>(substring_of_columned_line(pdb_file_line, 61, 66), 0)),
			element(substring_of_columned_line(pdb_file_line, 77, 78)),
			charge(substring_of_columned_line(pdb_file_line, 79, 80))
		{
			if (record_name.empty() || name.empty() || resName.empty())
			{
				throw std::runtime_error("Atom record has not enough string data.");
			}
			if(chainID.empty())
			{
				chainID="?";
			}
		}
	};

	std::vector<AtomRecord> read_atom_records_from_pdb_file_stream(std::istream& pdb_file_stream)
	{
		std::vector<AtomRecord> records;
		while(pdb_file_stream.good())
		{
			std::string line;
			std::getline(pdb_file_stream, line);
			const std::string record_name=substring_of_columned_line(line, 1, 6);
			if(record_name=="ATOM" || record_name=="HETATM")
			{
				try
				{
					const AtomRecord record(line);
					records.push_back(record);
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
	inline std::string substring_of_columned_line(const std::string& line, const int start, const int end)
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
	inline T convert_string(const std::string& str)
	{
		std::istringstream input(str);
		input.exceptions(std::istringstream::failbit | std::istringstream::badbit);
		T value;
		input >> value;
		return value;
	}
};

}

#endif /* PDB_FILE_PARSING_H_ */
