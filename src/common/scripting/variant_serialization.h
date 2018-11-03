#ifndef COMMON_SCRIPTING_VARIANT_SERIALIZATION_H_
#define COMMON_SCRIPTING_VARIANT_SERIALIZATION_H_

#include "variant_types.h"
#include "basic_types.h"

namespace common
{

namespace scripting
{

class VariantSerialization
{
public:
	static void write(const BoundingBox& input, VariantObject& output)
	{
		if(input.filled)
		{
			output.values_array("min").push_back(VariantValue(input.p_min.x));
			output.values_array("min").push_back(VariantValue(input.p_min.y));
			output.values_array("min").push_back(VariantValue(input.p_min.z));
			output.values_array("max").push_back(VariantValue(input.p_max.x));
			output.values_array("max").push_back(VariantValue(input.p_max.y));
			output.values_array("max").push_back(VariantValue(input.p_max.z));
		}
		else
		{
			output.value("min").set_null();
			output.value("max").set_null();
		}
	}

	static void write(const SummaryOfAtoms& input, VariantObject& output)
	{
		output.value("number_total")=input.number_total;
		if(input.volume>0.0)
		{
			output.value("volume")=input.volume;
		}
		else
		{
			output.value("volume").set_null();
		}
	}

	static void write(const SummaryOfContacts& input, VariantObject& output)
	{
		output.value("number_total")=input.number_total;
		output.value("number_drawable")=input.number_drawable;
		output.value("area")=input.area;
	}
};

}

}




#endif /* COMMON_SCRIPTING_VARIANT_SERIALIZATION_H_ */
