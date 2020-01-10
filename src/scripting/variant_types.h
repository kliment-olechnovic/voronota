#ifndef SCRIPTING_VARIANT_TYPES_H_
#define SCRIPTING_VARIANT_TYPES_H_

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace voronota
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

	VariantValue& operator=(const std::size_t value)
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
				output << (value_bool_ ? "true" : "false");
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

class VariantObject
{
public:
	VariantObject()
	{
	}

	bool empty() const
	{
		return names_.empty();
	}

	const std::map<std::string, VariantValue>& values() const
	{
		return values_;
	}

	const std::map< std::string, std::vector<VariantValue> >& values_arrays() const
	{
		return values_arrays_;
	}

	const std::map<std::string, VariantObject>& objects() const
	{
		return objects_;
	}

	const std::map< std::string, std::vector<VariantObject> >& objects_arrays() const
	{
		return objects_arrays_;
	}

	const std::set<std::string>& names() const
	{
		return names_;
	}

	const std::vector<std::string>& ordered_names() const
	{
		return ordered_names_;
	}

	VariantValue& value(const std::string& name)
	{
		values_arrays_.erase(name);
		objects_.erase(name);
		objects_arrays_.erase(name);
		record_name(name);
		return values_[name];
	}

	std::vector<VariantValue>& values_array(const std::string& name)
	{
		values_.erase(name);
		objects_.erase(name);
		objects_arrays_.erase(name);
		record_name(name);
		return values_arrays_[name];
	}

	VariantObject& object(const std::string& name)
	{
		values_.erase(name);
		values_arrays_.erase(name);
		objects_arrays_.erase(name);
		record_name(name);
		return objects_[name];
	}

	std::vector<VariantObject>& objects_array(const std::string& name)
	{
		values_.erase(name);
		values_arrays_.erase(name);
		objects_.erase(name);
		record_name(name);
		return objects_arrays_[name];
	}

	void erase(const std::string& name)
	{
		values_.erase(name);
		values_arrays_.erase(name);
		objects_.erase(name);
		objects_arrays_.erase(name);
		unrecord_name(name);
	}

private:
	void record_name(const std::string& name)
	{
		if(names_.count(name)==0)
		{
			names_.insert(name);
			ordered_names_.push_back(name);
		}
	}

	void unrecord_name(const std::string& name)
	{
		if(names_.count(name)>0)
		{
			names_.erase(name);
			ordered_names_.erase(std::remove(ordered_names_.begin(), ordered_names_.end(), name), ordered_names_.end());
		}
	}

	std::map<std::string, VariantValue> values_;
	std::map< std::string, std::vector<VariantValue> > values_arrays_;
	std::map<std::string, VariantObject> objects_;
	std::map< std::string, std::vector<VariantObject> > objects_arrays_;
	std::set<std::string> names_;
	std::vector<std::string> ordered_names_;
};

}

}

#endif /* SCRIPTING_VARIANT_TYPES_H_ */
