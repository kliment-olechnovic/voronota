#ifndef VORONOTALT_SIMPLIFIED_AW_TESSELLATION_FULL_CONSTRUCTION_H_
#define VORONOTALT_SIMPLIFIED_AW_TESSELLATION_FULL_CONSTRUCTION_H_

#include "spheres_searcher.h"
#include "simplified_aw_tessellation_contact_construction.h"
#include "time_recorder.h"

namespace voronotalt
{

class SimplifiedAWTessellationFullConstruction
{
public:
	struct ContactDescriptorSummary
	{
		std::vector<SimplifiedAWTessellationContactConstruction::ContourGraphics> graphics;
		Float area;
		UnsignedInt id_a;
		UnsignedInt id_b;

		ContactDescriptorSummary() :
			area(FLOATCONST(0.0)),
			id_a(0),
			id_b(0)
		{
		}

		void set(const SimplifiedAWTessellationContactConstruction::ContactDescriptor& cd, const bool with_graphics)
		{
			if(cd.area>FLOATCONST(0.0))
			{
				id_a=cd.id_a;
				id_b=cd.id_b;
				area=cd.area;
				if(with_graphics)
				{
					graphics=cd.graphics;
				}
			}
		}
	};

	struct TotalContactDescriptorsSummary
	{
		Float area;
		UnsignedInt count;

		TotalContactDescriptorsSummary() :
			area(FLOATCONST(0.0)),
			count(0)
		{
		}

		void add(const ContactDescriptorSummary& cds)
		{
			if(cds.area>FLOATCONST(0.0))
			{
				count++;
				area+=cds.area;
			}
		}
	};

	struct Result
	{
		TotalContactDescriptorsSummary total_contacts_summary;
		std::vector<ContactDescriptorSummary> contacts_summaries;
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

		time_recorder.record_elapsed_miliseconds_and_reset("allocate possible contact summaries");

		#pragma omp parallel
		{
			SimplifiedAWTessellationContactConstruction::ContactDescriptor cd;
			cd.neighbor_descriptors.reserve(24);

			#pragma omp for
			for(UnsignedInt i=0;i<relevant_collision_ids.size();i++)
			{
				const UnsignedInt id_a=relevant_collision_ids[i].first;
				const UnsignedInt id_b=relevant_collision_ids[i].second;
				if(SimplifiedAWTessellationContactConstruction::construct_contact_descriptor(spheres, id_a, id_b, all_colliding_ids[id_a], 0.2, 5, cd))
				{
					possible_contacts_summaries[i].set(cd, with_graphics);
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
	}
};

}

#endif /* VORONOTALT_SIMPLIFIED_AW_TESSELLATION_FULL_CONSTRUCTION_H_ */
