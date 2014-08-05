#ifndef MODESCOMMON_READ_SPHERE_H_
#define MODESCOMMON_READ_SPHERE_H_

#include "auxiliaries/io_utilities.h"

namespace modescommon
{

template<typename SphereType>
inline bool add_sphere_from_stream_to_vector(std::istream& input, std::vector<SphereType>& spheres)
{
	SphereType sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
		return true;
	}
	return false;
}

template<typename SphereType, typename CommentType>
inline bool add_sphere_and_comments_from_stream_to_vectors(std::istream& input, std::pair< std::vector<SphereType>*, std::vector<CommentType>* >& spheres_with_comments)
{
	SphereType sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	std::string separator;
	input >> separator;
	if(!input.fail() && separator=="#")
	{
		CommentType comment;
		input >> comment.serial >> comment.chainID >> comment.resSeq >> comment.resName >> comment.name;
		if(input.good())
		{
			input >> comment.altLoc >> comment.iCode;
		}
		if(!input.fail())
		{
			if(comment.altLoc.find_first_of(".?")==0)
			{
				comment.altLoc.clear();
			}
			if(comment.iCode.find_first_of(".?")==0)
			{
				comment.iCode.clear();
			}
			if(comment.valid())
			{
				spheres_with_comments.first->push_back(sphere);
				spheres_with_comments.second->push_back(comment);
				return true;
			}
		}
	}
	return false;
}

}

#endif /* MODESCOMMON_READ_SPHERE_H_ */
