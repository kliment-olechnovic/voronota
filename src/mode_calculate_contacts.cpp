#include <iostream>

#include "apollota/constrained_contacts_construction.h"

#include "modes_commons.h"

namespace
{

struct Comment
{
	int serial;
	std::string chainID;
	int resSeq;
	std::string resName;
	std::string name;
	std::string altLoc;
	std::string iCode;

	std::string str(bool with_residue, bool with_atom) const
	{
		static const std::string any="*";
		static const std::string sep=":";
		const bool with_residue_and_atom=(with_residue && with_atom);
		std::ostringstream output;
		output << "(" << chainID << sep;
		if(with_residue) { output << resSeq << iCode; } else { output << any; }
		output << sep;
		if(with_residue_and_atom) { output << serial << altLoc; } else { output << any; }
		output << ")[" << (with_residue ? resName : any) << sep << (with_residue_and_atom ? name : any) << "]";
		return output.str();
	}
};

inline void add_sphere_and_comments_from_stream_to_vectors(std::istream& input, std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* >& spheres_with_comments)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	std::string separator;
	input >> separator;
	if(!input.fail() && separator=="#")
	{
		Comment comment;
		input >> comment.serial >> comment.chainID >> comment.resSeq >> comment.resName >> comment.name;
		if(input.good())
		{
			input >> comment.altLoc >> comment.iCode;
		}
		if(!input.fail())
		{
			if(comment.altLoc.find_first_of(".?")==0)
			{
				comment.altLoc.clear();
			}
			if(comment.iCode.find_first_of(".?")==0)
			{
				comment.iCode.clear();
			}
			spheres_with_comments.first->push_back(sphere);
			spheres_with_comments.second->push_back(comment);
		}
	}
}

void record_annotated_solvent_contact(const Comment& comment, const double area, std::map< std::pair<std::string, std::string>, double >& map_of_named_contacts)
{
	static const std::string solvent_str("[solvent]");
	map_of_named_contacts[std::make_pair(comment.str(true, true), solvent_str)]+=area;
	map_of_named_contacts[std::make_pair(comment.str(true, false), solvent_str)]+=area;
	map_of_named_contacts[std::make_pair(comment.str(false, false), solvent_str)]+=area;
}

void record_annotated_inter_atom_contact(const Comment& comment1, const Comment& comment2, const double area, std::map< std::pair<std::string, std::string>, double >& map_of_named_contacts)
{
	static const std::string nonsolvent_str("[nonsolvent]");
	const std::string full_str1=comment1.str(true, true);
	const std::string full_str2=comment2.str(true, true);
	if(full_str1!=full_str2)
	{
		map_of_named_contacts[std::make_pair(full_str1, full_str2)]+=area;
		map_of_named_contacts[std::make_pair(full_str2, full_str1)]+=area;
		map_of_named_contacts[std::make_pair(full_str1, nonsolvent_str)]+=area;
		map_of_named_contacts[std::make_pair(full_str2, nonsolvent_str)]+=area;
		const std::string residue_str1=comment1.str(true, false);
		const std::string residue_str2=comment2.str(true, false);
		if(residue_str1!=residue_str2)
		{
			map_of_named_contacts[std::make_pair(residue_str1, residue_str2)]+=area;
			map_of_named_contacts[std::make_pair(residue_str2, residue_str1)]+=area;
			map_of_named_contacts[std::make_pair(residue_str1, nonsolvent_str)]+=area;
			map_of_named_contacts[std::make_pair(residue_str2, nonsolvent_str)]+=area;
			const std::string chain_str1=comment1.str(false, false);
			const std::string chain_str2=comment2.str(false, false);
			if(chain_str1!=chain_str2)
			{
				map_of_named_contacts[std::make_pair(chain_str1, chain_str2)]+=area;
				map_of_named_contacts[std::make_pair(chain_str2, chain_str1)]+=area;
				map_of_named_contacts[std::make_pair(chain_str1, nonsolvent_str)]+=area;
				map_of_named_contacts[std::make_pair(chain_str2, nonsolvent_str)]+=area;
			}
		}
	}
}

