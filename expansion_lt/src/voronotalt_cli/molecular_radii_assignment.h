#ifndef VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_
#define VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_

#include <string>
#include <map>
#include <sstream>
#include <cstdlib>

namespace voronotalt
{

class MolecularRadiiAssignment
{
public:
	static double& default_radius_value() noexcept
	{
		static double default_radius=1.8;
		return default_radius;
	}

	static double get_atom_radius(const std::string& resName, const std::string& name) noexcept
	{
		const double default_radius=default_radius_value();
		static const std::string wcany="*";

		if(name.empty() || name==wcany)
		{
			return default_radius;
		}

		const std::map<Descriptor, double>& mrd=initialized_radii_by_descriptors();

		for(int v=0;v<2;v++)
		{
			{
				std::map<Descriptor, double>::const_iterator it=mrd.find(Descriptor((v==0 ? resName : wcany), name));
				if(it!=mrd.end())
				{
					return it->second;
				}
			}

			for(std::size_t i=0;i<name.size();i++)
			{
				std::map<Descriptor, double>::const_iterator it=mrd.find(Descriptor((v==0 ? resName : wcany), name.substr(0, name.size()-i)+wcany));
				if(it!=mrd.end())
				{
					return it->second;
				}
			}
		}

		for(int v=0;v<2;v++)
		{
			std::map<Descriptor, double>::const_iterator it=mrd.find(Descriptor((v==0 ? resName : wcany), wcany));
			if(it!=mrd.end())
			{
				return it->second;
			}
		}

		return default_radius;
	}

	static void clear_radius_value_rules() noexcept
	{
		radii_by_descriptors().clear();
	}

	static bool set_radius_value_rules(const std::string& input_string) noexcept
	{
		if(input_string.empty())
		{
			return false;
		}
		std::map<Descriptor, double> map_of_rules;
		std::istringstream input(input_string);
		while(input.good())
		{
			std::string line;
			std::getline(input, line);
			if(!line.empty() && line[0]!='#')
			{
				if(!set_radius_value_rule(line, map_of_rules))
				{
					return false;
				}
			}
		}
		if(map_of_rules.empty())
		{
			return false;
		}
		radii_by_descriptors()=map_of_rules;
		return true;
	}

private:
	struct Descriptor
	{
		std::string resName_pattern;
		std::string name_pattern;

		Descriptor(const std::string& resName_pattern, const std::string& name_pattern) noexcept : resName_pattern(resName_pattern), name_pattern(name_pattern)
		{
		}

		bool operator<(const Descriptor& d) const noexcept
		{
			return ( resName_pattern<d.resName_pattern || (resName_pattern==d.resName_pattern && (name_pattern<d.name_pattern)) );
		}
	};

	static std::map<Descriptor, double>& radii_by_descriptors() noexcept
	{
		static std::map<Descriptor, double> map_of_radii_by_descriptors;
		return map_of_radii_by_descriptors;
	}

