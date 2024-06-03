#ifndef VORONOTALT_SPHERES_CONTAINER_H_
#define VORONOTALT_SPHERES_CONTAINER_H_

#include "spheres_searcher.h"
#include "time_recorder.h"

namespace voronotalt
{

class SpheresContainer
{
public:
	struct PeriodicBox
	{
		SimplePoint corner_a;
		SimplePoint corner_b;
		SimplePoint shift;
		bool enabled;

		PeriodicBox() : enabled(false)
		{
		}

		void init(const std::vector<SimplePoint>& periodic_box_corners)
		{
			enabled=(periodic_box_corners.size()>=2);
			if(enabled)
			{
				corner_a=periodic_box_corners[0];
				corner_b=periodic_box_corners[0];

				for(UnsignedInt i=1;i<periodic_box_corners.size();i++)
				{
					const SimplePoint& corner=periodic_box_corners[i];
					corner_a.x=std::min(corner_a.x, corner.x);
					corner_a.y=std::min(corner_a.y, corner.y);
					corner_a.z=std::min(corner_a.z, corner.z);
					corner_b.x=std::max(corner_b.x, corner.x);
					corner_b.y=std::max(corner_b.y, corner.y);
					corner_b.z=std::max(corner_b.z, corner.z);
				}

				shift=sub_of_points(corner_b, corner_a);
			}
		}
	};

	struct ResultOfPreparationForTessellation
	{
		std::vector< std::pair<UnsignedInt, UnsignedInt> > relevant_collision_ids;

		ResultOfPreparationForTessellation()
		{
		}
	};

	SpheresContainer() : total_collisions_(0)
	{
	}

	void init(const std::vector<SimpleSphere>& input_spheres, TimeRecorder& time_recorder)
	{
		init(input_spheres, std::vector<SimplePoint>(), time_recorder);
	}

