#ifndef COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_

#include "data_manager.h"

namespace common
{

namespace scripting
{

class CongregationOfDataManagers
{
public:
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
			map_of_names[it->name]=true;
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

	std::vector<DataManager*> get_objects(const bool only_picked, const bool only_visible)
	{
		std::vector<DataManager*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if((!only_picked || it->picked) && (!only_visible || it->visible))
			{
				result.push_back(&it->data_manager);
			}
		}
		return result;
	}

	std::vector<DataManager*> get_objects(const std::vector<std::string>& names)
	{
		std::set<std::string> set_of_names(names.begin(), names.end());
		std::vector<DataManager*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(set_of_names.count(it->name)>0)
			{
				result.push_back(&it->data_manager);
			}
		}
		return result;
	}

	DataManager* get_object(const std::string& name)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(name==it->name)
			{
				return (&it->data_manager);
			}
		}
		return 0;
	}

	std::string get_object_name(DataManager* id)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			return (it->name);
		}
		return std::string();
	}

	bool get_object_picked(DataManager* id)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			return (it->picked);
		}
		return false;
	}

	bool get_object_picked(const std::string& name)
	{
		return get_object_picked(get_object(name));
	}

	bool get_object_visible(DataManager* id)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			return (it->visible);
		}
		return false;
	}

	bool get_object_visible(const std::string& name)
	{
		return get_object_visible(get_object(name));
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
			if(new_name!=it->name)
			{
				it->name=unique_name(new_name);
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

	void unpick_all_objects()
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			it->picked=false;
		}
	}

	bool pick_object(DataManager* id)
	{
		unpick_all_objects();
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			it->picked=true;
			return true;
		}
		return false;
	}

	bool pick_object(const std::string& name)
	{
		return pick_object(get_object(name));
	}

	void set_all_objects_visible(const bool visible)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			it->visible=visible;
		}
	}

	bool set_object_visible(DataManager* id, const bool visible)
	{
		std::list<ObjectDescriptor>::iterator it=get_iterator(id);
		if(it!=objects_.end())
		{
			it->visible=visible;
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
		std::string name;
		bool picked;
		bool visible;

		ObjectDescriptor(const DataManager& data_manager, const std::string& name) :
			data_manager(data_manager),
			name(name),
			picked(false),
			visible(true)
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

}

#endif /* COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
