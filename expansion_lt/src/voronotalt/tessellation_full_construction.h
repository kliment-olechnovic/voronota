#ifndef VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_
#define VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_

#include "spheres_searcher.h"
#include "tessellation_contact_construction.h"
#include "time_recorder.h"

namespace voronotalt
{

class TessellationFullConstruction
{
public:
	struct ContactDescriptorSummary
	{
		Float area;
		Float arc_length;
		Float solid_angle_a;
		Float solid_angle_b;
		Float pyramid_volume_a;
		Float pyramid_volume_b;
		UnsignedInt id_a;
		UnsignedInt id_b;
		UnsignedInt complexity;

		ContactDescriptorSummary() :
			area(FLOATCONST(0.0)),
			arc_length(FLOATCONST(0.0)),
			solid_angle_a(FLOATCONST(0.0)),
			solid_angle_b(FLOATCONST(0.0)),
			pyramid_volume_a(FLOATCONST(0.0)),
			pyramid_volume_b(FLOATCONST(0.0)),
			id_a(0),
			id_b(0),
			complexity(0)
		{
		}

		void set(const TessellationContactConstruction::ContactDescriptor& cd)
		{
			if(cd.area>FLOATCONST(0.0))
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
			}
		}
	};

	struct CellContactDescriptorsSummary
	{
		Float area;
		Float arc_length;
		Float explained_solid_angle_positive;
		Float explained_solid_angle_negative;
		Float explained_pyramid_volume_positive;
		Float explained_pyramid_volume_negative;
		Float sas_area;
		Float sas_inside_volume;
		UnsignedInt id;
		UnsignedInt complexity;
		UnsignedInt count;
		int stage;

		CellContactDescriptorsSummary() :
			area(FLOATCONST(0.0)),
			arc_length(FLOATCONST(0.0)),
			explained_solid_angle_positive(FLOATCONST(0.0)),
			explained_solid_angle_negative(FLOATCONST(0.0)),
			explained_pyramid_volume_positive(FLOATCONST(0.0)),
			explained_pyramid_volume_negative(FLOATCONST(0.0)),
			sas_area(FLOATCONST(0.0)),
			sas_inside_volume(FLOATCONST(0.0)),
			id(0),
			complexity(0),
			count(0),
			stage(0)
		{
		}

