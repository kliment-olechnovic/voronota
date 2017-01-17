#include <iostream>
#include <stdexcept>
#include <bitset>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contact_value.h"
#include "modescommon/svg_writer.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::map< int, std::pair<int, int> > construct_map_of_chains_intervals(const std::map<CRAD, std::size_t>& axis)
{
	std::map< std::string, std::pair<int, int> > map_of_chains_intervals;
	for(std::map<CRAD, std::size_t>::const_iterator axis_it=axis.begin();axis_it!=axis.end();++axis_it)
	{
		const std::string& chain=axis_it->first.chainID;
		const int x=static_cast<int>(axis_it->second);
		std::map< std::string, std::pair<int, int> >::iterator map_of_chains_intervals_it=map_of_chains_intervals.find(chain);
		if(map_of_chains_intervals_it==map_of_chains_intervals.end())
		{
			map_of_chains_intervals[chain]=std::make_pair(x, x);
		}
		else
		{
			std::pair<int, int>& interval=map_of_chains_intervals_it->second;
			if(x<interval.first)
			{
				interval.first=x;
			}
			if(x>interval.second)
			{
				interval.second=x;
			}
		}
	}
	std::map< int, std::pair<int, int> > map_of_chain_numbers_intervals;
	{
		int i=0;
		for(std::map< std::string, std::pair<int, int> >::const_iterator it=map_of_chains_intervals.begin();it!=map_of_chains_intervals.end();++it)
		{
			map_of_chain_numbers_intervals[i++]=it->second;
		}
	}
	return map_of_chain_numbers_intervals;
}

int get_chain_number_from_coordinate(const std::map< int, std::pair<int, int> >& map_of_chains_intervals, const int x)
{
	for(std::map< int, std::pair<int, int> >::const_iterator it=map_of_chains_intervals.begin();it!=map_of_chains_intervals.end();++it)
	{
		const std::pair<int, int>& interval=it->second;
		if(x>=interval.first && x<=interval.second)
		{
			return (it->first);
		}
	}
	return 0;
}

std::set< std::pair<int, int> > collect_points(const std::map<CRAD, std::size_t>& axis, const std::map<CRADsPair, ContactValue>& map_of_contacts)
{
	std::set< std::pair<int, int> > points;
	for(std::map<CRADsPair, ContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		std::map<CRAD, std::size_t>::const_iterator x_it=axis.find(it->first.a);
		std::map<CRAD, std::size_t>::const_iterator y_it=axis.find(it->first.b);
		if(x_it!=axis.end() && y_it!=axis.end())
		{
			int x=static_cast<int>(x_it->second);
			int y=static_cast<int>(y_it->second);
			if(x>y)
			{
				std::swap(x, y);
			}
			points.insert(std::make_pair(x, y));
		}
	}
	return points;
}

std::bitset<49> get_pattern_from_coordinates(
		const std::map< int, std::pair<int, int> >& map_of_chains_intervals,
		const std::set< std::pair<int, int> >& points,
		const int ox,
		const int oy)
{
	const int ox_chain=get_chain_number_from_coordinate(map_of_chains_intervals, ox);
	const int oy_chain=get_chain_number_from_coordinate(map_of_chains_intervals, oy);
	std::bitset<49> pattern;
	int index=0;
	for(int dx=-3;dx<=3;dx++)
	{
		for(int dy=-3;dy<=3;dy++)
		{
			const int x=ox+dx;
			const int y=oy+dy;
			if(x>=0 && y>=0
					&& points.count(std::make_pair(x, y))
					&& get_chain_number_from_coordinate(map_of_chains_intervals, x)==ox_chain
					&& get_chain_number_from_coordinate(map_of_chains_intervals, y)==oy_chain)
			{
				pattern.set(index, true);
			}
			index++;
		}
	}
	return pattern;
}

std::map<int, CRAD> construct_reversed_axis_map(const std::map<CRAD, std::size_t>& axis)
{
	std::map<int, CRAD> reversed_axis_map;
	for(std::map<CRAD, std::size_t>::const_iterator axis_it=axis.begin();axis_it!=axis.end();++axis_it)
	{
		reversed_axis_map[static_cast<int>(axis_it->second)]=axis_it->first;
	}
	return reversed_axis_map;
}

}

