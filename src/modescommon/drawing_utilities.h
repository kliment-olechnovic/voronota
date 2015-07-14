#ifndef DRAWING_UTILITIES_H_
#define DRAWING_UTILITIES_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"
#include "../auxiliaries/opengl_printer.h"

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

struct DrawingParametersWrapper
{
	unsigned int default_color;
	bool adjuncts_rgb;
	bool use_labels;

	DrawingParametersWrapper() : default_color(0xFFFFFF), adjuncts_rgb(false), use_labels(false)
	{
	}

	void process(const CRAD& crad, const std::map<std::string, double>& adjuncts, auxiliaries::OpenGLPrinter& opengl_printer) const
	{
		if(use_labels)
		{
			opengl_printer.add_label(construct_label_from_crad(crad));
		}

		if(adjuncts_rgb)
		{
			const bool rp=adjuncts.count("r")>0;
			const bool gp=adjuncts.count("g")>0;
			const bool bp=adjuncts.count("b")>0;
			if(!(rp || gp || bp))
			{
				opengl_printer.add_color(default_color);
			}
			else
			{
				opengl_printer.add_color(
						(rp ? adjuncts.find("r")->second : 0.0),
						(gp ? adjuncts.find("g")->second : 0.0),
						(bp ? adjuncts.find("b")->second : 0.0));
			}
		}
	}
};

}

#endif /* DRAWING_UTILITIES_H_ */
