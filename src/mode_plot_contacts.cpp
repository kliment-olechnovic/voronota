#include <iostream>
#include <stdexcept>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contact_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

class XMLWriter
{
public:
	XMLWriter(const std::string& type) : type_(type)
	{
	}

	template<typename T>
	XMLWriter& set(const std::string& name, const T& value, const std::string& units="")
	{
		std::ostringstream output;
		output << value << units;
		parameters_[name]=output.str();
		return (*this);
	}

	XMLWriter& set(const std::string& contents)
	{
		contents_=contents;
		return (*this);
	}

	XMLWriter& add_child(const XMLWriter& child)
	{
		children_.push_back(child);
		return (*this);
	}

	void write(std::ostream& output, const std::size_t tabs=0) const
	{
		output << std::string(tabs, ' ') << "<" << type_;
		for(std::map<std::string, std::string>::const_iterator it=parameters_.begin();it!=parameters_.end();++it)
		{
			output << " " << it->first << "=\"" << it->second << "\"";
		}
		if(contents_.empty() && children_.empty())
		{
			output << "/>\n";
		}
		else
		{
			output << ">\n";
			if(!contents_.empty())
			{
				output << std::string(tabs+2, ' ') << contents_ << "\n";
			}
			for(std::size_t i=0;i<children_.size();i++)
			{
				children_[i].write(output, tabs+2);
			}
			output << "</" << type_ << ">\n";
		}
	}

private:
	std::string type_;
	std::map<std::string, std::string> parameters_;
	std::string contents_;
	std::vector<XMLWriter> children_;
};

class SVGWriter : public XMLWriter
{
public:
	SVGWriter(const unsigned int width, const unsigned int height) : XMLWriter("svg")
	{
		set("width", width);
		set("height", height);
	}

	SVGWriter& add_rect(const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height, const std::string& color)
	{
		add_child(XMLWriter("rect").set("x", x).set("y", y).set("width", width).set("height", height).set("fill", color));
		return (*this);
	}
};

std::string color_from_red_green_blue_components(const double r, const double g, const double b, const double scale)
{
	std::ostringstream output;
	output << "rgb(" << static_cast<unsigned int>(r*scale) << "," << static_cast<unsigned int>(g*scale) << "," << static_cast<unsigned int>(b*scale) << ")";
	return output.str();
}

}

void plot_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--background-color", "string", "color string in SVG-acceptable format"));
		ods.push_back(OD("--default-color", "string", "color string in SVG-acceptable format"));
		ods.push_back(OD("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");
			poh.print_io_description("stdout", false, true, "plot of contacts in SVG format");
			return;
		}
	}

	const std::string background_color=poh.argument<std::string>("--background-color", "#000000");
	const std::string default_color=poh.argument<std::string>("--default-color", "#FFFFFF");
	const bool adjuncts_rgb=poh.contains_option("--adjuncts-rgb");

	std::map<CRADsPair, ContactValue> map_of_contacts;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRAD, std::size_t> axis;
	{
		for(std::map<CRADsPair, ContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			axis[it->first.a]=0;
			axis[it->first.b]=0;
		}
		std::size_t i=0;
		for(std::map<CRAD, std::size_t>::iterator it=axis.begin();it!=axis.end();++it)
		{
			it->second=i++;
		}
	}

	SVGWriter svg(axis.size(), axis.size());
	svg.add_rect(0, 0, axis.size(), axis.size(), background_color);
	for(std::map<CRADsPair, ContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const std::size_t x=axis[it->first.a];
		const std::size_t y=axis[it->first.b];
		std::string color=default_color;
		const std::map<std::string, double>& adjuncts=it->second.props.adjuncts;
		if(adjuncts_rgb && (adjuncts.count("r")>0 || adjuncts.count("g")>0 || adjuncts.count("b")>0))
		{
			const double r=(adjuncts.count("r")>0 ? adjuncts.find("r")->second : 0.0);
			const double g=(adjuncts.count("g")>0 ? adjuncts.find("g")->second : 0.0);
			const double b=(adjuncts.count("b")>0 ? adjuncts.find("b")->second : 0.0);
			color=color_from_red_green_blue_components(r, g, b, 255);
		}
		svg.add_rect(x, y, 1, 1, color);
		svg.add_rect(y, x, 1, 1, color);
	}
	svg.write(std::cout);
}
