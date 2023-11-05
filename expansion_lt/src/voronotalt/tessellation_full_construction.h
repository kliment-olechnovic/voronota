#ifndef VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_
#define VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_

#include "spheres_searcher.h"
#include "tessellation_contact_construction.h"

namespace voronotalt
{

struct ContactDescriptorSummary
{
	std::size_t id_a;
	std::size_t id_b;
	double area;
	double arc_length;
	int complexity;
	double solid_angle_a;
	double solid_angle_b;
	double pyramid_volume_a;
	double pyramid_volume_b;
	bool valid;

	ContactDescriptorSummary() : id_a(0), id_b(0), area(0.0), arc_length(0.0), complexity(0), solid_angle_a(0.0), solid_angle_b(0.0), pyramid_volume_a(0.0), pyramid_volume_b(0.0), valid(false)
	{
	}

	void set(const ContactDescriptor& cd)
	{
		if(cd.valid)
		{
			id_a=cd.id_a;
			id_b=cd.id_b;
			area=cd.area;
			arc_length=(cd.sum_of_arc_angles*cd.intersection_circle_sphere.r);
			complexity=cd.contour.size();
			solid_angle_a=cd.solid_angle_a;
			solid_angle_b=cd.solid_angle_b;
			pyramid_volume_a=cd.pyramid_volume_a;
			pyramid_volume_b=cd.pyramid_volume_b;
			valid=true;
		}
	}
};

struct CellContactDescriptorsSummary
{
	std::size_t id;
	int count;
	double area;
	double arc_length;
	int complexity;
	double explained_solid_angle_positive;
	double explained_solid_angle_negative;
	double explained_pyramid_volume_positive;
	double explained_pyramid_volume_negative;
	bool valid;
	double sas_r;
	double sas_area;
	double sas_inside_volume;
	bool sas_computed;

	CellContactDescriptorsSummary() : id(0), count(0), area(0.0), arc_length(0.0), complexity(0), explained_solid_angle_positive(0.0), explained_solid_angle_negative(0.0), explained_pyramid_volume_positive(0.0), explained_pyramid_volume_negative(0.0), valid(false), sas_r(0.0), sas_area(0.0), sas_inside_volume(0.0), sas_computed(false)
	{
	}

	void clear(const std::size_t new_id)
	{
		id=new_id;
		count=0;
		area=0.0;
		arc_length=0.0;
		complexity=0;
		explained_solid_angle_positive=0.0;
		explained_solid_angle_negative=0.0;
		explained_pyramid_volume_positive=0.0;
		explained_pyramid_volume_negative=0.0;
		valid=false;
		sas_r=0.0;
		sas_area=0.0;
		sas_inside_volume=0.0;
		sas_computed=false;
	}