void print_map_of_named_contacts(const std::map< std::pair<std::string, std::string>, double >& map_of_named_contacts, const std::vector<std::string>& constraints_first, const std::vector<std::string>& constraints_second)
{
	const std::size_t default_width=std::cout.width();
	std::size_t max_width1=0;
	std::size_t max_width2=0;
	for(std::map< std::pair<std::string, std::string>, double >::const_iterator it=map_of_named_contacts.begin();it!=map_of_named_contacts.end();++it)
	{
		max_width1=std::max(max_width1, it->first.first.size());
		max_width2=std::max(max_width2, it->first.second.size());
	}
	for(std::map< std::pair<std::string, std::string>, double >::const_iterator it=map_of_named_contacts.begin();it!=map_of_named_contacts.end();++it)
	{
		bool match_first=constraints_first.empty();
		for(std::size_t i=0;!match_first && i<constraints_first.size();i++)
		{
			match_first=(it->first.first.find(constraints_first[i])!=std::string::npos);
		}
		if(match_first)
		{
			bool match_second=constraints_second.empty();
			for(std::size_t i=0;!match_second && i<constraints_second.size();i++)
			{
				match_second=(it->first.second.find(constraints_second[i])!=std::string::npos);
			}
			if(match_second)
			{
				std::cout.width(max_width1+2);
				std::cout << std::left << it->first.first;
				std::cout.width(max_width2+2);
				std::cout << std::left << it->first.second;
				std::cout.width(default_width);
				std::cout << std::left << it->second << "\n";
			}
		}
	}
	std::cout.width(default_width);
}

}

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		basic_map_of_option_descriptions["--annotate"].init("", "flag to annotate contacts using balls comments");
		basic_map_of_option_descriptions["--probe"].init("number", "probe radius");
		basic_map_of_option_descriptions["--annotation-constraints-first"].init("list", "list of substrings to match first annotations output");
		basic_map_of_option_descriptions["--annotation-constraints-second"].init("list", "list of substrings to match second annotations output");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		full_map_of_option_descriptions["--step"].init("number", "curve step length");
		full_map_of_option_descriptions["--projections"].init("number", "curve optimization depth");
		full_map_of_option_descriptions["--sih-depth"].init("number", "spherical surface optimization depth");
		full_map_of_option_descriptions["--max-dist"].init("number", "maximal distance to record, may exceed probe diameter");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls\n";
			std::cerr << "                (default line format: 'x y z r # comments')\n";
			std::cerr << "                (annotation mode line format: 'x y z r # atomSerial chainID resSeq resName atomName')\n";
			std::cerr << "  stdout  ->  list of contacts\n";
			std::cerr << "                (default line format: 'b1 b2 distance area')\n";
			std::cerr << "                (annotation mode line format: 'annotation1 annotation2 area')\n";
			std::cerr << "                  (annotation string format: '(chainID:resSeq:atomSerial)[resName:atomName]')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool print_log=poh.contains_option("--print-log");
	const bool annotate=poh.contains_option("--annotate");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double init_radius_for_BSH=std::max(1.0, poh.argument<double>("--init-radius-for-BSH", 3.5));
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const double step=std::max(0.05, std::min(0.5, poh.argument<double>("--step", 0.2)));
	const int projections=std::max(1, std::min(10, poh.argument<int>("--projections", 5)));
	const int sih_depth=std::max(1, std::min(5, poh.argument<int>("--sih-depth", 3)));
	const double max_dist=std::max(0.0, std::min(14.0*4.0, poh.argument<double>("--max-dist", probe*4.0)));
	const std::vector<std::string> annotation_constraints_first=poh.argument_vector<std::string>("--annotation-constraints-first");
	const std::vector<std::string> annotation_constraints_second=poh.argument_vector<std::string>("--annotation-constraints-second");

	std::vector<apollota::SimpleSphere> spheres;
	std::vector<Comment> input_spheres_comments;
	if(annotate)
	{
		std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* > spheres_with_comments(&spheres, &input_spheres_comments);
		auxiliaries::read_lines_to_container(std::cin, "", add_sphere_and_comments_from_stream_to_vectors, spheres_with_comments);
		if(spheres.size()!=input_spheres_comments.size())
		{
			throw std::runtime_error("Number of comments does not match number of spheres.");
		}
	}
	else
	{
		auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::ConstrainedContactsConstruction::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, std::pair<double, double> > interactions_map;
	for(apollota::TriangulationQueries::IDsMap::const_iterator it=ids_map.begin();it!=ids_map.end();++it)
	{
		const std::size_t a_id=it->first;
		if(a_id<input_spheres_count)
		{
			for(std::set<std::size_t>::const_iterator jt=it->second.begin();jt!=it->second.end();++jt)
			{
				const std::size_t b_id=(*jt);
				if(a_id<b_id && b_id<input_spheres_count)
				{
					const double dist=apollota::minimal_distance_from_sphere_to_sphere(spheres[a_id], spheres[b_id]);
					if(dist<=max_dist)
					{
						interactions_map[apollota::Pair(a_id, b_id)].first=dist;
					}
				}
			}
		}
	}

	const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections);
	for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
	{
		if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
		{
			interactions_map[it->first].second=it->second;
		}
	}

	const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth);
	for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
	{
		if(it->first<input_spheres_count)
		{
			interactions_map[apollota::Pair(it->first, it->first)]=std::make_pair(-(spheres[it->first].r*2.0), it->second);
		}
	}

	if(!input_spheres_comments.empty())
	{
		std::map< std::pair<std::string, std::string>, double > map_of_named_contacts;
		for(std::map<apollota::Pair, std::pair<double, double> >::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second.second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				if(a_id==b_id)
				{
					record_annotated_solvent_contact(input_spheres_comments[a_id], area, map_of_named_contacts);
				}
				else
				{
					record_annotated_inter_atom_contact(input_spheres_comments[a_id], input_spheres_comments[b_id], area, map_of_named_contacts);
				}
			}
		}
		print_map_of_named_contacts(map_of_named_contacts, annotation_constraints_first, annotation_constraints_second);
	}
	else
	{
		for(std::map<apollota::Pair, std::pair<double, double> >::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			std::cout << it->first.get(0) << " " << it->first.get(1) << " " << it->second.first << " " << it->second.second<< "\n";
		}
	}

	if(print_log)
	{
		std::clog << "balls " << input_spheres_count << "\n";
		std::clog << "probe " << probe << "\n";
		std::clog << "step " << step << "\n";
		std::clog << "projections " << projections << "\n";
		std::clog << "sih_depth " << sih_depth << "\n";
		std::clog << "max_dist " << max_dist << "\n";
		std::clog << "output_pairs " << interactions_map.size() << "\n";
		std::clog << "contacts_count_internal " << constrained_contacts.size() << "\n";
		std::clog << "contacts_count_external " << constrained_contact_remainders.size() << "\n";

		double contacts_sum_internal=0.0;
		for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
		{
			if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
			{
				contacts_sum_internal+=it->second;
			}
		}

		double contacts_sum_external=0.0;
		for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
		{
			if(it->first<input_spheres_count)
			{
				contacts_sum_external+=it->second;
			}
		}

		std::clog << "contacts_sum_internal " << contacts_sum_internal << "\n";
		std::clog << "contacts_sum_external " << contacts_sum_external << "\n";
	}
}
