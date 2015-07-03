#ifndef DRAWING_UTILITIES_H_
#define DRAWING_UTILITIES_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

unsigned int calc_string_color_integer(const std::string& str)
{
	const long generator=123456789;
	const long limiter=0xFFFFFF;
	long hash=generator;
	for(std::size_t i=0;i<str.size();i++)
	{
		hash+=static_cast<long>(str[i]+1)*static_cast<long>(i+1)*generator;
	}
	return static_cast<unsigned int>(hash%limiter);
}

unsigned int calc_two_crads_color_integer(const CRAD& a, const CRAD& b)
{
	return calc_string_color_integer(a<b ? (a.str()+b.str()) : (b.str()+a.str()));
}

std::string construct_label_from_crad(const CRAD& crad)
{
	std::ostringstream output;
	output << "[ ";
	if(!crad.chainID.empty())
	{
		output << crad.chainID;
		output << " ";
	}
	if(crad.resSeq!=CRAD::null_num())
	{
		output << crad.resSeq;
		if(!crad.iCode.empty())
		{
			output << crad.iCode;
		}
		output << " ";
	}
	if(!crad.resName.empty())
	{
		output << crad.resName;
		output << " ";
	}
	if(!crad.name.empty())
	{
		output << crad.name;
		output << " ";
	}
	output << "]";
	return output.str();
}

std::string construct_label_from_two_crads(const CRAD& a, const CRAD& b)
{
	std::ostringstream output;
	output << construct_label_from_crad(a) << " " << construct_label_from_crad(b);
	return output.str();
}

}

#endif /* DRAWING_UTILITIES_H_ */
