#ifndef COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "data_manager.h"
#include "change_indicator_for_data_manager.h"

namespace common
{

namespace scripting
{

struct ChangeIndicatorForCongregationOfDataManagers
{
	bool changed_objects;
	bool changed_objects_names;
	bool changed_objects_picks;
	bool changed_objects_visibilities;
	std::set<DataManager*> added_objects;
	std::set<DataManager*> deleted_objects;
	std::map<DataManager*, ChangeIndicatorForDataManager> handled_objects;

	ChangeIndicatorForCongregationOfDataManagers() :
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
		for(std::map<DataManager*, ChangeIndicatorForDataManager>::const_iterator it=handled_objects.begin();it!=handled_objects.end();++it)
		{
			if(it->second.changed())
			{
				return true;
			}
		}
		return false;
	}
};

}

}

#endif /* COMMON_SCRIPTING_CHANGE_INDICATOR_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */

