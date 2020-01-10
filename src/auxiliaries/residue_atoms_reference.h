#ifndef AUXILIARIES_RESIDUE_ATOMS_REFERENCE_H_
#define AUXILIARIES_RESIDUE_ATOMS_REFERENCE_H_

#include <string>
#include <map>

namespace voronota
{

namespace auxiliaries
{

class ResidueAtomsReference
{
public:
	inline static int get_residue_atoms_count(const std::string& code)
	{
		static const std::map<std::string, int> m=create_map_of_residue_atoms_counts();
		const std::map<std::string, int>::const_iterator it=m.find(code);
		return (it==m.end() ? 1 : it->second);
	}

private:
	static std::map<std::string, int> create_map_of_residue_atoms_counts()
	{
		std::map<std::string, int> m;
		m["ALA"]=5;
		m["ARG"]=11;
		m["ASN"]=8;
		m["ASP"]=8;
		m["ASX"]=8;
		m["CYS"]=6;
		m["GLU"]=9;
		m["GLN"]=9;
		m["GLX"]=9;
		m["GLY"]=4;
		m["HIS"]=10;
		m["ILE"]=8;
		m["LEU"]=8;
		m["XLE"]=8;
		m["LYS"]=9;
		m["MET"]=8;
		m["PHE"]=11;
		m["PRO"]=7;
		m["SER"]=6;
		m["THR"]=7;
		m["TRP"]=14;
		m["TYR"]=12;
		m["VAL"]=7;
		return m;
	}
};

}

}

#endif /* AUXILIARIES_RESIDUE_ATOMS_REFERENCE_H_ */
