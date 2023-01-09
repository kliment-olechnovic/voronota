#ifndef SCRIPTING_JSON_WRITER_H_
#define SCRIPTING_JSON_WRITER_H_

#include "variant_types.h"

namespace voronota
{

namespace scripting
{

class JSONWriter
{
public:
	class Configuration
	{
	public:
		int indentation_max_level;
		int indentation_length;
		bool indentation_enabled_for_value_arrays;
		int value_string_length_limit;

		Configuration() :
			indentation_max_level(100),
			indentation_length(2),
			indentation_enabled_for_value_arrays(false),
			value_string_length_limit(0)
		{
		}

		explicit Configuration(int indentation_max_level) :
			indentation_max_level(indentation_max_level),
			indentation_length(2),
			indentation_enabled_for_value_arrays(false),
			value_string_length_limit(0)
		{
		}

		Configuration(int indentation_max_level, int indentation_length, bool indentation_enabled_for_value_arrays) :
			indentation_max_level(indentation_max_level),
			indentation_length(indentation_length),
			indentation_enabled_for_value_arrays(indentation_enabled_for_value_arrays),
			value_string_length_limit(0)
		{
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static void setup_default_configuration(const Configuration& configuration)
		{
			get_default_configuration_mutable()=configuration;
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	static void write(const Configuration& configuration, const VariantObject& object, std::ostream& output)
	{
		JSONWriter(configuration).print(object, output);
	}

	static void write(const VariantObject& object, std::ostream& output)
	{
		write(Configuration::get_default_configuration(), object, output);
	}

	static std::string write(const Configuration& configuration, const VariantObject& object)
	{
		std::ostringstream output;
		write(configuration, object, output);
		return output.str();
	}

	static std::string write(const VariantObject& object)
	{
		std::ostringstream output;
		write(Configuration::get_default_configuration(), object, output);
		return output.str();
	}

	static std::string replace_special_characters_with_escape_sequences(const std::string& input)
	{
		if(input.find_first_of("\"\n\r\f\t\\")==std::string::npos)
		{
			return input;
		}

		std::string output;

		for(std::size_t i=0;i<input.size();i++)
		{
			const char c=input[i];
			if(c=='\"')
			{
				output+="\\\"";
			}
			else if(c=='\n')
			{
				output+="\\n";
			}
			else if(c=='\r')
			{
				output+="\\r";
			}
			else if(c=='\f')
			{
				output+="\\f";
			}
			else if(c=='\t')
			{
				output+="\\t";
			}
			else if(c=='\\')
			{
				output+="\\\\";
			}
			else
			{
				output+=c;
			}
		}

		return output;
	}

private:
	explicit JSONWriter(const Configuration& configuration) :
		level_(0),
		config_(configuration)
	{
	}

	void print(const VariantValue& value, std::ostream& output) const
	{
		if(value.null())
		{
			output << "null";
		}
		else
		{
			if(value.value_type()==VariantValue::VARIANT_STRING)
			{
				output << "\"";
				if(config_.value_string_length_limit>0 && config_.value_string_length_limit<static_cast<int>(value.value_string().size()))
				{
					output << "STRING_TOO_LONG_TO_PRINT";
				}
				else
				{
					output << replace_special_characters_with_escape_sequences(value.value_string());
				}
				output << "\"";
			}
			else
			{
				output << value.value_as_string();
			}
		}
	}

	void print(const std::vector<VariantValue>& values, std::ostream& output)
	{
		bool separated=false;
		print_array_start(output);
		for(std::size_t i=0;i<values.size();i++)
		{
			print_separator(separated, output);
			if(config_.indentation_enabled_for_value_arrays)
			{
				print_indentation(output);
			}
			print(values[i], output);
		}
		print_array_end(output);
	}

	void print(const VariantObject& object, std::ostream& output)
	{
		bool separated=false;
		print_object_start(output);
		for(std::size_t i=0;i<object.ordered_names().size();i++)
		{
			const std::string& name=object.ordered_names()[i];
			bool printed=false;
			printed=printed || print_from_map(object.values(), name, separated, output);
			printed=printed || print_from_map(object.values_arrays(), name, separated, output);
			printed=printed || print_from_map(object.objects(), name, separated, output);
			printed=printed || print_from_map(object.objects_arrays(), name, separated, output);
			if(!printed)
			{
				print(VariantValue(), output);
			}
		}
		print_object_end(output);
	}

	void print(const std::vector<VariantObject>& objects, std::ostream& output)
	{
		bool separated=false;
		print_array_start(output);
		for(std::size_t i=0;i<objects.size();i++)
		{
			print_separator(separated, output);
			print_indentation(output);
			print(objects[i], output);
		}
		print_array_end(output);
	}

	template<typename T>
	void print(const std::string& name, const T& value, std::ostream& output)
	{
		print_indentation(output);
		output << "\"" << name << "\":";
		print(value, output);
	}

	template<typename T>
	bool print_from_map(const T& map, const std::string& name, bool& separation_controller, std::ostream& output)
	{
		typename T::const_iterator it=map.find(name);
		if(it!=map.end())
		{
			print_separator(separation_controller, output);
			print(it->first, it->second, output);
			return true;
		}
		return false;
	}

	void print_indentation(std::ostream& output) const
	{
		if(level_<=config_.indentation_max_level)
		{
			output << "\n";
			for(int i=0;i<level_;i++)
			{
				for(int j=0;j<config_.indentation_length;j++)
				{
					output << " ";
				}
			}
		}
	}

	void print_separator(bool& controller, std::ostream& output) const
	{
		if(controller)
		{
			output << ",";
		}
		controller=true;
	}

	void print_array_start(std::ostream& output)
	{
		output << "[";
		level_++;
	}

	void print_array_end(std::ostream& output)
	{
		output << "]";
		level_--;
	}

	void print_object_start(std::ostream& output)
	{
		output << "{";
		level_++;
	}

	void print_object_end(std::ostream& output)
	{
		output << "}";
		level_--;
	}

	int level_;
	Configuration config_;
};

}

}

#endif /* SCRIPTING_JSON_WRITER_H_ */
