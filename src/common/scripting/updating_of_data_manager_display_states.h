#ifndef COMMON_SCRIPTING_UPDATING_OF_DATA_MANAGER_DISPLAY_STATES_H_
#define COMMON_SCRIPTING_UPDATING_OF_DATA_MANAGER_DISPLAY_STATES_H_

#include "../../auxiliaries/color_utilities.h"

#include "data_manager.h"

namespace common
{

namespace scripting
{

class UpdatingOfDataManagerDisplayStates
{
public:
	struct Parameters
	{
		bool mark;
		bool unmark;
		bool show;
		bool hide;
		auxiliaries::ColorUtilities::ColorInteger color;
		std::set<std::size_t> visual_ids;

		Parameters() :
			mark(false),
			unmark(false),
			show(false),
			hide(false),
			color(auxiliaries::ColorUtilities::null_color())
		{
		}

		bool color_valid() const
		{
			return auxiliaries::ColorUtilities::color_valid(color);
		}

		void assert_correctness() const
		{
			if(hide && show)
			{
				throw std::runtime_error(std::string("Cannot show and hide at the same time."));
			}

			if(mark && unmark)
			{
				throw std::runtime_error(std::string("Cannot mark and unmark at the same time."));
			}
		}
	};

	static bool update_display_state(const Parameters& params, const std::size_t id, std::vector<DataManager::DisplayState>& display_states)
	{
		bool updated=false;
		if((params.show || params.hide || params.mark || params.unmark || params.color_valid()) && id<display_states.size())
		{
			DataManager::DisplayState& ds=display_states[id];

			if(params.mark || params.unmark)
			{
				updated=(updated || (ds.marked!=params.mark));
				ds.marked=params.mark;
			}

			if(ds.implemented())
			{
				if(params.show || params.hide || params.color_valid())
				{
					if(params.visual_ids.empty())
					{
						for(std::size_t i=0;i<ds.visuals.size();i++)
						{
							if(update_display_state_visual(params, ds.visuals[i]))
							{
								updated=true;
							}
						}
					}
					else
					{
						for(std::set<std::size_t>::const_iterator jt=params.visual_ids.begin();jt!=params.visual_ids.end();++jt)
						{
							const std::size_t visual_id=(*jt);
							if(visual_id<ds.visuals.size())
							{
								if(update_display_state_visual(params, ds.visuals[visual_id]))
								{
									updated=true;
								}
							}
						}
					}
				}
			}
		}
		return updated;
	}

	static bool update_display_states(const Parameters& params, const std::set<std::size_t>& ids, std::vector<DataManager::DisplayState>& display_states)
	{
		bool updated=false;
		if(params.show || params.hide || params.mark || params.unmark || params.color_valid())
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				if(update_display_state(params, (*it), display_states))
				{
					updated=true;
				}
			}
		}
		return updated;
	}

	static bool update_display_states(const Parameters& params, std::vector<DataManager::DisplayState>& display_states)
	{
		bool updated=false;
		if(params.show || params.hide || params.mark || params.unmark || params.color_valid())
		{
			for(std::size_t i=0;i<display_states.size();i++)
			{
				if(update_display_state(params, i, display_states))
				{
					updated=true;
				}
			}
		}
		return updated;
	}

private:
	static bool update_display_state_visual(const Parameters& params, DataManager::DisplayState::Visual& visual)
	{
		bool updated=false;

		if(visual.implemented)
		{
			if(params.show || params.hide)
			{
				updated=(updated || (visual.visible!=params.show));
				visual.visible=params.show;
			}

			if(params.color_valid())
			{
				updated=(updated || (visual.color!=params.color));
				visual.color=params.color;
			}
		}

		return updated;
	}
};

}

}

#endif /* COMMON_SCRIPTING_UPDATING_OF_DATA_MANAGER_DISPLAY_STATES_H_ */

