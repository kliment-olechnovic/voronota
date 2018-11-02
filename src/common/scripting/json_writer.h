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
	JSONWriter() :
		indentation_max_level(2),
		indentation_length(2),
		indentation_enabled_for_value_arrays(false),
		level_(0)
	{
	}

	void write(const VariantCollection& collection, std::ostream& output)
	{
		print(collection, output);
	}

	int indentation_max_level;
	int indentation_length;
	bool indentation_enabled_for_value_arrays;

private:
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
			if(indentation_enabled_for_value_arrays)
			{
				print_indentation(output);
			}
			print(values[i], output);
		}
		print_array_end(output);
	}

	void print(const VariantCollection& collection, std::ostream& output)
	{
		bool separated=false;
		print_object_start(output);
		for(std::size_t i=0;i<collection.ordered_names().size();i++)
		{
			const std::string& name=collection.ordered_names()[i];
			bool printed=false;
			printed=printed || print_from_map(collection.values(), name, separated, output);
			printed=printed || print_from_map(collection.values_arrays(), name, separated, output);
			printed=printed || print_from_map(collection.objects(), name, separated, output);
			printed=printed || print_from_map(collection.objects_arrays(), name, separated, output);
		}
		print_object_end(output);
	}

	void print(const std::vector<VariantCollection>& collections, std::ostream& output)
	{
		bool separated=false;
		print_array_start(output);
		for(std::size_t i=0;i<collections.size();i++)
		{
			print_separator(separated, output);
			print_indentation(output);
			print(collections[i], output);
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
		if(level_<=indentation_max_level)
		{
			output << "\n";
			for(int i=0;i<level_;i++)
			{
				for(int j=0;j<indentation_length;j++)
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
};

}

}

#endif /* COMMON_SCRIPTING_JSON_WRITER_H_ */
