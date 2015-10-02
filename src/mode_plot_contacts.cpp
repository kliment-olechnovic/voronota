#include <iostream>
#include <stdexcept>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contact_value.h"
#include "modescommon/svg_writer.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void plot_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");
	pohw.describe_io("stdout", false, true, "plot of contacts in SVG format");

	const std::string background_color=poh.argument<std::string>(pohw.describe_option("--background-color", "string", "color string in SVG-acceptable format"), "#000000");
	const std::string default_color=poh.argument<std::string>(pohw.describe_option("--default-color", "string", "color string in SVG-acceptable format"), "#FFFFFF");
	const bool adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	const bool no_contraction=poh.contains_option(pohw.describe_option("--no-contraction", "", "flag to not contract gaps"));
	const std::string axis_output=poh.argument<std::string>(pohw.describe_option("--axis-output", "string", "file path to output axis"), "");
	const std::string points_output=poh.argument<std::string>(pohw.describe_option("--points-output", "string", "file path to output points"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

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

	if(no_contraction)
	{
		std::size_t i=0;
		std::map<CRAD, std::size_t>::iterator it=axis.begin();
		it->second=i++;
		++it;
		while(it!=axis.end())
		{
			std::map<CRAD, std::size_t>::iterator prev_it=it;
			--prev_it;
			const CRAD& a=prev_it->first;
			const CRAD& b=it->first;
			if(a.chainID==b.chainID)
			{
				for(int j=1;j<(b.resSeq-a.resSeq);j++)
				{
					i++;
				}
			}
			it->second=i++;
			++it;
		}
	}

	const std::size_t max_coordinate=(axis.rbegin()->second+1);

	SVGWriter svg(max_coordinate, max_coordinate);
	svg.add_rect(0, 0, max_coordinate, max_coordinate, background_color);
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

	if(!axis_output.empty())
	{
		auxiliaries::IOUtilities().write_map_to_file(axis, axis_output);
	}

	if(!points_output.empty())
	{
		std::ofstream output(points_output.c_str(), std::ios::out);
		if(output.good())
		{
			for(std::map<CRADsPair, ContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
			{
				std::size_t x=axis[it->first.a];
				std::size_t y=axis[it->first.b];
				if(x>y)
				{
					std::swap(x, y);
				}
				output << x << " " << y << "\n";
			}
		}
	}
}
