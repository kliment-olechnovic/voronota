#ifndef UV_VIEWER_APPLICATION_H_
#define UV_VIEWER_APPLICATION_H_

#include "common.h"
#include "shading_controller.h"
#include "transformation_matrix_controller.h"
#include "zoom_calculator.h"
#include "framebuffer_controller.h"
#include "drawing_for_screen_controller.h"
#include "drawing_controller.h"
#include "drawing_with_instancing_controller.h"
#include "drawing_with_impostoring_controller.h"

#include <GLFW/glfw3.h>

namespace voronota
{

namespace uv
{

class ViewerApplication : private Noncopyable
{
public:
	struct InitializationParameters
	{
		int suggested_window_width;
		int suggested_window_height;
		bool no_fps_limit;
		bool verbose;
		bool hidden;
		std::string title;
		std::string shader_vertex_screen;
		std::string shader_vertex;
		std::string shader_vertex_with_instancing;
		std::string shader_vertex_with_impostoring;
		std::string shader_fragment_screen;
		std::string shader_fragment;
		std::string shader_fragment_with_instancing;
		std::string shader_fragment_with_impostoring;

		InitializationParameters() :
			suggested_window_width(800),
			suggested_window_height(600),
			no_fps_limit(false),
			verbose(false),
			hidden(false)
		{
		}
	};

	static ViewerApplication& instance()
	{
		return (*instance_ptr());
	}

	static void instance_render_frame()
	{
		instance().render_frame_wrapped();
	}

	static void instance_refresh_frame(const bool force)
	{
		if(instance().allowed_to_refresh_frame_)
		{
			if(force)
			{
				instance().render_frame_raw();
			}
			else
			{
				instance().num_of_refresh_calls_since_last_render_++;
			}
		}
	}

