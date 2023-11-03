#include <iostream>
#include <sstream>

#include "voronota_lt.h"

int main(const int argc, const char** argv)
{
	unsigned int max_number_of_processors=40;
	double probe=1.4;
	bool output_csa=false;
	bool output_csa_with_graphics=false;
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
					std::cerr << "Error: invalid command line argument for the maximum number of processors (-processors), must be an integer from 1 to 1000.\n";
					return 1;
				}
			}
			else if(name=="-probe")
			{
				bool success=false;
				if(i+1<argc)
				{
					i++;
					const std::string value(argv[i]);
					if(!value.empty())
					{
						std::istringstream input(value);
						input >> probe;
						if(!input.fail() && probe>0.01 && probe<=30.0)
						{
							success=true;
						}
					}
				}
				if(!success)
				{
					std::cerr << "Error: invalid command line argument for the rolling probe radius (-probe), must be a value from 0.01 to 30.0.\n";
					return 1;
				}
			}
			else if(name=="-output-csa")
			{
				output_csa=true;
			}
			else if(name=="-output-csa-with-graphics")
			{
				output_csa=true;
				output_csa_with_graphics=true;
			}
			else if(name=="-output-sasa")
			{
				output_sasa=true;
			}
			else
			{
				std::cerr << "Error: invalid command line argument '" << name << "'\n";
				return 1;
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
			sphere.r+=probe;
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

	std::vector<voronotalt::ConstrainedContactsConstruction::ContactDescriptorsGraphics> possible_pair_graphics;
	if(output_csa_with_graphics)
	{
		possible_pair_graphics.resize(possible_pair_summaries.size());
	}

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
					if(output_csa_with_graphics)
					{
						voronotalt::ConstrainedContactsConstruction::construct_contact_descriptor_graphics(allocated_contact_descriptors[proc], 0.2, possible_pair_graphics[i]);
					}
				}
			}
		}
	}

	voronotalt::ConstrainedContactsConstruction::TotalContactDescriptorsSummary total_contacts_summary;
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		total_contacts_summary.add(possible_pair_summaries[i]);
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

	voronotalt::ConstrainedContactsConstruction::TotalCellContactDescriptorsSummary total_cells_summary;
	for(std::size_t i=0;i<cells_summaries.size();i++)
	{
		cells_summaries[i].compute_sas(spheres_searcher.all_spheres()[i].r);
		total_cells_summary.add(cells_summaries[i]);
	}

	std::cout << "total_contacts_count: " << total_contacts_summary.count << "\n";
	std::cout << "total_contacts_area: " << total_contacts_summary.area << "\n";
	std::cout << "total_contacts_complexity: " << total_contacts_summary.complexity << "\n";
	std::cout << "total_cells_count: " << total_cells_summary.count << "\n";
	std::cout << "total_cells_sas_area: " << total_cells_summary.sas_area << "\n";
	std::cout << "total_cells_sas_inside_volume: " << total_cells_summary.sas_inside_volume << "\n";

	if(output_csa)
	{
		for(std::size_t i=0;i<possible_pair_summaries.size();i++)
		{
			const voronotalt::ConstrainedContactsConstruction::ContactDescriptorSummary& pair_summary=possible_pair_summaries[i];
			if(pair_summary.valid)
			{
				std::cout << "csa " << pair_summary.id_a << " " <<  pair_summary.id_b << " " << pair_summary.area << " " << pair_summary.solid_angle_a << " " << pair_summary.solid_angle_b;
				if(output_csa_with_graphics)
				{
					const voronotalt::ConstrainedContactsConstruction::ContactDescriptorsGraphics& pair_graphics=possible_pair_graphics[i];
					std::cout << " BEGIN,TRIANGLE_FAN";
					if(pair_graphics.valid && !pair_graphics.outer_points.empty())
					{
						std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
						std::cout << ",VERTEX," << pair_graphics.barycenter.x << "," << pair_graphics.barycenter.y << "," << pair_graphics.barycenter.z;
						for(std::size_t j=0;j<pair_graphics.outer_points.size();j++)
						{
							std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
							std::cout << ",VERTEX," << pair_graphics.outer_points[j].x << "," << pair_graphics.outer_points[j].y << "," << pair_graphics.outer_points[j].z;
						}
						std::cout << ",NORMAL," << pair_graphics.normal.x << "," << pair_graphics.normal.y << "," << pair_graphics.normal.z;
						std::cout << ",VERTEX," << pair_graphics.outer_points[0].x << "," << pair_graphics.outer_points[0].y << "," << pair_graphics.outer_points[0].z;
					}
					std::cout << ",END";
				}
				std::cout << "\n";
			}
		}
	}

	if(output_sasa)
	{
		for(std::size_t i=0;i<cells_summaries.size();i++)
		{
			const voronotalt::ConstrainedContactsConstruction::CellContactDescriptorsSummary& cell_summary=cells_summaries[i];
			if(cell_summary.sas_computed)
			{
				std::cout << "sasa " << cell_summary.id << " " << cell_summary.sas_area << " " << cell_summary.sas_inside_volume << "\n";
			}
		}
	}

	return 1;
}

