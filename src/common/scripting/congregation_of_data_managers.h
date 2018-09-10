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
	struct ObjectDescriptor
	{
		DataManager data_manager;
		std::string name;
		bool picked;

		ObjectDescriptor(const DataManager& data_manager, const std::string& name) :
			data_manager(data_manager),
			name(name),
			picked(false)
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

	std::vector<ObjectDescriptor*> get_descriptors(const bool only_picked)
	{
		std::vector<ObjectDescriptor*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(!only_picked || it->picked)
			{
				result.push_back(&(*it));
			}
		}
		return result;
	}

	std::vector<ObjectDescriptor*> get_descriptors(const std::vector<std::string>& names)
	{
		std::set<std::string> set_of_names(names.begin(), names.end());
		std::vector<ObjectDescriptor*> result;
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(set_of_names.count(it->name)>0)
			{
				result.push_back(&(*it));
			}
		}
		return result;
	}

	ObjectDescriptor* get_descriptor(const std::string& name)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(name==it->name)
			{
				return (&(*it));
			}
		}
		return 0;
	}

	ObjectDescriptor* get_descriptor(DataManager* data_manager_ptr)
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			if(data_manager_ptr==&it->data_manager)
			{
				return (&(*it));
			}
		}
		return 0;
	}

	ObjectDescriptor* add_object(const DataManager& data_manager, const std::string& name)
	{
		objects_.push_back(ObjectDescriptor(data_manager, unique_name(name)));
		return (&objects_.back());
	}

	ObjectDescriptor* rename_object(const std::string& current_name, const std::string& new_name)
	{
		ObjectDescriptor* result=get_descriptor(current_name);
		if(result!=0)
		{
			if(new_name!=current_name)
			{
				result->name=unique_name(new_name);
			}
			return result;
		}
		return 0;
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

	DataManager* delete_object(const std::string& name)
	{
		std::list<ObjectDescriptor>::iterator it=objects_.begin();
		while(it!=objects_.end())
		{
			if(it->name==name)
			{
				DataManager* result=&it->data_manager;
				objects_.erase(it++);
				return result;
			}
			else
			{
				++it;
			}
		}
		return 0;
	}

	void unpick_all_objects()
	{
		for(std::list<ObjectDescriptor>::iterator it=objects_.begin();it!=objects_.end();++it)
		{
			it->picked=false;
		}
	}

	ObjectDescriptor* pick_object(const std::string& name)
	{
		unpick_all_objects();
		ObjectDescriptor* result=get_descriptor(name);
		if(result!=0)
		{
			result->picked=true;
		}
		return result;
	}

	ObjectDescriptor* pick_object(DataManager* data_manager_ptr)
	{
		unpick_all_objects();
		ObjectDescriptor* result=get_descriptor(data_manager_ptr);
		if(result!=0)
		{
			result->picked=true;
		}
		return result;
	}

private:
	std::string unique_name(const std::string& name)
	{
		std::string candidate=name;
		ObjectDescriptor* present_object=get_descriptor(candidate);
		int i=1;
		while(present_object!=0)
		{
			i++;
			std::ostringstream output;
			output << name << "_" << i;
			candidate=output.str();
			present_object=get_descriptor(candidate);
		}
		return candidate;
	}

	std::list<ObjectDescriptor> objects_;
};

}

}

#endif /* COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
