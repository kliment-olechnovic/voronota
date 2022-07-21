#ifndef UV_COMMON_H_
#define UV_COMMON_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cmath>

#include <GL/glew.h>

#define GLM_FORCE_RADIANS

#ifdef FOR_WEB
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <emscripten/emscripten.h>
#else
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#endif

namespace voronota
{

namespace uv
{

struct ShadingMode
{
	enum Mode
	{
		simple,
		with_instancing,
		with_impostoring
	};
};

class Utilities
{
public:
	static bool read_file_to_memory(const char* filename, std::vector<char>& output)
	{
		std::ifstream input(filename, std::ios::binary | std::ios::ate);
		if(input.good())
		{
			const std::streamsize size=input.tellg();
			if(size>0)
			{
				input.seekg(0, std::ios::beg);
				output.resize(size+1);
				if(input.read(output.data(), size))
				{
					output[size]=0;
					return true;
				}
			}
		}
		return false;
	}

	static void calculate_color_from_integer(unsigned int rgb, GLfloat& r, GLfloat& g, GLfloat& b)
	{
		r=static_cast<GLfloat>((rgb & 0xFF0000) >> 16)/255.0f;
		g=static_cast<GLfloat>((rgb & 0x00FF00) >> 8)/255.0f;
		b=static_cast<GLfloat>((rgb & 0x0000FF))/255.0f;
	}

	static void calculate_color_from_integer(unsigned int rgb, GLfloat* output)
	{
		calculate_color_from_integer(rgb, output[0], output[1], output[2]);
	}

	static void calculate_grid_dimensions(const int grid_size, const int window_width, const int window_height, int& n_rows, int& n_columns)
	{
		n_columns=1;
		n_rows=1;
		if(grid_size>1)
		{
			const double real_columns=sqrt(static_cast<double>(grid_size)/(static_cast<double>(window_height)/static_cast<double>(window_width)));
			const double real_rows=static_cast<double>(grid_size)/real_columns;
			if(real_columns<=1.0)
			{
				n_columns=1;
				n_rows=grid_size;
			}
			else if(real_rows<=1.0)
			{
				n_columns=grid_size;
				n_rows=1;
			}
			else
			{
				const int candidate_columns[2]={static_cast<int>(ceil(real_columns)), static_cast<int>(floor(real_columns))};
				const int candidate_rows[2]={static_cast<int>(ceil(real_rows)), static_cast<int>(floor(real_rows))};
				n_columns=candidate_columns[0];
				n_rows=candidate_rows[0];
				int error=(n_columns*n_rows)-grid_size;
				if(error<0)
				{
					if(window_width>window_height)
					{
						n_columns=grid_size;
						n_rows=1;
					}
					else
					{
						n_columns=1;
						n_rows=grid_size;
					}
				}
				else if(error>0)
				{
					for(int i=1;i<4;i++)
					{
						const int new_n_colums=candidate_columns[i/2];
						const int new_n_rows=candidate_rows[i%2];
						const int new_error=(new_n_colums*new_n_rows)-grid_size;
						if(new_error>=0 && new_error<error)
						{
							n_columns=new_n_colums;
							n_rows=new_n_rows;
							error=new_error;
						}
					}
				}
			}
		}
	}
};

class Noncopyable
{
protected:
	Noncopyable() {}
	~Noncopyable() {}
private:
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator=(const Noncopyable&);
};

}

}

#endif /* UV_COMMON_H_ */