	bool init(const InitializationParameters& parameters)
	{
		if(good())
		{
			return false;
		}

		if(!glfwInit())
		{
			std::cerr << "Error: failed to init GLFW3" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		if(parameters.hidden)
		{
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		}

		window_=glfwCreateWindow(parameters.suggested_window_width, parameters.suggested_window_height, parameters.title.c_str(), 0, 0);
		if(!window_)
		{
			std::cerr << "Error: failed to create window with GLFW3" << std::endl;
			glfwTerminate();
			return false;
		}

		glfwSetWindowUserPointer(window_, this);

		glfwGetWindowSize(window_, &window_width_, &window_height_);
		glfwGetFramebufferSize(window_, &framebuffer_width_, &framebuffer_height_);

		glfwSetWindowSizeCallback(window_, callback_on_window_resized);
		glfwSetFramebufferSizeCallback(window_, callback_on_framebuffer_resized);
		glfwSetScrollCallback(window_, callback_on_window_scrolled);
		glfwSetMouseButtonCallback(window_, callback_on_mouse_button_used);
		glfwSetCursorPosCallback(window_, callback_on_mouse_cursor_moved);
		glfwSetCursorEnterCallback(window_, callback_on_mouse_cursor_entered);
		glfwSetKeyCallback(window_, callback_on_key_used);
		glfwSetCharCallback(window_, callback_on_character_used);

		glfwMakeContextCurrent(window_);

		if(parameters.no_fps_limit)
		{
			glfwSwapInterval(0);
		}

		glewExperimental=GL_TRUE;
		glewInit();

		if(parameters.verbose)
		{
			{
				const GLubyte* renderer;
				renderer=glGetString(GL_RENDERER);
				std::cerr << "Renderer: " << renderer << std::endl;
			}

			{
				const GLubyte* version;
				version=glGetString(GL_VERSION);
				std::cerr << "Version: " << version << std::endl;
			}
		}

		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		wrapped_glEnable_GL_VERTEX_PROGRAM_POINT_SIZE();

		if(multisampling_mode_!=MultisamplingMode::none)
		{
			wrapped_glEnable_GL_MULTISAMPLE();
		}
		else
		{
			wrapped_glDisable_GL_MULTISAMPLE();
		}

		if(!shading_screen_.init(parameters.shader_vertex_screen, parameters.shader_fragment_screen, DrawingForScreenController::ordered_used_shader_attribute_names()))
		{
			std::cerr << "Error: failed to init shading for screen." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!shading_simple_.init(parameters.shader_vertex, parameters.shader_fragment, DrawingController::ordered_used_shader_attribute_names()))
		{
			std::cerr << "Error: failed to init shading simple." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!shading_with_instancing_.init(parameters.shader_vertex_with_instancing, parameters.shader_fragment_with_instancing, DrawingWithInstancingController::ordered_used_shader_attribute_names()))
		{
			std::cerr << "Error: failed to init shading with instancing." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!shading_with_impostoring_.init(parameters.shader_vertex_with_impostoring, parameters.shader_fragment_with_impostoring, DrawingWithImpostoringController::ordered_used_shader_attribute_names()))
		{
			std::cerr << "Error: failed to init shading with impostoring." << std::endl;
			glfwTerminate();
			return false;
		}


		if(!virtual_screen_a_framebuffer_controller_.init(framebuffer_width_, framebuffer_height_))
		{
			std::cerr << "Error: failed to init first virtual screen framebuffer controller." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!virtual_screen_b_framebuffer_controller_.init(framebuffer_width_, framebuffer_height_))
		{
			std::cerr << "Error: failed to init second virtual screen framebuffer controller." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!drawing_for_screen_controller_.init())
		{
			std::cerr << "Error: failed to init screen drawing controller." << std::endl;
			glfwTerminate();
			return false;
		}

		modeltransform_matrix_=TransformationMatrixController();

		refresh_shading_projection();
		refresh_shading_viewtransform();
		refresh_shading_modeltransform();

		good_=true;

		on_after_init_success();

		return true;
	}

	void run_loop()
	{
		while(good() && !glfwWindowShouldClose(window_))
		{
			render_frame_wrapped();
		}
	}

	bool good() const
	{
		return good_;
	}

	GLFWwindow* window()
	{
		return window_;
	}

	int window_width() const
	{
		return window_width_;
	}

	int window_height() const
	{
		return window_height_;
	}

	int framebuffer_width() const
	{
		return framebuffer_width_;
	}

	int framebuffer_height() const
	{
		return framebuffer_height_;
	}

	double framebuffer_and_window_ratio() const
	{
		if(window_width_<=0 || framebuffer_width_<=0 || window_width_==framebuffer_width_)
		{
			return 1.0;
		}
		return (static_cast<double>(framebuffer_width_)/static_cast<double>(window_width_));
	}

	int effective_rendering_window_width() const
	{
		return adjust_length_with_margin(window_width_, margin_right_fixed_);
	}

	int effective_rendering_window_height() const
	{
		return adjust_length_with_margin(window_height_, margin_top_fixed_);
	}

	int effective_rendering_framebuffer_width() const
	{
		return adjust_length_with_margin(framebuffer_width_, static_cast<int>(margin_right_fixed_*framebuffer_and_window_ratio()));
	}

	int effective_rendering_framebuffer_height() const
	{
		return adjust_length_with_margin(framebuffer_height_, static_cast<int>(margin_top_fixed_*framebuffer_and_window_ratio()));
	}

	float mouse_x() const
	{
		return static_cast<float>(mouse_cursor_x_);
	}

	float mouse_y() const
	{
		return static_cast<float>(mouse_cursor_y_);
	}

	bool hovered() const
	{
		return hovered_;
	}

	const float* background_color() const
	{
		return &background_color_[0];
	}

	const float* margin_color() const
	{
		return &margin_color_[0];
	}

	bool rendering_mode_is_simple() const
	{
		return (rendering_mode_==RenderingMode::simple);
	}

	bool rendering_mode_is_stereo() const
	{
		return (rendering_mode_==RenderingMode::stereo);
	}

	bool rendering_mode_is_grid() const
	{
		return (rendering_mode_==RenderingMode::grid);
	}

	bool projection_mode_is_ortho() const
	{
		return (projection_mode_==ProjectionMode::ortho);
	}

	bool projection_mode_is_perspective() const
	{
		return (projection_mode_==ProjectionMode::perspective);
	}

	float stereo_angle() const
	{
		return stereo_angle_;
	}

	float stereo_offset() const
	{
		return stereo_offset_;
	}

	float perspective_field_of_view() const
	{
		return perspective_field_of_view_;
	}

	float perspective_near_z() const
	{
		return perspective_near_z_;
	}

	float perspective_far_z() const
	{
		return perspective_far_z_;
	}

	int grid_size() const
	{
		return grid_size_;
	}

	int num_of_refresh_calls_since_last_render() const
	{
		return num_of_refresh_calls_since_last_render_;
	}

	bool occlusion_mode_is_none() const
	{
		return (occlusion_mode_==OcclusionMode::none);
	}

	bool occlusion_mode_is_noisy() const
	{
		return (occlusion_mode_==OcclusionMode::noisy);
	}

	bool occlusion_mode_is_smooth() const
	{
		return (occlusion_mode_==OcclusionMode::smooth);
	}

	bool antialiasing_mode_is_none() const
	{
		return (antialiasing_mode_==AntialiasingMode::none);
	}

	bool antialiasing_mode_is_fast() const
	{
		return (antialiasing_mode_==AntialiasingMode::fast);
	}

	bool multisampling_mode_is_none() const
	{
		return (multisampling_mode_==MultisamplingMode::none);
	}

	bool multisampling_mode_is_basic() const
	{
		return (multisampling_mode_==MultisamplingMode::basic);
	}

	void close()
	{
		if(good())
		{
			glfwSetWindowShouldClose(window_, 1);
		}
	}

	void zoom(const ZoomCalculator& zoom_calculator)
	{
		if(!good())
		{
			return;
		}

		zoom_value_=zoom_calculator.get_zoom_factor();
		modeltransform_matrix_.center(zoom_calculator.get_center_position());
		refresh_shading_viewtransform();
		refresh_shading_modeltransform();
	}

	void rotate(const glm::vec3& axis, const float angle)
	{
		if(!good())
		{
			return;
		}

		if(angle!=0.0f && glm::length(axis)>0.0)
		{
			modeltransform_matrix_.add_rotation(angle, glm::normalize(axis));
			refresh_shading_modeltransform();
		}
	}

	void reset_view(const glm::mat4& matrix)
	{
		if(!good())
		{
			return;
		}

		modeltransform_matrix_.reset(matrix);
		refresh_shading_modeltransform();
	}

	void set_window_size(const int width, const int height)
	{
		glfwSetWindowSize(window_, width, height);
	}

	void set_background_color(unsigned int rgb)
	{
		Utilities::calculate_color_from_integer(rgb, background_color_);
	}

	void set_margin_color(unsigned int rgb)
	{
		Utilities::calculate_color_from_integer(rgb, margin_color_);
	}

	void set_rendering_mode_to_simple()
	{
		rendering_mode_=RenderingMode::simple;
	}

	void set_rendering_mode_to_stereo()
	{
		rendering_mode_=RenderingMode::stereo;
	}

	void set_rendering_mode_to_stereo(const float stereo_angle, const float stereo_offset)
	{
		set_stereo_angle(stereo_angle);
		set_stereo_offset(stereo_offset);
		set_rendering_mode_to_stereo();
	}

	void set_rendering_mode_to_grid()
	{
		rendering_mode_=RenderingMode::grid;
	}

	void set_rendering_mode_to_grid(const int grid_size)
	{
		set_grid_size(grid_size);
		set_rendering_mode_to_grid();
	}

	void set_projection_mode_to_ortho()
	{
		projection_mode_=ProjectionMode::ortho;
		refresh_shading_viewtransform();
		refresh_shading_projection();
	}

	void set_projection_mode_to_perspective()
	{
		projection_mode_=ProjectionMode::perspective;
		refresh_shading_viewtransform();
		refresh_shading_projection();
	}

	void set_projection_mode_to_perspective(const float field_of_view, const float near_z, const float far_z)
	{
		set_perspective_field_of_view(field_of_view);
		set_perspective_near_z(near_z);
		set_perspective_far_z(far_z);
		set_projection_mode_to_perspective();
	}

	void set_occlusion_mode_to_none()
	{
		occlusion_mode_=OcclusionMode::none;
	}

	void set_occlusion_mode_to_noisy()
	{
		occlusion_mode_=OcclusionMode::noisy;
	}

	void set_occlusion_mode_to_smooth()
	{
		occlusion_mode_=OcclusionMode::smooth;
	}

	void set_antialiasing_mode_to_none()
	{
		antialiasing_mode_=AntialiasingMode::none;
	}

	void set_antialiasing_mode_to_fast()
	{
		antialiasing_mode_=AntialiasingMode::fast;
	}

	void set_multisampling_mode_to_none()
	{
		changed_multisampling_mode_=(multisampling_mode_!=MultisamplingMode::none);
		multisampling_mode_=MultisamplingMode::none;
	}

	void set_multisampling_mode_to_basic()
	{
		changed_multisampling_mode_=(multisampling_mode_!=MultisamplingMode::basic);
		multisampling_mode_=MultisamplingMode::basic;
	}

	void set_stereo_angle(const float stereo_angle)
	{
		stereo_angle_=stereo_angle;
	}

	void set_stereo_offset(const float stereo_offset)
	{
		stereo_offset_=stereo_offset;
	}

	void set_perspective_field_of_view(const float field_of_view)
	{
		perspective_field_of_view_=field_of_view;
	}

	void set_perspective_near_z(const float near_z)
	{
		if(near_z>0.0f)
		{
			perspective_near_z_=near_z;
		}
		else
		{
			perspective_near_z_=0.01f;
		}
	}

	void set_perspective_far_z(const float far_z)
	{
		if(far_z>perspective_near_z_)
		{
			perspective_far_z_=far_z;
		}
		else
		{
			perspective_far_z_=perspective_near_z_+0.01f;
		}
	}

	void set_grid_size(const int grid_size)
	{
		grid_size_=(grid_size>=1 ? grid_size : 1);
	}

	void set_margin_right_fixed(const int margin_right_size)
	{
		margin_right_fixed_=margin_right_size;
		refresh_shading_projection();
	}

	void set_margin_top_fixed(const int margin_top_size)
	{
		margin_top_fixed_=margin_top_size;
		refresh_shading_projection();
	}

	bool render_in_screenshot_mode_and_read_pixels(const int image_width, const int image_height, std::vector<unsigned char>& image_data)
	{
		if(allowed_to_refresh_frame_)
		{
			render_frame_raw_to_buffer(image_width, image_height);

			image_data.clear();
			image_data.resize(image_width*image_height*4);

			glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_PACK_SKIP_ROWS, 0);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, &image_data[0]);

			return true;
		}
		return false;
	}

