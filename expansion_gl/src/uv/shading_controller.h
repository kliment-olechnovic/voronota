#ifndef UV_SHADING_CONTROLLER_H_
#define UV_SHADING_CONTROLLER_H_

#include "common.h"
#include "stocked_default_shaders.h"

namespace voronota
{

namespace uv
{

class ShadingController : private Noncopyable
{
public:
	ShadingController() :
		good_(false),
		shader_program_(0),
		projection_matrix_id_(0),
		viewtransform_matrix_id_(0),
		modeltransform_matrix_id_(0),
		viewport_id_(0),
		selection_mode_enabled_id_(0),
		mode_number_id_(0)
	{
	}

	~ShadingController()
	{
		if(good())
		{
			glDeleteProgram(shader_program_);
		}
	}

	static void managed_glUseProgram(const GLuint program)
	{
		static GLuint last_used_program=0;
		if(program==0)
		{
			last_used_program=0;
		}
		else if(program!=last_used_program)
		{
			glUseProgram(program);
			last_used_program=program;
		}
	}

	bool init(const std::string& vertex_shader_filename, const std::string& fragment_shader_filename, const std::vector<std::string>& attribute_names_to_bind_locations)
	{
		if(good())
		{
			return false;
		}

		std::vector<char> vertex_shader_source;
		if(!read_file_to_memory(vertex_shader_filename, vertex_shader_source))
		{
			std::cerr << "Error: failed to read vertex shader source file '" << vertex_shader_filename << "'." << std::endl;
			return false;
		}

		std::vector<char> fragment_shader_source;
		if(!read_file_to_memory(fragment_shader_filename, fragment_shader_source))
		{
			std::cerr << "Error: failed to read fragment shader source file '" << fragment_shader_filename << "'." << std::endl;
			return false;
		}

		GLuint vertex_shader=glCreateShader(GL_VERTEX_SHADER);
		{
			const char* vertex_shader_source_data=vertex_shader_source.data();
			glShaderSource(vertex_shader, 1, &vertex_shader_source_data, 0);
		}
		glCompileShader(vertex_shader);
		if(check_shader_compilation_status(vertex_shader))
		{
			std::cerr << "Error: failed to compile vertex shader." << std::endl;
			return false;
		}

		GLuint fragment_shader=glCreateShader(GL_FRAGMENT_SHADER);
		{
			const char* fragment_shader_source_data=fragment_shader_source.data();
			glShaderSource(fragment_shader, 1, &fragment_shader_source_data, 0);
		}
		glCompileShader(fragment_shader);
		if(check_shader_compilation_status(fragment_shader))
		{
			std::cerr << "Error: failed to compile fragment shader." << std::endl;
			return false;
		}

		shader_program_=glCreateProgram();

		glAttachShader(shader_program_, vertex_shader);
		glAttachShader(shader_program_, fragment_shader);

		for(std::size_t i=0;i<attribute_names_to_bind_locations.size();i++)
		{
			glBindAttribLocation(shader_program_, static_cast<GLuint>(i), static_cast<const GLchar*>(attribute_names_to_bind_locations[i].c_str()));
		}

		glLinkProgram(shader_program_);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		projection_matrix_id_=glGetUniformLocation(shader_program_, "projection_matrix");
		viewtransform_matrix_id_=glGetUniformLocation(shader_program_, "viewtransform_matrix");
		modeltransform_matrix_id_=glGetUniformLocation(shader_program_, "modeltransform_matrix");
		viewport_id_=glGetUniformLocation(shader_program_, "viewport");
		selection_mode_enabled_id_=glGetUniformLocation(shader_program_, "selection_mode_enabled");
		mode_number_id_=glGetUniformLocation(shader_program_, "mode_number");

		good_=true;

		return true;
	}

	bool good() const
	{
		return good_;
	}

	bool enable() const
	{
		if(good())
		{
			managed_glUseProgram(shader_program_);
			return true;
		}
		return false;
	}

	bool set_projection_matrix(const GLfloat* data)
	{
		if(enable())
		{
			glUniformMatrix4fv(projection_matrix_id_, 1, GL_FALSE, data);
			return true;
		}
		return false;
	}

