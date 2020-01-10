#ifndef COMMON_CONTACTS_SCORING_UTILITIES_H_
#define COMMON_CONTACTS_SCORING_UTILITIES_H_

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

struct InteractionName
{
	ChainResidueAtomDescriptorsPair crads;
	std::string tag;

	InteractionName()
	{
	}

	InteractionName(const ChainResidueAtomDescriptorsPair& crads, const std::string& tag) : crads(crads), tag(tag)
	{
	}

	bool operator==(const InteractionName& v) const
	{
		return (crads==v.crads && tag==v.tag);
	}

	bool operator<(const InteractionName& v) const
	{
		return ((crads<v.crads) || (crads==v.crads && tag<v.tag));
	}
};

inline std::ostream& operator<<(std::ostream& output, const InteractionName& v)
{
	output << v.crads << " " << v.tag;
	return output;
}

inline std::istream& operator>>(std::istream& input, InteractionName& v)
{
	input >> v.crads >> v.tag;
	return input;
}

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;
	int contacts_count;

	EnergyDescriptor() : total_area(0), strange_area(0), energy(0), contacts_count(0)
	{
	}

	void add(const EnergyDescriptor& ed)
	{
		total_area+=ed.total_area;
		strange_area+=ed.strange_area;
		energy+=ed.energy;
		contacts_count+=ed.contacts_count;
	}
};

inline std::ostream& operator<<(std::ostream& output, const EnergyDescriptor& v)
{
	output << v.total_area << " " << v.strange_area << " " << v.energy << " " << v.contacts_count;
	return output;
}

inline std::istream& operator>>(std::istream& input, EnergyDescriptor& v)
{
	input >> v.total_area >> v.strange_area >> v.energy >> v.contacts_count;
	return input;
}

inline ChainResidueAtomDescriptor generalize_crad(const ChainResidueAtomDescriptor& input_crad)
{
	ChainResidueAtomDescriptor crad=input_crad.without_numbering();

	if(crad.name=="OXT")
	{
		crad.name="O";
	}

	if(crad.name=="H1" || crad.name=="H2" || crad.name=="H3")
	{
		crad.name="H";
	}

	if(crad.resName=="MSE")
	{
		crad.resName="MET";
		if(crad.name=="SE")
		{
			crad.name="SD";
		}
	}

	if(crad.resName=="SEC")
	{
		crad.resName="CYS";
		if(crad.name=="SE")
		{
			crad.name="SG";
		}
	}

	if(crad.resName=="ALA")
	{
		if(crad.name=="HB1" || crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB1";
		}
	}

	if(crad.resName=="ARG")
	{
		if(crad.name=="NH1" || crad.name=="NH2")
		{
			crad.name="NH1";
		}
		else if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
		else if(crad.name=="HD2" || crad.name=="HD3")
		{
			crad.name="HD2";
		}
		else if(crad.name=="HH11" || crad.name=="HH12" || crad.name=="HH21" || crad.name=="HH22")
		{
			crad.name="HH11";
		}
	}

	if(crad.resName=="ASP")
	{
		if(crad.name=="OD1" || crad.name=="OD2")
		{
			crad.name="OD1";
		}
		else if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
	}

	if(crad.resName=="ASN")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HD21" || crad.name=="HD22")
		{
			crad.name="HD21";
		}
	}

	if(crad.resName=="CYS")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
	}

	if(crad.resName=="GLU")
	{
		if(crad.name=="OE1" || crad.name=="OE2")
		{
			crad.name="OE1";
		}
		else if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
	}

	if(crad.resName=="GLN")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
		else if(crad.name=="HE21" || crad.name=="HE22")
		{
			crad.name="HE21";
		}
	}

	if(crad.resName=="GLY")
	{
		if(crad.name=="HA2" || crad.name=="HA3")
		{
			crad.name="HA2";
		}
	}

	if(crad.resName=="HIS")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
	}

	if(crad.resName=="ILE")
	{
		if(crad.name=="HG12" || crad.name=="HG13")
		{
			crad.name="HG12";
		}
		else if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
		{
			crad.name="HG21";
		}
		else if(crad.name=="HD11" || crad.name=="HD12" || crad.name=="HD13")
		{
			crad.name="HD11";
		}
	}

	if(crad.resName=="LEU")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HD11" || crad.name=="HD12" || crad.name=="HD13")
		{
			crad.name="HD11";
		}
		else if(crad.name=="HD21" || crad.name=="HD22" || crad.name=="HD23")
		{
			crad.name="HD21";
		}
	}

	if(crad.resName=="LYS")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
		else if(crad.name=="HD2" || crad.name=="HD3")
		{
			crad.name="HD2";
		}
		else if(crad.name=="HE2" || crad.name=="HE3")
		{
			crad.name="HE2";
		}
		else if(crad.name=="HZ1" || crad.name=="HZ2" || crad.name=="HZ3")
		{
			crad.name="HZ1";
		}
	}

	if(crad.resName=="MET")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
		else if(crad.name=="HE1" || crad.name=="HE2" || crad.name=="HE3")
		{
			crad.name="HE1";
		}
	}

	if(crad.resName=="PHE")
	{
		if(crad.name=="CD1" || crad.name=="CD2")
		{
			crad.name="CD1";
		}
		else if(crad.name=="CE1" || crad.name=="CE2")
		{
			crad.name="CE1";
		}
		else if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HD1" || crad.name=="HD2")
		{
			crad.name="HD1";
		}
		else if(crad.name=="HE1" || crad.name=="HE2")
		{
			crad.name="HE1";
		}
	}

	if(crad.resName=="PRO")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HG2" || crad.name=="HG3")
		{
			crad.name="HG2";
		}
		else if(crad.name=="HD2" || crad.name=="HD3")
		{
			crad.name="HD2";
		}
	}

	if(crad.resName=="SER")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
	}

	if(crad.resName=="THR")
	{
		if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
		{
			crad.name="HG21";
		}
	}

	if(crad.resName=="TRP")
	{
		if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
	}

	if(crad.resName=="TYR")
	{
		if(crad.name=="CD1" || crad.name=="CD2")
		{
			crad.name="CD1";
		}
		else if(crad.name=="CE1" || crad.name=="CE2")
		{
			crad.name="CE1";
		}
		else if(crad.name=="HB2" || crad.name=="HB3")
		{
			crad.name="HB2";
		}
		else if(crad.name=="HD1" || crad.name=="HD2")
		{
			crad.name="HD1";
		}
		else if(crad.name=="HE1" || crad.name=="HE2")
		{
			crad.name="HE1";
		}
	}

	if(crad.resName=="VAL")
	{
		if(crad.name=="HG11" || crad.name=="HG12" || crad.name=="HG13")
		{
			crad.name="HG11";
		}
		else if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
		{
			crad.name="HG21";
		}
	}

	return crad;
}

inline ChainResidueAtomDescriptorsPair generalize_crads_pair(const ChainResidueAtomDescriptorsPair& input_crads)
{
	return ChainResidueAtomDescriptorsPair(generalize_crad(input_crads.a), generalize_crad(input_crads.b));
}

inline bool check_crads_pair_for_peptide_bond(const ChainResidueAtomDescriptorsPair& crads)
{
	return (((crads.a.name=="C" && crads.b.name=="N" && crads.a.resSeq<crads.b.resSeq) || (crads.a.name=="N" && crads.b.name=="C" && crads.b.resSeq<crads.a.resSeq))
			&& ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, 0, 1, false));
}

}

}

#endif /* COMMON_CONTACTS_SCORING_UTILITIES_H_ */