	void save_view_to_stream(std::ostream& output) const
	{
		output << "UV_VIEWER_APPLICATION_EXPORTED_VIEW\n";

		output << zoom_value_ << "\n";

		output << modeltransform_matrix_ << "\n";

		output << background_color_[0] << " " << background_color_[1] << " " << background_color_[2] << "\n";

		if(projection_mode_==ProjectionMode::perspective) { output << "perspective"; }
		else { output << "ortho"; }
		output << "\n";

		if(rendering_mode_==RenderingMode::grid) { output << "grid"; }
		else if(rendering_mode_==RenderingMode::stereo) { output << "stereo"; }
		else { output << "simple"; }
		output << "\n";

		output << window_width() << " " << window_height() << "\n";
		output << effective_rendering_window_width() << " " << effective_rendering_window_height() << "\n";
	}

	bool load_view_from_stream(std::istream& input, std::vector<int>& recommended_effective_rendering_size)
	{
		recommended_effective_rendering_size.clear();

		bool good_start=false;
		while(!good_start && input.good())
		{
			std::string token;
			input >> token;
			good_start=(token=="UV_VIEWER_APPLICATION_EXPORTED_VIEW");
		}
		if(!good_start)
		{
			return false;
		}

		double zoom_value=0.0;
		TransformationMatrixController modeltransform_matrix;
		float background_color[3];
		std::string projection_mode;
		std::string rendering_mode;
		int used_window_width=0;
		int used_window_height=0;
		int recommended_width=0;
		int recommended_height=0;

		input >> zoom_value >> modeltransform_matrix;
		input >> background_color[0] >> background_color[1] >> background_color[2];
		input >> projection_mode >> rendering_mode;
		input >> used_window_width >> used_window_height;
		input >> recommended_width >> recommended_height;

		if(input.fail() || zoom_value<=0.0)
		{
			return false;
		}

		if(projection_mode=="perspective") { projection_mode_=ProjectionMode::perspective; }
		else if(projection_mode=="ortho") { projection_mode_=ProjectionMode::ortho; }
		else { return false; }

		if(rendering_mode=="grid") { rendering_mode_=RenderingMode::grid; }
		else if(rendering_mode=="stereo") { rendering_mode_=RenderingMode::stereo; }
		else if(rendering_mode=="simple") { rendering_mode_=RenderingMode::simple; }
		else { return false; }

		zoom_value_=zoom_value;
		modeltransform_matrix_=modeltransform_matrix;
		background_color_[0]=background_color[0];
		background_color_[1]=background_color[1];
		background_color_[2]=background_color[2];

		if(recommended_width>0 && recommended_height>0 && used_window_width<=window_width() && used_window_height<=window_height())
		{
			recommended_effective_rendering_size.push_back(recommended_width);
			recommended_effective_rendering_size.push_back(recommended_height);
		}

		refresh_shading_viewtransform();
		refresh_shading_modeltransform();
		refresh_shading_projection();

		return true;
	}

protected:
	ViewerApplication() :
		good_(false),
		allowed_to_refresh_frame_(false),
		window_(0),
		window_width_(0),
		window_height_(0),
		framebuffer_width_(0),
		framebuffer_height_(0),
		margin_right_fixed_(0),
		margin_top_fixed_(0),
		zoom_value_(1.0),
		zoom_value_step_(1.05),
		ortho_z_near_(-3.0f),
		ortho_z_far_(3.0f),
		mouse_button_left_down_(false),
		mouse_button_right_down_(false),
		mouse_button_left_click_possible_(false),
		mouse_button_right_click_possible_(false),
		mouse_cursor_x_(0.0),
		mouse_cursor_y_(0.0),
		mouse_cursor_prev_x_(0.0),
		mouse_cursor_prev_y_(0.0),
		hovered_(false),
		call_for_selection_(0),
		stereo_angle_(0.09),
		stereo_offset_(0.0),
		perspective_field_of_view_(0.70f),
		perspective_near_z_(1.0f),
		perspective_far_z_(1000.0f),
		grid_size_(1),
		num_of_refresh_calls_since_last_render_(0),
		skipping_virtual_screen_framebuffers_(false),
		rendering_mode_(RenderingMode::simple),
		projection_mode_(ProjectionMode::ortho),
		occlusion_mode_(OcclusionMode::none),
		antialiasing_mode_(AntialiasingMode::none),
		multisampling_mode_(MultisamplingMode::none),
		changed_multisampling_mode_(false)
	{
		instance_ptr()=this;
		Utilities::calculate_color_from_integer(0, background_color_);
		Utilities::calculate_color_from_integer(0, margin_color_);
	}