	static std::map<Descriptor, double>& initialized_radii_by_descriptors() noexcept
	{
		std::map<Descriptor, double>& mrd=radii_by_descriptors();
		if(mrd.empty())
		{
			mrd[Descriptor("*", "C*")]=1.80;
			mrd[Descriptor("*", "N*")]=1.60;
			mrd[Descriptor("*", "O*")]=1.50;
			mrd[Descriptor("*", "P*")]=1.90;
			mrd[Descriptor("*", "S*")]=1.90;
			mrd[Descriptor("*", "H*")]=1.30;

			mrd[Descriptor("*", "C")]=1.75;
			mrd[Descriptor("*", "CA")]=1.90;
			mrd[Descriptor("*", "N")]=1.70;
			mrd[Descriptor("*", "O")]=1.49;

			mrd[Descriptor("ALA", "CB")]=1.92;

			mrd[Descriptor("ARG", "CB")]=1.91;
			mrd[Descriptor("ARG", "CD*")]=1.88;
			mrd[Descriptor("ARG", "CG*")]=1.92;
			mrd[Descriptor("ARG", "CZ*")]=1.80;
			mrd[Descriptor("ARG", "NE*")]=1.62;
			mrd[Descriptor("ARG", "NH1")]=1.62;
			mrd[Descriptor("ARG", "NH2")]=1.67;

			mrd[Descriptor("ASN", "CB")]=1.91;
			mrd[Descriptor("ASN", "CG*")]=1.81;
			mrd[Descriptor("ASN", "ND2")]=1.62;
			mrd[Descriptor("ASN", "OD1")]=1.52;

			mrd[Descriptor("ASP", "CB")]=1.91;
			mrd[Descriptor("ASP", "CG*")]=1.76;
			mrd[Descriptor("ASP", "OD1")]=1.49;
			mrd[Descriptor("ASP", "OD2")]=1.49;

			mrd[Descriptor("CYS", "CB")]=1.91;
			mrd[Descriptor("CYS", "S*")]=1.88;

			mrd[Descriptor("GLN", "CB")]=1.91;
			mrd[Descriptor("GLN", "CD*")]=1.81;
			mrd[Descriptor("GLN", "CG*")]=1.80;
			mrd[Descriptor("GLN", "NE2")]=1.62;
			mrd[Descriptor("GLN", "OE1")]=1.52;

			mrd[Descriptor("GLU", "CB")]=1.91;
			mrd[Descriptor("GLU", "CD*")]=1.76;
			mrd[Descriptor("GLU", "CG*")]=1.88;
			mrd[Descriptor("GLU", "OE1")]=1.49;
			mrd[Descriptor("GLU", "OE2")]=1.49;

			mrd[Descriptor("HIS", "CB")]=1.91;
			mrd[Descriptor("HIS", "CD*")]=1.74;
			mrd[Descriptor("HIS", "CE*")]=1.74;
			mrd[Descriptor("HIS", "CG*")]=1.80;
			mrd[Descriptor("HIS", "ND1")]=1.60;
			mrd[Descriptor("HIS", "NE2")]=1.60;

			mrd[Descriptor("ILE", "CB")]=2.01;
			mrd[Descriptor("ILE", "CD1")]=1.92;
			mrd[Descriptor("ILE", "CG1")]=1.92;
			mrd[Descriptor("ILE", "CG2")]=1.92;

			mrd[Descriptor("LEU", "CB")]=1.91;
			mrd[Descriptor("LEU", "CD1")]=1.92;
			mrd[Descriptor("LEU", "CD2")]=1.92;
			mrd[Descriptor("LEU", "CG*")]=2.01;

			mrd[Descriptor("LYS", "CB")]=1.91;
			mrd[Descriptor("LYS", "CD*")]=1.92;
			mrd[Descriptor("LYS", "CE*")]=1.88;
			mrd[Descriptor("LYS", "CG*")]=1.92;
			mrd[Descriptor("LYS", "NZ*")]=1.67;

			mrd[Descriptor("MET", "CB")]=1.91;
			mrd[Descriptor("MET", "CE*")]=1.80;
			mrd[Descriptor("MET", "CG*")]=1.92;
			mrd[Descriptor("MET", "S*")]=1.94;

			mrd[Descriptor("PHE", "CB")]=1.91;
			mrd[Descriptor("PHE", "CD*")]=1.82;
			mrd[Descriptor("PHE", "CE*")]=1.82;
			mrd[Descriptor("PHE", "CG*")]=1.74;
			mrd[Descriptor("PHE", "CZ*")]=1.82;

			mrd[Descriptor("PRO", "CB")]=1.91;
			mrd[Descriptor("PRO", "CD*")]=1.92;
			mrd[Descriptor("PRO", "CG*")]=1.92;

			mrd[Descriptor("SER", "CB")]=1.91;
			mrd[Descriptor("SER", "OG*")]=1.54;

			mrd[Descriptor("THR", "CB")]=2.01;
			mrd[Descriptor("THR", "CG2")]=1.92;
			mrd[Descriptor("THR", "OG*")]=1.54;

			mrd[Descriptor("TRP", "CB")]=1.91;
			mrd[Descriptor("TRP", "CD*")]=1.82;
			mrd[Descriptor("TRP", "CE*")]=1.82;
			mrd[Descriptor("TRP", "CE2")]=1.74;
			mrd[Descriptor("TRP", "CG*")]=1.74;
			mrd[Descriptor("TRP", "CH*")]=1.82;
			mrd[Descriptor("TRP", "CZ*")]=1.82;
			mrd[Descriptor("TRP", "NE1")]=1.66;

			mrd[Descriptor("TYR", "CB")]=1.91;
			mrd[Descriptor("TYR", "CD*")]=1.82;
			mrd[Descriptor("TYR", "CE*")]=1.82;
			mrd[Descriptor("TYR", "CG*")]=1.74;
			mrd[Descriptor("TYR", "CZ*")]=1.80;
			mrd[Descriptor("TYR", "OH*")]=1.54;

			mrd[Descriptor("VAL", "CB")]=2.01;
			mrd[Descriptor("VAL", "CG1")]=1.92;
			mrd[Descriptor("VAL", "CG2")]=1.92;

			mrd[Descriptor("*", "F*")]=1.33;
			mrd[Descriptor("*", "CL*")]=1.81;
			mrd[Descriptor("*", "BR*")]=1.96;
			mrd[Descriptor("*", "I*")]=2.20;

			mrd[Descriptor("AL", "AL")]=0.60;
			mrd[Descriptor("AS", "AS")]=0.58;
			mrd[Descriptor("AU", "AU")]=1.37;
			mrd[Descriptor("BA", "BA")]=1.35;
			mrd[Descriptor("BE", "BE")]=0.45;
			mrd[Descriptor("BI", "BI")]=1.03;
			mrd[Descriptor("CA", "CA")]=1.00;
			mrd[Descriptor("CD", "CD")]=0.95;
			mrd[Descriptor("CO", "CO")]=0.65;
			mrd[Descriptor("CR", "CR")]=0.73;
			mrd[Descriptor("CS", "CS")]=1.67;
			mrd[Descriptor("CU", "CU")]=0.73;
			mrd[Descriptor("FE", "FE")]=0.61;
			mrd[Descriptor("HE*", "FE")]=0.61;
			mrd[Descriptor("GA", "GA")]=0.62;
			mrd[Descriptor("GE", "GE")]=0.73;
			mrd[Descriptor("HG", "HG")]=1.02;
			mrd[Descriptor("K", "K")]=1.38;
			mrd[Descriptor("LI", "LI")]=0.76;
			mrd[Descriptor("MG", "MG")]=0.72;
			mrd[Descriptor("MN", "MN")]=0.83;
			mrd[Descriptor("MO", "MO")]=0.69;
			mrd[Descriptor("NA", "NA")]=1.02;
			mrd[Descriptor("NI", "NI")]=0.69;
			mrd[Descriptor("PB", "PB")]=1.19;
			mrd[Descriptor("PD", "PD")]=0.86;
			mrd[Descriptor("PT", "PT")]=0.80;
			mrd[Descriptor("RB", "RB")]=1.52;
			mrd[Descriptor("SB", "SB")]=0.76;
			mrd[Descriptor("SC", "SC")]=0.75;
			mrd[Descriptor("SN", "SN")]=0.69;
			mrd[Descriptor("SR", "SR")]=1.18;
			mrd[Descriptor("TC", "TC")]=0.65;
			mrd[Descriptor("TI", "TI")]=0.86;
			mrd[Descriptor("V", "V")]=0.79;
			mrd[Descriptor("ZN", "ZN")]=0.74;
			mrd[Descriptor("ZR", "ZR")]=0.72;
		}
		return mrd;
	}

