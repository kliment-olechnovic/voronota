#ifndef VORONOTALT_PREPARATION_FOR_TESSELLATION_H_
#define VORONOTALT_PREPARATION_FOR_TESSELLATION_H_

#include "spheres_searcher.h"
#include "time_recorder.h"

namespace voronotalt
{

class PreparationForTessellation
{
public:
	struct Result
	{
		std::vector< std::vector<UnsignedInt> > all_colliding_ids;
		std::vector< std::pair<UnsignedInt, UnsignedInt> > relevant_collision_ids;
		UnsignedInt total_spheres;
		UnsignedInt total_collisions;

		Result() : total_spheres(0), total_collisions(0)
		{
		}
	};

	static void prepare_for_tessellation(const std::vector<SimpleSphere>& spheres, Result& result, TimeRecorder& time_recorder)
	{
		result=Result();

		const UnsignedInt N=spheres.size();
		result.total_spheres=N;

		time_recorder.reset();

		SpheresSearcher spheres_searcher(spheres);

		time_recorder.record_elapsed_miliseconds_and_reset("init spheres searcher");

		result.all_colliding_ids.resize(N);
		for(UnsignedInt i=0;i<N;i++)
		{
			result.all_colliding_ids[i].reserve(100);
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
					result.all_colliding_ids[i]=colliding_ids;
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("detect all collisions");

		for(UnsignedInt i=0;i<result.all_colliding_ids.size();i++)
		{
			result.total_collisions+=result.all_colliding_ids[i].size();
		}

		time_recorder.record_elapsed_miliseconds_and_reset("count all collisions");

		result.relevant_collision_ids.reserve(result.total_collisions/2);
		for(UnsignedInt id_a=0;id_a<N;id_a++)
		{
			for(UnsignedInt j=0;j<result.all_colliding_ids[id_a].size();j++)
			{
				const UnsignedInt id_b=result.all_colliding_ids[id_a][j];
				if(result.all_colliding_ids[id_a].size()<result.all_colliding_ids[id_b].size() || (id_a<id_b && result.all_colliding_ids[id_a].size()==result.all_colliding_ids[id_b].size()))
				{
					result.relevant_collision_ids.push_back(std::pair<UnsignedInt, UnsignedInt>(id_a, id_b));
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("collect relevant collision indices");
	}
};

}

#endif /* VORONOTALT_PREPARATION_FOR_TESSELLATION_H_ */
