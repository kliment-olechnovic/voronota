#ifndef VIEWER_CONSOLE_OBJECTS_INFO_H_
#define VIEWER_CONSOLE_OBJECTS_INFO_H_

#include <string>
#include <vector>
#include <map>

namespace voronota
{

namespace viewer
{

namespace console
{

class ObjectsInfo
{
public:
	struct ObjectState
	{
		std::string name;
		bool picked;
		bool visible;
	};

	struct ObjectSequenceInfo
	{
		struct ResidueInfo
		{
			std::string name;
			std::string num_label;
			int num;
			bool marked;
			float rgb[3];

			ResidueInfo() : num(0), marked(false)
			{
			}

			std::size_t display_size() const
			{
				return std::max(name.size(), num_label.size());
			}
		};

		struct ChainInfo
		{
			std::string name;
			std::vector<ResidueInfo> residues;
		};

		std::vector<ChainInfo> chains;

		bool empty() const
		{
			return chains.empty();
		}
	};

	struct ObjectDetails
	{
		ObjectSequenceInfo sequence;
	};

	ObjectsInfo() : num_of_picked_objects_(0), num_of_visible_objects_(0)
	{
	}

	const std::vector<ObjectState>& get_object_states() const
	{
		return object_states_;
	}

	const std::map<std::string, ObjectDetails>& get_object_details() const
	{
		return object_details_;
	}

	int num_of_picked_objects() const
	{
		return num_of_picked_objects_;
	}

	int num_of_visible_objects() const
	{
		return num_of_visible_objects_;
	}

	bool object_has_details(const std::string& name) const
	{
		return (object_details_.count(name)>0);
	}

	void set_object_states(const std::vector<ObjectState>& object_states, const bool preserve_details_if_all_names_match)
	{
		if(object_states.empty())
		{
			object_details_.clear();
			object_states_.clear();
		}
		else
		{
			if(preserve_details_if_all_names_match)
			{
				bool same_names=(object_states.size()==object_states_.size());
				for(std::size_t i=0;same_names && i<object_states.size();i++)
				{
					same_names=(same_names && object_states[i].name==object_states_[i].name);
				}
				if(!same_names)
				{
					object_details_.clear();
				}
			}
			else
			{
				object_details_.clear();
			}
			object_states_=object_states;
		}
		count_objects();
	}

	void set_object_sequence_info(const std::string& name, const ObjectSequenceInfo& sequence)
	{
		bool found_name=false;
		for(std::size_t i=0;!found_name && i<object_states_.size();i++)
		{
			found_name=(found_name || name==object_states_[i].name);
		}
		if(found_name)
		{
			object_details_[name].sequence=sequence;
		}
	}
private:
	void count_objects()
	{
		num_of_picked_objects_=0;
		num_of_visible_objects_=0;
		for(std::size_t i=0;i<object_states_.size();i++)
		{
			const ObjectState& os=object_states_[i];
			if(os.picked)
			{
				num_of_picked_objects_++;
			}
			if(os.visible)
			{
				num_of_visible_objects_++;
			}
		}
	}

	std::vector<ObjectState> object_states_;
	std::map<std::string, ObjectDetails> object_details_;
	int num_of_picked_objects_;
	int num_of_visible_objects_;
};

}

}

}

#endif /* VIEWER_CONSOLE_OBJECTS_INFO_H_ */
