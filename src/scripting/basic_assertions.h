#ifndef SCRIPTING_BASIC_ASSERTIONS_H_
#define SCRIPTING_BASIC_ASSERTIONS_H_

#include "basic_types.h"

namespace voronota
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
		throw std::runtime_error(std::string("Selection name '")+name+"' contains invalid symbols.");
	}
	else if(name.rfind("-", 0)==0 || name.rfind("_", 0)==0)
	{
		throw std::runtime_error(std::string("Selection name '")+name+"' starts with invalid symbol.");
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
		throw std::runtime_error(std::string("Tag '")+tag+"' contains invalid symbols.");
	}
	else if(tag.rfind("-", 0)==0)
	{
		throw std::runtime_error(std::string("Tag '")+tag+"' starts with invalid symbol.");
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
		throw std::runtime_error(std::string("Adjunct name '")+name+"' contains invalid symbols.");
	}
	else if(name.rfind("-", 0)==0)
	{
		throw std::runtime_error(std::string("Adjunct name '")+name+"' starts with invalid symbol.");
	}
}

inline void assert_file_name_input(const std::string& name, const bool allow_empty)
{
	if(name.empty())
	{
		if(!allow_empty)
		{
			throw std::runtime_error(std::string("File name is empty."));
		}
	}
	else if(name.find_first_of("?*$'\";<>,|")!=std::string::npos)
	{
		throw std::runtime_error(std::string("File name '")+name+"' contains invalid symbols.");
	}
}

inline void assert_extension_in_file_name_input(const std::string& name, const std::string& extension)
{
	if(!(name.size()>extension.size() && name.substr(name.size()-extension.size())==extension))
	{
		throw std::runtime_error(std::string("File name '")+name+"' does not have required extension '"+extension+"'.");
	}
}

inline void assert_io_stream(const std::string& name, const std::ostream& output)
{
	if(!output.good())
	{
		throw std::runtime_error(std::string("Failed to use output stream '")+name+"'");
	}
}

inline void assert_io_stream(const std::string& name, const std::istream& input)
{
	if(!input.good())
	{
		throw std::runtime_error(std::string("Failed to use input stream '")+name+"'");
	}
}

}

}

#endif /* SCRIPTING_BASIC_ASSERTIONS_H_ */
