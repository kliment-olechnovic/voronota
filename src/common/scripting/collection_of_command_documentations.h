#ifndef COMMON_SCRIPTING_COLLECTION_OF_COMMAND_DOCUMENTATIONS_H_
#define COMMON_SCRIPTING_COLLECTION_OF_COMMAND_DOCUMENTATIONS_H_

#include <map>

#include "command_documentation.h"

namespace common
{

namespace scripting
{

class CollectionOfCommandDocumentations
{
public:
	CollectionOfCommandDocumentations()
	{
	}

	const std::map<std::string, CommandDocumentation>& map_of_documentations() const
	{
		return map_of_documentations_;
	}

	const std::vector<std::string> get_all_names() const
	{
		std::vector<std::string> names;
		names.reserve(map_of_documentations_.size());
		for(std::map<std::string, CommandDocumentation>::const_iterator it=map_of_documentations_.begin();it!=map_of_documentations_.end();++it)
		{
			names.push_back(it->first);
		}
		return names;
	}

	CommandDocumentation get_documentation(const std::string& name) const
	{
		std::map<std::string, CommandDocumentation>::const_iterator it=map_of_documentations_.find(name);
		if(it==map_of_documentations_.end())
		{
			return CommandDocumentation();
		}
		else
		{
			return it->second;
		}
	}

	void set_documentation(const std::string& name, const CommandDocumentation& doc)
	{
		map_of_documentations_[name]=doc;
	}

	void delete_documentation(const std::string& name)
	{
		map_of_documentations_.erase(name);
	}

private:
	std::map<std::string, CommandDocumentation> map_of_documentations_;
};

}

}

#endif /* COMMON_SCRIPTING_COLLECTION_OF_COMMAND_DOCUMENTATIONS_H_ */

