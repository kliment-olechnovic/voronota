#ifndef SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class CongregationOfDataManagers
{
public:
	class ChangeIndicator
	{
	public:
		ChangeIndicator() :
			changed_objects_(false),
			changed_objects_names_(false),
			changed_objects_picks_(false),
			changed_objects_visibilities_(false)
		{
		}

		bool changed_objects() const { return changed_objects_; }
		bool changed_objects_names() const { return changed_objects_names_; }
		bool changed_objects_picks() const { return changed_objects_picks_; }
		bool changed_objects_visibilities() const { return changed_objects_visibilities_; }
		const std::set<DataManager*>& added_objects() const { return added_objects_; }
		const std::set<DataManager*>& deleted_objects() const { return deleted_objects_; }

		void set_changed_objects(const bool value) { changed_objects_=value; ensure_correctness(); }
		void set_changed_objects_names(const bool value) { changed_objects_names_=value; ensure_correctness(); }
		void set_changed_objects_picks(const bool value) { changed_objects_picks_=value; ensure_correctness(); }
		void set_changed_objects_visibilities(const bool value) { changed_objects_visibilities_=value; ensure_correctness(); }

		void add_to_added_objects(DataManager* value)
		{
			added_objects_.insert(value);
			ensure_correctness();
		}

		void add_to_deleted_objects(DataManager* value)
		{
			if(added_objects_.count(value)>0)
			{
				added_objects_.erase(value);
			}
			else
			{
				deleted_objects_.insert(value);
			}
			ensure_correctness();
		}

		bool changed() const
		{
			return (changed_objects_
					|| changed_objects_names_
					|| changed_objects_picks_
					|| changed_objects_visibilities_
					|| !added_objects_.empty()
					|| !deleted_objects_.empty());
		}

		bool only_changed_objects_picks() const
		{
			return (!changed_objects_
					&& !changed_objects_names_
					&& changed_objects_picks_
					&& !changed_objects_visibilities_
					&& added_objects_.empty()
					&& deleted_objects_.empty());
		}

		bool only_changed_objects_visibilities() const
		{
			return (!changed_objects_
					&& !changed_objects_names_
					&& !changed_objects_picks_
					&& changed_objects_visibilities_
					&& added_objects_.empty()
					&& deleted_objects_.empty());
		}

		bool only_changed_objects_picks_or_visibilities() const
		{
			return (only_changed_objects_picks() || only_changed_objects_visibilities());
		}

	private:
		void ensure_correctness()
		{
			changed_objects_=(changed_objects_ || !added_objects_.empty() || !deleted_objects_.empty());
			changed_objects_names_=(changed_objects_names_ || changed_objects_);
			changed_objects_picks_=(changed_objects_picks_ || changed_objects_);
			changed_objects_visibilities_=(changed_objects_visibilities_ || changed_objects_);
		}

		bool changed_objects_;
		bool changed_objects_names_;
		bool changed_objects_picks_;
		bool changed_objects_visibilities_;
		std::set<DataManager*> added_objects_;
		std::set<DataManager*> deleted_objects_;
	};

	struct ObjectAttributes
	{
		std::string name;
		bool valid;
		bool picked;
		bool visible;

		ObjectAttributes() :
			valid(false),
			picked(false),
			visible(false)
		{
		}

		explicit ObjectAttributes(const std::string& name) :
			name(name),
			valid(true),
			picked(false),
			visible(true)
		{
		}
	};

	struct ObjectQuery
	{
		bool picked;
		bool not_picked;
		bool visible;
		bool not_visible;
		std::set<std::string> names;

		ObjectQuery() :
			picked(false),
			not_picked(false),
			visible(false),
			not_visible(false)
		{
		}
	};

	CongregationOfDataManagers() : next_reversable_(false), next_currently_reversed_(false)
	{
	}

	const ChangeIndicator& change_indicator() const
	{
		return change_indicator_;
	}

	void assert_objects_availability() const
	{
		if(objects_.empty())
		{
			throw std::runtime_error(std::string("No objects available."));
		}
	}

	void assert_objects_availability(const std::vector<std::string>& names) const
	{
		std::map<std::string, bool> map_of_names;
		for(std::size_t i=0;i<names.size();i++)
		{
			map_of_names[names[i]]=false;
		}
		for(std::list<ObjectDescriptor>::const_iterator it=objects_.begin();it!=objects_.end();++it)
		{
			map_of_names[it->attributes.name]=true;
		}
		for(std::map<std::string, bool>::const_iterator it=map_of_names.begin();it!=map_of_names.end();++it)
		{
			if(!it->second)
			{
				throw std::runtime_error(std::string("Invalid object name '")+(it->first)+"'.");
			}
		}
	}

	void assert_object_availability(const std::string& name) const
	{
		assert_objects_availability(std::vector<std::string>(1, name));
	}

	unsigned int count_objects(const bool only_picked, const bool only_visible) const
	{
		unsigned int counter=0;
		for(std::list<ObjectDescriptor>::const_iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if((!only_picked || it->attributes.picked) && (!only_visible || it->attributes.visible))
			{
				counter++;
			}
		}
		return counter;
	}

	void reset_change_indicator()
	{
		change_indicator_=ChangeIndicator();
	}

	void reset_change_indicators_of_all_objects()
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			it->data_manager.reset_change_indicator();
		}
	}

	void sync_selections_with_display_states_of_all_objects_if_requested_in_string(const std::string& request)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			it->data_manager.sync_selections_with_display_states_if_requested_in_string(request);
		}
	}

	std::vector<DataManager*> get_objects(const ObjectQuery& query)
	{
		std::vector<DataManager*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(
					(!query.picked || it->attributes.picked) &&
					(!query.not_picked || !it->attributes.picked) &&
					(!query.visible || it->attributes.visible) &&
					(!query.not_visible || !it->attributes.visible) &&
					(query.names.empty() || query.names.count(it->attributes.name)>0)
				)
			{
				result.push_back(&it->data_manager);
			}
		}
		return result;
	}

	std::vector<DataManager*> get_objects()
	{
		return get_objects(ObjectQuery());
	}

	DataManager* get_object(const std::string& name)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(name==it->attributes.name)
			{
				return (&it->data_manager);
			}
		}
		return 0;
	}

	ObjectAttributes get_object_attributes(DataManager* id)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			return (it->attributes);
		}
		return ObjectAttributes();
	}

	ObjectAttributes get_object_attributes(const std::string& name)
	{
		return get_object_attributes(get_object(name));
	}

	DataManager* add_object(const DataManager& data_manager, const std::string& name)
	{
		objects_.push_back(ObjectDescriptor(data_manager, unique_name(name)));
		change_indicator_.add_to_added_objects(&objects_.back().data_manager);
		return (&objects_.back().data_manager);
	}

	DataManager* rename_object(DataManager* id, const std::string& new_name)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			if(new_name!=it->attributes.name)
			{
				change_indicator_.set_changed_objects_names(true);
				it->attributes.name=unique_name(new_name);
			}
			return (&it->data_manager);
		}
		return 0;
	}

	DataManager* rename_object(const std::string& current_name, const std::string& new_name)
	{
		return rename_object(get_object(current_name), new_name);
	}

	std::vector<DataManager*> delete_all_objects()
	{
		std::vector<DataManager*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			result.push_back(&it->data_manager);
			change_indicator_.add_to_deleted_objects(&it->data_manager);
		}
		objects_.clear();
		return result;
	}

	DataManager* delete_object(DataManager* id)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			DataManager* result=&it->data_manager;
			change_indicator_.add_to_deleted_objects(result);
			objects_.erase(it++);
			return result;
		}
		return 0;
	}

	DataManager* delete_object(const std::string& name)
	{
		return delete_object(get_object(name));
	}

	void set_all_objects_picked(const bool picked)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(it->attributes.picked!=picked)
			{
				change_indicator_.set_changed_objects_picks(true);
				it->attributes.picked=picked;
			}
		}
	}

	bool set_object_picked(DataManager* id, const bool picked)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			if(it->attributes.picked!=picked)
			{
				change_indicator_.set_changed_objects_picks(true);
				it->attributes.picked=picked;
			}
			return true;
		}
		return false;
	}

	bool set_object_picked(const std::string& name, const bool picked)
	{
		return set_object_picked(get_object(name), picked);
	}

	void set_all_objects_visible(const bool visible)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(it->attributes.visible!=visible)
			{
				change_indicator_.set_changed_objects_visibilities(true);
				it->attributes.visible=visible;
			}
		}
	}

	bool set_object_visible(DataManager* id, const bool visible)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			if(it->attributes.visible!=visible)
			{
				change_indicator_.set_changed_objects_visibilities(true);
				it->attributes.visible=visible;
			}
			return true;
		}
		return false;
	}

	bool set_object_visible(const std::string& name, const bool visible)
	{
		return set_object_visible(get_object(name), visible);
	}

	bool set_next_picked_object_visible(const bool next_reversable)
	{
		if(!next_reversable || next_reversable!=next_reversable_)
		{
			next_currently_reversed_=false;
		}
		next_reversable_=next_reversable;

		bool something_picked=false;
		bool something_was_visible=false;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			something_picked=something_picked || it->attributes.picked;
			something_was_visible=something_was_visible ||it->attributes.visible;
		}

		if(!something_picked)
		{
			if(something_was_visible)
			{
				for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
				{
					it->attributes.visible=false;
				}
				change_indicator_.set_changed_objects_visibilities(something_was_visible);
			}
			return false;
		}

		std::list<ObjectDescriptor>::iterator it_first_picked=objects_.end();
		std::list<ObjectDescriptor>::iterator it_second_picked=objects_.end();
		std::list<ObjectDescriptor>::iterator it_first_picked_visible=objects_.end();
		std::list<ObjectDescriptor>::iterator it_second_picked_after_first_picked_visible=objects_.end();

		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(it->attributes.picked)
			{
				if(it_first_picked==objects_.end())
				{
					it_first_picked=it;
				}

				if(it_first_picked!=objects_.end() && it_second_picked==objects_.end())
				{
					it_second_picked=it;
				}

				if(it_first_picked_visible==objects_.end())
				{
					if(it->attributes.visible)
					{
						it_first_picked_visible=it;
					}
				}
				else if(it_second_picked_after_first_picked_visible==objects_.end())
				{
					it_second_picked_after_first_picked_visible=it;
				}
			}
		}

		std::list<ObjectDescriptor>::reverse_iterator rev_it_first_picked=objects_.rend();
		std::list<ObjectDescriptor>::reverse_iterator rev_it_second_picked=objects_.rend();
		std::list<ObjectDescriptor>::reverse_iterator rev_it_first_picked_visible=objects_.rend();
		std::list<ObjectDescriptor>::reverse_iterator rev_it_second_picked_after_first_picked_visible=objects_.rend();

		for(std::list<ObjectDescriptor>::reverse_iterator it=objects_.rbegin();it!=objects_.rend();++it)
		{
			if(it->attributes.picked)
			{
				if(rev_it_first_picked==objects_.rend())
				{
					rev_it_first_picked=it;
				}

				if(rev_it_first_picked!=objects_.rend() && rev_it_second_picked==objects_.rend())
				{
					rev_it_second_picked=it;
				}

				if(rev_it_first_picked_visible==objects_.rend())
				{
					if(it->attributes.visible)
					{
						rev_it_first_picked_visible=it;
					}
				}
				else if(rev_it_second_picked_after_first_picked_visible==objects_.rend())
				{
					rev_it_second_picked_after_first_picked_visible=it;
				}
			}
		}

		if(something_was_visible)
		{
			for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
			{
				it->attributes.visible=false;
			}
			change_indicator_.set_changed_objects_visibilities(something_was_visible);
		}

		if(next_currently_reversed_)
		{
			if(rev_it_first_picked_visible!=objects_.rend() && rev_it_second_picked_after_first_picked_visible!=objects_.rend())
			{
				rev_it_second_picked_after_first_picked_visible->attributes.visible=true;
				return true;
			}
			else
			{
				next_currently_reversed_=false;
				if(it_second_picked!=objects_.end())
				{
					it_second_picked->attributes.visible=true;
					return true;
				}
			}
		}
		else
		{
			if(it_first_picked_visible!=objects_.end() && it_second_picked_after_first_picked_visible!=objects_.end())
			{
				it_second_picked_after_first_picked_visible->attributes.visible=true;
				return true;
			}
			else
			{
				if(next_reversable_)
				{
					if(rev_it_second_picked!=objects_.rend())
					{
						next_currently_reversed_=true;
						rev_it_second_picked->attributes.visible=true;
						return true;
					}
				}
				else
				{
					if(it_first_picked!=objects_.end())
					{
						it_first_picked->attributes.visible=true;
						return true;
					}
				}
			}
		}

		return false;
	}

	bool check_if_saveable_to_stream(const ObjectQuery& query)
	{
		std::vector<DataManager*> objects=get_objects(query);
		if(objects.empty())
		{
			return false;
		}
		for(std::size_t i=0;i<objects.size();i++)
		{
			if(!(objects[i]->is_saveable_to_stream()))
			{
				return false;
			}
		}
		return true;
	}

	bool save_to_stream(const ObjectQuery& query, std::ostream& output)
	{
		std::vector<DataManager*> objects=get_objects(query);
		if(objects.empty())
		{
			return false;
		}
		for(std::size_t i=0;i<objects.size();i++)
		{
			if(!(objects[i]->is_saveable_to_stream()))
			{
				return false;
			}
		}

		output << "SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_EXPORTED\n";

		output << objects.size() << "\n";
		for(std::size_t i=0;i<objects.size();i++)
		{
			const ObjectAttributes attributes=get_object_attributes(objects[i]);
			output << attributes.name << " " << attributes.valid << " " << attributes.picked << " " << attributes.visible << "\n";
			objects[i]->save_to_stream(output);
		}

		return true;
	}

	std::vector<DataManager*> load_from_stream(std::istream& input)
	{
		bool good_start=false;
		while(!good_start && input.good())
		{
			std::string token;
			input >> token;
			good_start=(token=="SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_EXPORTED");
		}
		if(!good_start)
		{
			throw std::runtime_error(std::string("Invalid start of objects stream."));
		}

		int count=0;
		input >> count;
		if(count<=0)
		{
			throw std::runtime_error(std::string("No objects when loading from stream."));
		}
		AutodeleterOfObjects auotodeleter(*this);
		for(int i=0;i<count;i++)
		{
			ObjectAttributes attributes;
			input >> attributes.name >> attributes.valid >> attributes.picked >> attributes.visible;
			DataManager* object_new=add_object(DataManager(), attributes.name);
			auotodeleter.objects.push_back(object_new);
			DataManager& data_manager=*object_new;
			data_manager.load_from_stream(input);
			set_object_picked(object_new, attributes.picked);
			set_object_visible(object_new, attributes.visible);
		}
		std::vector<DataManager*> result;
		result.swap(auotodeleter.objects);
		return result;
	}