	void add(const ContactDescriptorSummary& cds)
	{
		if(cds.valid && (cds.id_a==id || cds.id_b==id))
		{
			count++;
			area+=cds.area;
			arc_length+=cds.arc_length;
			complexity+=cds.complexity;
			explained_solid_angle_positive+=std::max(0.0, (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
			explained_solid_angle_negative+=0.0-std::min(0.0, (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
			explained_pyramid_volume_positive+=std::max(0.0, (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
			explained_pyramid_volume_negative+=0.0-std::min(0.0, (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
			valid=true;
		}
	}

	void add(const std::size_t new_id, const ContactDescriptorSummary& cds)
	{
		if(cds.valid)
		{
			if(!valid)
			{
				clear(new_id);
			}
			add(cds);
		}
	}

	void compute_sas(const double r)
	{
		if(valid)
		{
			sas_r=r;
			sas_area=0.0;
			sas_inside_volume=0.0;
			if(arc_length>0.0)
			{
				if(explained_solid_angle_positive>explained_solid_angle_negative)
				{
					sas_area=((4.0*pi_value())-std::max(0.0, explained_solid_angle_positive-explained_solid_angle_negative))*(r*r);
				}
				else if(explained_solid_angle_negative>explained_solid_angle_positive)
				{
					sas_area=(std::max(0.0, explained_solid_angle_negative-explained_solid_angle_positive))*(r*r);
				}
				sas_inside_volume=(sas_area*r/3.0)+explained_pyramid_volume_positive-explained_pyramid_volume_negative;
			}
			else
			{
				sas_inside_volume=explained_pyramid_volume_positive-explained_pyramid_volume_negative;
			}
			sas_computed=true;
		}
	}
};

struct TotalContactDescriptorsSummary
{
	int count;
	double area;
	double arc_length;
	int complexity;
	bool valid;

	TotalContactDescriptorsSummary() : count(0), area(0.0), arc_length(0.0), complexity(0), valid(false)
	{
	}

	void clear()
	{
		count=0;
		area=0.0;
		arc_length=0.0;
		complexity=0;
		valid=false;
	}

	void add(const ContactDescriptorSummary& cds)
	{
		if(cds.valid)
		{
			count++;
			area+=cds.area;
			arc_length+=cds.arc_length;
			complexity+=cds.complexity;
			valid=true;
		}
	}
};

struct TotalCellContactDescriptorsSummary
{
	int count;
	double sas_area;
	double sas_inside_volume;
	bool valid;

	TotalCellContactDescriptorsSummary() : count(0), sas_area(0.0), sas_inside_volume(0.0), valid(false)
	{
	}

	void clear()
	{
		count=0;
		sas_area=0.0;
		sas_inside_volume=0.0;
		valid=false;
	}

	void add(const CellContactDescriptorsSummary& ccds)
	{
		if(ccds.sas_computed)
		{
			count++;
			sas_area+=ccds.sas_area;
			sas_inside_volume+=ccds.sas_inside_volume;
			valid=true;
		}
	}
};

struct TessellationFullConstructionResult
{
	std::size_t total_collisions;
	std::vector<bool> hidden_statuses;
	std::vector< std::vector<SpheresCollisionDescriptor> > all_collision_descriptors;
	std::vector<ContactDescriptorSummary> contacts_descriptors_summaries;
	std::vector<ContactDescriptorsGraphics> contacts_descriptors_graphics;
	std::vector<CellContactDescriptorsSummary> cells_summaries;
	TotalContactDescriptorsSummary total_contacts_summary;
	TotalCellContactDescriptorsSummary total_cells_summary;

	TessellationFullConstructionResult() : total_collisions(0)
	{
	}
};

void construct_full_tessellation(const unsigned int max_number_of_processors, const std::vector<SimpleSphere>& spheres, const bool with_graphics, TessellationFullConstructionResult& result)
{
	result=TessellationFullConstructionResult();

	const std::size_t N=spheres.size();

	SpheresSearcher spheres_searcher(spheres);

	std::vector< std::vector<std::size_t> > all_colliding_ids(N);

	result.hidden_statuses.resize(N, false);

	{
#pragma omp parallel for
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			for(std::size_t i=proc;i<N;i+=max_number_of_processors)
			{
				spheres_searcher.find_colliding_ids(i, all_colliding_ids[i]);
				bool hidden=false;
				for(std::size_t j=0;j<all_colliding_ids[i].size() && !hidden;j++)
				{
					if(sphere_contains_sphere(spheres[all_colliding_ids[i][j]], spheres[i]))
					{
						hidden=true;
					}
				}
				result.hidden_statuses[i]=hidden;
			}
		}
	}

	result.all_collision_descriptors.resize(N);

	{
#pragma omp parallel for
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			for(std::size_t i=proc;i<N;i+=max_number_of_processors)
			{
				if(!result.hidden_statuses[i])
				{
					result.all_collision_descriptors[i].reserve(all_colliding_ids[i].size());
					for(std::size_t j=0;j<all_colliding_ids[i].size();j++)
					{
						if(!result.hidden_statuses[all_colliding_ids[i][j]])
						{
							SpheresCollisionDescriptor scd;
							if(scd.set(spheres, i, all_colliding_ids[i][j]))
							{
								result.all_collision_descriptors[i].push_back(scd);
							}
						}
					}
				}
			}
		}
	}

	result.total_collisions=0;
	for(std::size_t i=0;i<result.all_collision_descriptors.size();i++)
	{
		result.total_collisions+=result.all_collision_descriptors[i].size();
	}

	std::vector< std::pair<std::size_t, std::size_t> > possible_pairs;
	possible_pairs.reserve(result.total_collisions/2);
	for(std::size_t i=0;i<result.all_collision_descriptors.size();i++)
	{
	    for(std::size_t j=0;j<result.all_collision_descriptors[i].size();j++)
	    {
	    	const SpheresCollisionDescriptor& scd=result.all_collision_descriptors[i][j];
	        if(result.all_collision_descriptors[scd.id_a].size()<result.all_collision_descriptors[scd.id_b].size() || (result.all_collision_descriptors[scd.id_a].size()==result.all_collision_descriptors[scd.id_b].size() && scd.id_a<scd.id_b))
	        {
	        	possible_pairs.push_back(std::pair<std::size_t, std::size_t>(i, j));
	        }
	    }
	}

	std::vector<ContactDescriptorSummary> possible_pair_summaries(possible_pairs.size());

	std::vector<ContactDescriptorsGraphics> possible_pair_graphics;
	if(with_graphics)
	{
		possible_pair_graphics.resize(possible_pair_summaries.size());
	}

	{
		std::vector<ContactDescriptor> allocated_contact_descriptors(max_number_of_processors);
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			allocated_contact_descriptors[proc].neighbor_descriptors.reserve(100);
			allocated_contact_descriptors[proc].contour.reserve(50);
		}

#pragma omp parallel for
		for(unsigned int proc=0;proc<max_number_of_processors;proc++)
		{
			for(std::size_t i=proc;i<possible_pairs.size();i+=max_number_of_processors)
			{
				const SpheresCollisionDescriptor& scd=result.all_collision_descriptors[possible_pairs[i].first][possible_pairs[i].second];
				if(TessellationContactsConstruction::construct_contact_descriptor(spheres, scd, result.all_collision_descriptors[scd.id_a], allocated_contact_descriptors[proc]))
				{
					possible_pair_summaries[i].set(allocated_contact_descriptors[proc]);
					if(with_graphics)
					{
						TessellationContactsConstruction::construct_contact_descriptor_graphics(allocated_contact_descriptors[proc], 0.2, possible_pair_graphics[i]);
					}
				}
			}
		}
	}

	TotalContactDescriptorsSummary total_contacts_summary;
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		result.total_contacts_summary.add(possible_pair_summaries[i]);
	}

	result.contacts_descriptors_summaries.reserve(total_contacts_summary.count);
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		if(possible_pair_summaries[i].valid)
		{
			result.contacts_descriptors_summaries.push_back(possible_pair_summaries[i]);
		}
	}

	if(with_graphics)
	{
		result.contacts_descriptors_graphics.reserve(total_contacts_summary.count);
		for(std::size_t i=0;i<possible_pair_graphics.size();i++)
		{
			if(possible_pair_graphics[i].valid)
			{
				result.contacts_descriptors_graphics.push_back(possible_pair_graphics[i]);
			}
		}
	}

	result.cells_summaries.resize(N);
	for(std::size_t i=0;i<result.contacts_descriptors_summaries.size();i++)
	{
		const ContactDescriptorSummary& cds=result.contacts_descriptors_summaries[i];
		result.cells_summaries[cds.id_a].add(cds.id_a, cds);
		result.cells_summaries[cds.id_b].add(cds.id_b, cds);
	}

	for(std::size_t i=0;i<result.cells_summaries.size();i++)
	{
		result.cells_summaries[i].compute_sas(spheres[i].r);
		result.total_cells_summary.add(result.cells_summaries[i]);
	}
}

}

#endif /* VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_ */
