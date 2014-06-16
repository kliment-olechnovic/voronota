#include <iostream>
#include <utility>

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

	Comment() : serial(0), resSeq(0)
	{
	}

	Comment without_atom() const
	{
		Comment v=(*this);
		v.serial=std::numeric_limits<int>::min();
		v.altLoc.clear();
		v.name.clear();
		return v;
	}

	Comment without_residue() const
	{
		Comment v=without_atom();
		v.resSeq=std::numeric_limits<int>::min();
		v.iCode.clear();
		v.resName.clear();
		return v;
	}

	bool operator==(const Comment& v) const
	{
		return (serial==v.serial && resSeq==v.resSeq && chainID==v.chainID && iCode==v.iCode && altLoc==v.altLoc && resName==v.resName && name==v.name);
	}

	bool operator<(const Comment& v) const
	{
		if(chainID<v.chainID) { return true; }
		else if(chainID==v.chainID)
		{
			if(resSeq<v.resSeq) { return true; }
			else if(resSeq==v.resSeq)
			{
				if(iCode<v.iCode) { return true; }
				else if(iCode==v.iCode)
				{
					if(serial<v.serial) { return true; }
					else if(serial==v.serial)
					{
						if(altLoc<v.altLoc) { return true; }
						else if(altLoc==v.altLoc)
						{
							if(resName<v.resName) { return true; }
							else if(resName==v.resName)
							{
								return (name<v.name);
							}
						}
					}
				}
			}
		}
		return false;
	}

	std::string str(const std::string& suffix) const
	{
		static const std::string b="=";
		static const std::string e="; ";
		const bool with_residue=(resSeq!=std::numeric_limits<int>::min());
		const bool with_residue_and_atom=(with_residue && (serial!=std::numeric_limits<int>::min()));
		std::ostringstream output;
		output << "s" << suffix << b << "c" << (with_residue ? "r" : "") << (with_residue_and_atom ? "a" : "") << e;
		output << "c" << suffix << b << chainID << e;
		if(with_residue)
		{
			output << "r" << suffix << b << resSeq << e;
			if(!iCode.empty())
			{
				output << "i" << suffix << b << iCode << e;
			}
		}
		if(with_residue_and_atom)
		{
			output << "a" << suffix << b << serial << e;
			if(!altLoc.empty())
			{
				output << "l" << suffix << b << altLoc << e;
			}
		}
		if(with_residue)
		{
			output << "R" << suffix << b << resName << e;
		}
		if(with_residue_and_atom)
		{
			output << "A" << suffix << b << name << e;
		}
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

void record_annotated_solvent_contact_area(
		const Comment& comment,
		const double area,
		std::map<Comment, double>& map_of_solvent_contact_areas)
{
	map_of_solvent_contact_areas[comment]+=area;
	map_of_solvent_contact_areas[comment.without_atom()]+=area;
	map_of_solvent_contact_areas[comment.without_residue()]+=area;
}

void record_annotated_inter_atom_contact_area(
		const Comment& comment1,
		const Comment& comment2,
		const double area,
		std::map< std::pair<Comment, Comment>, double >& map_of_inter_atom_contact_areas,
		std::map<Comment, double>& map_of_nonsolvent_contact_areas)
{
	using namespace std::rel_ops;
	if(comment1!=comment2)
	{
		const Comment residue_comment1=comment1.without_atom();
		const Comment residue_comment2=comment2.without_atom();
		if(residue_comment1!=residue_comment2)
		{
			map_of_inter_atom_contact_areas[std::make_pair(comment1, comment2)]+=area;
			map_of_inter_atom_contact_areas[std::make_pair(comment2, comment1)]+=area;
			map_of_nonsolvent_contact_areas[comment1]+=area;
			map_of_nonsolvent_contact_areas[comment2]+=area;
			map_of_inter_atom_contact_areas[std::make_pair(residue_comment1, residue_comment2)]+=area;
			map_of_inter_atom_contact_areas[std::make_pair(residue_comment2, residue_comment1)]+=area;
			map_of_nonsolvent_contact_areas[residue_comment1]+=area;
			map_of_nonsolvent_contact_areas[residue_comment2]+=area;
			const Comment chain_comment1=comment1.without_residue();
			const Comment chain_comment2=comment2.without_residue();
			if(chain_comment1!=chain_comment2)
			{
				map_of_inter_atom_contact_areas[std::make_pair(chain_comment1, chain_comment2)]+=area;
				map_of_inter_atom_contact_areas[std::make_pair(chain_comment2, chain_comment1)]+=area;
				map_of_nonsolvent_contact_areas[chain_comment1]+=area;
				map_of_nonsolvent_contact_areas[chain_comment2]+=area;
			}
		}
	}
}

void print_map_of_named_contact_areas(
		const std::map< std::pair<Comment, Comment>, double >& map_of_inter_atom_contact_areas,
		const std::map<Comment, double>& map_of_nonsolvent_contact_areas,
		const std::map<Comment, double>& map_of_solvent_contact_areas)
{
	typedef std::map< Comment, std::vector< std::pair<std::string, double> > > NamedContacts;
	NamedContacts named_contacts;
	for(std::map< std::pair<Comment, Comment>, double >::const_iterator it=map_of_inter_atom_contact_areas.begin();it!=map_of_inter_atom_contact_areas.end();++it)
	{
		named_contacts[it->first.first].push_back(std::make_pair(it->first.second.str("2"), it->second));
	}
	for(std::map<Comment, double>::const_iterator it=map_of_nonsolvent_contact_areas.begin();it!=map_of_nonsolvent_contact_areas.end();++it)
	{
		named_contacts[it->first].push_back(std::make_pair(std::string("s2=nonsolvent;"), it->second));
	}
	for(std::map<Comment, double>::const_iterator it=map_of_solvent_contact_areas.begin();it!=map_of_solvent_contact_areas.end();++it)
	{
		named_contacts[it->first].push_back(std::make_pair(std::string("s2=solvent;"), it->second));
	}

	const std::size_t default_width=std::cout.width();
	std::size_t max_width=0;
	for(NamedContacts::const_iterator it=named_contacts.begin();it!=named_contacts.end();++it)
	{
		max_width=std::max(max_width, it->first.str("1").size());
		for(std::size_t i=0;i<it->second.size();i++)
		{
			max_width=std::max(max_width, it->second[i].first.size());
		}
	}
	max_width+=4;

	for(NamedContacts::const_iterator it=named_contacts.begin();it!=named_contacts.end();++it)
	{
		const std::string str1=it->first.str("1");
		for(std::size_t i=0;i<it->second.size();i++)
		{
			const std::string str2=it->second[i].first;
			std::cout.width(max_width);
			std::cout << std::left << str1;
			std::cout.width(max_width);
			std::cout << std::left << str2;
			std::cout.width(default_width);
			std::cout << std::left << "area=" << it->second[i].second << ";\n";
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
		std::map< std::pair<Comment, Comment>, double > map_of_inter_atom_contact_areas;
		std::map< Comment, double > map_of_nonsolvent_contact_areas;
		std::map< Comment, double > map_of_solvent_contact_areas;
		for(std::map<apollota::Pair, std::pair<double, double> >::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second.second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				if(a_id==b_id)
				{
					record_annotated_solvent_contact_area(input_spheres_comments[a_id], area, map_of_solvent_contact_areas);
				}
				else
				{
					record_annotated_inter_atom_contact_area(input_spheres_comments[a_id], input_spheres_comments[b_id], area, map_of_inter_atom_contact_areas, map_of_nonsolvent_contact_areas);
				}
			}
		}
		print_map_of_named_contact_areas(map_of_inter_atom_contact_areas, map_of_nonsolvent_contact_areas, map_of_solvent_contact_areas);
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
