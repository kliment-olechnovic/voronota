#ifndef UV_DRAWING_FOR_SCREEN_CONTROLLER_H_
#define UV_DRAWING_FOR_SCREEN_CONTROLLER_H_

#include "common.h"

namespace voronota
{

namespace uv
{

class DrawingForScreenController : private Noncopyable
{
public:
	DrawingForScreenController() :
		vao_(0),
		vbo_(0)
	{
	}

	virtual ~DrawingForScreenController()
	{
		reset();
	}

	static const std::vector<std::string>& ordered_used_shader_attribute_names()
	{
		static std::vector<std::string> names;
		if(names.empty())
		{
			names.push_back("a_position");
			names.push_back("a_texcoord");
		}
		return names;
	}

	bool init()
	{
		if(good())
		{
			return true;
		}

		std::vector<GLfloat> vertices(24, 0.0f);
		{
			int i=0;
			vertices[i++]=-1.0f; vertices[i++]= 1.0f; vertices[i++]= 0.0f; vertices[i++]= 1.0f;
			vertices[i++]=-1.0f; vertices[i++]=-1.0f; vertices[i++]= 0.0f; vertices[i++]= 0.0f;
			vertices[i++]= 1.0f; vertices[i++]=-1.0f; vertices[i++]= 1.0f; vertices[i++]= 0.0f;
			vertices[i++]=-1.0f; vertices[i++]= 1.0f; vertices[i++]= 0.0f; vertices[i++]= 1.0f;
			vertices[i++]= 1.0f; vertices[i++]=-1.0f; vertices[i++]= 1.0f; vertices[i++]= 0.0f;
			vertices[i++]= 1.0f; vertices[i++]= 1.0f; vertices[i++]= 1.0f; vertices[i++]= 1.0f;
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &vao_);
		glGenBuffers(1, &vbo_);
		glBindVertexArray(vao_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return good();
	}

	bool good() const
	{
		return (vbo_!=0 && vao_!=0);
	}

	bool draw(const GLuint texture_color_buffer)
	{
		if(good())
		{
			glBindVertexArray(vao_);
			glBindTexture(GL_TEXTURE_2D, texture_color_buffer);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			{
				GLenum err=glGetError();
				while(err!=GL_NO_ERROR)
				{
					std::cerr << "OpenGL error " << err << " in file " __FILE__ << " line " << __LINE__ << "\n";
					err=glGetError();
				}
			}
			glBindVertexArray(0);
			return true;
		}
		return true;
	}

private:
	void reset()
	{
		if(vbo_!=0)
		{
			glDeleteBuffers(1, &vbo_);
			vbo_=0;
		}

		if(vao_!=0)
		{
			glDeleteVertexArrays(1, &vao_);
			vao_=0;
		}
	}

	GLuint vao_;
	GLuint vbo_;
};

}

}

#endif /* UV_DRAWING_FOR_SCREEN_CONTROLLER_H_ */

