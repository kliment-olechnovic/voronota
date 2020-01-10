#ifndef SCRIPTING_VARIANT_SERIALIZATION_H_
#define SCRIPTING_VARIANT_SERIALIZATION_H_

#include "variant_types.h"
#include "basic_types.h"

namespace voronota
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

	static void write(const SummaryOfTriangulation& input, VariantObject& output)
	{
		output.value("number_input_balls")=input.number_of_input_balls;
		output.value("number_all_balls")=input.number_of_all_balls;
		output.value("number_quadruples")=input.number_of_quadruples;
		output.value("number_voronoi_vertices")=input.number_of_voronoi_vertices;
	}

	static void write(const SummaryOfContacts& input, VariantObject& output)
	{
		output.value("number_total")=input.number_total;
		output.value("number_drawable")=input.number_drawable;
		output.value("area")=input.area;
	}

	static void write(const common::ChainResidueAtomDescriptor& input, VariantObject& output)
	{
		if(!input.chainID.empty())
		{
			output.value("chainID")=input.chainID;
		}

		if(input.resSeq!=common::ChainResidueAtomDescriptor::null_num())
		{
			output.value("resSeq")=input.resSeq;
		}

		if(!input.iCode.empty())
		{
			output.value("iCode")=input.iCode;
		}

		if(input.serial!=common::ChainResidueAtomDescriptor::null_num())
		{
			output.value("serial")=input.serial;
		}

		if(!input.altLoc.empty())
		{
			output.value("altLoc")=input.altLoc;
		}

		if(!input.resName.empty())
		{
			output.value("resName")=input.resName;
		}

		if(!input.name.empty())
		{
			output.value("name")=input.name;
		}
	}

	static void write(const common::PropertiesValue& input, VariantObject& output)
	{
		for(std::set<std::string>::const_iterator it=input.tags.begin();it!=input.tags.end();++it)
		{
			output.values_array("tags").push_back(VariantValue(*it));
		}

		for(std::map<std::string, double>::const_iterator it=input.adjuncts.begin();it!=input.adjuncts.end();++it)
		{
			output.object("adjuncts").value(it->first)=(it->second);
		}
	}

	static void write(const common::BallValue& input, VariantObject& output)
	{
		{
			VariantObject& ball=output.object("ball");
			std::vector<VariantValue>& center=ball.values_array("center");
			center.resize(3);
			center[0]=input.x;
			center[1]=input.y;
			center[2]=input.z;
			ball.value("radius")=input.r;
		}

		if(!input.props.empty())
		{
			write(input.props, output);
		}
	}

	static void write(const Atom& input, VariantObject& output)
	{
		write(input.crad, output.object("descriptor"));
		write(input.value, output);
	}

	static void write(const common::ContactValue& input, VariantObject& output)
	{
		output.value("area")=input.area;
		output.value("distance")=input.dist;
		if(input.accumulated)
		{
			output.value("summed")=input.accumulated;
		}
		if(!input.props.empty())
		{
			write(input.props, output);
		}
	}

	static void write(const common::ChainResidueAtomDescriptorsPair& crads, const common::ContactValue& value, VariantObject& output)
	{
		write(crads.a, output.object("descriptor1"));
		write(crads.b, output.object("descriptor2"));
		write(value, output);
	}

	static void write(const std::vector<Atom>& atoms, const Contact& input, VariantObject& output)
	{
		if(input.ids[0]<atoms.size() && input.ids[1]<atoms.size())
		{
			if(input.solvent())
			{
				write(common::ChainResidueAtomDescriptorsPair(atoms[input.ids[0]].crad, common::ChainResidueAtomDescriptor::solvent()), input.value, output);
			}
			else
			{
				write(common::ChainResidueAtomDescriptorsPair(atoms[input.ids[0]].crad, atoms[input.ids[1]].crad), input.value, output);
			}
		}
	}
};

}

}

#endif /* SCRIPTING_VARIANT_SERIALIZATION_H_ */
