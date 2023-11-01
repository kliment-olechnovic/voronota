#include <iostream>

#include "voronota_lt.h"

int main(const int /*argc*/, const char** /*argv*/)
{
	const unsigned int max_number_of_processors=40;

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

	return 1;
}

