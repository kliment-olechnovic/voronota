#ifndef VORONOTALT_PRINTING_CUSTOM_TYPES_H_
#define VORONOTALT_PRINTING_CUSTOM_TYPES_H_

#include <iostream>

#include "spheres_input.h"
#include "tessellation_full_construction.h"
#include "simplified_aw_tessellation_full_construction.h"

namespace voronotalt
{

inline std::ostream& operator<<(std::ostream& output, const SpheresInput::SphereLabel& obj)
{
	if(obj.chain_id.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.chain_id;
	}
	output << " ";
	if(obj.residue_id.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.residue_id;
	}
	output << " ";
	if(obj.atom_name.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.atom_name;
	}
	return output;
}

inline std::ostream& print_sphere_label_residue_level(std::ostream& output, const SpheresInput::SphereLabel& obj)
{
	if(obj.chain_id.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.chain_id;
	}
	output << " ";
	if(obj.residue_id.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.residue_id;
	}
	return output;
}

inline std::ostream& print_sphere_label_chain_level(std::ostream& output, const SpheresInput::SphereLabel& obj)
{
	if(obj.chain_id.empty())
	{
		output << ".";
	}
	else
	{
		output << obj.chain_id;
	}
	return output;
}

inline std::ostream& print_pair_of_sphere_labels_residue_level(std::ostream& output, const SpheresInput::SphereLabel& obj1, const SpheresInput::SphereLabel& obj2)
{
	if(obj1.chain_id<obj2.chain_id || (obj1.chain_id==obj2.chain_id && obj1.residue_id<obj2.residue_id))
	{
		print_sphere_label_residue_level(output, obj1);
		output << " ";
		print_sphere_label_residue_level(output, obj2);
	}
	else
	{
		print_sphere_label_residue_level(output, obj2);
		output << " ";
		print_sphere_label_residue_level(output, obj1);
	}
	return output;
}

inline std::ostream& print_pair_of_sphere_labels_chain_level(std::ostream& output, const SpheresInput::SphereLabel& obj1, const SpheresInput::SphereLabel& obj2)
{
	if(obj1.chain_id<obj2.chain_id)
	{
		print_sphere_label_chain_level(output, obj1);
		output << " ";
		print_sphere_label_chain_level(output, obj2);
	}
	else
	{
		print_sphere_label_chain_level(output, obj2);
		output << " ";
		print_sphere_label_chain_level(output, obj1);
	}
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::TessellationFullConstruction::ContactDescriptorSummary& obj)
{
	output << obj.id_a << " " <<  obj.id_b << " " << obj.area << " " << obj.arc_length << " " << obj.solid_angle_a << " " << obj.solid_angle_b;
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::TessellationFullConstruction::TotalContactDescriptorsSummary& obj)
{
	output << obj.area << " " << obj.arc_length << " " << obj.count;
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::TessellationFullConstruction::CellContactDescriptorsSummary& obj)
{
	output << obj.id << " " << obj.sas_area << " " << obj.sas_inside_volume;
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::TessellationFullConstruction::TotalCellContactDescriptorsSummary& obj)
{
	output << obj.sas_area << " " << obj.sas_inside_volume << " " << obj.count;
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& obj)
{
	output << obj.id_a << " " <<  obj.id_b << " " << obj.area;
	return output;
}

inline std::ostream& operator<<(std::ostream& output, const voronotalt::SimplifiedAWTessellationFullConstruction::TotalContactDescriptorsSummary& obj)
{
	output << obj.area << " " << obj.count;
	return output;
}

}

#endif /* VORONOTALT_PRINTING_CUSTOM_TYPES_H_ */
