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
		bool picked;
		std::string name;

		explicit ObjectDescriptor(const DataManager& data_manager) :
			data_manager(data_manager),
			picked(false)
		{
		}
	};

	CongregationOfDataManagers()
	{
	}

	void assert_objects_availability() const
	{
		if(all_data_managers_.empty())
		{
			throw std::runtime_error(std::string("No objects available."));
		}
	}

	void assert_objects_availability(const std::vector<std::string>& names)
	{
		ensure_unique_names();
		std::map<std::string, bool> map_of_names;
		for(std::size_t i=0;i<names.size();i++)
		{
			map_of_names[names[i]]=false;
		}
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
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

	void assert_object_availability(const std::string& name)
	{
		assert_objects_availability(std::vector<std::string>(1, name));
	}

	void assert_object_availability(DataManager* pointer)
	{
		if(get_iterator(pointer)==all_data_managers_.end())
		{
			throw std::runtime_error(std::string("Invalid object."));
		}
	}

	std::vector<ObjectDescriptor*> get_descriptors(const bool only_picked)
	{
		ensure_unique_names();
		std::vector<ObjectDescriptor*> result;
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
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
		ensure_unique_names();
		std::set<std::string> set_of_names(names.begin(), names.end());
		std::vector<ObjectDescriptor*> result;
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(set_of_names.count(it->name)>0)
			{
				result.push_back(&(*it));
			}
		}
		return result;
	}

	std::vector<ObjectDescriptor*> get_descriptor(const std::string& name)
	{
		return get_descriptors(std::vector<std::string>(1, name));
	}

	std::vector<ObjectDescriptor*> get_descriptor(DataManager* pointer)
	{
		std::vector<ObjectDescriptor*> result;
		std::list<ObjectDescriptor>::iterator it=get_iterator(pointer);
		if(it!=all_data_managers_.end())
		{
			ensure_unique_names();
			result.push_back(&(*it));
		}
		return result;
	}

	ObjectDescriptor* add_object(const DataManager& data_manager)
	{
		all_data_managers_.push_back(ObjectDescriptor(data_manager));
		return (&all_data_managers_.back());
	}

	std::vector<ObjectDescriptor*> delete_all_objects()
	{
		std::vector<ObjectDescriptor*> result=get_descriptors(false);
		all_data_managers_.clear();
		return result;
	}

	std::vector<ObjectDescriptor*> delete_object(const std::string& name)
	{
		std::vector<ObjectDescriptor*> result=get_descriptor(name);
		if(!result.empty())
		{
			all_data_managers_.erase(get_iterator(&result[0]->data_manager));
		}
		return result;
	}

	std::vector<ObjectDescriptor*> delete_object(DataManager* pointer)
	{
		std::vector<ObjectDescriptor*> result=get_descriptor(pointer);
		if(!result.empty())
		{
			all_data_managers_.erase(get_iterator(&result[0]->data_manager));
		}
		return result;
	}

	void unpick_all_objects()
	{
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			it->picked=false;
		}
	}

	std::vector<ObjectDescriptor*> pick_object(const std::string& name)
	{
		unpick_all_objects();
		std::vector<ObjectDescriptor*> result=get_descriptor(name);
		if(!result.empty())
		{
			get_iterator(&result[0]->data_manager)->picked=true;
		}
		return result;
	}

	std::vector<ObjectDescriptor*> pick_object(DataManager* pointer)
	{
		unpick_all_objects();
		std::vector<ObjectDescriptor*> result=get_descriptor(pointer);
		if(!result.empty())
		{
			get_iterator(&result[0]->data_manager)->picked=true;
		}
		return result;
	}

private:
	void ensure_unique_names()
	{
		std::map<std::string, int> max_counters;
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			const std::string& title=it->data_manager.title();
			max_counters[title]++;
		}
		std::map<std::string, int> current_counters;
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			const std::string& title=it->data_manager.title();
			if(max_counters[title]<=1)
			{
				it->name=title;
			}
			else
			{
				std::ostringstream output;
				output << title << "[" << (current_counters[title]++) << "]";
				it->name=output.str();
			}
		}
	}

	std::list<ObjectDescriptor>::iterator get_iterator(DataManager* pointer)
	{
		for(std::list<ObjectDescriptor>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(pointer==&(it->data_manager))
			{
				return it;
			}
		}
		return all_data_managers_.end();
	}

	std::list<ObjectDescriptor> all_data_managers_;
};

}

}

#endif /* COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
