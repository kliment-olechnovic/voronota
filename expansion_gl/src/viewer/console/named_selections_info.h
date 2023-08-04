#ifndef VIEWER_CONSOLE_NAMED_SELECTIONS_INFO_H_
#define VIEWER_CONSOLE_NAMED_SELECTIONS_INFO_H_

#include <string>
#include <vector>
#include <map>
#include <set>

namespace voronota
{

namespace viewer
{

namespace console
{

class NamedSelectionsInfo
{
public:
	class MappingOfNames
	{
	public:
		std::map< std::string, std::set<std::string> > selections_to_objects;
		std::map< std::string, std::set<std::string> > objects_to_selections;

		const bool empty() const
		{
			return selections_to_objects.empty();
		}

		void set_mapping(const std::string& object, const std::vector<std::string>& selections)
		{
			{
				std::map< std::string, std::set<std::string> >::iterator objects_to_selections_it=objects_to_selections.find(object);
				if(objects_to_selections_it==objects_to_selections.end())
				{
					if(!selections.empty())
					{
						objects_to_selections[object].insert(selections.begin(), selections.end());
					}
				}
				else
				{
					for(std::map< std::string, std::set<std::string> >::iterator it=selections_to_objects.begin();it!=selections_to_objects.end();)
					{
						it->second.erase(object);
						if(it->second.empty())
						{
							selections_to_objects.erase(it++);
						}
						else
						{
							++it;
						}
					}
					if(selections.empty())
					{
						objects_to_selections.erase(objects_to_selections_it);
					}
					else
					{
						objects_to_selections_it->second.clear();
						objects_to_selections_it->second.insert(selections.begin(), selections.end());
					}
				}
			}

			for(std::vector<std::string>::const_iterator it=selections.begin();it!=selections.end();++it)
			{
				selections_to_objects[*it].insert(object);
			}
		}

		void reset()
		{
			selections_to_objects.clear();
			objects_to_selections.clear();
		}
	};

	MappingOfNames atoms_mapping_of_names;
	MappingOfNames contacts_mapping_of_names;

	NamedSelectionsInfo()
	{
	}

	void reset()
	{
		atoms_mapping_of_names.reset();
		contacts_mapping_of_names.reset();
	}
};

}

}

}

#endif /* VIEWER_CONSOLE_NAMED_SELECTIONS_INFO_H_ */
