#ifndef AUXILIARIES_RESIDUE_LETTERS_CODING_H_
#define AUXILIARIES_RESIDUE_LETTERS_CODING_H_

#include <string>
#include <map>

namespace auxiliaries
{

class ResidueLettersCoding
{
public:
	inline static std::string convert_residue_code_big_to_small(const std::string& big_code)
	{
		static const std::map<std::string, std::string> m=create_map_of_residue_codes_big_to_small();
		const std::map<std::string, std::string>::const_iterator it=m.find(big_code);
		return (it==m.end() ? std::string("X") : it->second);
	}

	inline static std::string convert_residue_code_small_to_big(const std::string& small_code)
	{
		static const std::map<std::string, std::string> m=create_map_of_residue_codes_small_to_big();
		const std::map<std::string, std::string>::const_iterator it=m.find(small_code);
		return (it==m.end() ? std::string("XAA") : it->second);
	}
private:
	static std::map<std::string, std::string> create_map_of_residue_codes_big_to_small()
	{
		std::map<std::string, std::string> m;
		m["LEU"]="L";
		m["VAL"]="V";
		m["ILE"]="I";
		m["ALA"]="A";
		m["PHE"]="F";
		m["TRP"]="W";
		m["MET"]="M";
		m["PRO"]="P";
		m["ASP"]="D";
		m["GLU"]="E";
		m["LYS"]="K";
		m["ARG"]="R";
		m["HIS"]="H";
		m["CYS"]="C";
		m["SER"]="S";
		m["THR"]="T";
		m["TYR"]="Y";
		m["ASN"]="N";
		m["GLN"]="Q";
		m["GLY"]="G";
		m["ASX"]="B";
		m["GLX"]="Z";
		m["XLE"]="J";
		m["A"]="a";
		m["C"]="c";
		m["G"]="g";
		m["T"]="t";
		m["U"]="u";
		m["XAA"]="X";
		return m;
	}

	static std::map<std::string, std::string> create_map_of_residue_codes_small_to_big()
	{
		const std::map<std::string, std::string> m_big_to_small=create_map_of_residue_codes_big_to_small();
		std::map<std::string, std::string> m;
		for(std::map<std::string, std::string>::const_iterator it=m_big_to_small.begin();it!=m_big_to_small.end();++it)
		{
			m[it->second]=it->first;
		}
		return m;
	}
};

}

#endif /* AUXILIARIES_RESIDUE_LETTERS_CODING_H_ */
