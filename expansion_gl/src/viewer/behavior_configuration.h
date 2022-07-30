#ifndef VIEWER_BEHAVIOR_CONFIGURATION_H_
#define VIEWER_BEHAVIOR_CONFIGURATION_H_

#include <string>

namespace voronota
{

namespace viewer
{

class BehaviorConfiguration
{
public:
	std::string initial_atom_representation_to_show_after_loading;
	std::string initial_heteroatom_representation_to_show_after_loading;

	static BehaviorConfiguration& instance()
	{
		static BehaviorConfiguration bc;
		return bc;
	}

private:
	BehaviorConfiguration() :
		initial_atom_representation_to_show_after_loading("cartoon"),
		initial_heteroatom_representation_to_show_after_loading("sticks")
	{
	}
};

}

}


#endif /* VIEWER_BEHAVIOR_CONFIGURATION_H_ */