		void add(const ContactDescriptorSummary& cds)
		{
			if(cds.area>FLOATCONST(0.0) && (cds.id_a==id || cds.id_b==id))
			{
				count++;
				area+=cds.area;
				arc_length+=cds.arc_length;
				complexity+=cds.complexity;
				explained_solid_angle_positive+=std::max(FLOATCONST(0.0), (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
				explained_solid_angle_negative+=FLOATCONST(0.0)-std::min(FLOATCONST(0.0), (cds.id_a==id ? cds.solid_angle_a : cds.solid_angle_b));
				explained_pyramid_volume_positive+=std::max(FLOATCONST(0.0), (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
				explained_pyramid_volume_negative+=FLOATCONST(0.0)-std::min(FLOATCONST(0.0), (cds.id_a==id ? cds.pyramid_volume_a : cds.pyramid_volume_b));
				stage=1;
			}
		}

		void add(const UnsignedInt new_id, const ContactDescriptorSummary& cds)
		{
			if(cds.area>FLOATCONST(0.0))
			{
				if(stage==0)
				{
					id=new_id;
				}
				add(cds);
			}
		}

		void compute_sas(const Float r)
		{
			if(stage!=0)
			{
				sas_area=FLOATCONST(0.0);
				sas_inside_volume=FLOATCONST(0.0);
				if(arc_length>FLOATCONST(0.0))
				{
					if(explained_solid_angle_positive>explained_solid_angle_negative)
					{
						sas_area=((FLOATCONST(4.0)*PIVALUE)-std::max(FLOATCONST(0.0), explained_solid_angle_positive-explained_solid_angle_negative))*(r*r);
					}
					else if(explained_solid_angle_negative>explained_solid_angle_positive)
					{
						sas_area=(std::max(FLOATCONST(0.0), explained_solid_angle_negative-explained_solid_angle_positive))*(r*r);
					}
					sas_inside_volume=(sas_area*r/FLOATCONST(3.0))+explained_pyramid_volume_positive-explained_pyramid_volume_negative;
				}
				else
				{
					sas_inside_volume=explained_pyramid_volume_positive-explained_pyramid_volume_negative;
				}
				stage=2;
			}
		}
	};

	struct TotalContactDescriptorsSummary
	{
		Float area;
		Float arc_length;
		UnsignedInt complexity;
		UnsignedInt count;

		TotalContactDescriptorsSummary() :
			area(FLOATCONST(0.0)),
			arc_length(FLOATCONST(0.0)),
			complexity(0),
			count(0)
		{
		}

		void add(const ContactDescriptorSummary& cds)
		{
			if(cds.area>FLOATCONST(0.0))
			{
				count++;
				area+=cds.area;
				arc_length+=cds.arc_length;
				complexity+=cds.complexity;
			}
		}
	};

	struct TotalCellContactDescriptorsSummary
	{
		Float sas_area;
		Float sas_inside_volume;
		UnsignedInt count;

		TotalCellContactDescriptorsSummary() :
			sas_area(FLOATCONST(0.0)),
			sas_inside_volume(FLOATCONST(0.0)),
			count(0)
		{
		}

		void add(const CellContactDescriptorsSummary& ccds)
		{
			if(ccds.stage==2)
			{
				count++;
				sas_area+=ccds.sas_area;
				sas_inside_volume+=ccds.sas_inside_volume;
			}
		}
	};

	struct Result
	{
		TotalContactDescriptorsSummary total_contacts_summary;
		TotalCellContactDescriptorsSummary total_cells_summary;
		std::vector<ContactDescriptorSummary> contacts_summaries;
		std::vector<TessellationContactConstruction::ContactDescriptorGraphics> contacts_graphics;
		std::vector<CellContactDescriptorsSummary> cells_summaries;
		UnsignedInt total_collisions;

		Result() : total_collisions(0)
		{
		}
	};

	static void construct_full_tessellation(const std::vector<SimpleSphere>& spheres, const bool with_graphics, Result& result, TimeRecorder& time_recorder)
	{
		result=Result();

		const UnsignedInt N=spheres.size();

		time_recorder.reset();

		SpheresSearcher spheres_searcher(spheres);

		time_recorder.record_elapsed_miliseconds_and_reset("init spheres searcher");

		std::vector< std::vector<UnsignedInt> > all_colliding_ids(N);
		for(UnsignedInt i=0;i<N;i++)
		{
			all_colliding_ids[i].reserve(100);
		}

		time_recorder.record_elapsed_miliseconds_and_reset("pre-allocate colliding IDs");

		#pragma omp parallel
		{
			std::vector<UnsignedInt> colliding_ids;
			colliding_ids.reserve(100);

			#pragma omp for
			for(UnsignedInt i=0;i<N;i++)
			{
				spheres_searcher.find_colliding_ids(i, colliding_ids, true);
				if(!colliding_ids.empty())
				{
					all_colliding_ids[i]=colliding_ids;
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("detect all collisions");

		result.total_collisions=0;
		for(UnsignedInt i=0;i<all_colliding_ids.size();i++)
		{
			result.total_collisions+=all_colliding_ids[i].size();
		}

		time_recorder.record_elapsed_miliseconds_and_reset("count all collisions");

		std::vector< std::pair<UnsignedInt, UnsignedInt> > relevant_collision_ids;
		relevant_collision_ids.reserve(result.total_collisions/2);
		for(UnsignedInt id_a=0;id_a<N;id_a++)
		{
			for(UnsignedInt j=0;j<all_colliding_ids[id_a].size();j++)
			{
				const UnsignedInt id_b=all_colliding_ids[id_a][j];
				if(all_colliding_ids[id_a].size()<all_colliding_ids[id_b].size() || (id_a<id_b && all_colliding_ids[id_a].size()==all_colliding_ids[id_b].size()))
				{
					relevant_collision_ids.push_back(std::pair<UnsignedInt, UnsignedInt>(id_a, id_b));
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("collect relevant collision indices");

		std::vector<ContactDescriptorSummary> possible_contacts_summaries(relevant_collision_ids.size());

		std::vector<TessellationContactConstruction::ContactDescriptorGraphics> possible_contacts_graphics;
		if(with_graphics)
		{
			possible_contacts_graphics.resize(possible_contacts_summaries.size());
		}

		time_recorder.record_elapsed_miliseconds_and_reset("allocate possible contact summaries");

		#pragma omp parallel
		{
			TessellationContactConstruction::ContactDescriptor cd;
			cd.contour.reserve(12);
			cd.neighbor_descriptors.reserve(24);

			#pragma omp for
			for(UnsignedInt i=0;i<relevant_collision_ids.size();i++)
			{
				const UnsignedInt id_a=relevant_collision_ids[i].first;
				const UnsignedInt id_b=relevant_collision_ids[i].second;
				if(TessellationContactConstruction::construct_contact_descriptor(spheres, id_a, id_b, all_colliding_ids[id_a], cd))
				{
					possible_contacts_summaries[i].set(cd);
					if(with_graphics)
					{
						TessellationContactConstruction::construct_contact_descriptor_graphics(cd, 0.2, possible_contacts_graphics[i]);
					}
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("construct contacts");

		UnsignedInt number_of_valid_contact_summaries=0;
		for(UnsignedInt i=0;i<possible_contacts_summaries.size();i++)
		{
			if(possible_contacts_summaries[i].area>FLOATCONST(0.0))
			{
				number_of_valid_contact_summaries++;
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("count valid contact summaries");

		std::vector<UnsignedInt> ids_of_valid_pairs;
		ids_of_valid_pairs.reserve(number_of_valid_contact_summaries);

		for(UnsignedInt i=0;i<possible_contacts_summaries.size();i++)
		{
			if(possible_contacts_summaries[i].area>FLOATCONST(0.0))
			{
				ids_of_valid_pairs.push_back(i);
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("collect indices of valid contact summaries");

		result.contacts_summaries.resize(ids_of_valid_pairs.size());

		#pragma omp parallel
		{
			#pragma omp for
			for(UnsignedInt i=0;i<ids_of_valid_pairs.size();i++)
			{
				result.contacts_summaries[i]=possible_contacts_summaries[ids_of_valid_pairs[i]];
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("copy valid contact summaries");

		for(UnsignedInt i=0;i<result.contacts_summaries.size();i++)
		{
			result.total_contacts_summary.add(result.contacts_summaries[i]);
		}

		time_recorder.record_elapsed_miliseconds_and_reset("accumulate total contacts summary");

		if(with_graphics)
		{
			result.contacts_graphics.resize(ids_of_valid_pairs.size());

			for(UnsignedInt i=0;i<ids_of_valid_pairs.size();i++)
			{
				result.contacts_graphics[i]=possible_contacts_graphics[ids_of_valid_pairs[i]];
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("copy valid contacts graphics");

		result.cells_summaries.resize(N);

		for(UnsignedInt i=0;i<result.contacts_summaries.size();i++)
		{
			const ContactDescriptorSummary& cds=result.contacts_summaries[i];
			result.cells_summaries[cds.id_a].add(cds.id_a, cds);
			result.cells_summaries[cds.id_b].add(cds.id_b, cds);
		}

		time_recorder.record_elapsed_miliseconds_and_reset("accumulate cell summaries");

		for(UnsignedInt i=0;i<result.cells_summaries.size();i++)
		{
			result.cells_summaries[i].compute_sas(spheres[i].r);
		}

		time_recorder.record_elapsed_miliseconds_and_reset("compute sas for cell summaries");

		for(UnsignedInt i=0;i<result.cells_summaries.size();i++)
		{
			result.total_cells_summary.add(result.cells_summaries[i]);
		}

		time_recorder.record_elapsed_miliseconds_and_reset("accumulate total cells summary");
	}
};

}

#endif /* VORONOTALT_TESSELLATION_FULL_CONSTRUCTION_H_ */