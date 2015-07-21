#ifndef DRAWING_UTILITIES_H_
#define DRAWING_UTILITIES_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"
#include "../auxiliaries/opengl_printer.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

class DrawingParametersWrapper
{
public:
	unsigned int default_color;
	std::string adjunct_gradient;
	double adjunct_gradient_blue;
	double adjunct_gradient_red;
	bool adjuncts_rgb;
	bool random_colors;
	bool random_colors_by_chain;
	double alpha_opacity;
	bool use_labels;
	bool rainbow_gradient;

	DrawingParametersWrapper() :
		default_color(0xFFFFFF),
		adjunct_gradient(""),
		adjunct_gradient_blue(0.0),
		adjunct_gradient_red(1.0),
		adjuncts_rgb(false),
		random_colors(false),
		random_colors_by_chain(false),
		alpha_opacity(1.0),
		use_labels(false),
		rainbow_gradient(false)
	{
	}

	template<typename T>
	void process(const T& descriptor, const std::map<std::string, double>& adjuncts, auxiliaries::OpenGLPrinter& opengl_printer) const
	{
		if(use_labels)
		{
			opengl_printer.add_label(construct_label(descriptor));
		}

		if(!adjunct_gradient.empty() && adjunct_gradient_blue!=adjunct_gradient_red)
		{
			if(adjuncts.count(adjunct_gradient)<1)
			{
				opengl_printer.add_color(default_color);
			}
			else
			{
				double value=adjuncts.find(adjunct_gradient)->second;
				if(adjunct_gradient_blue<adjunct_gradient_red)
				{
					value=(value-adjunct_gradient_blue)/(adjunct_gradient_red-adjunct_gradient_blue);
				}
				else
				{
					value=1.0-((value-adjunct_gradient_red)/(adjunct_gradient_blue-adjunct_gradient_red));
				}
				if(rainbow_gradient)
				{
					opengl_printer.add_color_from_rainbow_gradient(value);
				}
				else
				{
					opengl_printer.add_color_from_blue_white_red_gradient(value);
				}
			}
		}
		else if(adjuncts_rgb)
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
		else if(random_colors)
		{
			opengl_printer.add_color(calc_color_integer(descriptor));
		}
		else if(random_colors_by_chain)
		{
			opengl_printer.add_color(calc_color_integer_by_chain(descriptor));
		}
	}

private:
	static unsigned int calc_string_color_integer(const std::string& str)
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

	static unsigned int calc_lighter_color_integer(const unsigned int color)
	{
		return std::min(color+static_cast<unsigned int>(0x444444), static_cast<unsigned int>(0xFFFFFF));
	}

	static unsigned int calc_color_integer(const CRAD& crad)
	{
		return calc_string_color_integer(crad.str());
	}

	static unsigned int calc_color_integer(const std::pair<CRAD, CRAD>& crads)
	{
		return calc_string_color_integer(
				crads.first<crads.second ?
						(crads.first.str()+crads.second.str()) :
						(crads.second.str()+crads.first.str()));
	}

	static unsigned int calc_color_integer_by_chain(const CRAD& crad)
	{
		return calc_lighter_color_integer(calc_string_color_integer(crad.chainID));
	}

	static unsigned int calc_color_integer_by_chain(const std::pair<CRAD, CRAD>& crads)
	{
		return calc_lighter_color_integer(calc_string_color_integer(crads.first.chainID<crads.second.chainID ? (crads.first.chainID+crads.second.chainID) : (crads.second.chainID+crads.first.chainID)));
	}

	static std::string construct_label(const CRAD& crad)
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

	static std::string construct_label(const std::pair<CRAD, CRAD>& crads)
	{
		std::ostringstream output;
		output << construct_label(crads.first) << " " << construct_label(crads.second);
		return output.str();
	}
};

}

#endif /* DRAWING_UTILITIES_H_ */
