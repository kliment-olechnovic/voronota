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
	};

	CongregationOfDataManagers()
	{
	}

	std::vector<ObjectDescriptor> get_descriptors(const bool only_enabled)
	{
		ensure_unique_names();
		std::vector<ObjectDescriptor> result;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(!only_enabled || it->enabled)
			{
				result.push_back(ObjectDescriptor(it->name, it->enabled, it->data_manager));
			}
		}
		return result;
	}

	std::vector<ObjectDescriptor> get_descriptor(const std::string& name)
	{
		ensure_unique_names();
		std::vector<ObjectDescriptor> result;
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(name==it->name)
			{
				result.push_back(ObjectDescriptor(it->name, it->enabled, it->data_manager));
				return result;
			}
		}
		return result;
	}

	void add_object(const DataManager& data_manager)
	{
		all_data_managers_.push_back(WrapperForDataManager(data_manager));
	}

	void delete_object(const std::string& name)
	{
		ensure_unique_names();
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(name==it->name)
			{
				all_data_managers_.erase(it);
				return;
			}
		}
	}

	void disable_objects()
	{
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			it->enabled=false;
		}
	}

	void enable_object(const std::string& name)
	{
		ensure_unique_names();
		disable_objects();
		for(std::list<WrapperForDataManager>::iterator it=all_data_managers_.begin();it!=all_data_managers_.end();++it)
		{
			if(name==it->name)
			{
				it->enabled=true;
				return;
			}
		}
	}

private:
	struct WrapperForDataManager
	{
		bool enabled;
		DataManager data_manager;
		std::string name;

		WrapperForDataManager(const DataManager& data_manager) :
			enabled(false),
			data_manager(data_manager)
		{
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

	std::list<WrapperForDataManager> all_data_managers_;
};

}

}

#endif /* COMMON_SCRIPTING_CONGREGATION_OF_DATA_MANAGERS_H_ */
