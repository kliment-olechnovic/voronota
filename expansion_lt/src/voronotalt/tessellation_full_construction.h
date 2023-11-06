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
				id=new_id;
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
	std::vector<bool> spheres_hidden;
	std::vector<ContactDescriptorSummary> contacts_summaries;
	std::vector<ContactDescriptorGraphics> contacts_graphics;
	std::vector<CellContactDescriptorsSummary> cells_summaries;
	TotalContactDescriptorsSummary total_contacts_summary;
	TotalCellContactDescriptorsSummary total_cells_summary;

	TessellationFullConstructionResult() : total_collisions(0)
	{
	}
};

void construct_full_tessellation(const std::vector<SimpleSphere>& spheres, const bool with_graphics, TessellationFullConstructionResult& result)
{
	result=TessellationFullConstructionResult();

	const std::size_t N=spheres.size();

	SpheresSearcher spheres_searcher(spheres);

	std::vector< std::vector<std::size_t> > all_colliding_ids(N);
	for(std::size_t i=0;i<N;i++)
	{
		all_colliding_ids[i].reserve(20);
	}

	result.spheres_hidden.resize(N, false);

	{
#pragma omp parallel for
		for(std::size_t i=0;i<N;i++)
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
			if(hidden)
			{
				result.spheres_hidden[i]=true;
				all_colliding_ids[i].clear();
			}
		}
	}

	result.total_collisions=0;
	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
		result.total_collisions+=all_colliding_ids[i].size();
	}

	std::vector< std::pair<std::size_t, std::size_t> > relevant_collision_ids;
	relevant_collision_ids.reserve(result.total_collisions/2);
	for(std::size_t id_a=0;id_a<N;id_a++)
	{
		for(std::size_t j=0;j<all_colliding_ids[id_a].size();j++)
		{
			const std::size_t id_b=all_colliding_ids[id_a][j];
			if(all_colliding_ids[id_a].size()<all_colliding_ids[id_b].size() || (all_colliding_ids[id_a].size()==all_colliding_ids[id_b].size() && id_a<id_b))
			{
				relevant_collision_ids.push_back(std::pair<std::size_t, std::size_t>(id_a, id_b));
			}
		}
	}

	std::vector<ContactDescriptorSummary> possible_contacts_summaries(relevant_collision_ids.size());

	std::vector<ContactDescriptorGraphics> possible_contacts_graphics;
	if(with_graphics)
	{
		possible_contacts_graphics.resize(possible_contacts_summaries.size());
	}

	{
#pragma omp parallel for
		for(std::size_t i=0;i<relevant_collision_ids.size();i++)
		{
			const std::size_t id_a=relevant_collision_ids[i].first;
			const std::size_t id_b=relevant_collision_ids[i].second;
			ContactDescriptor cd;
			cd.contour.reserve(12);
			cd.neighbor_descriptors.reserve(12);
			if(TessellationContactsConstruction::construct_contact_descriptor(spheres, id_a, id_b, all_colliding_ids[id_a], cd))
			{
				possible_contacts_summaries[i].set(cd);
				if(with_graphics)
				{
					TessellationContactsConstruction::construct_contact_descriptor_graphics(cd, 0.2, possible_contacts_graphics[i]);
				}
			}
		}
	}

	TotalContactDescriptorsSummary total_contacts_summary;
	for(std::size_t i=0;i<possible_contacts_summaries.size();i++)
	{
		result.total_contacts_summary.add(possible_contacts_summaries[i]);
	}

	result.contacts_summaries.reserve(total_contacts_summary.count);
	for(std::size_t i=0;i<possible_contacts_summaries.size();i++)
	{
		if(possible_contacts_summaries[i].valid)
		{
			result.contacts_summaries.push_back(possible_contacts_summaries[i]);
		}
	}

	if(with_graphics)
	{
		result.contacts_graphics.reserve(total_contacts_summary.count);
		for(std::size_t i=0;i<possible_contacts_graphics.size();i++)
		{
			if(possible_contacts_graphics[i].valid)
			{
				result.contacts_graphics.push_back(possible_contacts_graphics[i]);
			}
		}
	}

	result.cells_summaries.resize(N);
	for(std::size_t i=0;i<result.contacts_summaries.size();i++)
	{
		const ContactDescriptorSummary& cds=result.contacts_summaries[i];
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
