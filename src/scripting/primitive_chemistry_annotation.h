#ifndef SCRIPTING_PRIMITIVE_CHEMISTRY_ANNOTATION_H_
#define SCRIPTING_PRIMITIVE_CHEMISTRY_ANNOTATION_H_

#include "basic_types.h"
#include "virtual_file_storage.h"

#include "../auxiliaries/residue_letters_coding.h"

namespace voronota
{

namespace scripting
{

class PrimitiveChemistryAnnotation
{
public:
	class Configuration
	{
	public:
		std::map<common::ChainResidueAtomDescriptor, std::string> map_of_knodle_atom_types;
		std::vector<std::string> list_of_knodle_atom_types;
		std::map<std::string, int> map_of_knodle_atom_type_numbers;

		bool valid() const
		{
			return (!map_of_knodle_atom_types.empty() && !list_of_knodle_atom_types.empty() && list_of_knodle_atom_types.size()==map_of_knodle_atom_type_numbers.size());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& knodle_atom_types_file)
		{
			if(knodle_atom_types_file.empty())
			{
				return false;
			}

			Configuration new_configuration;

			{
				InputSelector potential_file_input_selector(knodle_atom_types_file);
				std::istream& input=potential_file_input_selector.stream();

				while(input.good())
				{
					common::ChainResidueAtomDescriptor crad;
					std::string atom_type_name;
					input >> crad.resName >> crad.name >> atom_type_name;
					if(input.fail())
					{
						return false;
					}
					new_configuration.map_of_knodle_atom_types[crad]=atom_type_name;
					input >> std::ws;
				}

				if(new_configuration.map_of_knodle_atom_types.empty())
				{
					return false;
				}
			}

			{
				std::set<std::string> set_of_knodle_atom_types;
				for(std::map<common::ChainResidueAtomDescriptor, std::string>::const_iterator it=new_configuration.map_of_knodle_atom_types.begin();it!=new_configuration.map_of_knodle_atom_types.end();++it)
				{
					set_of_knodle_atom_types.insert(it->second);
				}
				new_configuration.list_of_knodle_atom_types.insert(new_configuration.list_of_knodle_atom_types.end(), set_of_knodle_atom_types.begin(), set_of_knodle_atom_types.end());
			}

			for(std::size_t i=0;i<new_configuration.list_of_knodle_atom_types.size();i++)
			{
				new_configuration.map_of_knodle_atom_type_numbers[new_configuration.list_of_knodle_atom_types[i]]=i;
			}

			get_default_configuration_mutable()=new_configuration;

			return true;
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	static const std::string& get_knodle_atom_type_string(const Configuration& configuration, const int atom_type_number)
	{
		static const std::string default_atom_type_string="";
		return ((atom_type_number>=0 && atom_type_number<static_cast<int>(configuration.list_of_knodle_atom_types.size())) ? configuration.list_of_knodle_atom_types[atom_type_number] : default_atom_type_string);
	}

	static const std::string& get_knodle_atom_type_string(const int atom_type_number)
	{
		return get_knodle_atom_type_string(Configuration::get_default_configuration(), atom_type_number);
	}

	static const std::string& get_knodle_atom_type_string(const Configuration& configuration, const common::ChainResidueAtomDescriptor& crad)
	{
		common::ChainResidueAtomDescriptor plain_crad;
		plain_crad.resName=crad.resName;
		plain_crad.name=crad.name;
		std::map<common::ChainResidueAtomDescriptor, std::string>::const_iterator it=configuration.map_of_knodle_atom_types.find(plain_crad);
		return (it!=configuration.map_of_knodle_atom_types.end() ? it->second : get_knodle_atom_type_string(-1));
	}

	static const std::string& get_knodle_atom_type_string(const common::ChainResidueAtomDescriptor& crad)
	{
		return get_knodle_atom_type_string(Configuration::get_default_configuration(), crad);
	}

	static int get_knodle_atom_type_number(const Configuration& configuration, const common::ChainResidueAtomDescriptor& crad)
	{
		const std::string atom_type_string=get_knodle_atom_type_string(configuration, crad);
		std::map<std::string, int>::const_iterator it=configuration.map_of_knodle_atom_type_numbers.find(atom_type_string);
		return (it!=configuration.map_of_knodle_atom_type_numbers.end() ? it->second : -1);
	}

	static int get_knodle_atom_type_number(const common::ChainResidueAtomDescriptor& crad)
	{
		return get_knodle_atom_type_number(Configuration::get_default_configuration(), crad);
	}

	static const std::string& get_CNOSP_atom_type_string(const int atom_type_number)
	{
		static std::vector<std::string> atom_type_strings;
		if(atom_type_strings.empty())
		{
			atom_type_strings.reserve(6);
			atom_type_strings.push_back("C");
			atom_type_strings.push_back("N");
			atom_type_strings.push_back("O");
			atom_type_strings.push_back("S");
			atom_type_strings.push_back("P");
			atom_type_strings.push_back("");
		}
		return ((atom_type_number>=0 && atom_type_number<static_cast<int>(atom_type_strings.size())) ? atom_type_strings[atom_type_number] : atom_type_strings.back());
	}

	static int get_CNOSP_atom_type_number(const common::ChainResidueAtomDescriptor& crad)
	{
		if(!crad.name.empty() && auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(crad.resName)!="X")
		{
			if(crad.name[0]=='C')
			{
				return 0;
			}
			else if(crad.name[0]=='N')
			{
				return 1;
			}
			else if(crad.name[0]=='O')
			{
				return 2;
			}
			else if(crad.name[0]=='S')
			{
				return 3;
			}
			else if(crad.name[0]=='P')
			{
				return 4;
			}
		}
		return -1;
	}

	static int get_CNOSP_atom_type_number(const Atom& atom)
	{
		if(atom.value.props.tags.count("el=C")>0)
		{
			return 0;
		}
		else if(atom.value.props.tags.count("el=N")>0)
		{
			return 1;
		}
		else if(atom.value.props.tags.count("el=O")>0)
		{
			return 2;
		}
		else if(atom.value.props.tags.count("el=S")>0)
		{
			return 3;
		}
		else if(atom.value.props.tags.count("el=P")>0)
		{
			return 4;
		}
		return get_CNOSP_atom_type_number(atom.crad);
	}

	static double get_protein_atom_hydropathy(const Atom& atom)
	{
		const std::string& residue_name=atom.crad.resName;
		double value=-9.0;
		if(residue_name=="ILE"){value=4.5;}
		else if(residue_name=="VAL"){value=4.2;}
		else if(residue_name=="LEU"){value=3.8;}
		else if(residue_name=="PHE"){value=2.8;}
		else if(residue_name=="CYS"){value=2.5;}
		else if(residue_name=="MET"){value=1.9;}
		else if(residue_name=="ALA"){value=1.8;}
		else if(residue_name=="GLY"){value=-0.4;}
		else if(residue_name=="THR"){value=-0.7;}
		else if(residue_name=="SER"){value=-0.8;}
		else if(residue_name=="TRP"){value=-0.9;}
		else if(residue_name=="TYR"){value=-1.3;}
		else if(residue_name=="PRO"){value=-1.6;}
		else if(residue_name=="HIS"){value=-3.2;}
		else if(residue_name=="GLU"){value=-3.5;}
		else if(residue_name=="GLN"){value=-3.5;}
		else if(residue_name=="ASP"){value=-3.5;}
		else if(residue_name=="ASN"){value=-3.5;}
		else if(residue_name=="LYS"){value=-3.9;}
		else if(residue_name=="ARG"){value=-4.5;}
		return (value/9.0);
	}
};

}

}

#endif /* SCRIPTING_PRIMITIVE_CHEMISTRY_ANNOTATION_H_ */

