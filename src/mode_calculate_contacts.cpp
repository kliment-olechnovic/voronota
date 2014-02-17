#include <iostream>

#include "apollota/constrained_contacts_construction.h"

#include "modes_commons.h"

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		basic_map_of_option_descriptions["--probe"].init("number", "probe radius");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--exclude-hidden-balls"].init("", "flag to exclude hidden input balls");
		full_map_of_option_descriptions["--include-surplus-quadruples"].init("", "flag to include surplus quadruples");
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		full_map_of_option_descriptions["--step"].init("number", "curve step length");
		full_map_of_option_descriptions["--projections"].init("number", "curve optimization depth");
		full_map_of_option_descriptions["--sih-depth"].init("number", "spherical surface optimization depth");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r # comments')\n";
			std::cerr << "  stdout  ->  list of contacts (line format: 'b1 b2 a')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const bool print_log=poh.contains_option("--print-log");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const bool include_surplus_quadruples=poh.contains_option("--include-redundant-quadruples");
	const double init_radius_for_BSH=std::max(1.0, poh.argument<double>("--init-radius-for-BSH", 3.5));
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const double step=std::max(0.05, std::min(0.5, poh.argument<double>("--step", 0.2)));
	const int projections=std::max(1, std::min(10, poh.argument<int>("--projections", 5)));
	const int sih_depth=std::max(1, std::min(5, poh.argument<int>("--sih-depth", 3)));

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::ConstrainedContactsConstruction::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, init_radius_for_BSH, exclude_hidden_balls, include_surplus_quadruples);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections);
	const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth);

	for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
	{
		constrained_contacts[apollota::Pair(it->first, it->first)]=it->second;
	}

	for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
	{
		if(it->first.get(0)<input_spheres_count)
		{
			std::cout << it->first.get(0) << " " << it->first.get(1) << " " << it->second << "\n";
		}
	}

	if(print_log)
	{
		std::clog << "balls " << input_spheres_count << "\n";
		std::clog << "probe " << probe << "\n";
		std::clog << "step " << step << "\n";
		std::clog << "projections " << projections << "\n";
		std::clog << "sih_depth " << sih_depth << "\n";
		std::clog << "contacts_count_all " << constrained_contacts.size() << "\n";
		std::clog << "contacts_count_internal " << (constrained_contacts.size()-constrained_contact_remainders.size()) << "\n";
		std::clog << "contacts_count_external " << constrained_contact_remainders.size() << "\n";

		double contacts_sum_internal=0.0;
		double contacts_sum_external=0.0;
		for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
		{
			if(it->first.get(0)<input_spheres_count)
			{
				if(it->first.get(0)!=it->first.get(1))
				{
					contacts_sum_internal+=it->second;
				}
				else
				{
					contacts_sum_external+=it->second;
				}
			}
		}

		std::clog << "contacts_sum_all " << (contacts_sum_internal+contacts_sum_external) << "\n";
		std::clog << "contacts_sum_internal " << contacts_sum_internal << "\n";
		std::clog << "contacts_sum_external " << contacts_sum_external << "\n";
	}
}
