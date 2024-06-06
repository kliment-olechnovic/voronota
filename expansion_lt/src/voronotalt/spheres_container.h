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

		bool equals(const PeriodicBox& pb) const
		{
			return (enabled==pb.enabled && point_equals_point(corner_a, pb.corner_a) && point_equals_point(corner_b, pb.corner_b) && point_equals_point(shift, pb.shift));
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

		if(periodic_box_.enabled)
		{
			for(UnsignedInt i=0;i<input_spheres_.size();i++)
			{
				set_exclusion_status_periodic_instances(i);
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

	bool update(const std::vector<SimpleSphere>& new_input_spheres, const std::vector<UnsignedInt>& provided_ids_of_changed_input_spheres, const bool trust_provided_ids_of_changed_input_spheres, std::vector<UnsignedInt>& ids_of_affected_input_spheres, TimeRecorder& time_recorder)
	{
		time_recorder.reset();

		ids_of_affected_input_spheres.clear();

		if(new_input_spheres.size()!=input_spheres_.size())
		{
			reinit(new_input_spheres, ids_of_affected_input_spheres, time_recorder);
			return true;
		}

		std::vector<UnsignedInt> identified_ids_of_changed_input_spheres;

		if(!trust_provided_ids_of_changed_input_spheres)
		{
			for(UnsignedInt i=0;i<new_input_spheres.size();i++)
			{
				if(!sphere_equals_sphere(new_input_spheres[i], input_spheres_[i]))
				{
					if(identified_ids_of_changed_input_spheres.size()<size_threshold_for_full_reinit())
					{
						identified_ids_of_changed_input_spheres.push_back(i);
					}
					else
					{
						reinit(new_input_spheres, ids_of_affected_input_spheres, time_recorder);
						return true;
					}
				}
			}

			time_recorder.record_elapsed_miliseconds_and_reset("identify changed spheres ids for update");
		}

		const std::vector<UnsignedInt>& ids_of_changed_input_spheres=(trust_provided_ids_of_changed_input_spheres ? provided_ids_of_changed_input_spheres : identified_ids_of_changed_input_spheres);

		if(ids_of_changed_input_spheres.empty())
		{
			return false;
		}

		if(ids_of_changed_input_spheres.size()>size_threshold_for_full_reinit())
		{
			reinit(new_input_spheres, ids_of_affected_input_spheres, time_recorder);
			return true;
		}

		for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
		{
			if(ids_of_changed_input_spheres[i]>=input_spheres_.size())
			{
				reinit(new_input_spheres, ids_of_affected_input_spheres, time_recorder);
				return true;
			}
		}

		{
			bool update_needed=false;
			for(UnsignedInt i=0;!update_needed && i<ids_of_changed_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
				if(!sphere_equals_sphere(new_input_spheres[sphere_id], input_spheres_[sphere_id]))
				{
					update_needed=true;
				}
			}
			if(!update_needed)
			{
				return false;
			}
		}

		{
			ids_of_affected_input_spheres=ids_of_changed_input_spheres;
			std::sort(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end());

			for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
				for(UnsignedInt j=0;j<all_colliding_ids_[sphere_id].size();j++)
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
						reinit(new_input_spheres, ids_of_affected_input_spheres, time_recorder);
						return true;
					}
				}
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("gather affected spheres ids for update");

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
				spheres_searcher_.update(populated_spheres_, ids_of_changed_populated_spheres);
			}
			else
			{
				for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
					input_spheres_[sphere_id]=new_input_spheres[sphere_id];
					populated_spheres_[sphere_id]=input_spheres_[sphere_id];
				}
				spheres_searcher_.update(input_spheres_, ids_of_changed_input_spheres);
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

			if(periodic_box_.enabled)
			{
				for(UnsignedInt i=0;i<ids_of_affected_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_affected_input_spheres[i];
					set_exclusion_status_periodic_instances(sphere_id);
				}
			}

			buffered_temporary_storage_.clear();

			for(UnsignedInt i=0;i<ids_of_changed_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_changed_input_spheres[i];
				for(UnsignedInt j=0;j<all_colliding_ids_[sphere_id].size();j++)
				{
					const UnsignedInt affected_sphere_id=all_colliding_ids_[sphere_id][j].index%input_spheres_.size();
					std::vector<UnsignedInt>::iterator it=std::lower_bound(buffered_temporary_storage_.more_ids_of_affected_input_spheres.begin(), buffered_temporary_storage_.more_ids_of_affected_input_spheres.end(), affected_sphere_id);
					if(it==buffered_temporary_storage_.more_ids_of_affected_input_spheres.end() || (*it)!=affected_sphere_id)
					{
						if(!std::binary_search(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end(), affected_sphere_id))
						{
							buffered_temporary_storage_.more_ids_of_affected_input_spheres.insert(it, affected_sphere_id);
						}
					}
				}
			}

			if(!buffered_temporary_storage_.more_ids_of_affected_input_spheres.empty())
			{
				#pragma omp parallel
				{
					#pragma omp for
					for(UnsignedInt i=0;i<buffered_temporary_storage_.more_ids_of_affected_input_spheres.size();i++)
					{
						const UnsignedInt sphere_id=buffered_temporary_storage_.more_ids_of_affected_input_spheres[i];
						all_colliding_ids_[sphere_id].clear();
						spheres_searcher_.find_colliding_ids(sphere_id, all_colliding_ids_[sphere_id], true, all_exclusion_statuses_[sphere_id]);
					}
				}

				if(periodic_box_.enabled)
				{
					for(UnsignedInt i=0;i<ids_of_affected_input_spheres.size();i++)
					{
						const UnsignedInt sphere_id=ids_of_affected_input_spheres[i];
						set_exclusion_status_periodic_instances(sphere_id);
					}
				}

				buffered_temporary_storage_.merged_ids_of_affected_input_spheres.resize(ids_of_affected_input_spheres.size()+buffered_temporary_storage_.more_ids_of_affected_input_spheres.size());

				std::merge(ids_of_affected_input_spheres.begin(), ids_of_affected_input_spheres.end(),
						buffered_temporary_storage_.more_ids_of_affected_input_spheres.begin(), buffered_temporary_storage_.more_ids_of_affected_input_spheres.end(),
						buffered_temporary_storage_.merged_ids_of_affected_input_spheres.begin());

				ids_of_affected_input_spheres.swap(buffered_temporary_storage_.merged_ids_of_affected_input_spheres);
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

		return true;
	}

	void assign(const SpheresContainer& obj)
	{
		periodic_box_=obj.periodic_box_;
		total_collisions_=obj.total_collisions_;

		spheres_searcher_.assign(obj.spheres_searcher_);

		input_spheres_.resize(obj.input_spheres_.size());
		populated_spheres_.resize(obj.populated_spheres_.size());
		all_exclusion_statuses_.resize(obj.all_exclusion_statuses_.size());
		all_colliding_ids_.resize(obj.all_colliding_ids_.size());

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.input_spheres_.size();i++)
			{
				input_spheres_[i]=obj.input_spheres_[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.populated_spheres_.size();i++)
			{
				populated_spheres_[i]=obj.populated_spheres_[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.all_exclusion_statuses_.size();i++)
			{
				all_exclusion_statuses_[i]=obj.all_exclusion_statuses_[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.all_colliding_ids_.size();i++)
			{
				all_colliding_ids_[i]=obj.all_colliding_ids_[i];
			}
		}
	}

	void assign(const SpheresContainer& obj, const std::vector<UnsignedInt>& subset_of_ids)
	{
		if(subset_of_ids.empty()
				|| obj.input_spheres_.empty()
				|| !periodic_box_.equals(obj.periodic_box_)
				|| input_spheres_.size()!=obj.input_spheres_.size()
				|| populated_spheres_.size()!=obj.populated_spheres_.size()
				|| all_exclusion_statuses_.size()!=obj.all_exclusion_statuses_.size()
				|| all_colliding_ids_.size()!=obj.all_colliding_ids_.size()
				|| subset_of_ids.size()>=size_threshold_for_full_reinit())
		{
			assign(obj);
		}
		else
		{
			for(UnsignedInt i=0;i<subset_of_ids.size();i++)
			{
				const UnsignedInt sphere_id=subset_of_ids[i];
				if(sphere_id>=input_spheres_.size())
				{
					assign(obj);
					return;
				}
			}

			periodic_box_=obj.periodic_box_;
			total_collisions_=obj.total_collisions_;

			spheres_searcher_.assign(obj.spheres_searcher_);

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<subset_of_ids.size();i++)
				{
					const UnsignedInt sphere_id=subset_of_ids[i];
					input_spheres_[sphere_id]=obj.input_spheres_[sphere_id];
					populated_spheres_[sphere_id]=obj.populated_spheres_[sphere_id];
					all_exclusion_statuses_[sphere_id]=obj.all_exclusion_statuses_[sphere_id];
					all_colliding_ids_[sphere_id]=obj.all_colliding_ids_[sphere_id];
				}
			}

			if(periodic_box_.enabled && populated_spheres_.size()==input_spheres_.size()*27 && all_exclusion_statuses_.size()==all_exclusion_statuses_.size()*27)
			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<subset_of_ids.size();i++)
				{
					const UnsignedInt sphere_id=subset_of_ids[i];
					for(UnsignedInt m=1;m<27;m++)
					{
						const UnsignedInt shifted_sphere_id=(m*input_spheres_.size()+sphere_id);
						populated_spheres_[shifted_sphere_id]=obj.populated_spheres_[sphere_id];
						all_exclusion_statuses_[shifted_sphere_id]=all_exclusion_statuses_[sphere_id];
					}
				}
			}
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
					if((involvement_of_spheres.empty() || id_b_canonical>=involvement_of_spheres.size() || involvement_of_spheres[id_b_canonical]>0) && all_exclusion_statuses_[id_b_canonical]==0)
					{
						if(id_b>=input_spheres_.size() || (all_colliding_ids_[id_a].size()<all_colliding_ids_[id_b_canonical].size()) || (id_a<id_b && all_colliding_ids_[id_a].size()==all_colliding_ids_[id_b_canonical].size()))
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
	struct BufferedTemporaryStorage
	{
		std::vector<UnsignedInt> more_ids_of_affected_input_spheres;
		std::vector<UnsignedInt> merged_ids_of_affected_input_spheres;

		void clear()
		{
			more_ids_of_affected_input_spheres.clear();
			merged_ids_of_affected_input_spheres.clear();
		}
	};

	UnsignedInt size_threshold_for_full_reinit() const
	{
		return static_cast<UnsignedInt>(input_spheres_.size()/2);
	}

	void reinit(const std::vector<SimpleSphere>& new_input_spheres, std::vector<UnsignedInt>& ids_of_affected_input_spheres, TimeRecorder& time_recorder)
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

	void set_exclusion_status_periodic_instances(const UnsignedInt i)
	{
		if(i<input_spheres_.size() && all_exclusion_statuses_.size()==input_spheres_.size()*27)
		{
			for(UnsignedInt m=1;m<27;m++)
			{
				all_exclusion_statuses_[m*input_spheres_.size()+i]=all_exclusion_statuses_[i];
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
	BufferedTemporaryStorage buffered_temporary_storage_;
};

}

#endif /* VORONOTALT_SPHERES_CONTAINER_H_ */
