#include <iostream>
#include <sstream>

#include "voronota_lt.h"

int main(const int argc, const char** argv)
{
	unsigned int max_number_of_processors=40;
	bool output_csa=false;
	bool output_sasa=false;

	{
		int i=1;
		while(i<argc)
		{
			const std::string name(argv[i]);
			if(name=="-processors")
			{
				bool success=false;
				if(i+1<argc)
				{
					i++;
					const std::string value(argv[i]);
					if(!value.empty())
					{
						std::istringstream input(value);
						input >> max_number_of_processors;
						if(!input.fail() && max_number_of_processors>=1 && max_number_of_processors<=1000)
						{
							success=true;
						}
					}
				}
				if(!success)
				{
					std::cerr << "Error: invalid command line argument for the maximum number of processors (-P), must be an integer from 1 to 1000.\n";
					return 1;
				}
			}
			else if(name=="-output-csa")
			{
				output_csa=true;
			}
			else if(name=="-output-sasa")
			{
				output_sasa=true;
			}
			i++;
		}
	}

	std::vector<voronotalt::SimpleSphere> spheres;
	{
		voronotalt::SimpleSphere sphere;
		std::cin >> std::ws;
		while(std::cin.good())
		{
			std::cin >> sphere.p.x;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.x at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.p.y;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.y at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.p.z;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.z at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.r;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.r at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> std::ws;
			sphere.r+=1.4;
			spheres.push_back(sphere);
		}
	}

	std::cout << "total_balls: " << spheres.size() << "\n";

	voronotalt::SpheresSearcher spheres_searcher(spheres);

	std::vector< std::vector<std::size_t> > all_colliding_ids(spheres_searcher.all_spheres().size());

	{
#pragma omp parallel for
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			for(std::size_t i=proc;i<spheres_searcher.all_spheres().size();i+=max_number_of_processors)
			{
				spheres_searcher.find_colliding_ids(i, all_colliding_ids[i]);
			}
		}
	}

	std::size_t total_collisions=0;
	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
		total_collisions+=all_colliding_ids[i].size();
	}

	std::cout << "total_collisions: " << total_collisions << "\n";

	std::vector< std::pair<std::size_t, std::size_t> > possible_pairs;
	possible_pairs.reserve(total_collisions/2);
	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
	    for(std::size_t j=0;j<all_colliding_ids[i].size();j++)
	    {
	        const std::size_t a_id=i;
	        const std::size_t b_id=all_colliding_ids[i][j];
	        if(a_id<b_id)
	        {
	        	possible_pairs.push_back(std::pair<std::size_t, std::size_t>(a_id, b_id));
	        }
	    }
	}

	std::vector<voronotalt::ConstrainedContactsConstruction::ContactDescriptorSummary> possible_pair_summaries(possible_pairs.size());

	{
		std::vector<voronotalt::ConstrainedContactsConstruction::ContactDescriptor> allocated_contact_descriptors(max_number_of_processors);
#pragma omp parallel for
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			for(std::size_t i=proc;i<possible_pairs.size();i+=max_number_of_processors)
			{
				const std::size_t a_id=possible_pairs[i].first;
				const std::size_t b_id=possible_pairs[i].second;
				if(voronotalt::ConstrainedContactsConstruction::construct_contact_descriptor(spheres_searcher.all_spheres(), a_id, b_id, all_colliding_ids[a_id], all_colliding_ids[b_id], allocated_contact_descriptors[proc]))
				{
					possible_pair_summaries[i].set(allocated_contact_descriptors[proc]);
				}
			}
		}
	}

	voronotalt::ConstrainedContactsConstruction::TotalContactDescriptorsSummary total_summary;
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		total_summary.add(possible_pair_summaries[i]);
	}

	std::vector<voronotalt::ConstrainedContactsConstruction::CellContactDescriptorsSummary> cells_summaries(spheres_searcher.all_spheres().size());
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		const voronotalt::ConstrainedContactsConstruction::ContactDescriptorSummary& pair_summary=possible_pair_summaries[i];
		if(pair_summary.valid)
		{
			cells_summaries[pair_summary.id_a].add(pair_summary.id_a, pair_summary);
			cells_summaries[pair_summary.id_b].add(pair_summary.id_b, pair_summary);
		}
	}

	double total_sasa=0.0;
	for(std::size_t i=0;i<cells_summaries.size();i++)
	{
		cells_summaries[i].compute_sas(spheres_searcher.all_spheres()[i].r);
		total_sasa+=cells_summaries[i].sas_area;
	}

	std::cout << "total_contacts_count: " << total_summary.count << "\n";
	std::cout << "total_contacts_area: " << total_summary.area << "\n";
	std::cout << "total_contacts_complexity: " << total_summary.complexity << "\n";
	std::cout << "total_sasa: " << total_sasa << "\n";

	if(output_csa)
	{
		for(std::size_t i=0;i<possible_pair_summaries.size();i++)
		{
			const voronotalt::ConstrainedContactsConstruction::ContactDescriptorSummary& pair_summary=possible_pair_summaries[i];
			if(pair_summary.valid)
			{
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << "\n";
			}
		}
	}

	if(output_sasa)
	{
		for(std::size_t i=0;i<cells_summaries.size();i++)
		{
			const voronotalt::ConstrainedContactsConstruction::CellContactDescriptorsSummary& cell_summary=cells_summaries[i];
			if(cell_summary.sas_area>0.0001)
			{
				std::cout << "sasa " << cell_summary.id << " " << cell_summary.sas_area << "\n";
			}
		}
	}

	return 1;
}