void plot_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");

	const std::string background_color=poh.argument<std::string>(pohw.describe_option("--background-color", "string", "color string in SVG-acceptable format"), "#000000");
	const std::string default_color=poh.argument<std::string>(pohw.describe_option("--default-color", "string", "color string in SVG-acceptable format"), "#FFFFFF");
	const std::string adjunct_gradient=poh.argument<std::string>(pohw.describe_option("--adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"), "");
	const double adjunct_gradient_blue=poh.argument<double>(pohw.describe_option("--adjunct-gradient-blue", "number", "blue adjunct gradient value"), 0.0);
	const double adjunct_gradient_red=poh.argument<double>(pohw.describe_option("--adjunct-gradient-red", "number", "red adjunct gradient value"), 1.0);
	const bool adjuncts_rgb=poh.contains_option(pohw.describe_option("--adjuncts-rgb", "", "flag to use RGB color values from adjuncts"));
	const bool no_contraction=poh.contains_option(pohw.describe_option("--no-contraction", "", "flag to not contract gaps"));
	const std::string svg_output=poh.argument<std::string>(pohw.describe_option("--svg-output", "string", "file path to output plot of contacts in SVG format"), "");
	const std::string axis_output=poh.argument<std::string>(pohw.describe_option("--axis-output", "string", "file path to output axis"), "");
	const std::string points_output=poh.argument<std::string>(pohw.describe_option("--points-output", "string", "file path to output points"), "");
	const std::string patterns_output=poh.argument<std::string>(pohw.describe_option("--patterns-output", "string", "file path to output 7x7 patterns encoded as numbers"), "");
	const bool binarize_patterns=poh.contains_option(pohw.describe_option("--binarize-patterns", "", "flag to output patterns in binary format"));

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

	if(!svg_output.empty())
	{
		std::ofstream output(svg_output.c_str(), std::ios::out);
		if(output.good())
		{
			const std::size_t max_coordinate=(axis.rbegin()->second+1);
			SVGWriter svg(max_coordinate, max_coordinate);
			svg.add_rect(0, 0, max_coordinate, max_coordinate, std::string("fill:")+background_color);
			for(std::map<CRADsPair, ContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
			{
				const std::size_t x=axis[it->first.a];
				const std::size_t y=axis[it->first.b];
				std::string color=default_color;
				const std::map<std::string, double>& adjuncts=it->second.props.adjuncts;
				if(!adjunct_gradient.empty() && adjunct_gradient_blue!=adjunct_gradient_red)
				{
					if(adjuncts.count(adjunct_gradient)==1)
					{
						color=SVGWriter::color_from_blue_white_red_gradient(adjuncts.find(adjunct_gradient)->second, adjunct_gradient_blue, adjunct_gradient_red);
					}
				}
				else if(adjuncts_rgb && (adjuncts.count("r")>0 || adjuncts.count("g")>0 || adjuncts.count("b")>0))
				{
					const double r=(adjuncts.count("r")>0 ? adjuncts.find("r")->second : 0.0);
					const double g=(adjuncts.count("g")>0 ? adjuncts.find("g")->second : 0.0);
					const double b=(adjuncts.count("b")>0 ? adjuncts.find("b")->second : 0.0);
					color=SVGWriter::color_from_red_green_blue_components(r, g, b, 255);
				}
				svg.add_rect(x, y, 1, 1, std::string("fill:")+color);
				svg.add_rect(y, x, 1, 1, std::string("fill:")+color);
			}
			svg.write(output);
		}
	}

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
				output << axis[it->first.a] << " " << axis[it->first.b] << "\n";
			}
		}
	}

	if(!patterns_output.empty())
	{
		std::ofstream output(patterns_output.c_str(), std::ios::out);
		if(output.good())
		{
			const std::map< int, std::pair<int, int> > map_of_chains_intervals=construct_map_of_chains_intervals(axis);
			const std::set< std::pair<int, int> > points=collect_points(axis, map_of_contacts);
			std::map<int, CRAD> reversed_axis_map=construct_reversed_axis_map(axis);
			for(std::set< std::pair<int, int> >::const_iterator points_it=points.begin();points_it!=points.end();++points_it)
			{
				const std::bitset<49> pattern=get_pattern_from_coordinates(map_of_chains_intervals, points, points_it->first, points_it->second);
				if(pattern.count()>4)
				{
					if(binarize_patterns)
					{
						output << pattern;
					}
					else
					{
						output << reversed_axis_map[points_it->first] << " " << reversed_axis_map[points_it->second] << " " << pattern.to_ulong();
					}
					output << "\n";
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
}
