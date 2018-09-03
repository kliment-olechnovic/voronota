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
		std::string name;
		bool enabled;
		DataManager* data_manager_ptr;

		ObjectDescriptor(
				const std::string& name,
				const bool enabled,
				DataManager& data_manager) :
					name(name),
					enabled(enabled),
					data_manager_ptr(&data_manager)
		{
		}

		DataManager& data_manager()
		{
			return (*data_manager_ptr);
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
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
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

	std::vector<ObjectDescriptor> get_descriptors(const bool only_enabled)
	{
		ensure_unique_names();
		std::vector<ObjectDescriptor> result;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(!only_enabled || it->enabled)
			{
				result.push_back(it->get_descriptor());
			}
		}
		return result;
	}

	std::vector<ObjectDescriptor> get_descriptors(const std::vector<std::string>& names)
	{
		ensure_unique_names();
		std::set<std::string> set_of_names(names.begin(), names.end());
		std::vector<ObjectDescriptor> result;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(set_of_names.count(it->name)>0)
			{
				result.push_back(it->get_descriptor());
			}
		}
		return result;
	}

	std::vector<ObjectDescriptor> get_descriptor(const std::string& name)
	{
		return get_descriptors(std::vector<std::string>(1, name));
	}

	std::vector<ObjectDescriptor> get_descriptor(DataManager* pointer)
	{
		std::vector<ObjectDescriptor> result;
		std::list<WrapperForDataManager>::iterator it=get_iterator(pointer);
		if(it!=all_data_managers_.end())
		{
			ensure_unique_names();
			result.push_back(it->get_descriptor());
		}
		return result;
	}

	ObjectDescriptor add_object(const DataManager& data_manager)
	{
		all_data_managers_.push_back(WrapperForDataManager(data_manager));
		return all_data_managers_.back().get_descriptor();
	}

	std::vector<ObjectDescriptor> delete_all_objects()
	{
		std::vector<ObjectDescriptor> result=get_descriptors(false);
		all_data_managers_.clear();
		return result;
	}

	std::vector<ObjectDescriptor> delete_object(const std::string& name)
	{
		std::vector<ObjectDescriptor> result=get_descriptor(name);
		if(!result.empty())
		{
			all_data_managers_.erase(get_iterator(result[0].data_manager_ptr));
		}
		return result;
	}

	std::vector<ObjectDescriptor> delete_object(DataManager* pointer)
	{
		std::vector<ObjectDescriptor> result=get_descriptor(pointer);
		if(!result.empty())
		{
			all_data_managers_.erase(get_iterator(result[0].data_manager_ptr));
		}
		return result;
	}

	void disable_all_objects()
	{
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			it->enabled=false;
		}
	}

	std::vector<ObjectDescriptor> enable_object(const std::string& name)
	{
		disable_all_objects();
		std::vector<ObjectDescriptor> result=get_descriptor(name);
		if(!result.empty())
		{
			get_iterator(result[0].data_manager_ptr)->enabled=true;
		}
		return result;
	}

	std::vector<ObjectDescriptor> enable_object(DataManager* pointer)
	{
		disable_all_objects();
		std::vector<ObjectDescriptor> result=get_descriptor(pointer);
		if(!result.empty())
		{
			get_iterator(result[0].data_manager_ptr)->enabled=true;
		}
		return result;
	}

private:
	struct WrapperForDataManager
	{
		bool enabled;
		DataManager data_manager;
		std::string name;

		explicit WrapperForDataManager(const DataManager& data_manager) :
			enabled(false),
			data_manager(data_manager)
		{
		}

		ObjectDescriptor get_descriptor()
		{
			return ObjectDescriptor(name, enabled, data_manager);
		}
	};

	void ensure_unique_names()
	{
		std::map<std::string, int> max_counters;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			const std::string& title=it->data_manager.title();
			max_counters[title]++;
		}
		std::map<std::string, int> current_counters;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
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

	std::list<WrapperForDataManager>::iterator get_iterator(DataManager* pointer)
	{
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(pointer==&(it->data_manager))
			{
				return it;
			}
		}
		return all_data_managers_.end();
	}

	std::list<WrapperForDataManager> all_data_managers_;
};

}

}

#endif /* COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
