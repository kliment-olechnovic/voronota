#ifndef COMMON_SCRIPTING_JSON_WRITER_H_
#define COMMON_SCRIPTING_JSON_WRITER_H_

#include "variant_types.h"

namespace common
{

namespace scripting
{

class JSONWriter
{
public:
	struct Configuration
	{
		int indentation_max_level;
		int indentation_length;
		bool indentation_enabled_for_value_arrays;

		Configuration() :
			indentation_max_level(2),
			indentation_length(2),
			indentation_enabled_for_value_arrays(false)
		{
		}

		Configuration(int indentation_max_level) :
			indentation_max_level(indentation_max_level),
			indentation_length(2),
			indentation_enabled_for_value_arrays(false)
		{
		}

		Configuration(int indentation_max_level, int indentation_length, bool indentation_enabled_for_value_arrays) :
			indentation_max_level(indentation_max_level),
			indentation_length(indentation_length),
			indentation_enabled_for_value_arrays(indentation_enabled_for_value_arrays)
		{
		}
	};

	static Configuration& default_configuration()
	{
		static Configuration configuration;
		return configuration;
	}

	static void write(const Configuration& configuration, const VariantObject& object, std::ostream& output)
	{
		JSONWriter(configuration).print(object, output);
	}

	static void write(const VariantObject& object, std::ostream& output)
	{
		write(default_configuration(), object, output);
	}

private:
	JSONWriter(const Configuration& configuration) :
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
			output << "\"" << value.value_as_string() << "\"";
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

#endif /* COMMON_SCRIPTING_JSON_WRITER_H_ */