	virtual ~ViewerApplication()
	{
		if(good())
		{
			glfwTerminate();
		}
	}

	virtual void on_after_init_success()
	{
	}

	virtual void on_window_resized(int width, int height)
	{
	}

	virtual void on_framebuffer_resized(int width, int height)
	{
	}

	virtual bool check_window_scroll_intercepted(double xoffset, double yoffset)
	{
		return false;
	}

	virtual void on_window_scrolled(double xoffset, double yoffset)
	{
	}

	virtual bool check_mouse_button_use_intercepted(int button, int action, int mods)
	{
		return false;
	}

	virtual void on_mouse_button_used(int button, int action, int mods)
	{
	}

	virtual bool check_mouse_cursor_move_intercepted(double xpos, double ypos)
	{
		return false;
	}

	virtual void on_mouse_cursor_moved(double xpos, double ypos)
	{
	}

	virtual void on_key_used(int key, int scancode, int action, int mods)
	{
	}

	virtual void on_character_used(unsigned int codepoint)
	{
	}

	virtual void on_draw(const ShadingMode::Mode shading_mode, const int grid_id)
	{
	}

	virtual void on_draw_overlay_start(const int box_x, const int box_y, const int box_w, const int box_h)
	{
	}

	virtual void on_draw_overlay_middle(const int box_x, const int box_y, const int box_w, const int box_h, const bool stereo, const bool grid, const int id)
	{
	}

	virtual void on_draw_overlay_end(const int box_x, const int box_y, const int box_w, const int box_h)
	{
	}

	virtual void on_before_rendered_frame()
	{
	}

	virtual void on_after_rendered_frame()
	{
	}

	virtual void on_selection(const unsigned int color, const int button_code, const bool mod_ctrl_left, const bool mod_shift_left, const bool mod_ctrl_right, const bool mod_shift_right)
	{
	}

private:
	class ModKeysStatusController
	{
	public:
		bool shift_left;
		bool shift_right;
		bool ctrl_left;
		bool ctrl_right;

		ModKeysStatusController() :
			shift_left(false),
			shift_right(false),
			ctrl_left(false),
			ctrl_right(false)
		{
		}

		bool shift_any() const
		{
			return (shift_left || shift_right);
		}

		bool ctrl_any() const
		{
			return (ctrl_left || ctrl_right);
		}

		void update(int key, int scancode, int action, int mods)
		{
			if(key==GLFW_KEY_LEFT_SHIFT)
			{
				update(action, shift_left);
			}
			else if(key==GLFW_KEY_RIGHT_SHIFT)
			{
				update(action, shift_right);
			}
			else if(key==GLFW_KEY_LEFT_CONTROL)
			{
				update(action, ctrl_left);
			}
			else if(key==GLFW_KEY_RIGHT_CONTROL)
			{
				update(action, ctrl_right);
			}
		}

	private:
		static void update(const int action, bool& value)
		{
			value=((action==GLFW_PRESS) || (value && action!=GLFW_RELEASE));
		}
	};

	struct RenderingMode
	{
		enum Mode
		{
			simple,
			stereo,
			grid
		};
	};

	struct ProjectionMode
	{
		enum Mode
		{
			ortho,
			perspective
		};
	};

	struct OcclusionMode
	{
		enum Mode
		{
			none,
			noisy,
			smooth
		};
	};

	struct AntialiasingMode
	{
		enum Mode
		{
			none,
			fast
		};
	};

	struct MultisamplingMode
	{
		enum Mode
		{
			none,
			basic
		};
	};

	static ViewerApplication*& instance_ptr()
	{
		static ViewerApplication* ptr=0;
		return ptr;
	}

	static double calculate_stepped_value(const double value, const double value_step, const bool step_up, const bool step_down)
	{
		const double new_value=value*(step_up ? value_step : (step_down ? 1.0/value_step : 1.0));
		if(new_value!=0.0)
		{
			return new_value;
		}
		return value;
	}

	static int adjust_length_with_margin(const int length, const int margin_fixed)
	{
		int value=length;
		if(margin_fixed>0)
		{
			value=(length-margin_fixed);
		}
		value=std::max(value, 1);
		value=std::min(value, length);
		return value;
	}

	static void callback_on_window_resized(GLFWwindow* window, int width, int height)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_window_resized(width, height);
	}

