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

	UpdateableRadicalTessellation() : undoable_(false), undone_(false)
	{
	}

	UpdateableRadicalTessellation(const bool undoable) : undoable_(undoable), undone_(false)
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
		time_recorder.reset();

		if(undoable_)
		{
			state_backup_.assign_to_apply_update(state_);
			undone_=false;
		}

		time_recorder.record_elapsed_miliseconds_and_reset("backup state");

		state_.spheres_container.init(input_spheres, periodic_box_corners, time_recorder);

		RadicalTessellation::ResultGraphics result_graphics;
		RadicalTessellation::construct_full_tessellation(state_.spheres_container, std::vector<int>(), std::vector<int>(), false, true, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);

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

		if(undoable_)
		{
			state_backup_.assign_to_apply_update(state_);
			undone_=false;
		}

		time_recorder.record_elapsed_miliseconds_and_reset("backup state");

		state_.ids_of_changed_input_spheres.clear();
		state_.ids_of_affected_input_spheres.clear();
		state_.last_update_was_full_reinit=false;

		if(trust_provided_ids_of_changed_input_spheres && provided_ids_of_changed_input_spheres.empty())
		{
			return false;
		}

		if(!state_.spheres_container.update(new_input_spheres, provided_ids_of_changed_input_spheres, trust_provided_ids_of_changed_input_spheres, state_.ids_of_changed_input_spheres, state_.ids_of_affected_input_spheres, time_recorder))
		{
			return false;
		}

		if(state_.ids_of_affected_input_spheres.empty())
		{
			RadicalTessellation::ResultGraphics result_graphics;
			RadicalTessellation::construct_full_tessellation(state_.spheres_container, std::vector<int>(), std::vector<int>(), false, true, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);
			init_result_from_tessellation_result();
			return true;
		}

		if(involvement_of_spheres_for_update_.size()!=state_.spheres_container.input_spheres().size())
		{
			involvement_of_spheres_for_update_.clear();
			involvement_of_spheres_for_update_.resize(state_.spheres_container.input_spheres().size(), 0);
		}

		for(UnsignedInt i=0;i<state_.ids_of_affected_input_spheres.size();i++)
		{
			involvement_of_spheres_for_update_[state_.ids_of_affected_input_spheres[i]]=1;
		}

		{
			RadicalTessellation::ResultGraphics result_graphics;
			RadicalTessellation::construct_full_tessellation(state_.spheres_container, involvement_of_spheres_for_update_, std::vector<int>(), false, false, buffered_temporary_storage_.tessellation_result, result_graphics, time_recorder);

			{
				const ConditionToRemoveContact condition_to_remove_contact(involvement_of_spheres_for_update_);

				for(UnsignedInt i=0;i<state_.ids_of_affected_input_spheres.size();i++)
				{
					const UnsignedInt sphere_id=state_.ids_of_affected_input_spheres[i];
					{
						std::vector<RadicalTessellation::ContactDescriptorSummary>& v=state_.result.contacts_summaries[sphere_id];
						std::vector<RadicalTessellation::ContactDescriptorSummary>::iterator it=std::remove_if(v.begin(), v.end(), condition_to_remove_contact);
						v.erase(it, v.end());
					}
					if(!state_.result.contacts_summaries_with_redundancy_in_periodic_box.empty())
					{
						std::vector<RadicalTessellation::ContactDescriptorSummary>& v=state_.result.contacts_summaries_with_redundancy_in_periodic_box[sphere_id];
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
					state_.result.contacts_summaries[cds.id_a].push_back(cds);
					state_.result.contacts_summaries[cds.id_b].push_back(cds);
				}

				if(!state_.result.contacts_summaries_with_redundancy_in_periodic_box.empty())
				{
					const std::vector<RadicalTessellation::ContactDescriptorSummary>& all_contacts_summaries=(buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box.empty() ? buffered_temporary_storage_.tessellation_result.contacts_summaries : buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box);

					for(UnsignedInt i=0;i<all_contacts_summaries.size();i++)
					{
						const RadicalTessellation::ContactDescriptorSummary& cds=all_contacts_summaries[i];
						if(cds.id_a<state_.result.contacts_summaries_with_redundancy_in_periodic_box.size())
						{
							state_.result.contacts_summaries_with_redundancy_in_periodic_box[cds.id_a].push_back(cds);
						}
						if(cds.id_b<state_.result.contacts_summaries_with_redundancy_in_periodic_box.size())
						{
							state_.result.contacts_summaries_with_redundancy_in_periodic_box[cds.id_b].push_back(cds);
						}
					}
				}
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update contacts summaries");
		}

		{
			const std::vector< std::vector<RadicalTessellation::ContactDescriptorSummary> >& all_contacts_summaries=(state_.result.contacts_summaries_with_redundancy_in_periodic_box.empty() ? state_.result.contacts_summaries : state_.result.contacts_summaries_with_redundancy_in_periodic_box);

			for(UnsignedInt i=0;i<state_.ids_of_affected_input_spheres.size();i++)
			{
				const UnsignedInt sphere_id=state_.ids_of_affected_input_spheres[i];
				RadicalTessellation::CellContactDescriptorsSummary& cs=state_.result.cells_summaries[sphere_id];
				cs=RadicalTessellation::CellContactDescriptorsSummary();
				const std::vector<RadicalTessellation::ContactDescriptorSummary>& v=all_contacts_summaries[sphere_id];
				for(UnsignedInt j=0;j<v.size();j++)
				{
					cs.add(sphere_id, v[j]);
				}
				cs.compute_sas(state_.spheres_container.input_spheres()[sphere_id].r);
				if(cs.stage==0 && state_.spheres_container.all_exclusion_statuses()[sphere_id]==0 && state_.spheres_container.all_colliding_ids()[sphere_id].empty())
				{
					cs.compute_sas_detached(sphere_id, state_.spheres_container.input_spheres()[sphere_id].r);
				}
			}

			time_recorder.record_elapsed_miliseconds_and_reset("update cell summaries");
		}

		for(UnsignedInt i=0;i<state_.ids_of_affected_input_spheres.size();i++)
		{
			involvement_of_spheres_for_update_[state_.ids_of_affected_input_spheres[i]]=0;
		}

		return true;
	}

	void undo()
	{
		if(undoable_ && !undone_)
		{
			state_.assign_to_undo_update(state_backup_);
			undone_=true;
		}
	}

	bool undoable() const
	{
		return undoable_;
	}

	bool undone() const
	{
		return undone_;
	}

	const SpheresContainer& spheres_container() const
	{
		return state_.spheres_container;
	}

	const Result& result() const
	{
		return state_.result;
	}

	ResultSummary result_summary() const
	{
		ResultSummary result_summary;
		for(UnsignedInt i=0;i<state_.result.contacts_summaries.size();i++)
		{
			for(UnsignedInt j=0;j<state_.result.contacts_summaries[i].size();j++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=state_.result.contacts_summaries[i][j];
				if(cds.id_a==i)
				{
					result_summary.total_contacts_summary.add(cds);
				}
			}
		}
		for(UnsignedInt i=0;i<state_.result.cells_summaries.size();i++)
		{
			result_summary.total_cells_summary.add(state_.result.cells_summaries[i]);
		}
		return result_summary;
	}

	const std::vector<UnsignedInt>& last_update_ids_of_changed_input_spheres() const
	{
		return state_.ids_of_changed_input_spheres;
	}

	const std::vector<UnsignedInt>& last_update_ids_of_affected_input_spheres() const
	{
		return state_.ids_of_affected_input_spheres;
	}

	bool last_update_was_full_reinit() const
	{
		return state_.last_update_was_full_reinit;
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
		state_.ids_of_changed_input_spheres.clear();
		state_.ids_of_affected_input_spheres.clear();
		state_.last_update_was_full_reinit=true;

		state_.result.cells_summaries.swap(buffered_temporary_storage_.tessellation_result.cells_summaries);

		{
			state_.result.contacts_summaries.resize(state_.spheres_container.input_spheres().size());
			for(UnsignedInt i=0;i<state_.result.contacts_summaries.size();i++)
			{
				state_.result.contacts_summaries[i].clear();
			}
			for(UnsignedInt i=0;i<buffered_temporary_storage_.tessellation_result.contacts_summaries.size();i++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=buffered_temporary_storage_.tessellation_result.contacts_summaries[i];
				state_.result.contacts_summaries[cds.id_a].push_back(cds);
				state_.result.contacts_summaries[cds.id_b].push_back(cds);
			}
		}

		if(state_.spheres_container.periodic_box().enabled)
		{
			state_.result.contacts_summaries_with_redundancy_in_periodic_box.resize(state_.spheres_container.input_spheres().size());
			for(UnsignedInt i=0;i<state_.result.contacts_summaries_with_redundancy_in_periodic_box.size();i++)
			{
				state_.result.contacts_summaries_with_redundancy_in_periodic_box[i].clear();
			}
			const std::vector<RadicalTessellation::ContactDescriptorSummary>& all_contacts_summaries=(buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box.empty() ? buffered_temporary_storage_.tessellation_result.contacts_summaries : buffered_temporary_storage_.tessellation_result.contacts_summaries_with_redundancy_in_periodic_box);
			for(UnsignedInt i=0;i<all_contacts_summaries.size();i++)
			{
				const RadicalTessellation::ContactDescriptorSummary& cds=all_contacts_summaries[i];
				if(cds.id_a<state_.result.contacts_summaries_with_redundancy_in_periodic_box.size())
				{
					state_.result.contacts_summaries_with_redundancy_in_periodic_box[cds.id_a].push_back(cds);
				}
				if(cds.id_b<state_.result.contacts_summaries_with_redundancy_in_periodic_box.size())
				{
					state_.result.contacts_summaries_with_redundancy_in_periodic_box[cds.id_b].push_back(cds);
				}
			}
		}
		else
		{
			state_.result.contacts_summaries_with_redundancy_in_periodic_box.clear();
		}
	}

	struct State
	{
		SpheresContainer spheres_container;
		Result result;
		std::vector<UnsignedInt> ids_of_changed_input_spheres;
		std::vector<UnsignedInt> ids_of_affected_input_spheres;
		bool last_update_was_full_reinit;

		State() : last_update_was_full_reinit(true)
		{
		}

		void assign(const State& obj)
		{
			spheres_container.assign(obj.spheres_container);

			result.cells_summaries.resize(obj.result.cells_summaries.size());
			result.contacts_summaries.resize(obj.result.contacts_summaries.size());
			result.contacts_summaries_with_redundancy_in_periodic_box.resize(obj.result.contacts_summaries_with_redundancy_in_periodic_box.size());
			ids_of_changed_input_spheres.resize(obj.ids_of_changed_input_spheres.size());
			ids_of_affected_input_spheres.resize(obj.ids_of_affected_input_spheres.size());

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.result.cells_summaries.size();i++)
				{
					result.cells_summaries[i]=obj.result.cells_summaries[i];
				}
			}

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.result.contacts_summaries.size();i++)
				{
					result.contacts_summaries[i]=obj.result.contacts_summaries[i];
				}
			}

			if(!obj.result.contacts_summaries_with_redundancy_in_periodic_box.empty())
			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.result.contacts_summaries_with_redundancy_in_periodic_box.size();i++)
				{
					result.contacts_summaries_with_redundancy_in_periodic_box[i]=obj.result.contacts_summaries_with_redundancy_in_periodic_box[i];
				}
			}

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.ids_of_changed_input_spheres.size();i++)
				{
					ids_of_changed_input_spheres[i]=obj.ids_of_changed_input_spheres[i];
				}
			}

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.ids_of_affected_input_spheres.size();i++)
				{
					ids_of_affected_input_spheres[i]=obj.ids_of_affected_input_spheres[i];
				}
			}

			last_update_was_full_reinit=obj.last_update_was_full_reinit;
		}

		void assign(const State& obj, const bool assign_everything, const std::vector<UnsignedInt>& subset_of_ids_of_spheres)
		{
			if(!assign_everything && subset_of_ids_of_spheres.empty())
			{
				return;
			}

			if(assign_everything
					|| result.cells_summaries.size()!=obj.result.cells_summaries.size()
					|| result.contacts_summaries.size()!=obj.result.contacts_summaries.size()
					|| result.contacts_summaries_with_redundancy_in_periodic_box.size()!=obj.result.contacts_summaries_with_redundancy_in_periodic_box.size())
			{
				assign(obj);
				return;
			}

			const bool periodic=!obj.result.contacts_summaries_with_redundancy_in_periodic_box.empty();

			for(UnsignedInt i=0;i<subset_of_ids_of_spheres.size();i++)
			{
				const UnsignedInt sphere_id=subset_of_ids_of_spheres[i];
				if(sphere_id>=result.cells_summaries.size() || sphere_id>=result.cells_summaries.size() || (periodic && sphere_id>=result.contacts_summaries_with_redundancy_in_periodic_box.size()))
				{
					assign(obj);
					return;
				}
			}

			spheres_container.assign(obj.spheres_container, ids_of_changed_input_spheres);

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<subset_of_ids_of_spheres.size();i++)
				{
					const UnsignedInt sphere_id=subset_of_ids_of_spheres[i];
					result.cells_summaries[sphere_id]=obj.result.cells_summaries[sphere_id];
					result.contacts_summaries[sphere_id]=obj.result.contacts_summaries[sphere_id];
					if(periodic)
					{
						result.contacts_summaries_with_redundancy_in_periodic_box[sphere_id]=obj.result.contacts_summaries_with_redundancy_in_periodic_box[sphere_id];
					}
				}
			}

			ids_of_changed_input_spheres.resize(obj.ids_of_changed_input_spheres.size());
			ids_of_affected_input_spheres.resize(obj.ids_of_affected_input_spheres.size());

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.ids_of_changed_input_spheres.size();i++)
				{
					ids_of_changed_input_spheres[i]=obj.ids_of_changed_input_spheres[i];
				}
			}

			{
				#pragma omp parallel for
				for(UnsignedInt i=0;i<obj.ids_of_affected_input_spheres.size();i++)
				{
					ids_of_affected_input_spheres[i]=obj.ids_of_affected_input_spheres[i];
				}
			}

			last_update_was_full_reinit=obj.last_update_was_full_reinit;
		}

		void assign_to_undo_update(const State& obj)
		{
			assign(obj, last_update_was_full_reinit, ids_of_affected_input_spheres);
		}

		void assign_to_apply_update(const State& obj)
		{
			assign(obj, obj.last_update_was_full_reinit, obj.ids_of_affected_input_spheres);
		}
	};

	State state_;
	State state_backup_;
	bool undoable_;
	bool undone_;
	std::vector<int> involvement_of_spheres_for_update_;
	BufferedTemporaryStorage buffered_temporary_storage_;
};

}

#endif /* VORONOTALT_UPDATEABLE_RADICAL_TESSELLATION_H_ */
