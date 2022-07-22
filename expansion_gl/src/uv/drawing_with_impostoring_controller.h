#ifndef UV_DRAWING_WITH_IMPOSTORING_CONTROLLER_H_
#define UV_DRAWING_WITH_IMPOSTORING_CONTROLLER_H_

#include "common.h"
#include "generic_drawing_controller.h"

namespace voronota
{

namespace uv
{

class DrawingWithImpostoringController : public GenericDrawingController, private Noncopyable
{
public:
	DrawingWithImpostoringController() :
		number_of_vertices_(0),
		vao_(0),
		vbo_for_vertices1_(0),
		vbo_for_vertices2_(0),
		vbo_for_colors_for_selection_(0),
		vbo_for_colors_for_display_(0),
		vbo_for_adjuncts_(0),
		vbo_for_indices_(0),
		change_of_colors_for_selection_(false),
		change_of_colors_for_display_(false),
		change_of_adjuncts_(false),
		change_of_indices_(false)
	{
		cached_map_iterator_=map_of_records_.end();
	}

	virtual ~DrawingWithImpostoringController()
	{
		if(good())
		{
			glDeleteBuffers(1, &vbo_for_vertices1_);
			glDeleteBuffers(1, &vbo_for_vertices2_);
			glDeleteBuffers(1, &vbo_for_colors_for_selection_);
			glDeleteBuffers(1, &vbo_for_colors_for_display_);
			glDeleteBuffers(1, &vbo_for_adjuncts_);
			glDeleteBuffers(1, &vbo_for_indices_);
			glDeleteVertexArrays(1, &vao_);
		}
	}

	static const std::vector<std::string>& ordered_used_shader_attribute_names()
	{
		static std::vector<std::string> names;
		if(names.empty())
		{
			names.push_back("vertex_position1");
			names.push_back("vertex_position2");
			names.push_back("vertex_color_for_selection");
			names.push_back("vertex_color_for_display");
			names.push_back("vertex_adjunct");
		}
		return names;
	}

	bool init(
			const std::vector<GLfloat>& vertices1,
			const std::vector<GLfloat>& vertices2)
	{
		if(good())
		{
			return false;
		}

		if(
				vertices1.size()<4 ||
				vertices1.size()%4!=0 ||
				vertices1.size()!=vertices2.size())
		{
			return false;
		}

		number_of_vertices_=vertices1.size()/4;

		std::vector<GLuint> indices(number_of_vertices_, 0);
		for(GLuint i=0;i<static_cast<GLuint>(number_of_vertices_);i++)
		{
			indices[i]=i;
		}

		buffer_for_colors_for_selection_.resize(number_of_vertices_*3, 1.0f);
		buffer_for_colors_for_display_.resize(number_of_vertices_*3, 1.0f);
		buffer_for_adjuncts_.resize(number_of_vertices_*3, 0.0f);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vbo_for_vertices1_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_vertices1_);
		glBufferData(GL_ARRAY_BUFFER, vertices1.size()*sizeof(GLfloat), vertices1.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_vertices2_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_vertices2_);
		glBufferData(GL_ARRAY_BUFFER, vertices2.size()*sizeof(GLfloat), vertices2.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_colors_for_selection_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_selection_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_selection_.size()*sizeof(GLfloat), buffer_for_colors_for_selection_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_colors_for_display_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_display_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_display_.size()*sizeof(GLfloat), buffer_for_colors_for_display_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_adjuncts_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_adjuncts_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_adjuncts_.size()*sizeof(GLfloat), buffer_for_adjuncts_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_indices_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_for_indices_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &vao_);
		glBindVertexArray(vao_);

		{
			const GLuint vbos[5]={vbo_for_vertices1_, vbo_for_vertices2_, vbo_for_colors_for_selection_, vbo_for_colors_for_display_, vbo_for_adjuncts_};
			for(int i=0;i<5;i++)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
				glVertexAttribPointer(i, (i<2 ? 4 : 3), GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(i);
			}
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_for_indices_);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		buffer_for_indices_.reserve(indices.size());

		return true;
	}

	bool good() const
	{
		return (number_of_vertices_>=1);
	}

	bool draw()
	{
		if(good())
		{
			refresh();
			if(!buffer_for_indices_.empty())
			{
				glBindVertexArray(vao_);
				glDrawElements(GL_POINTS, buffer_for_indices_.size(), GL_UNSIGNED_INT, 0);
				{
					GLenum err=glGetError();
					while(err!=GL_NO_ERROR)
					{
						std::cerr << "OpenGL error " << err << " in file " __FILE__ << " line " << __LINE__ << "\n";
						err=glGetError();
					}
				}
				glBindVertexArray(0);
			}
			return true;
		}
		return true;
	}

	bool object_register(unsigned int id, const GLuint index)
	{
		if(good())
		{
			if(index<number_of_vertices_)
			{
				cached_map_iterator_=map_of_records_.insert(cached_map_iterator_, std::make_pair(id, Record()));
				Record& record=cached_map_iterator_->second;
				record.indices.resize(1);
				record.indices[0]=index;
				Utilities::calculate_color_from_integer(id, &buffer_for_colors_for_selection_[record.indices[0]*3]);
				change_of_colors_for_selection_=true;
				change_of_indices_=true;
				return true;
			}
		}
		return false;
	}

