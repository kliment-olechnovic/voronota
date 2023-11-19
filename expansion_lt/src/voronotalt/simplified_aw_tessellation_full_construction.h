#ifndef VORONOTALT_SIMPLIFIED_AW_TESSELLATION_FULL_CONSTRUCTION_H_
#define VORONOTALT_SIMPLIFIED_AW_TESSELLATION_FULL_CONSTRUCTION_H_

#include "preparation_for_tessellation.h"
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
		UnsignedInt total_spheres;
		UnsignedInt total_collisions;
		UnsignedInt total_relevant_collisions;
		std::vector<ContactDescriptorSummary> contacts_summaries;
		TotalContactDescriptorsSummary total_contacts_summary;

		Result() : total_spheres(0), total_collisions(0), total_relevant_collisions(0)
		{
		}
	};

	static void construct_full_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const std::vector<int>& grouping_of_spheres,
			const bool with_graphics,
			Result& result,
			TimeRecorder& time_recorder)
	{
		PreparationForTessellation::Result preparation_result;
		PreparationForTessellation::prepare_for_tessellation(spheres, grouping_of_spheres, preparation_result, time_recorder);
		construct_full_tessellation(spheres, preparation_result, with_graphics, result, time_recorder);
	}

	static void construct_full_tessellation(
			const std::vector<SimpleSphere>& spheres,
			const PreparationForTessellation::Result& preparation_result,
			const bool with_graphics,
			Result& result,
			TimeRecorder& time_recorder)
	{
		time_recorder.reset();

		result=Result();

		result.total_spheres=preparation_result.total_spheres;
		result.total_collisions=preparation_result.total_collisions;
		result.total_relevant_collisions=preparation_result.relevant_collision_ids.size();

		std::vector<ContactDescriptorSummary> possible_contacts_summaries(preparation_result.relevant_collision_ids.size());

		time_recorder.record_elapsed_miliseconds_and_reset("allocate possible contact summaries");

		#pragma omp parallel
		{
			SimplifiedAWTessellationContactConstruction::ContactDescriptor cd;
			cd.neighbor_descriptors.reserve(24);

			#pragma omp for
			for(UnsignedInt i=0;i<preparation_result.relevant_collision_ids.size();i++)
			{
				const UnsignedInt id_a=preparation_result.relevant_collision_ids[i].first;
				const UnsignedInt id_b=preparation_result.relevant_collision_ids[i].second;
				if(SimplifiedAWTessellationContactConstruction::construct_contact_descriptor(spheres, id_a, id_b, preparation_result.all_colliding_ids[id_a], 0.2, 5, with_graphics, cd))
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