	static bool read_config_line(const std::string& input, std::string& resName, std::string& name, double& radius_value) noexcept
	{
	    const std::string::size_type n=input.size();
	    std::string::size_type start=0;
	    std::string::size_type pos=0;

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
	    while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    resName=input.substr(start, pos-start);

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
	    while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    name=input.substr(start, pos-start);

	    while(pos<n && std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    if(pos>=n)
	    {
	        return false;
	    }
	    start=pos;
	    while(pos<n && !std::isspace(static_cast<unsigned char>(input[pos])))
	    {
	    	++pos;
	    }
	    const std::string value_str=input.substr(start, pos-start);
	    const char* str=&value_str[0];
	    char* str_next=0;
	    const double value=std::strtod(str, &str_next);
		if(str==str_next)
		{
			return false;
		}
		else
		{
			radius_value=value;
		}

	    return (!(resName.empty() || name.empty() || radius_value<0.0));
	}

	static bool set_radius_value_rule(const std::string& resName, const std::string& name, const double radius_value, std::map<Descriptor, double>& map_of_rules) noexcept
	{
		if(resName.empty() || name.empty() || radius_value<0.0)
		{
			return false;
		}
		map_of_rules[Descriptor(resName, name)]=radius_value;
		return true;
	}

	static bool set_radius_value_rule(const std::string& input, std::map<Descriptor, double>& map_of_rules) noexcept
	{
		std::string resName;
		std::string name;
		double radius_value=default_radius_value();

		if(!read_config_line(input, resName, name, radius_value))
		{
			return false;
		}

		return set_radius_value_rule(resName, name, radius_value, map_of_rules);
	}
};

}

#endif /* VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_ */
