#ifndef VORONOTALT_GRAPHICS_OUTPUT_H_
#define VORONOTALT_GRAPHICS_OUTPUT_H_

#include <iostream>
#include <vector>
#include <string>

#include "basic_types_and_functions.h"

namespace voronotalt
{

inline void print_triangle_fan_for_pymol(const std::vector<SimplePoint>& outer_points, const SimplePoint& center, const SimplePoint& normal, const std::string& prefix, std::ostream& output)
{
	output << prefix << "BEGIN,TRIANGLE_FAN";
	if(!outer_points.empty())
	{
		output << ",NORMAL," << normal.x << "," << normal.y << "," << normal.z;
		output << ",VERTEX," << center.x << "," << center.y << "," << center.z;
		for(std::size_t j=0;j<outer_points.size();j++)
		{
			output << ",NORMAL," << normal.x << "," << normal.y << "," << normal.z;
			output << ",VERTEX," << outer_points[j].x << "," << outer_points[j].y << "," << outer_points[j].z;
		}
		output << ",NORMAL," << normal.x << "," << normal.y << "," << normal.z;
		output << ",VERTEX," << outer_points[0].x << "," << outer_points[0].y << "," << outer_points[0].z;
	}
	output << ",END";
}

}

#endif /* VORONOTALT_GRAPHICS_OUTPUT_H_ */
