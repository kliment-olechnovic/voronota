#ifndef COMMON_SCRIPTING_VARIANT_TYPES_H_
#define COMMON_SCRIPTING_VARIANT_TYPES_H_

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace common
{

namespace scripting
{

class VariantValue
{
public:
	enum ValueType
	{
		VARIANT_NULL,
		VARIANT_BOOL,
		VARIANT_INT,
		VARIANT_FLOAT,
		VARIANT_STRING
	};

	VariantValue() :
		value_type_(VARIANT_NULL),
		value_bool_(false),
		value_int_(0),
		value_float_(0.0)
	{
	}

	template<typename T>
	explicit VariantValue(const T& value) :
		value_type_(VARIANT_NULL),
		value_bool_(false),
		value_int_(0),
		value_float_(0.0)
	{
		(*this)=value;
	}

	VariantValue& operator=(const bool value)
	{
		set_value_bool(value);
		return (*this);
	}

	VariantValue& operator=(const long value)
	{
		set_value_int(value);
		return (*this);
	}

	VariantValue& operator=(const int value)
	{
		set_value_int(value);
		return (*this);
	}

	VariantValue& operator=(const unsigned long value)
	{
		set_value_int(value);
		return (*this);
	}

	VariantValue& operator=(const unsigned int value)
	{
		set_value_int(value);
		return (*this);
	}

	VariantValue& operator=(const double value)
	{
		set_value_float(value);
		return (*this);
	}

	VariantValue& operator=(const float value)
	{
		set_value_float(value);
		return (*this);
	}

	VariantValue& operator=(const std::string& value)
	{
		set_value_string(value);
		return (*this);
	}

	VariantValue& operator=(const char* value)
	{
		set_value_string(std::string(value));
		return (*this);
	}

	bool operator==(const VariantValue& v) const
	{
		if(value_type_==v.value_type())
		{
			if(value_type_==VARIANT_NULL)
			{
				return true;
			}
			else if(value_type_==VARIANT_BOOL)
			{
				return (value_bool_==v.value_bool());
			}
			else if(value_type_==VARIANT_INT)
			{
				return (value_int_==v.value_int());
			}
			else if(value_type_==VARIANT_FLOAT)
			{
				return (value_float_==v.value_float());
			}
			else if(value_type_==VARIANT_STRING)
			{
				return (value_string_==v.value_string());
			}
		}
		return false;
	}

	bool operator<(const VariantValue& v) const
	{
		if(value_type_==v.value_type())
		{
			if(value_type_==VARIANT_NULL)
			{
				return false;
			}
			else if(value_type_==VARIANT_BOOL)
			{
				return (value_bool_<v.value_bool());
			}
			else if(value_type_==VARIANT_INT)
			{
				return (value_int_<v.value_int());
			}
			else if(value_type_==VARIANT_FLOAT)
			{
				return (value_float_<v.value_float());
			}
			else if(value_type_==VARIANT_STRING)
			{
				return (value_string_<v.value_string());
			}
		}
		return false;
	}

	ValueType value_type() const
	{
		return value_type_;
	}

	bool null() const
	{
		return value_type_==VARIANT_NULL;
	}

	bool value_bool() const
	{
		return value_bool_;
	}

	long value_int() const
	{
		return value_int_;
	}

	double value_float() const
	{
		return value_float_;
	}

	const std::string& value_string() const
	{
		return value_string_;
	}

	template<typename T>
	T value_as_number() const
	{
		T value=0;
		if(value_type_==VARIANT_NULL)
		{
			value=0;
		}
		else if(value_type_==VARIANT_BOOL)
		{
			value=static_cast<T>(value_bool_);
		}
		else if(value_type_==VARIANT_INT)
		{
			value=static_cast<T>(value_int_);
		}
		else if(value_type_==VARIANT_FLOAT)
		{
			value=static_cast<T>(value_float_);
		}
		else if(value_type_==VARIANT_STRING)
		{
			std::istringstream input(value_string_);
			input >> value;
			if(input.fail())
			{
				throw std::runtime_error(std::string("Failed to convert string '")+value_string_+"' to number.");
			}
		}
		return value;
	}

	std::string value_as_string() const
	{
		if(value_type_==VARIANT_NULL)
		{
			return std::string();
		}
		else if(value_type_==VARIANT_BOOL || value_type_==VARIANT_INT || value_type_==VARIANT_FLOAT)
		{
			std::ostringstream output;
			if(value_type_==VARIANT_BOOL)
			{
				output << value_bool_;
			}
			else if(value_type_==VARIANT_INT)
			{
				output << value_int_;
			}
			else if(value_type_==VARIANT_FLOAT)
			{
				output << value_float_;
			}
			return output.str();
		}
		else if(value_type_==VARIANT_STRING)
		{
			return value_string_;
		}
		return std::string();
	}

	void set_null()
	{
		value_type_=VARIANT_NULL;
	}

	void set_value_bool(const bool value)
	{
		value_type_=VARIANT_BOOL;
		value_bool_=value;
	}

	void set_value_int(const long value)
	{
		value_type_=VARIANT_INT;
		value_int_=value;
	}

	void set_value_float(const double value)
	{
		value_type_=VARIANT_FLOAT;
		value_float_=value;
	}

	void set_value_string(const std::string& value)
	{
		value_type_=VARIANT_STRING;
		value_string_=value;
	}

private:
	ValueType value_type_;
	bool value_bool_;
	long value_int_;
	double value_float_;
	std::string value_string_;
};

class VariantCollection
{
public:
	VariantCollection()
	{
	}

	std::map<std::string, VariantValue> values;
	std::map< std::string, std::vector<VariantValue> > values_array;
	std::map<std::string, VariantCollection> objects;
	std::map< std::string, std::vector<VariantCollection> > objects_array;
};

class JSONWriter
{
public:
	static void write(const VariantCollection& collection, std::ostream& output)
	{
		write_unnamed(collection, output);
	}

private:
	static void write_unnamed(const VariantValue& value, std::ostream& output)
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

	static void write_unnamed(const std::vector<VariantValue>& values, std::ostream& output)
	{
		output << "[";
		for(std::size_t i=0;i<values.size();i++)
		{
			if(i>0)
			{
				output << ",";
			}
			write_unnamed(values[i], output);
		}
		output << "]";
	}

	static void write_unnamed(const VariantCollection& collection, std::ostream& output)
	{
		bool filled=false;
		output << "{";
		for(std::map<std::string, VariantValue>::const_iterator it=collection.values.begin();it!=collection.values.end();++it)
		{
			if(filled)
			{
				output << ",";
			}
			filled=true;
			write_named(it->first, it->second, output);
		}
		for(std::map< std::string, std::vector<VariantValue> >::const_iterator it=collection.values_array.begin();it!=collection.values_array.end();++it)
		{
			if(filled)
			{
				output << ",";
			}
			filled=true;
			write_named(it->first, it->second, output);
		}
		for(std::map<std::string, VariantCollection>::const_iterator it=collection.objects.begin();it!=collection.objects.end();++it)
		{
			if(filled)
			{
				output << ",";
			}
			filled=true;
			write_named(it->first, it->second, output);
		}
		for(std::map< std::string, std::vector<VariantCollection> >::const_iterator it=collection.objects_array.begin();it!=collection.objects_array.end();++it)
		{
			if(filled)
			{
				output << ",";
			}
			filled=true;
			write_named(it->first, it->second, output);
		}
		output << "}";
	}

	static void write_unnamed(const std::vector<VariantCollection>& collections, std::ostream& output)
	{
		output << "[";
		for(std::size_t i=0;i<collections.size();i++)
		{
			if(i>0)
			{
				output << ",";
			}
			write_unnamed(collections[i], output);
		}
		output << "]";
	}

	template<typename T>
	static void write_named(const std::string& name, const T& value, std::ostream& output)
	{
		output << "\"" << name << "\":";
		write_unnamed(value, output);
	}
};

}

}

#endif /* COMMON_SCRIPTING_VARIANT_TYPES_H_ */
