#ifndef UV_SHADING_CONTROLLER_H_
#define UV_SHADING_CONTROLLER_H_

#include "common.h"

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
		selection_mode_enabled_id_(0),
		fog_enabled_id_(0)
	{
	}

	~ShadingController()
	{
		if(good())
		{
			glDeleteProgram(shader_program_);
		}
	}

	bool init(const std::string& vertex_shader_filename, const std::string& fragment_shader_filename)
	{
		if(good())
		{
			return false;
		}

		std::vector<char> vertex_shader_source;
		if(!read_file_to_memory(vertex_shader_filename, vertex_shader_source))
		{
			std::cout << "Error: failed to read vertex shader source file '" << vertex_shader_filename << "'." << std::endl;
			return false;
		}

		std::vector<char> fragment_shader_source;
		if(!read_file_to_memory(fragment_shader_filename, fragment_shader_source))
		{
			std::cout << "Error: failed to read fragment shader source file '" << fragment_shader_filename << "'." << std::endl;
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
			std::cout << "Error: failed to compile vertex shader." << std::endl;
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
			std::cout << "Error: failed to compile fragment shader." << std::endl;
			return false;
		}

		shader_program_=glCreateProgram();
		glAttachShader(shader_program_, vertex_shader);
		glAttachShader(shader_program_, fragment_shader);
		glLinkProgram(shader_program_);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		projection_matrix_id_=glGetUniformLocation(shader_program_, "projection_matrix");
		viewtransform_matrix_id_=glGetUniformLocation(shader_program_, "viewtransform_matrix");
		modeltransform_matrix_id_=glGetUniformLocation(shader_program_, "modeltransform_matrix");
		selection_mode_enabled_id_=glGetUniformLocation(shader_program_, "selection_mode_enabled");
		fog_enabled_id_=glGetUniformLocation(shader_program_, "fog_enabled");

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
			glUseProgram(shader_program_);
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

	bool set_selection_mode_enabled(const bool status)
	{
		if(enable())
		{
			glUniform1i(selection_mode_enabled_id_, (status ? 1 : 0));
			return true;
		}
		return false;
	}

	bool set_fog_enabled(const bool status)
	{
		if(enable())
		{
			glUniform1i(fog_enabled_id_, (status ? 1 : 0));
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
				std::cout << "Shader compile log:\n" << log << std::endl;
			}
		}
		return (status!=GL_TRUE);
	}

	static bool read_file_to_memory(const std::string& filename, std::vector<char>& output)
	{
		std::string predefined_input;

		if(filename=="_vertex_shader_simple")
		{
			predefined_input=
					"#version 100\n"
					"uniform mat4 projection_matrix;\n"
					"uniform mat4 viewtransform_matrix;\n"
					"uniform mat4 modeltransform_matrix;\n"
					"attribute vec3 vertex_position;\n"
					"attribute vec3 vertex_normal;\n"
					"attribute vec3 vertex_color_for_selection;\n"
					"attribute vec3 vertex_color_for_display;\n"
					"attribute vec3 vertex_adjunct;\n"
					"varying vec3 fragment_position;\n"
					"varying vec3 fragment_normal;\n"
					"varying vec3 fragment_color_for_selection;\n"
					"varying vec3 fragment_color_for_display;\n"
					"varying vec3 fragment_adjunct;\n"
					"void main()\n"
					"{\n"
					"    vec4 vertex_position_in_world=modeltransform_matrix*vec4(vertex_position, 1.0);\n"
					"    fragment_position=vec3(vertex_position_in_world);\n"
					"    fragment_normal=mat3(modeltransform_matrix)*vertex_normal;\n"
					"    fragment_color_for_selection=vertex_color_for_selection;\n"
					"    fragment_color_for_display=vertex_color_for_display;\n"
					"    fragment_adjunct=vertex_adjunct;\n"
					"    gl_Position=projection_matrix*viewtransform_matrix*vertex_position_in_world;\n"
					"}\n";
		}
		else if(filename=="_vertex_shader_with_instancing")
		{
			predefined_input=
					"#version 100\n"
					"uniform mat4 projection_matrix;\n"
					"uniform mat4 viewtransform_matrix;\n"
					"uniform mat4 modeltransform_matrix;\n"
					"attribute vec3 vertex_position;\n"
					"attribute vec3 vertex_normal;\n"
					"attribute vec3 vertex_color_for_selection;\n"
					"attribute vec3 vertex_color_for_display;\n"
					"attribute vec3 vertex_adjunct;\n"
					"attribute vec4 vertex_transformation_0;\n"
					"attribute vec4 vertex_transformation_1;\n"
					"attribute vec4 vertex_transformation_2;\n"
					"attribute vec4 vertex_transformation_3;\n"
					"varying vec3 fragment_position;\n"
					"varying vec3 fragment_normal;\n"
					"varying vec3 fragment_color_for_selection;\n"
					"varying vec3 fragment_color_for_display;\n"
					"varying vec3 fragment_adjunct;\n"
					"void main()\n"
					"{\n"
					"    mat4 vertex_transformation_matrix=mat4(vertex_transformation_0, vertex_transformation_1, vertex_transformation_2, vertex_transformation_3);\n"
					"    vec4 transformed_vertex_position=vertex_transformation_matrix*vec4(vertex_position, 1.0);\n"
					"    vec3 transformed_vertex_normal=mat3(vertex_transformation_matrix)*vertex_normal;\n"
					"    vec4 vertex_position_in_world=modeltransform_matrix*transformed_vertex_position;\n"
					"    fragment_position=vec3(vertex_position_in_world);\n"
					"    fragment_normal=mat3(modeltransform_matrix)*transformed_vertex_normal;\n"
					"    fragment_color_for_selection=vertex_color_for_selection;\n"
					"    fragment_color_for_display=vertex_color_for_display;\n"
					"    fragment_adjunct=vertex_adjunct;\n"
					"    gl_Position=projection_matrix*viewtransform_matrix*vertex_position_in_world;\n"
					"}\n";
		}
		else if(filename=="_fragment_shader_simple")
		{
			predefined_input=
					"#version 100\n"
					"precision mediump float;\n"
					"uniform int selection_mode_enabled;\n"
					"uniform int fog_enabled;\n"
					"varying vec3 fragment_position;\n"
					"varying vec3 fragment_normal;\n"
					"varying vec3 fragment_color_for_selection;\n"
					"varying vec3 fragment_color_for_display;\n"
					"varying vec3 fragment_adjunct;\n"
					"void main()\n"
					"{\n"
					"    if(selection_mode_enabled==0)\n"
					"    {\n"
					"        vec3 light_direction=vec3(1.0, 1.0, 1.0);\n"
					"        vec3 light_color=vec3(1.0, 1.0, 1.0);\n"
					"        float ambient_value=0.2;\n"
					"        vec3 ambient=ambient_value*light_color;\n"
					"        float diffuse_value=abs(dot(normalize(fragment_normal), normalize(light_direction)));\n"
					"        vec3 diffuse=diffuse_value*light_color;\n"
					"        vec3 final_color=(ambient+diffuse)*fragment_color_for_display;"
					"        if(fog_enabled==1)\n"
					"        {\n"
					"            float fog_density=1.0/(1.0+exp(0.1*(fragment_position.z+0.0)));\n"
					"            final_color=mix(final_color, vec3(1.0, 1.0, 1.0), fog_density);\n"
					"        }\n"
					"        if((fragment_adjunct[0]>0.5) && (mod(floor(gl_FragCoord.x), 2.0)<0.5 || mod(floor(gl_FragCoord.y), 2.0)<0.5))\n"
					"        {\n"
					"            final_color=vec3(1.0, 0.0, 1.0);\n"
					"            if(fragment_color_for_display[0]>0.5 && fragment_color_for_display[1]<0.25 && fragment_color_for_display[2]>0.5)\n"
					"            {\n"
					"                final_color=vec3(0.0, 1.0, 0.0);\n"
					"            }\n"
					"        }\n"
					"        gl_FragColor=vec4(final_color, 1.0);\n"
					"    }\n"
					"    else\n"
					"    {\n"
					"        gl_FragColor=vec4(fragment_color_for_selection, 1.0);\n"
					"    }\n"
					"}\n";
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
	GLuint selection_mode_enabled_id_;
	GLuint fog_enabled_id_;
};

}

#endif /* UV_SHADING_CONTROLLER_H_ */
