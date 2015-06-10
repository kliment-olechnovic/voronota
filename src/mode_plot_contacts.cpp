#include <iostream>
#include <stdexcept>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/contact_value.h"
#include "modescommon/svg_writer.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

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
			color=SVGWriter::color_from_red_green_blue_components(r, g, b, 255);
		}
		svg.add_rect(x, y, 1, 1, color);
		svg.add_rect(y, x, 1, 1, color);
	}
	svg.write(std::cout);
}
