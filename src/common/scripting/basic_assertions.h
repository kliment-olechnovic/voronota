#ifndef COMMON_SCRIPTING_BASIC_ASSERTIONS_H_
#define COMMON_SCRIPTING_BASIC_ASSERTIONS_H_

#include "basic_types.h"

namespace common
{

namespace scripting
{

inline void assert_selection_name_input(const std::string& name, const bool allow_empty)
{
	if(name.empty())
	{
		if(!allow_empty)
		{
			throw std::runtime_error(std::string("Selection name is empty."));
		}
	}
	else if(name.find_first_of(" {}()[]<>,;.:\\/+*/='\"@#$%^&`~?|")!=std::string::npos)
	{
		throw std::runtime_error(std::string("Selection name contains invalid symbols."));
	}
	else if(name.rfind("-", 0)==0 || name.rfind("_", 0)==0)
	{
		throw std::runtime_error(std::string("Selection name starts with invalid symbol."));
	}
}

inline void assert_tag_input(const std::string& tag, const bool allow_empty)
{
	if(tag.empty())
	{
		if(!allow_empty)
		{
			throw std::runtime_error(std::string("Tag is empty."));
		}
	}
	else if(tag.find_first_of(" {}()[]<>,;.:\\/+*/'\"@#$%^&`~?|")!=std::string::npos)
	{
		throw std::runtime_error(std::string("Tag contains invalid symbols."));
	}
	else if(tag.rfind("-", 0)==0)
	{
		throw std::runtime_error(std::string("Tag starts with invalid symbol."));
	}
}

inline void assert_adjunct_name_input(const std::string& name, const bool allow_empty)
{
	if(name.empty())
	{
		if(!allow_empty)
		{
			throw std::runtime_error(std::string("Adjunct name is empty."));
		}
	}
	else if(name.find_first_of(" {}()[]<>,;.:\\/+*/'\"@#$%^&`~?|=")!=std::string::npos)
	{
		throw std::runtime_error(std::string("Adjunct name contains invalid symbols."));
	}
	else if(name.rfind("-", 0)==0)
	{
		throw std::runtime_error(std::string("Adjunct name starts with invalid symbol."));
	}
}

}

}

#endif /* COMMON_SCRIPTING_BASIC_ASSERTIONS_H_ */