	static void callback_on_framebuffer_resized(GLFWwindow* window, int width, int height)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_framebuffer_resized(width, height);
	}

	static void callback_on_window_scrolled(GLFWwindow* window, double xoffset, double yoffset)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_window_scrolled(xoffset, yoffset);
	}

	static void callback_on_mouse_button_used(GLFWwindow* window, int button, int action, int mods)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_mouse_button_used(button, action, mods);
	}

	static void callback_on_mouse_cursor_moved(GLFWwindow* window, double xpos, double ypos)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_mouse_cursor_moved(xpos, ypos);
	}

	static void callback_on_mouse_cursor_entered(GLFWwindow* window, int entered)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_mouse_cursor_entered(entered);
	}

	static void callback_on_key_used(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_key_used(key, scancode, action, mods);
	}

	static void callback_on_character_used(GLFWwindow* window, unsigned int codepoint)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_character_used(codepoint);
	}

	static void wrapped_glEnable_GL_VERTEX_PROGRAM_POINT_SIZE()
	{
#ifndef FOR_WEB
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
	}

	static void wrapped_glEnable_GL_MULTISAMPLE()
	{
#ifndef FOR_WEB
		glEnable(GL_MULTISAMPLE);
#endif
	}

	static void wrapped_glDisable_GL_MULTISAMPLE()
	{
#ifndef FOR_WEB
		glDisable(GL_MULTISAMPLE);
#endif
	}

	void callback_on_window_resized(int width, int height)
	{
		window_width_=width;
		window_height_=height;
		refresh_shading_projection();
		on_window_resized(width, height);
	}

	void callback_on_framebuffer_resized(int width, int height)
	{
		framebuffer_width_=width;
		framebuffer_height_=height;
		refresh_shading_projection();
		on_framebuffer_resized(width, height);
	}

	void callback_on_window_scrolled(double xoffset, double yoffset)
	{
		if(check_window_scroll_intercepted(xoffset, yoffset))
		{
			return;
		}
		const bool step_up=(yoffset<-0.1);
		const bool step_down=(yoffset>0.1);
		zoom_value_=calculate_stepped_value(zoom_value_, zoom_value_step_, step_up, step_down);
		refresh_shading_viewtransform();
		on_window_scrolled(xoffset, yoffset);
	}

	void callback_on_mouse_button_used(int button, int action, int mods)
	{
		if(check_mouse_button_use_intercepted(button, action, mods))
		{
			return;
		}
		if(button==GLFW_MOUSE_BUTTON_LEFT)
		{
			if(action==GLFW_PRESS)
			{
				mouse_button_left_down_=true;
				mouse_button_left_click_possible_=true;
			}
			else if(action==GLFW_RELEASE)
			{
				mouse_button_left_down_=false;
				if(mouse_button_left_click_possible_)
				{
					call_for_selection_=1;
				}
				mouse_button_left_click_possible_=false;
			}
		}
		if(button==GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(action==GLFW_PRESS)
			{
				mouse_button_right_down_=true;
				mouse_button_right_click_possible_=true;
			}
			else if(action==GLFW_RELEASE)
			{
				mouse_button_right_down_=false;
				if(mouse_button_right_click_possible_)
				{
					call_for_selection_=2;
				}
				mouse_button_right_click_possible_=false;
			}
		}
		on_mouse_button_used(button, action, mods);
	}

	void callback_on_mouse_cursor_moved(double xpos, double ypos)
	{
		if(check_mouse_cursor_move_intercepted(xpos, ypos))
		{
			return;
		}
		mouse_cursor_prev_x_=mouse_cursor_x_;
		mouse_cursor_prev_y_=mouse_cursor_y_;
		mouse_cursor_x_=xpos;
		mouse_cursor_y_=ypos;
		mouse_button_left_click_possible_=false;
		mouse_button_right_click_possible_=false;
		perform_trackball_operation(
				mouse_button_left_down_,
				(mouse_button_right_down_ && !modkeys_status_.shift_any() && !modkeys_status_.ctrl_any()),
				(mouse_button_right_down_ && modkeys_status_.shift_any() && !modkeys_status_.ctrl_any()),
				(mouse_button_right_down_ && !modkeys_status_.shift_any() && modkeys_status_.ctrl_any()));
		on_mouse_cursor_moved(xpos, ypos);
	}

	void callback_on_mouse_cursor_entered(int entered)
	{
		if(entered)
		{
			hovered_=true;
		}
		else
		{
			hovered_=false;
		}
	}

	void callback_on_key_used(int key, int scancode, int action, int mods)
	{
		modkeys_status_.update(key, scancode, action, mods);
		on_key_used(key, scancode, action, mods);
	}

	void callback_on_character_used(unsigned int codepoint)
	{
		on_character_used(codepoint);
	}

	bool mouse_cursor_inside() const
	{
		const int x=static_cast<int>(mouse_cursor_x_);
		const int y=window_height_-static_cast<int>(mouse_cursor_y_);
		return (x>0 && x<effective_rendering_window_width() && y>0 && y<effective_rendering_window_height());
	}

	int calc_pixel_x(const double screen_x) const
	{
		return static_cast<int>((screen_x/static_cast<double>(effective_rendering_window_width()))*static_cast<double>(effective_rendering_framebuffer_width()));
	}

	int calc_pixel_y(const double screen_y) const
	{
		return static_cast<int>((screen_y/static_cast<double>(effective_rendering_window_height()))*static_cast<double>(effective_rendering_framebuffer_height()));
	}

	void render_frame_wrapped()
	{
		if(!good())
		{
			return;
		}

		render_frame_raw();

		allowed_to_refresh_frame_=true;

		on_after_rendered_frame();

		allowed_to_refresh_frame_=false;
	}

	void render_frame_raw()
	{
		num_of_refresh_calls_since_last_render_=0;

		if(!good())
		{
			return;
		}

		glfwPollEvents();

		on_before_rendered_frame();

		glEnable(GL_DEPTH_TEST);

		if(changed_multisampling_mode_)
		{
			if(multisampling_mode_!=MultisamplingMode::none)
			{
				wrapped_glEnable_GL_MULTISAMPLE();
			}
			else
			{
				wrapped_glDisable_GL_MULTISAMPLE();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glScissor(0, 0, framebuffer_width_, framebuffer_height_);
		glViewport(0, 0, framebuffer_width_, framebuffer_height_);
		glClearColor(margin_color_[0], margin_color_[1], margin_color_[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ShadingController::managed_glUseProgram(0);

		skipping_virtual_screen_framebuffers_=(occlusion_mode_==OcclusionMode::none && antialiasing_mode_==AntialiasingMode::none);

		if(!skipping_virtual_screen_framebuffers_)
		{
			virtual_screen_a_framebuffer_controller_.init(effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
			virtual_screen_b_framebuffer_controller_.init(effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
			glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_a_framebuffer_controller_.framebuffer());
		}

		glScissor(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
		glViewport(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
		refresh_shading_viewport(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
		glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unsigned int call_for_selection_color=0xFFFFFF;
		int call_for_selection_happenned=0;

		if(call_for_selection_>0 && mouse_cursor_inside())
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			{
				wrapped_glDisable_GL_MULTISAMPLE();

				shading_simple_.set_selection_mode_enabled(true);
				render_scene(ShadingMode::simple);
				shading_simple_.set_selection_mode_enabled(false);

				shading_with_instancing_.set_selection_mode_enabled(true);
				render_scene(ShadingMode::with_instancing);
				shading_with_instancing_.set_selection_mode_enabled(false);

				shading_with_impostoring_.set_selection_mode_enabled(true);
				render_scene(ShadingMode::with_impostoring);
				shading_with_impostoring_.set_selection_mode_enabled(false);

				if(multisampling_mode_!=MultisamplingMode::none)
				{
					wrapped_glEnable_GL_MULTISAMPLE();
				}
			}

			unsigned char pixel[4]={0, 0, 0, 0};
			glReadPixels(calc_pixel_x(mouse_cursor_x_), calc_pixel_y(static_cast<double>(window_height_)-mouse_cursor_y_), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
			call_for_selection_color=static_cast<unsigned int>(pixel[0])*256*256+static_cast<unsigned int>(pixel[1])*256+static_cast<unsigned int>(pixel[2]);
			if(call_for_selection_color!=0xFFFFFF)
			{
				call_for_selection_happenned=call_for_selection_;
			}

			glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		call_for_selection_=0;

		shading_simple_.enable();
		render_scene(ShadingMode::simple);

		shading_with_instancing_.enable();
		render_scene(ShadingMode::with_instancing);

		shading_with_impostoring_.enable();
		render_scene(ShadingMode::with_impostoring);

		if(!skipping_virtual_screen_framebuffers_)
		{
			shading_screen_.set_viewport(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());

			glDisable(GL_DEPTH_TEST);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glScissor(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());
			glViewport(0, 0, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height());

			if(occlusion_mode_==OcclusionMode::noisy)
			{
				shading_screen_.set_mode_number(12);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}
			else if(occlusion_mode_==OcclusionMode::smooth)
			{
				shading_screen_.set_mode_number(11);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				shading_screen_.set_mode_number(21);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_a_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_b_framebuffer_controller_.texture());

				shading_screen_.set_mode_number(22);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}

			if(antialiasing_mode_==AntialiasingMode::fast)
			{
				shading_screen_.set_mode_number(30);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}

			shading_screen_.set_mode_number(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

			glScissor(0, 0, framebuffer_width_, framebuffer_height_);
			glViewport(0, 0, framebuffer_width_, framebuffer_height_);
		}

		glUseProgram(0);
		render_overlay();

		glfwSwapBuffers(window_);

		if(call_for_selection_happenned>0)
		{
			on_selection(call_for_selection_color, call_for_selection_happenned, modkeys_status_.ctrl_left, modkeys_status_.shift_left, modkeys_status_.ctrl_right, modkeys_status_.shift_right);
		}
	}

	void render_frame_raw_to_buffer(const int image_width, const int image_height)
	{
		if(!good())
		{
			return;
		}

		on_before_rendered_frame();

		glEnable(GL_DEPTH_TEST);

		glUseProgram(0);

		virtual_screen_a_framebuffer_controller_.init(image_width, image_height);
		glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_a_framebuffer_controller_.framebuffer());

		glScissor(0, 0, image_width, image_height);
		glViewport(0, 0, image_width, image_height);
		refresh_shading_viewport(0, 0, image_width, image_height);
		glClearColor(background_color_[0], background_color_[1], background_color_[2], 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shading_simple_.enable();
		render_scene(ShadingMode::simple, image_width, image_height, true);

		shading_with_instancing_.enable();
		render_scene(ShadingMode::with_instancing, image_width, image_height, true);

		shading_with_impostoring_.enable();
		render_scene(ShadingMode::with_impostoring, image_width, image_height, true);

		if(occlusion_mode_!=OcclusionMode::none || antialiasing_mode_!=AntialiasingMode::none)
		{
			virtual_screen_b_framebuffer_controller_.init(image_width, image_height);

			shading_screen_.set_viewport(0, 0, image_width, image_height);

			glDisable(GL_DEPTH_TEST);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glScissor(0, 0, image_width, image_height);
			glViewport(0, 0, image_width, image_height);

			if(occlusion_mode_==OcclusionMode::noisy)
			{
				shading_screen_.set_mode_number(12);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}
			else if(occlusion_mode_==OcclusionMode::smooth)
			{
				shading_screen_.set_mode_number(11);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				shading_screen_.set_mode_number(21);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_a_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_b_framebuffer_controller_.texture());

				shading_screen_.set_mode_number(22);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}

			if(antialiasing_mode_==AntialiasingMode::fast)
			{
				shading_screen_.set_mode_number(30);
				glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_b_framebuffer_controller_.framebuffer());
				drawing_for_screen_controller_.draw(virtual_screen_a_framebuffer_controller_.texture());

				virtual_screen_a_framebuffer_controller_.swap(virtual_screen_b_framebuffer_controller_);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, virtual_screen_a_framebuffer_controller_.framebuffer());
		}

		glUseProgram(0);
	}

	void render_scene(const ShadingMode::Mode shading_mode, const int effective_width, const int effective_height, const bool force_refresh_projection)
	{
		if(rendering_mode_==RenderingMode::stereo)
		{
			const int width=effective_width/2;
			{
				refresh_shading_projection(width, effective_height, shading_mode);
			}
			for(int i=0;i<2;i++)
			{
				if(projection_mode_==ProjectionMode::perspective)
				{
					refresh_shading_viewtransform(TransformationMatrixController::create_viewtransform_look_at_stereo(zoom_value_, stereo_angle_, stereo_offset_, i), shading_mode);
				}
				else
				{
					refresh_shading_viewtransform(TransformationMatrixController::create_viewtransform_simple_stereo(zoom_value_, stereo_angle_, stereo_offset_, i), shading_mode);
				}
				const int xpos=(width*i);
				glViewport(xpos, 0, width, effective_height);
				refresh_shading_viewport(xpos, 0, width, effective_height, shading_mode);
				draw_scene(shading_mode, 0);
			}
			refresh_shading_viewtransform(shading_mode);
			refresh_shading_projection(shading_mode);
		}
		else if(rendering_mode_==RenderingMode::grid && grid_size_>1)
		{
			int n_rows=1;
			int n_columns=1;
			Utilities::calculate_grid_dimensions(grid_size_, effective_width, effective_height, n_rows, n_columns);
			const int width=effective_width/n_columns;
			const int height=effective_height/n_rows;
			{
				refresh_shading_projection(width, height, shading_mode);
			}
			int grid_id=0;
			for(int i=0;(i<n_rows) && (grid_id<grid_size_);i++)
			{
				const int ypos=(height*(n_rows-1-i));
				for(int j=0;(j<n_columns) && (grid_id<grid_size_);j++)
				{
					const int xpos=(width*j);
					glViewport(xpos, ypos, width, height);
					refresh_shading_viewport(xpos, ypos, width, height, shading_mode);
					draw_scene(shading_mode, grid_id);
					grid_id++;
				}
			}
			refresh_shading_projection(shading_mode);
		}
		else
		{
			if(force_refresh_projection)
			{
				refresh_shading_projection(effective_width, effective_height, shading_mode);
			}
			draw_scene(shading_mode, 0);
			if(force_refresh_projection)
			{
				refresh_shading_projection(shading_mode);
			}
		}
	}

	void render_scene(const ShadingMode::Mode shading_mode)
	{
		render_scene(shading_mode, effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height(), false);
	}

	void draw_scene(const ShadingMode::Mode shading_mode, const int grid_id)
	{
		on_draw(shading_mode, grid_id);
	}

	void render_overlay()
	{
		on_draw_overlay_start(0, (window_height_-effective_rendering_window_height()), effective_rendering_window_width(), effective_rendering_window_height());
		if(rendering_mode_==RenderingMode::stereo)
		{
			const int width=effective_rendering_window_width()/2;
			for(int i=0;i<2;i++)
			{
				const int xpos=(width*i);
				on_draw_overlay_middle(xpos, 0, width, effective_rendering_window_height(), true, false, i);
			}
		}
		else if(rendering_mode_==RenderingMode::grid && grid_size_>1)
		{
			int n_rows=1;
			int n_columns=1;
			Utilities::calculate_grid_dimensions(grid_size_, effective_rendering_window_width(), effective_rendering_window_height(), n_rows, n_columns);
			const int width=effective_rendering_window_width()/n_columns;
			const int height=effective_rendering_window_height()/n_rows;
			int grid_id=0;
			for(int i=0;(i<n_rows) && (grid_id<grid_size_);i++)
			{
				const int ypos=(window_height_-effective_rendering_window_height())+(height*i);
				for(int j=0;(j<n_columns) && (grid_id<grid_size_);j++)
				{
					const int xpos=(width*j);
					on_draw_overlay_middle(xpos, ypos, width, height, false, true, grid_id);
					grid_id++;
				}
			}
		}
		else
		{
			on_draw_overlay_middle(0, (window_height_-effective_rendering_window_height()), effective_rendering_window_width(), effective_rendering_window_height(), false, false, 0);
		}
		on_draw_overlay_end(0, (window_height_-effective_rendering_window_height()), effective_rendering_window_width(), effective_rendering_window_height());
	}

	void refresh_shading_projection(const TransformationMatrixController& projection_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_projection_matrix(projection_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_projection_matrix(projection_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_impostoring)
		{
			shading_with_impostoring_.set_projection_matrix(projection_matrix.matrix_data());
		}
	}

	void refresh_shading_projection(const int new_width, const int new_height, const ShadingMode::Mode shading_mode)
	{
		if(projection_mode_==ProjectionMode::perspective)
		{
			refresh_shading_projection(TransformationMatrixController::create_projection_perspective(new_width, new_height, perspective_field_of_view_, perspective_near_z_, perspective_far_z_), shading_mode);
		}
		else
		{
			refresh_shading_projection(TransformationMatrixController::create_projection_ortho(new_width, new_height, ortho_z_near_, ortho_z_far_), shading_mode);
		}
	}

	void refresh_shading_projection(const ShadingMode::Mode shading_mode)
	{
		refresh_shading_projection(effective_rendering_framebuffer_width(), effective_rendering_framebuffer_height(), shading_mode);
	}

	void refresh_shading_projection()
	{
		refresh_shading_projection(ShadingMode::simple);
		refresh_shading_projection(ShadingMode::with_instancing);
		refresh_shading_projection(ShadingMode::with_impostoring);
	}

	void refresh_shading_viewtransform(const TransformationMatrixController& viewtransform_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_viewtransform_matrix(viewtransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_viewtransform_matrix(viewtransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_impostoring)
		{
			shading_with_impostoring_.set_viewtransform_matrix(viewtransform_matrix.matrix_data());
		}
	}

	void refresh_shading_viewtransform(const ShadingMode::Mode shading_mode)
	{
		if(projection_mode_==ProjectionMode::perspective)
		{
			refresh_shading_viewtransform(TransformationMatrixController::create_viewtransform_look_at(zoom_value_), shading_mode);
		}
		else
		{
			refresh_shading_viewtransform(TransformationMatrixController::create_viewtransform_simple(zoom_value_), shading_mode);
		}
	}

	void refresh_shading_viewtransform()
	{
		refresh_shading_viewtransform(ShadingMode::simple);
		refresh_shading_viewtransform(ShadingMode::with_instancing);
		refresh_shading_viewtransform(ShadingMode::with_impostoring);
	}

	void refresh_shading_modeltransform(const TransformationMatrixController& modeltransform_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_modeltransform_matrix(modeltransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_modeltransform_matrix(modeltransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_impostoring)
		{
			shading_with_impostoring_.set_modeltransform_matrix(modeltransform_matrix.matrix_data());
		}
	}

	void refresh_shading_modeltransform(const ShadingMode::Mode shading_mode)
	{
		refresh_shading_modeltransform(modeltransform_matrix_, shading_mode);
	}

	void refresh_shading_modeltransform()
	{
		refresh_shading_modeltransform(ShadingMode::simple);
		refresh_shading_modeltransform(ShadingMode::with_instancing);
		refresh_shading_modeltransform(ShadingMode::with_impostoring);
	}

	void refresh_shading_viewport(const int x, const int y, const int w, const int h, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_viewport(x, y, w, h);
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_viewport(x, y, w, h);
		}
		else if(shading_mode==ShadingMode::with_impostoring)
		{
			shading_with_impostoring_.set_viewport(x, y, w, h);
		}
	}

	void refresh_shading_viewport(const int x, const int y, const int w, const int h)
	{
		refresh_shading_viewport(x, y, w, h, ShadingMode::simple);
		refresh_shading_viewport(x, y, w, h, ShadingMode::with_instancing);
		refresh_shading_viewport(x, y, w, h, ShadingMode::with_impostoring);
	}

	bool perform_trackball_operation(const bool rotate, const bool translate, const bool scale, const bool cut)
	{
		if(!(rotate || translate || scale || cut))
		{
			return false;
		}

		if(mouse_cursor_x_==mouse_cursor_prev_x_ && mouse_cursor_y_==mouse_cursor_prev_y_)
		{
			return false;
		}

		if(!mouse_cursor_inside())
		{
			return false;
		}

		bool change_in_modeltransform=false;
		bool change_in_viewtransform=false;
		bool change_in_projection=false;

		const double cursor_x=(mouse_cursor_x_);
		const double cursor_y=(static_cast<double>(window_height_)-mouse_cursor_y_);
		const double cursor_prev_x=(mouse_cursor_prev_x_);
		const double cursor_prev_y=(static_cast<double>(window_height_)-mouse_cursor_prev_y_);

		const double area_width=static_cast<double>(effective_rendering_window_width());
		const double area_height=static_cast<double>(effective_rendering_window_height());
		const double trackball_size=std::min(area_width, area_height);
		const double area_width_in_trackball_size=area_width/trackball_size;
		const double area_height_in_trackball_size=area_height/trackball_size;

		const double p1_x=cursor_prev_x/trackball_size*2-area_width_in_trackball_size;
		const double p1_y=cursor_prev_y/trackball_size*2-area_height_in_trackball_size;

		const double p2_x=cursor_x/trackball_size*2-area_width_in_trackball_size;
		const double p2_y=cursor_y/trackball_size*2-area_height_in_trackball_size;

		if(rotate)
		{
			const double qr1=p1_x*p1_x+p1_y*p1_y;
			const double qr2=p2_x*p2_x+p2_y*p2_y;

			if(qr1<1.0 && qr2<1.0)
			{
				const glm::vec3 p1(static_cast<float>(p1_x), static_cast<float>(p1_y), static_cast<float>(sqrt(1-qr1)));
				const glm::vec3 p2(static_cast<float>(p2_x), static_cast<float>(p2_y), static_cast<float>(sqrt(1-qr2)));
				const glm::vec3 axis=glm::cross(p1, p2);
				const double axis_length=glm::length(axis);
				if(axis_length>0.0)
				{
					const float angle=asin(axis_length);
					if(angle!=0.0f)
					{
						modeltransform_matrix_.add_rotation(angle, glm::normalize(axis));
						change_in_modeltransform=true;
					}
				}
			}
		}

		if(translate)
		{
			const double mod=1.0/zoom_value_;
			const glm::vec3 offset(static_cast<float>((p2_x-p1_x)*mod), static_cast<float>((p2_y-p1_y)*mod), 0.0f);
			modeltransform_matrix_.add_translation(offset);
			change_in_modeltransform=true;
		}

		if(scale)
		{
			zoom_value_+=(p2_y-p1_y)*zoom_value_;
			change_in_viewtransform=true;
		}

		if(cut)
		{
			ortho_z_near_+=(p2_y-p1_y);
			change_in_projection=true;
		}

		if(change_in_modeltransform)
		{
			refresh_shading_modeltransform();
		}

		if(change_in_viewtransform)
		{
			refresh_shading_viewtransform();
		}

		if(change_in_projection)
		{
			refresh_shading_projection();
		}

		return (change_in_modeltransform || change_in_viewtransform || change_in_projection);
	}

	bool good_;
	bool allowed_to_refresh_frame_;
	GLFWwindow* window_;
	int window_width_;
	int window_height_;
	int framebuffer_width_;
	int framebuffer_height_;
	int margin_right_fixed_;
	int margin_top_fixed_;
	double zoom_value_;
	double zoom_value_step_;
	double ortho_z_near_;
	double ortho_z_far_;
	bool mouse_button_left_down_;
	bool mouse_button_right_down_;
	bool mouse_button_left_click_possible_;
	bool mouse_button_right_click_possible_;
	double mouse_cursor_x_;
	double mouse_cursor_y_;
	double mouse_cursor_prev_x_;
	double mouse_cursor_prev_y_;
	bool hovered_;
	int call_for_selection_;
	float stereo_angle_;
	float stereo_offset_;
	float perspective_field_of_view_;
	float perspective_near_z_;
	float perspective_far_z_;
	int grid_size_;
	int num_of_refresh_calls_since_last_render_;
	bool skipping_virtual_screen_framebuffers_;
	RenderingMode::Mode rendering_mode_;
	ProjectionMode::Mode projection_mode_;
	OcclusionMode::Mode occlusion_mode_;
	AntialiasingMode::Mode antialiasing_mode_;
	MultisamplingMode::Mode multisampling_mode_;
	bool changed_multisampling_mode_;
	float background_color_[3];
	float margin_color_[3];
	ShadingController shading_screen_;
	ShadingController shading_simple_;
	ShadingController shading_with_instancing_;
	ShadingController shading_with_impostoring_;
	FramebufferController virtual_screen_a_framebuffer_controller_;
	FramebufferController virtual_screen_b_framebuffer_controller_;
	DrawingForScreenController drawing_for_screen_controller_;
	TransformationMatrixController modeltransform_matrix_;
	ModKeysStatusController modkeys_status_;
};

}

}

#endif /* UV_VIEWER_APPLICATION_H_ */
