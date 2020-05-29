#ifndef SCRIPTING_COMMAND_DOCUMENTATION_H_
#define SCRIPTING_COMMAND_DOCUMENTATION_H_

#include <string>
#include <vector>
#include <algorithm>
#include <map>

namespace voronota
{

namespace scripting
{

class CommandDocumentation
{
public:
	struct OptionDescription
	{
		enum DataType
		{
			DATATYPE_UNDEFINED,
			DATATYPE_BOOL,
			DATATYPE_INT,
			DATATYPE_INT_ARRAY,
			DATATYPE_FLOAT,
			DATATYPE_FLOAT_ARRAY,
			DATATYPE_STRING,
			DATATYPE_STRING_ARRAY
		};

		std::string name;
		DataType data_type;
		std::string description;
		bool required;
		std::string default_value;

		template<typename T>
		OptionDescription(
				const std::string& name,
				const DataType data_type,
				const std::string& description,
				const T& raw_default_value) :
					name(name),
					data_type(data_type),
					description(description),
					required(false)
		{
			std::ostringstream output;
			output << raw_default_value;
			default_value=output.str();
		}

		OptionDescription(
				const std::string& name,
				const DataType data_type,
				const std::string& description) :
					name(name),
					data_type(data_type),
					description(description),
					required(data_type!=DATATYPE_BOOL),
					default_value(data_type!=DATATYPE_BOOL ? "" : "false")
		{
		}

		bool operator==(const OptionDescription& od) const
		{
			return (name==od.name);
		}

		bool operator==(const std::string& input_name) const
		{
			return (name==input_name);
		}

		std::string data_type_string() const
		{
			if(data_type==DATATYPE_BOOL)
			{
				return std::string("bool");
			}
			else if(data_type==DATATYPE_INT)
			{
				return std::string("int");
			}
			else if(data_type==DATATYPE_INT_ARRAY)
			{
				return std::string("ints");
			}
			else if(data_type==DATATYPE_FLOAT)
			{
				return std::string("float");
			}
			else if(data_type==DATATYPE_FLOAT_ARRAY)
			{
				return std::string("floats");
			}
			else if(data_type==DATATYPE_STRING)
			{
				return std::string("string");
			}
			else if(data_type==DATATYPE_STRING_ARRAY)
			{
				return std::string("strings");
			}
			return std::string("undefined");
		}
	};

	CommandDocumentation()
	{
	}

	explicit CommandDocumentation(const std::string& short_description) :
		short_description_(short_description)
	{
	}

	bool empty() const
	{
		return (short_description_.empty() && full_description_.empty() && option_descriptions_.empty());
	}

	const std::string& get_short_description() const
	{
		return short_description_;
	}

	const std::string& get_full_description() const
	{
		if(full_description_.empty())
		{
			return get_short_description();
		}
		return full_description_;
	}

	const std::vector<OptionDescription>& get_option_descriptions() const
	{
		return option_descriptions_;
	}

	void set_short_description(const std::string& description)
	{
		short_description_=description;
	}

	void set_full_description(const std::string& description)
	{
		full_description_=description;
	}

	void set_option_decription(const OptionDescription& od)
	{
		std::vector<OptionDescription>::iterator it=std::find(option_descriptions_.begin(), option_descriptions_.end(), od);
		if(it==option_descriptions_.end())
		{
			option_descriptions_.push_back(od);
		}
		else
		{
			(*it)=od;
		}
	}

	void delete_option_decription(const std::string& name)
	{
		std::vector<OptionDescription>::iterator it=std::find(option_descriptions_.begin(), option_descriptions_.end(), name);
		if(it!=option_descriptions_.end())
		{
			option_descriptions_.erase(it);
		}
	}

private:
	std::string short_description_;
	std::string full_description_;
	std::vector<OptionDescription> option_descriptions_;
};

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

#endif /* SCRIPTING_COMMAND_DOCUMENTATION_H_ */

