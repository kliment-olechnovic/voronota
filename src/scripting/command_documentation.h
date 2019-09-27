#ifndef SCRIPTING_COMMAND_DOCUMENTATION_H_
#define SCRIPTING_COMMAND_DOCUMENTATION_H_

#include <string>
#include <vector>
#include <algorithm>

namespace scripting
{

class CommandDocumentation
{
public:
	struct OptionDescription
	{
		bool required;
		std::string name;
		std::string value_type;
		std::string description;

		OptionDescription(
				const std::string& name,
				const std::string& value_type,
				const std::string& description) :
					required(false),
					name(name),
					value_type(value_type),
					description(description)
		{
		}

		OptionDescription(
				const std::string& name,
				const std::string& value_type,
				const std::string& description,
				const bool required) :
					required(required),
					name(name),
					value_type(value_type),
					description(description)
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

	void set_option_decription(const std::string& name, const std::string& value_type, const std::string& description)
	{
		set_option_decription(OptionDescription(name, value_type, description));
	}

	void set_option_decription(const std::string& name, const std::string& value_type, const std::string& description, const bool required)
	{
		set_option_decription(OptionDescription(name, value_type, description, required));
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

}

#endif /* SCRIPTING_COMMAND_DOCUMENTATION_H_ */