	bool set_viewtransform_matrix(const GLfloat* data)
	{
		if(enable())
		{
			glUniformMatrix4fv(viewtransform_matrix_id_, 1, GL_FALSE, data);
			return true;
		}
		return false;
	}

	bool set_modeltransform_matrix(const GLfloat* data)
	{
		if(enable())
		{
			glUniformMatrix4fv(modeltransform_matrix_id_, 1, GL_FALSE, data);
			return true;
		}
		return false;
	}

	bool set_viewport(const GLfloat x, const GLfloat y, const GLfloat w, const GLfloat h)
	{
		if(enable())
		{
			glUniform4f(viewport_id_, x, y, w, h);
			return true;
		}
		return false;
	}

	bool set_selection_mode_enabled(const bool status)
	{
		if(enable())
		{
			glUniform1i(selection_mode_enabled_id_, (status ? 1 : 0));
			return true;
		}
		return false;
	}

	bool set_mode_number(const int mode_number)
	{
		if(enable())
		{
			glUniform1i(mode_number_id_, mode_number);
			return true;
		}
		return false;
	}

	void print_uniform_values(std::ostream& output) const
	{
		if(!enable())
		{
			return;
		}

		{
			GLfloat data[16];
			glGetUniformfv(shader_program_, projection_matrix_id_, data);
			output << "projection matrix:";
			for(int i=0;i<16;i++)
			{
				output << " " << data[i];
			}
			output << "\n";
		}

		{
			GLfloat data[16];
			glGetUniformfv(shader_program_, viewtransform_matrix_id_, data);
			output << "viewtransform matrix:";
			for(int i=0;i<16;i++)
			{
				output << " " << data[i];
			}
			output << "\n";
		}

		{
			GLfloat data[16];
			glGetUniformfv(shader_program_, modeltransform_matrix_id_, data);
			output << "modeltransform matrix:";
			for(int i=0;i<16;i++)
			{
				output << " " << data[i];
			}
			output << "\n";
		}
	}

private:
	static bool check_shader_compilation_status(GLuint shader)
	{
		int status=0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		{
			char log[2048];
			int log_length=0;
			glGetShaderInfoLog(shader, 2048, &log_length, log);
			if(log_length>0)
			{
				std::cerr << "Shader compile log:\n" << log << std::endl;
			}
		}
		return (status!=GL_TRUE);
	}

	static bool read_file_to_memory(const std::string& filename, std::vector<char>& output)
	{
		std::string predefined_input;

		if(filename=="_shader_vertex_screen")
		{
			predefined_input=default_shader_vertex_screen();
		}
		else if(filename=="_shader_vertex_simple")
		{
			predefined_input=default_shader_vertex_simple();
		}
		else if(filename=="_shader_vertex_with_instancing")
		{
			predefined_input=default_shader_vertex_with_instancing();
		}
		else if(filename=="_shader_vertex_with_impostoring")
		{
			predefined_input=default_shader_vertex_with_impostoring();
		}
		else if(filename=="_shader_fragment_screen")
		{
			predefined_input=default_shader_fragment_screen();
		}
		else if(filename=="_shader_fragment_simple")
		{
			predefined_input=default_shader_fragment_simple();
		}
		else if(filename=="_shader_fragment_with_impostoring")
		{
			predefined_input=default_shader_fragment_with_impostoring();
		}

		if(!predefined_input.empty())
		{
			output.resize(predefined_input.size()+1);
			std::copy(predefined_input.c_str(), predefined_input.c_str()+predefined_input.size()+1, output.begin());
			return true;
		}

		return Utilities::read_file_to_memory(filename.c_str(), output);
	}

	bool good_;
	GLuint shader_program_;
	GLuint projection_matrix_id_;
	GLuint viewtransform_matrix_id_;
	GLuint modeltransform_matrix_id_;
	GLuint viewport_id_;
	GLuint selection_mode_enabled_id_;
	GLuint mode_number_id_;
};

}

}

#endif /* UV_SHADING_CONTROLLER_H_ */
