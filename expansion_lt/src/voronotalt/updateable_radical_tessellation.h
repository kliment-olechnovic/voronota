#ifndef VORONOTALT_UPDATEABLE_RADICAL_TESSELLATION_H_
#define VORONOTALT_UPDATEABLE_RADICAL_TESSELLATION_H_

#include "radical_tessellation.h"

namespace voronotalt
{

class UpdateableRadicalTessellation
{
public:
	struct Result
	{
		std::vector<RadicalTessellation::CellContactDescriptorsSummary> cells_summaries;
		std::vector< std::vector<RadicalTessellation::ContactDescriptorSummary> > contacts_summaries;
		std::vector< std::vector<RadicalTessellation::ContactDescriptorSummary> > contacts_summaries_with_redundancy_in_periodic_box;

		Result()
		{
		}
	};

	struct ResultSummary
	{
		RadicalTessellation::TotalContactDescriptorsSummary total_contacts_summary;
		RadicalTessellation::TotalCellContactDescriptorsSummary total_cells_summary;

		ResultSummary()
		{
		}
	};

	UpdateableRadicalTessellation() : last_update_was_full_reinit_(true)
	{
	}

	void init(const std::vector<SimpleSphere>& input_spheres)
	{
		TimeRecorder time_recorder;
		init(input_spheres, std::vector<SimplePoint>(), time_recorder);
	}

	void init(const std::vector<SimpleSphere>& input_spheres, TimeRecorder& time_recorder)
	{
		init(input_spheres, std::vector<SimplePoint>(), time_recorder);
	}

	void init(const std::vector<SimpleSphere>& input_spheres, const std::vector<SimplePoint>& periodic_box_corners)
	{
		TimeRecorder time_recorder;
		init(input_spheres, periodic_box_corners, time_recorder);
	}

	void init(const std::vector<SimpleSphere>& input_spheres, const std::vector<SimplePoint>& periodic_box_corners, TimeRecorder& time_recorder)
	{
		spheres_container_.init(input_spheres, periodic_box_corners, time_recorder);
		RadicalTessellation::ResultGraphics result_graphics;
		RadicalTessellation::construct_full_tessellation(spheres_container_, std::vector<int>(), std::vector<int>(), false, true, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);
		involvement_of_spheres_for_update_.clear();
		init_result_from_tessellation_result();
	}

	bool update(const std::vector<SimpleSphere>& new_input_spheres)
	{
		TimeRecorder time_recorder;
		return update(new_input_spheres, std::vector<UnsignedInt>(), false, time_recorder);
	}

	bool update(const std::vector<SimpleSphere>& new_input_spheres, TimeRecorder& time_recorder)
	{
		return update(new_input_spheres, std::vector<UnsignedInt>(), false, time_recorder);
	}

	bool update(const std::vector<SimpleSphere>& new_input_spheres, const std::vector<UnsignedInt>& ids_of_changed_input_spheres)
	{
		TimeRecorder time_recorder;
		return update(new_input_spheres, ids_of_changed_input_spheres, true, time_recorder);
	}

	bool update(const std::vector<SimpleSphere>& new_input_spheres, const std::vector<UnsignedInt>& ids_of_changed_input_spheres, TimeRecorder& time_recorder)
	{
		return update(new_input_spheres, ids_of_changed_input_spheres, true, time_recorder);
	}

