#ifndef SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_
#define SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_

#include "data_manager.h"

namespace scripting
{

class CongregationOfDataManagers
{
public:
	struct ChangeIndicator
	{
		bool changed_objects;
		bool changed_objects_names;
		bool changed_objects_picks;
		bool changed_objects_visibilities;
		std::set<DataManager*> added_objects;
		std::set<DataManager*> deleted_objects;
		std::map<DataManager*, DataManager::ChangeIndicator> handled_objects;

		ChangeIndicator() :
			changed_objects(false),
			changed_objects_names(false),
			changed_objects_picks(false),
			changed_objects_visibilities(false)
		{
		}

		void ensure_correctness()
		{
			changed_objects=(changed_objects || !added_objects.empty() || !deleted_objects.empty());
			changed_objects_names=(changed_objects_names || changed_objects);
			changed_objects_picks=(changed_objects_picks || changed_objects);
			changed_objects_visibilities=(changed_objects_visibilities || changed_objects);
		}

		bool changed() const
		{
			return (changed_objects
					|| changed_objects_names
					|| changed_objects_picks
					|| changed_objects_visibilities
					|| !added_objects.empty()
					|| !deleted_objects.empty());
		}

		bool handled_objects_changed() const
		{
			for(std::map<DataManager*, DataManager::ChangeIndicator>::const_iterator it=handled_objects.begin();it!=handled_objects.end();++it)
			{
				if(it->second.changed())
				{
					return true;
				}
			}
			return false;
		}
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

	CongregationOfDataManagers()
	{
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
		return (&objects_.back().data_manager);
	}

	DataManager* rename_object(DataManager* id, const std::string& new_name)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			if(new_name!=it->attributes.name)
			{
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
			it->attributes.picked=picked;
		}
	}

	bool set_object_picked(DataManager* id, const bool picked)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			it->attributes.picked=picked;
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
			it->attributes.visible=visible;
		}
	}

	bool set_object_visible(DataManager* id, const bool visible)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			it->attributes.visible=visible;
			return true;
		}
		return false;
	}

	bool set_object_visible(const std::string& name, const bool visible)
	{
		return set_object_visible(get_object(name), visible);
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
};

}

#endif /* SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
