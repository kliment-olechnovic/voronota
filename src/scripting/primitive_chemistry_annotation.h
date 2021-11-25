#ifndef SCRIPTING_PRIMITIVE_CHEMISTRY_ANNOTATION_H_
#define SCRIPTING_PRIMITIVE_CHEMISTRY_ANNOTATION_H_

#include "basic_types.h"

#include "../auxiliaries/residue_letters_coding.h"

namespace voronota
{

namespace scripting
{

class PrimitiveChemistryAnnotation
{
public:
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
		else if(!atom.crad.name.empty() && auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(atom.crad.resName)!="X")
		{
			if(atom.crad.name[0]=='C')
			{
				return 0;
			}
			else if(atom.crad.name[0]=='N')
			{
				return 1;
			}
			else if(atom.crad.name[0]=='O')
			{
				return 2;
			}
			else if(atom.crad.name[0]=='S')
			{
				return 3;
			}
			else if(atom.crad.name[0]=='P')
			{
				return 4;
			}
		}
		return -1;
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