	bool update(const std::vector<SimpleSphere>& new_input_spheres, const std::vector<UnsignedInt>& provided_ids_of_changed_input_spheres, const bool trust_provided_ids_of_changed_input_spheres, TimeRecorder& time_recorder)
	{
		time_recorder.reset();

		ids_of_changed_input_spheres_.clear();
		ids_of_affected_input_spheres_.clear();
		last_update_was_full_reinit_=false;

		if(trust_provided_ids_of_changed_input_spheres && provided_ids_of_changed_input_spheres.empty())
		{
			return false;
		}

		if(!spheres_container_.update(new_input_spheres, provided_ids_of_changed_input_spheres, trust_provided_ids_of_changed_input_spheres, ids_of_changed_input_spheres_, ids_of_affected_input_spheres_, time_recorder))
		{
			return false;
		}

		if(ids_of_affected_input_spheres_.empty())
		{
			RadicalTessellation::ResultGraphics result_graphics;
			RadicalTessellation::construct_full_tessellation(spheres_container_, std::vector<int>(), std::vector<int>(), false, true, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);
			init_result_from_tessellation_result();
			return true;
		}

		if(involvement_of_spheres_for_update_.size()!=spheres_container_.input_spheres().size())
		{
			involvement_of_spheres_for_update_.clear();
			involvement_of_spheres_for_update_.resize(spheres_container_.input_spheres().size(), 0);
		}

		for(UnsignedInt i=0;i<ids_of_affected_input_spheres_.size();i++)
		{
			involvement_of_spheres_for_update_[ids_of_affected_input_spheres_[i]]=1;
		}

		{
			RadicalTessellation::ResultGraphics result_graphics;
			RadicalTessellation::construct_full_tessellation(spheres_container_, involvement_of_spheres_for_update_, std::vector<int>(), false, false, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);

			{
				const ConditionToRemoveContact condition_to_remove_contact(involvement_of_spheres_for_update_);

				for(UnsignedInt i=0;i<ids_of_affected_input_spheres_.size();i++)
				{
					const UnsignedInt sphere_id=ids_of_affected_input_spheres_[i];
					{
						std::vector<RadicalTessellation::ContactDescriptorSummary>& v=result_.contacts_summaries[sphere_id];
						std::vector<RadicalTessellation::ContactDescriptorSummary>::iterator it=std::remove_if(v.begin(), v.end(), condition_to_remove_contact);
						v.erase(it, v.end());
					}
					if(!result_.contacts_summaries_with_redundancy_in_periodic_box.empty())
					{
						std::vector<RadicalTessellation::ContactDescriptorSummary>& v=result_.contacts_summaries_with_redundancy_in_periodic_box[sphere_id];
						std::vector<RadicalTessellation::ContactDescriptorSummary>::iterator it=std::remove_if(v.begin(), v.end(), condition_to_remove_contact);
						v.erase(it, v.end());
					}
				}
			}

			if(!buffered_temporary_storage_.tessellation_result.contacts_summaries.empty())
			{
				for(UnsignedInt i=0;i<buffered_temporary_storage_.tessellation_result.contacts_summaries.size();i++)
				{
					const RadicalTessellation::ContactDescriptorSummary& cds=buffered_temporary_storage_.tessellation_result.contacts_summaries[i];
					result_.contacts_summaries[cds.id_a].push_back(cds);
					result_.contacts_summaries[cds.id_b].push_back(cds);
				}

				if(!result_.contacts_summaries_with_redundancy_in_periodic_box.empty())
				{
					const std::vector<RadicalTessellation::ContactDescriptorSummary>& all_contacts_summaries=(buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box.empty() ? buffered_temporary_storage_.tessellation_result.contacts_summaries : buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box);

					for(UnsignedInt i=0;i<all_contacts_summaries.size();i++)
					{
						const RadicalTessellation::ContactDescriptorSummary& cds=all_contacts_summaries[i];
						if(cds.id_a<result_.contacts_summaries_with_redundancy_in_periodic_box.size())
						{
							result_.contacts_summaries_with_redundancy_in_periodic_box[cds.id_a].push_back(cds);
						}
						if(cds.id_b<result_.contacts_summaries_with_redundancy_in_periodic_box.size())
						{
							result_.contacts_summaries_with_redundancy_in_periodic_box[cds.id_b].push_back(cds);
						}
					}
				}
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update contacts summaries");
		}

		{
			const std::vector< std::vector<RadicalTessellation::ContactDescriptorSummary> >& all_contacts_summaries=(result_.contacts_summaries_with_redundancy_in_periodic_box.empty() ? result_.contacts_summaries : result_.contacts_summaries_with_redundancy_in_periodic_box);

			for(UnsignedInt i=0;i<ids_of_affected_input_spheres_.size();i++)
			{
				const UnsignedInt sphere_id=ids_of_affected_input_spheres_[i];
				RadicalTessellation::CellContactDescriptorsSummary& cs=result_.cells_summaries[sphere_id];
				cs=RadicalTessellation::CellContactDescriptorsSummary();
				const std::vector<RadicalTessellation::ContactDescriptorSummary>& v=all_contacts_summaries[sphere_id];
				for(UnsignedInt j=0;j<v.size();j++)
				{
					cs.add(sphere_id, v[j]);
				}
				cs.compute_sas(spheres_container_.input_spheres()[sphere_id].r);
				if(cs.stage==0 && spheres_container_.all_exclusion_statuses()[sphere_id]==0 && spheres_container_.all_colliding_ids()[sphere_id].empty())
				{
					cs.compute_sas_detached(sphere_id, spheres_container_.input_spheres()[sphere_id].r);
				}
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update cell summaries");
		}

		for(UnsignedInt i=0;i<ids_of_affected_input_spheres_.size();i++)
		{
			involvement_of_spheres_for_update_[ids_of_affected_input_spheres_[i]]=0;
		}

		return true;
	}

	void assign(const UpdateableRadicalTessellation& obj)
	{
		spheres_container_.assign(obj.spheres_container_);

		result_.cells_summaries.resize(obj.result_.cells_summaries.size());
		result_.contacts_summaries.resize(obj.result_.contacts_summaries.size());
		result_.contacts_summaries_with_redundancy_in_periodic_box.resize(obj.result_.contacts_summaries_with_redundancy_in_periodic_box.size());
		ids_of_changed_input_spheres_.resize(obj.ids_of_changed_input_spheres_.size());
		ids_of_affected_input_spheres_.resize(obj.ids_of_affected_input_spheres_.size());

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.result_.cells_summaries.size();i++)
			{
				result_.cells_summaries[i]=obj.result_.cells_summaries[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.result_.contacts_summaries.size();i++)
			{
				result_.contacts_summaries[i]=obj.result_.contacts_summaries[i];
			}
		}

		if(!obj.result_.contacts_summaries_with_redundancy_in_periodic_box.empty())
		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.result_.contacts_summaries_with_redundancy_in_periodic_box.size();i++)
			{
				result_.contacts_summaries_with_redundancy_in_periodic_box[i]=obj.result_.contacts_summaries_with_redundancy_in_periodic_box[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.ids_of_changed_input_spheres_.size();i++)
			{
				ids_of_changed_input_spheres_[i]=obj.ids_of_changed_input_spheres_[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.ids_of_affected_input_spheres_.size();i++)
			{
				ids_of_affected_input_spheres_[i]=obj.ids_of_affected_input_spheres_[i];
			}
		}

		last_update_was_full_reinit_=obj.last_update_was_full_reinit_;
	}

	void assign_to_undo_update(const UpdateableRadicalTessellation& obj)
	{
		assign(obj, last_update_was_full_reinit_, ids_of_affected_input_spheres_);
	}

	void assign_to_apply_update(const UpdateableRadicalTessellation& obj)
	{
		assign(obj, obj.last_update_was_full_reinit_, obj.ids_of_affected_input_spheres_);
	}

	const SpheresContainer& spheres_container() const
	{
		return spheres_container_;
	}

	const Result& result() const
	{
		return result_;
	}

	ResultSummary result_summary() const
	{
		ResultSummary result_summary;
		for(UnsignedInt i=0;i<result_.contacts_summaries.size();i++)
		{
			for(UnsignedInt j=0;j<result_.contacts_summaries[i].size();j++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=result_.contacts_summaries[i][j];
				if(cds.id_a==i)
				{
					result_summary.total_contacts_summary.add(cds);
				}
			}
		}
		for(UnsignedInt i=0;i<result_.cells_summaries.size();i++)
		{
			result_summary.total_cells_summary.add(result_.cells_summaries[i]);
		}
		return result_summary;
	}

	const std::vector<UnsignedInt>& last_update_ids_of_changed_input_spheres() const
	{
		return ids_of_changed_input_spheres_;
	}

	const std::vector<UnsignedInt>& last_update_ids_of_affected_input_spheres() const
	{
		return ids_of_affected_input_spheres_;
	}

	bool last_update_was_full_reinit() const
	{
		return last_update_was_full_reinit_;
	}

private:
	struct BufferedTemporaryStorage
	{
		RadicalTessellation::Result tessellation_result;

		void clear()
		{
			tessellation_result.clear();
		}
	};

	class ConditionToRemoveContact
	{
	public:
		ConditionToRemoveContact(const std::vector<int>& involvement) : involvement_(involvement)
		{
		}

		bool operator()(const RadicalTessellation::ContactDescriptorSummary& cds)
		{
			return (involvement_.empty() || (involvement_[cds.id_a%involvement_.size()]>0 && involvement_[cds.id_b%involvement_.size()]>0));
		}

	private:
		const std::vector<int>& involvement_;
	};

	void init_result_from_tessellation_result()
	{
		ids_of_changed_input_spheres_.clear();
		ids_of_affected_input_spheres_.clear();
		last_update_was_full_reinit_=true;

		result_.cells_summaries.swap(buffered_temporary_storage_.tessellation_result.cells_summaries);

		{
			result_.contacts_summaries.resize(spheres_container_.input_spheres().size());
			for(UnsignedInt i=0;i<result_.contacts_summaries.size();i++)
			{
				result_.contacts_summaries[i].clear();
			}
			for(UnsignedInt i=0;i<buffered_temporary_storage_.tessellation_result.contacts_summaries.size();i++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=buffered_temporary_storage_.tessellation_result.contacts_summaries[i];
				result_.contacts_summaries[cds.id_a].push_back(cds);
				result_.contacts_summaries[cds.id_b].push_back(cds);
			}
		}

		if(spheres_container_.periodic_box().enabled)
		{
			result_.contacts_summaries_with_redundancy_in_periodic_box.resize(spheres_container_.input_spheres().size());
			for(UnsignedInt i=0;i<result_.contacts_summaries_with_redundancy_in_periodic_box.size();i++)
			{
				result_.contacts_summaries_with_redundancy_in_periodic_box[i].clear();
			}
			const std::vector<RadicalTessellation::ContactDescriptorSummary>& all_contacts_summaries=(buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box.empty() ? buffered_temporary_storage_.tessellation_result.contacts_summaries : buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box);
			for(UnsignedInt i=0;i<all_contacts_summaries.size();i++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=all_contacts_summaries[i];
				if(cds.id_a<result_.contacts_summaries_with_redundancy_in_periodic_box.size())
				{
					result_.contacts_summaries_with_redundancy_in_periodic_box[cds.id_a].push_back(cds);
				}
				if(cds.id_b<result_.contacts_summaries_with_redundancy_in_periodic_box.size())
				{
					result_.contacts_summaries_with_redundancy_in_periodic_box[cds.id_b].push_back(cds);
				}
			}
		}
		else
		{
			result_.contacts_summaries_with_redundancy_in_periodic_box.clear();
		}
	}

	void assign(const UpdateableRadicalTessellation& obj, const bool assign_everything, const std::vector<UnsignedInt>& subset_of_ids_of_spheres)
	{
		if(!assign_everything && subset_of_ids_of_spheres.empty())
		{
			return;
		}

		if(assign_everything
				|| result_.cells_summaries.size()!=obj.result_.cells_summaries.size()
				|| result_.contacts_summaries.size()!=obj.result_.contacts_summaries.size()
				|| result_.contacts_summaries_with_redundancy_in_periodic_box.size()!=obj.result_.contacts_summaries_with_redundancy_in_periodic_box.size())
		{
			assign(obj);
			return;
		}

		const bool periodic=!obj.result_.contacts_summaries_with_redundancy_in_periodic_box.empty();

		for(UnsignedInt i=0;i<subset_of_ids_of_spheres.size();i++)
		{
			const UnsignedInt sphere_id=subset_of_ids_of_spheres[i];
			if(sphere_id>=result_.cells_summaries.size() || sphere_id>=result_.cells_summaries.size() || (periodic && sphere_id>=result_.contacts_summaries_with_redundancy_in_periodic_box.size()))
			{
				assign(obj);
				return;
			}
		}

		spheres_container_.assign(obj.spheres_container_, ids_of_changed_input_spheres_);

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<subset_of_ids_of_spheres.size();i++)
			{
				const UnsignedInt sphere_id=subset_of_ids_of_spheres[i];
				result_.cells_summaries[sphere_id]=obj.result_.cells_summaries[sphere_id];
				result_.contacts_summaries[sphere_id]=obj.result_.contacts_summaries[sphere_id];
				if(periodic)
				{
					result_.contacts_summaries_with_redundancy_in_periodic_box[sphere_id]=obj.result_.contacts_summaries_with_redundancy_in_periodic_box[sphere_id];
				}
			}
		}

		ids_of_changed_input_spheres_.resize(obj.ids_of_changed_input_spheres_.size());
		ids_of_affected_input_spheres_.resize(obj.ids_of_affected_input_spheres_.size());

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.ids_of_changed_input_spheres_.size();i++)
			{
				ids_of_changed_input_spheres_[i]=obj.ids_of_changed_input_spheres_[i];
			}
		}

		{
			#pragma omp parallel for
			for(UnsignedInt i=0;i<obj.ids_of_affected_input_spheres_.size();i++)
			{
				ids_of_affected_input_spheres_[i]=obj.ids_of_affected_input_spheres_[i];
			}
		}

		last_update_was_full_reinit_=obj.last_update_was_full_reinit_;
	}

	SpheresContainer spheres_container_;
	Result result_;
	std::vector<UnsignedInt> ids_of_changed_input_spheres_;
	std::vector<UnsignedInt> ids_of_affected_input_spheres_;
	bool last_update_was_full_reinit_;
	std::vector<int> involvement_of_spheres_for_update_;
	BufferedTemporaryStorage buffered_temporary_storage_;
};

}

#endif /* VORONOTALT_UPDATEABLE_RADICAL_TESSELLATION_H_ */