	bool object_unregister(unsigned int id)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				change_of_indices_=change_of_indices_ || (it->second.visible && it->second.enabled);
				map_of_records_.erase(it);
				cached_map_iterator_=map_of_records_.end();
				return true;
			}
		}
		return false;
	}

	bool object_set_visible(unsigned int id, const bool visible)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				change_of_indices_=change_of_indices_ || ((visible && !record.visible && record.enabled) || (!visible && record.visible && record.enabled));
				record.visible=visible;
				return true;
			}
		}
		return false;
	}

	bool object_set_enabled(unsigned int id, const bool enabled)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				change_of_indices_=change_of_indices_ || ((enabled && !record.enabled && record.visible) || (!enabled && record.enabled && record.visible));
				record.enabled=enabled;
				return true;
			}
		}
		return false;
	}

	bool object_set_color(unsigned int id, unsigned int rgb)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				for(std::size_t i=0;i<record.indices.size();i++)
				{
					Utilities::calculate_color_from_integer(rgb, &buffer_for_colors_for_display_[record.indices[i]*3]);
				}
				change_of_colors_for_display_=true;
				return true;
			}
		}
		return false;
	}

	bool object_set_adjunct(unsigned int id, const float a, const float b, const float c)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				for(std::size_t i=0;i<record.indices.size();i++)
				{
					GLfloat* v=&buffer_for_adjuncts_[record.indices[i]*3];
					v[0]=a;
					v[1]=b;
					v[2]=c;
				}
				change_of_adjuncts_=true;
				return true;
			}
		}
		return false;
	}

	void objects_set_visible(const bool visible)
	{
		if(good())
		{
			for(std::map<unsigned int, Record>::iterator it=map_of_records_.begin();it!=map_of_records_.end();++it)
			{
				it->second.visible=visible;
			}
			change_of_indices_=true;
		}
	}

	void objects_set_enabled(const bool enabled)
	{
		if(good())
		{
			for(std::map<unsigned int, Record>::iterator it=map_of_records_.begin();it!=map_of_records_.end();++it)
			{
				it->second.enabled=enabled;
			}
			change_of_indices_=true;
		}
	}

private:
	struct Record
	{
		std::vector<GLuint> indices;
		bool visible;
		bool enabled;

		Record() : visible(true), enabled(true)
		{
		}
	};

	std::map<unsigned int, Record>::iterator find_in_map_of_records(unsigned int id)
	{
		if(cached_map_iterator_!=map_of_records_.end())
		{
			if(cached_map_iterator_->first==id)
			{
				return cached_map_iterator_;
			}
			else
			{
				++cached_map_iterator_;
				if(cached_map_iterator_!=map_of_records_.end() && cached_map_iterator_->first==id)
				{
					return cached_map_iterator_;
				}
			}
		}
		cached_map_iterator_=map_of_records_.find(id);
		return cached_map_iterator_;
	}

	bool refresh_colors_for_selection()
	{
		if(change_of_colors_for_selection_)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_selection_);
			glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_colors_for_selection_.size()*sizeof(GLfloat)), buffer_for_colors_for_selection_.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			change_of_colors_for_selection_=false;
			return true;
		}
		return false;
	}

	bool refresh_colors_for_display()
	{
		if(change_of_colors_for_display_)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_display_);
			glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_colors_for_display_.size()*sizeof(GLfloat)), buffer_for_colors_for_display_.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			change_of_colors_for_display_=false;
			return true;
		}
		return false;
	}

	bool refresh_adjuncts()
	{
		if(change_of_adjuncts_)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_for_adjuncts_);
			glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_adjuncts_.size()*sizeof(GLfloat)), buffer_for_adjuncts_.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			change_of_adjuncts_=false;
			return true;
		}
		return false;
	}

	bool refresh_indices()
	{
		if(change_of_indices_)
		{
			buffer_for_indices_.clear();
			for(std::map<unsigned int, Record>::const_iterator it=map_of_records_.begin();it!=map_of_records_.end();++it)
			{
				const Record& record=it->second;
				if(record.visible && record.enabled)
				{
					for(std::size_t i=0;i<record.indices.size();i++)
					{
						buffer_for_indices_.push_back(record.indices[i]);
					}
				}
			}
			if(!buffer_for_indices_.empty())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_for_indices_);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_for_indices_.size()*sizeof(GLuint), buffer_for_indices_.data(), GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			change_of_indices_=false;
			return true;
		}
		return false;
	}

	bool refresh()
	{
		const bool status1=refresh_colors_for_selection();
		const bool status2=refresh_colors_for_display();
		const bool status3=refresh_adjuncts();
		const bool status4=refresh_indices();
		return (status1 || status2 || status3 || status4);
	}

	unsigned int number_of_vertices_;
	GLuint vao_;
	GLuint vbo_for_vertices1_;
	GLuint vbo_for_vertices2_;
	GLuint vbo_for_colors_for_selection_;
	GLuint vbo_for_colors_for_display_;
	GLuint vbo_for_adjuncts_;
	GLuint vbo_for_indices_;
	bool change_of_colors_for_selection_;
	bool change_of_colors_for_display_;
	bool change_of_adjuncts_;
	bool change_of_indices_;
	std::map<unsigned int, Record> map_of_records_;
	std::map<unsigned int, Record>::iterator cached_map_iterator_;
	std::vector<GLfloat> buffer_for_colors_for_selection_;
	std::vector<GLfloat> buffer_for_colors_for_display_;
	std::vector<GLfloat> buffer_for_adjuncts_;
	std::vector<GLuint> buffer_for_indices_;
};

}

}

#endif /* UV_DRAWING_WITH_IMPOSTORING_CONTROLLER_H_ */
