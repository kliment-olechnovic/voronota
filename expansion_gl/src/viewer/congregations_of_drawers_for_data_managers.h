#ifndef VIEWER_CONGREGATIONS_OF_DRAWERS_FOR_DATA_MANAGERS_H_
#define VIEWER_CONGREGATIONS_OF_DRAWERS_FOR_DATA_MANAGERS_H_

#include "drawer_for_data_manager.h"

namespace voronota
{

namespace viewer
{

class CongregationOfDrawersForDataManagers
{
public:
	struct DrawerElementID
	{
		DrawerForDataManager* drawer_ptr;
		DrawerForDataManager::ElementID element_id;

		DrawerElementID() : drawer_ptr(0)
		{
		}

		bool valid() const
		{
			return (drawer_ptr!=0 && element_id.valid());
		}
	};

	CongregationOfDrawersForDataManagers()
	{
	}

	~CongregationOfDrawersForDataManagers()
	{
		for(MapOfDrawers::iterator it=map_of_drawers_.begin();it!=map_of_drawers_.end();++it)
		{
			if(it->second!=0)
			{
				delete it->second;
			}
		}
	}

	std::vector<DrawerForDataManager*> get_objects()
	{
		std::vector<DrawerForDataManager*> result;
		for(MapOfDrawers::iterator it=map_of_drawers_.begin();it!=map_of_drawers_.end();++it)
		{
			result.push_back(it->second);
		}
		return result;
	}

	DrawerForDataManager* get_object(scripting::DataManager* data_manager_ptr)
	{
		MapOfDrawers::iterator it=map_of_drawers_.find(data_manager_ptr);
		if(it!=map_of_drawers_.end())
		{
			return it->second;
		}
		return 0;
	}

	bool add_object(scripting::DataManager& data_manager)
	{
		MapOfDrawers::iterator it=map_of_drawers_.find(&data_manager);
		if(it==map_of_drawers_.end())
		{
			DrawerForDataManager* drawer=new DrawerForDataManager(data_manager);
			map_of_drawers_[&data_manager]=drawer;
			return true;
		}
		return false;
	}

	bool delete_object(scripting::DataManager* data_manager_ptr)
	{
		MapOfDrawers::iterator it=map_of_drawers_.find(data_manager_ptr);
		if(it!=map_of_drawers_.end())
		{
			delete it->second;
			map_of_drawers_.erase(it);
			return true;
		}
		return false;
	}

	DrawerElementID resolve_drawing_id(const uv::DrawingID drawing_id)
	{
		for(MapOfDrawers::iterator it=map_of_drawers_.begin();it!=map_of_drawers_.end();++it)
		{
			DrawerElementID deid;
			deid.drawer_ptr=it->second;
			deid.element_id=deid.drawer_ptr->resolve_drawing_id(drawing_id);
			if(deid.valid())
			{
				return deid;
			}
		}
		return DrawerElementID();
	}

private:
	typedef std::map<scripting::DataManager*, DrawerForDataManager*> MapOfDrawers;

	MapOfDrawers map_of_drawers_;
};

}

}

#endif /* VIEWER_CONGREGATIONS_OF_DRAWERS_FOR_DATA_MANAGERS_H_ */