private:
	struct ObjectDescriptor
	{
		DataManager data_manager;
		ObjectAttributes attributes;

		ObjectDescriptor(const DataManager& data_manager, const std::string& name) :
			data_manager(data_manager),
			attributes(name)
		{
		}
	};

	class AutodeleterOfObjects
	{
	public:
		CongregationOfDataManagers& cdm;
		std::vector<DataManager*> objects;

		AutodeleterOfObjects(CongregationOfDataManagers& cdm) : cdm(cdm)
		{
		}

		~AutodeleterOfObjects()
		{
			for(std::size_t i=0;i<objects.size();i++)
			{
				cdm.delete_object(objects[i]);
			}
		}
	};

	std::list<ObjectDescriptor>::iterator get_iterator(DataManager* id)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(id==&it->data_manager)
			{
				return it;
			}
		}
		return objects_.end();
	}

	std::string unique_name(const std::string& name)
	{
		if(name.empty())
		{
			return unique_name(std::string("obj"));
		}
		std::string candidate=name;
		DataManager* present_id=get_object(candidate);
		int i=1;
		while(present_id!=0)
		{
			i++;
			std::ostringstream output;
			output << name << "_" << i;
			candidate=output.str();
			present_id=get_object(candidate);
		}
		return candidate;
	}

	std::list<ObjectDescriptor> objects_;
	ChangeIndicator change_indicator_;
	bool next_reversable_;
	bool next_currently_reversed_;
};

}

}

#endif /* SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