	void init(const std::vector<SimpleSphere>& input_spheres, const std::vector<SimplePoint>& periodic_box_corners, TimeRecorder& time_recorder)
	{
		time_recorder.reset();

		periodic_box_.init(periodic_box_corners);
		input_spheres_=input_spheres;

		if(periodic_box_.enabled)
		{
			populated_spheres_.resize(input_spheres_.size()*27);
			std::vector<UnsignedInt> collected_indices;
			for(UnsignedInt i=0;i<input_spheres_.size();i++)
			{
				set_sphere_periodic_instances(i, false, collected_indices);
			}
		}
		else
		{
			populated_spheres_=input_spheres_;
		}

		all_exclusion_statuses_.resize(populated_spheres_.size(), 0);

		time_recorder.record_elapsed_miliseconds_and_reset("populate spheres");

		spheres_searcher_.init(populated_spheres_);

		time_recorder.record_elapsed_miliseconds_and_reset("init spheres searcher");

		all_colliding_ids_.resize(input_spheres_.size());

		#pragma omp parallel
		{
			#pragma omp for
			for(UnsignedInt i=0;i<input_spheres_.size();i++)
			{
				all_colliding_ids_[i].reserve(100);
				spheres_searcher_.find_colliding_ids(i, all_colliding_ids_[i], true, all_exclusion_statuses_[i]);
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("detect all collisions");

		total_collisions_=0;

		for(UnsignedInt i=0;i<all_colliding_ids_.size();i++)
		{
			total_collisions_+=all_colliding_ids_[i].size();
		}

		total_collisions_=total_collisions_/2;

		time_recorder.record_elapsed_miliseconds_and_reset("count all collisions");
	}

	void update(const std::vector<SimpleSphere>& new_input_spheres, const std::vector<UnsignedInt>& ids_of_changed_input_spheres, std::vector<UnsignedInt>& ids_of_affected_input_spheres, TimeRecorder& time_recorder)
	{
		time_recorder.reset();

		ids_of_affected_input_spheres.clear();

		if(ids_of_changed_input_spheres.empty())
		{
			return;
		}

		bool full_reinit_needed=(new_input_spheres.size()!=input_spheres_.size() || ids_of_changed_input_spheres.size()>size_threshold_for_full_reinit());

		for(UnsignedInt i=0;!full_reinit_needed && i<ids_of_changed_input_spheres.size();i++)
		{
			if(ids_of_changed_input_spheres.size()>=input_spheres_.size())
			{
				full_reinit_needed=true;;
			}
		}

		if(!full_reinit_needed)
		{
			ids_of_affected_input_spheres=ids_of_changed_input_spheres;
			std::sort(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end());

			for(UnsignedInt i=0;!full_reinit_needed && i<ids_of_changed_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
				for(UnsignedInt j=0;!full_reinit_needed && j<all_colliding_ids_[sphere_id].size();j++)
				{
					if(ids_of_affected_input_spheres.size()<size_threshold_for_full_reinit())
					{
						const UnsignedInt affected_sphere_id=all_colliding_ids_[sphere_id][j].index%input_spheres_.size();
						std::vector<UnsignedInt>::iterator it=std::lower_bound(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end(), affected_sphere_id);
						if(it==ids_of_affected_input_spheres.end() || (*it)!=affected_sphere_id)
						{
							ids_of_affected_input_spheres.insert(it, affected_sphere_id);
						}
					}
					else
					{
						full_reinit_needed=true;
					}
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("gather affected spheres ids for update");

		if(full_reinit_needed)
		{
			std::vector<SimplePoint> periodic_box_corners;
			if(periodic_box_.enabled)
			{
				periodic_box_corners.reserve(2);
				periodic_box_corners.push_back(periodic_box_.corner_a);
				periodic_box_corners.push_back(periodic_box_.corner_b);
			}
			init(new_input_spheres, periodic_box_corners, time_recorder);
			ids_of_affected_input_spheres.clear();
		}
		else
		{
			if(periodic_box_.enabled)
			{
				std::vector<UnsignedInt> ids_of_changed_populated_spheres;
				ids_of_changed_populated_spheres.reserve(ids_of_changed_input_spheres.size()*27);
				for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
					input_spheres_[sphere_id]=new_input_spheres[sphere_id];
					if(periodic_box_.enabled)
					{
						set_sphere_periodic_instances(sphere_id, true, ids_of_changed_populated_spheres);
					}
				}
				spheres_searcher_.update_spheres(populated_spheres_, ids_of_changed_populated_spheres);
			}
			else
			{
				for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
					input_spheres_[sphere_id]=new_input_spheres[sphere_id];
					populated_spheres_[sphere_id]=input_spheres_[sphere_id];
				}
				spheres_searcher_.update_spheres(input_spheres_, ids_of_changed_input_spheres);
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update spheres searcher");

			#pragma omp parallel
			{
				#pragma omp for
				for(UnsignedInt i=0;i<ids_of_affected_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_affected_input_spheres[i];
					all_colliding_ids_[sphere_id].clear();
					spheres_searcher_.find_colliding_ids(sphere_id, all_colliding_ids_[sphere_id], true, all_exclusion_statuses_[sphere_id]);
				}
			}

			std::vector<UnsignedInt> more_ids_of_affected_input_spheres;

			for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
				for(UnsignedInt j=0;j<all_colliding_ids_[sphere_id].size();j++)
				{
					const UnsignedInt affected_sphere_id=all_colliding_ids_[sphere_id][j].index%input_spheres_.size();
					std::vector<UnsignedInt>::iterator it=std::lower_bound(more_ids_of_affected_input_spheres.begin(), more_ids_of_affected_input_spheres.end(), affected_sphere_id);
					if(it==more_ids_of_affected_input_spheres.end() || (*it)!=affected_sphere_id)
					{
						if(!std::binary_search(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end(), affected_sphere_id))
						{
							more_ids_of_affected_input_spheres.insert(it, affected_sphere_id);
						}
					}
				}
			}

			if(!more_ids_of_affected_input_spheres.empty())
			{
				#pragma omp parallel
				{
					#pragma omp for
					for(UnsignedInt i=0;i<more_ids_of_affected_input_spheres.size();i++)
					{
						const UnsignedInt sphere_id=more_ids_of_affected_input_spheres[i];
						all_colliding_ids_[sphere_id].clear();
						spheres_searcher_.find_colliding_ids(sphere_id, all_colliding_ids_[sphere_id], true, all_exclusion_statuses_[sphere_id]);
					}
				}

				std::vector<UnsignedInt> merged_ids_of_affected_input_spheres(ids_of_affected_input_spheres.size()+more_ids_of_affected_input_spheres.size());
				std::merge(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end(),
						more_ids_of_affected_input_spheres.begin(), more_ids_of_affected_input_spheres.end(),
						merged_ids_of_affected_input_spheres.begin());

				ids_of_affected_input_spheres.swap(merged_ids_of_affected_input_spheres);
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update relevant collisions");

			total_collisions_=0;

			for(UnsignedInt i=0;i<all_colliding_ids_.size();i++)
			{
				total_collisions_+=all_colliding_ids_[i].size();
			}

			total_collisions_=total_collisions_/2;

			time_recorder.record_elapsed_miliseconds_and_reset("recount all collisions");
		}
	}

	const PeriodicBox& periodic_box() const
	{
		return periodic_box_;
	}

	const std::vector<SimpleSphere>& input_spheres() const
	{
		return input_spheres_;
	}

	const std::vector<SimpleSphere>& populated_spheres() const
	{
		return populated_spheres_;
	}

	const std::vector<int>& all_exclusion_statuses() const
	{
		return all_exclusion_statuses_;
	}

	const std::vector< std::vector<ValuedID> >& all_colliding_ids() const
	{
		return all_colliding_ids_;
	}

	UnsignedInt total_collisions() const
	{
		return total_collisions_;
	}

	bool prepare_for_tessellation(const std::vector<int>& grouping_of_spheres, ResultOfPreparationForTessellation& result, TimeRecorder& time_recorder) const
	{
		return prepare_for_tessellation(std::vector<int>(), grouping_of_spheres, result, time_recorder);
	}

	bool prepare_for_tessellation(const std::vector<int>& involvement_of_spheres, const std::vector<int>& grouping_of_spheres, ResultOfPreparationForTessellation& result, TimeRecorder& time_recorder) const
	{
		time_recorder.reset();

		result.relevant_collision_ids.clear();

		if(populated_spheres_.empty())
		{
			return false;
		}

		result.relevant_collision_ids.reserve(total_collisions_);

		for(UnsignedInt id_a=0;id_a<input_spheres_.size();id_a++)
		{
			if((involvement_of_spheres.empty() || id_a>=involvement_of_spheres.size() || involvement_of_spheres[id_a]>0) && all_exclusion_statuses_[id_a]==0)
			{
				for(UnsignedInt j=0;j<all_colliding_ids_[id_a].size();j++)
				{
					const UnsignedInt id_b=all_colliding_ids_[id_a][j].index;
					const UnsignedInt id_b_canonical=(id_b%input_spheres_.size());
					if((involvement_of_spheres.empty() || id_b_canonical>=involvement_of_spheres.size() || involvement_of_spheres[id_b_canonical]>0) && all_exclusion_statuses_[id_b]==0)
					{
						if(id_b>=input_spheres_.size() || (all_colliding_ids_[id_a].size()<all_colliding_ids_[id_b].size()) || (id_a<id_b && all_colliding_ids_[id_a].size()==all_colliding_ids_[id_b].size()))
						{
							if(grouping_of_spheres.empty() || id_a>=grouping_of_spheres.size() || id_b_canonical>=grouping_of_spheres.size() || grouping_of_spheres[id_a]!=grouping_of_spheres[id_b_canonical])
							{
								result.relevant_collision_ids.push_back(std::pair<UnsignedInt, UnsignedInt>(id_a, id_b));
							}
						}
					}
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("collect relevant collision indices");

		return true;
	}

private:
	UnsignedInt size_threshold_for_full_reinit() const
	{
		return static_cast<UnsignedInt>(input_spheres_.size()/2);
	}

	void set_sphere_periodic_instances(const UnsignedInt i, const bool collect_indices, std::vector<UnsignedInt>& collected_indices)
	{
		if(i<input_spheres_.size())
		{
			const SimpleSphere& o=input_spheres_[i];
			if(!periodic_box_.enabled)
			{
				if(populated_spheres_.size()!=input_spheres_.size())
				{
					populated_spheres_.resize(input_spheres_.size());
				}
				populated_spheres_[i]=o;
				if(collect_indices)
				{
					collected_indices.push_back(i);
				}
			}
			else
			{
				if(populated_spheres_.size()!=(input_spheres_.size()*27))
				{
					populated_spheres_.resize(input_spheres_.size()*27);
				}
				populated_spheres_[i]=o;
				if(collect_indices)
				{
					collected_indices.push_back(i);
				}
				SimpleSphere m=o;
				UnsignedInt g=1;
				for(int sx=-1;sx<=1;sx++)
				{
					m.p.x=o.p.x+(periodic_box_.shift.x*static_cast<Float>(sx));
					for(int sy=-1;sy<=1;sy++)
					{
						m.p.y=o.p.y+(periodic_box_.shift.y*static_cast<Float>(sy));
						for(int sz=-1;sz<=1;sz++)
						{
							if(sx!=0 || sy!=0 || sz!=0)
							{
								m.p.z=o.p.z+(periodic_box_.shift.z*static_cast<Float>(sz));
								const UnsignedInt mi=(g*input_spheres_.size()+i);
								populated_spheres_[mi]=m;
								if(collect_indices)
								{
									collected_indices.push_back(mi);
								}
								g++;
							}
						}
					}
				}
			}
		}
	}

	PeriodicBox periodic_box_;
	std::vector<SimpleSphere> input_spheres_;
	std::vector<SimpleSphere> populated_spheres_;
	std::vector<int> all_exclusion_statuses_;
	SpheresSearcher spheres_searcher_;
	std::vector< std::vector<ValuedID> > all_colliding_ids_;
	UnsignedInt total_collisions_;
};

}

#endif /* VORONOTALT_SPHERES_CONTAINER_H_ */
