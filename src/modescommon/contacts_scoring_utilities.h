#ifndef CONTACTS_SCORING_UTILITIES_H_
#define CONTACTS_SCORING_UTILITIES_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

struct InteractionName
{
	CRADsPair crads;
	std::string tag;

	InteractionName()
	{
	}

	InteractionName(const CRADsPair& crads, const std::string& tag) : crads(crads), tag(tag)
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

inline CRAD generalize_crad(const CRAD& input_crad)
{
	CRAD crad=input_crad.without_numbering();
	if(crad.resName=="ARG" && (crad.name=="NH1" || crad.name=="NH2"))
	{
		crad.name="NH1";
	}
	else if(crad.resName=="ASP" && (crad.name=="OD1" || crad.name=="OD2"))
	{
		crad.name="OD1";
	}
	else if(crad.resName=="GLU" && (crad.name=="OE1" || crad.name=="OE2"))
	{
		crad.name="OE1";
	}
	else if(crad.resName=="PHE" && (crad.name=="CD1" || crad.name=="CD2"))
	{
		crad.name="CD1";
	}
	else if(crad.resName=="PHE" && (crad.name=="CE1" || crad.name=="CE2"))
	{
		crad.name="CE1";
	}
	else if(crad.resName=="TYR" && (crad.name=="CD1" || crad.name=="CD2"))
	{
		crad.name="CD1";
	}
	else if(crad.resName=="TYR" && (crad.name=="CE1" || crad.name=="CE2"))
	{
		crad.name="CE1";
	}
	else if(crad.name=="OXT")
	{
		crad.name="O";
	}
	else if(crad.resName=="MSE")
	{
		crad.resName="MET";
		if(crad.name=="SE")
		{
			crad.name="SD";
		}
	}
	else if(crad.resName=="SEC")
	{
		crad.resName="CYS";
		if(crad.name=="SE")
		{
			crad.name="SG";
		}
	}
	return crad;
}

inline CRADsPair generalize_crads_pair(const CRADsPair& input_crads)
{
	return CRADsPair(generalize_crad(input_crads.a), generalize_crad(input_crads.b));
}

inline bool check_crads_pair_for_peptide_bond(const CRADsPair& crads)
{
	return (((crads.a.name=="C" && crads.b.name=="N" && crads.a.resSeq<crads.b.resSeq) || (crads.a.name=="N" && crads.b.name=="C" && crads.b.resSeq<crads.a.resSeq))
			&& CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, 1, false));
}

}

#endif /* CONTACTS_SCORING_UTILITIES_H_ */
