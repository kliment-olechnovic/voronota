#ifndef UV_DRAWING_WITH_INSTANCING_CONTROLLER_H_
#define UV_DRAWING_WITH_INSTANCING_CONTROLLER_H_

#include "common.h"
#include "generic_drawing_controller.h"
#include "transformation_matrix_controller.h"

namespace uv
{

class DrawingWithInstancingController : public GenericDrawingController, private Noncopyable
{
public:
	DrawingWithInstancingController() :
		number_of_vertices_(0),
		number_of_indices_(0),
		number_of_instances_(0),
		vao_(0),
		vbo_for_vertices_(0),
		vbo_for_normals_(0),
		vbo_for_indices_(0),
		vbo_for_colors_for_selection_(0),
		vbo_for_colors_for_display_(0),
		vbo_for_adjuncts_(0),
		vbo_for_transformations_0_(0),
		vbo_for_transformations_1_(0),
		vbo_for_transformations_2_(0),
		vbo_for_transformations_3_(0),
		change_of_visibility_(false),
		change_of_colors_for_display_(false),
		change_of_adjuncts_(false),
		change_of_transformations_(false)
	{
		cached_map_iterator_=map_of_records_.end();
	}

	virtual ~DrawingWithInstancingController()
	{
		if(good())
		{
			glDeleteBuffers(1, &vbo_for_vertices_);
			glDeleteBuffers(1, &vbo_for_normals_);
			glDeleteBuffers(1, &vbo_for_indices_);
			glDeleteBuffers(1, &vbo_for_colors_for_selection_);
			glDeleteBuffers(1, &vbo_for_colors_for_display_);
			glDeleteBuffers(1, &vbo_for_adjuncts_);
			glDeleteBuffers(1, &vbo_for_transformations_0_);
			glDeleteBuffers(1, &vbo_for_transformations_1_);
			glDeleteBuffers(1, &vbo_for_transformations_2_);
			glDeleteBuffers(1, &vbo_for_transformations_3_);
			glDeleteVertexArrays(1, &vao_);
		}
	}

	bool init(
			const std::vector<GLfloat>& vertices,
			const std::vector<GLfloat>& normals,
			const std::vector<GLuint>& indices)
	{
		if(good())
		{
			return false;
		}

		if(
				vertices.size()<3 ||
				vertices.size()%3!=0 ||
				vertices.size()!=normals.size() ||
				indices.size()<3 ||
				indices.size()%3!=0)
		{
			return false;
		}

		buffer_for_colors_for_selection_.resize(3, 1.0f);
		buffer_for_colors_for_display_.resize(3, 1.0f);
		buffer_for_adjuncts_.resize(3, 1.0f);
		buffer_for_transformations_0_.resize(4, 1.0f);
		buffer_for_transformations_1_.resize(4, 1.0f);
		buffer_for_transformations_2_.resize(4, 1.0f);
		buffer_for_transformations_3_.resize(4, 1.0f);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vbo_for_vertices_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_vertices_);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_normals_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_normals_);
		glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_indices_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_for_indices_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_colors_for_selection_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_selection_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_selection_.size()*sizeof(GLfloat), buffer_for_colors_for_selection_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_colors_for_display_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_display_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_display_.size()*sizeof(GLfloat), buffer_for_colors_for_display_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_adjuncts_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_adjuncts_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_adjuncts_.size()*sizeof(GLfloat), buffer_for_adjuncts_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_transformations_0_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_0_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_0_.size()*sizeof(GLfloat), buffer_for_transformations_0_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_transformations_1_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_1_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_1_.size()*sizeof(GLfloat), buffer_for_transformations_1_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_transformations_2_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_2_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_2_.size()*sizeof(GLfloat), buffer_for_transformations_2_.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &vbo_for_transformations_3_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_3_);
		glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_0_.size()*sizeof(GLfloat), buffer_for_transformations_3_.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &vao_);
		glBindVertexArray(vao_);

		{
			const GLuint vbos[9]={
					vbo_for_vertices_, vbo_for_normals_, vbo_for_colors_for_selection_, vbo_for_colors_for_display_, vbo_for_adjuncts_,
					vbo_for_transformations_0_, vbo_for_transformations_1_, vbo_for_transformations_2_, vbo_for_transformations_3_
			};
			const GLuint sizes[9]={3, 3, 3, 3, 3, 4, 4, 4, 4};
			const bool instanced[9]={false, false, true, true, true, true, true, true, true};
			for(int i=0;i<9;i++)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
				glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, 0, 0);
				if(instanced[i])
				{
					glVertexAttribDivisor(i, 1);
				}
				glEnableVertexAttribArray(i);
			}
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_for_indices_);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		number_of_vertices_=vertices.size()/3;
		number_of_indices_=indices.size();

		return true;
	}

	bool init_as_sphere(const unsigned int depth)
	{
		if(good())
		{
			return false;
		}

		std::vector<GLfloat> vertices;
		std::vector<GLuint> indices;

		{
			const GLfloat t=(1+sqrt(5.0))/2.0;

			GLfloat initial_vertices[]={
					 t, 1, 0,
					-t, 1, 0,
					 t,-1, 0,
					-t,-1, 0,
					 1, 0, t,
					 1, 0,-t,
					-1, 0, t,
					-1, 0,-t,
					 0, t, 1,
					 0,-t, 1,
					 0, t,-1,
					 0,-t,-1
			};

			GLuint initial_indices[]={
					0, 8, 4,
					1, 10, 7,
					2, 9, 11,
					7, 3, 1,
					0, 5, 10,
					3, 9, 6,
					3, 11, 9,
					8, 6, 4,
					2, 4, 9,
					3, 7, 11,
					4, 2, 0,
					9, 4, 6,
					2, 11, 5,
					0, 10, 8,
					5, 0, 2,
					10, 5, 7,
					1, 6, 8,
					1, 8, 10,
					6, 1, 3,
					11, 7, 5
			};

			vertices.assign(initial_vertices, initial_vertices+12*3);
			indices.assign(initial_indices, initial_indices+20*3);
		}

		for(unsigned int d=0;d<depth && d<5;d++)
		{
			std::map< std::pair<GLuint, GLuint>, GLuint > map_of_edges_;
			std::vector<GLuint> new_indices;
			new_indices.reserve(indices.size()*4);
			for(std::size_t i=0;i<indices.size()/3;i++)
			{
				GLuint ids_outer[3]={indices[i*3+0], indices[i*3+1], indices[i*3+2]};
				std::sort(ids_outer, ids_outer+3);
				std::pair<GLuint, GLuint> pairs[3]={std::make_pair(ids_outer[0], ids_outer[1]), std::make_pair(ids_outer[0], ids_outer[2]), std::make_pair(ids_outer[1], ids_outer[2])};
				GLuint ids_inner[3]={0, 0, 0};
				for(int j=0;j<3;j++)
				{
					std::pair<GLuint, GLuint>& pair=pairs[j];
					std::map< std::pair<GLuint, GLuint>, GLuint >::const_iterator it=map_of_edges_.find(pair);
					if(it==map_of_edges_.end())
					{
						ids_inner[j]=vertices.size()/3;
						map_of_edges_[pair]=ids_inner[j];
						for(int n=0;n<3;n++)
						{
							vertices.push_back((vertices[pair.first*3+n]+vertices[pair.second*3+n])*0.5f);
						}
					}
					else
					{
						ids_inner[j]=it->second;
					}
				}

				new_indices.push_back(ids_outer[0]);
				new_indices.push_back(ids_inner[0]);
				new_indices.push_back(ids_inner[1]);

				new_indices.push_back(ids_outer[1]);
				new_indices.push_back(ids_inner[0]);
				new_indices.push_back(ids_inner[2]);

				new_indices.push_back(ids_outer[2]);
				new_indices.push_back(ids_inner[1]);
				new_indices.push_back(ids_inner[2]);

				new_indices.push_back(ids_inner[0]);
				new_indices.push_back(ids_inner[1]);
				new_indices.push_back(ids_inner[2]);
			}
			indices=new_indices;
		}

		for(std::size_t i=0;i<vertices.size()/3;i++)
		{
			const float l=sqrt(vertices[i*3]*vertices[i*3]+vertices[i*3+1]*vertices[i*3+1]+vertices[i*3+2]*vertices[i*3+2]);
			vertices[i*3]/=l;
			vertices[i*3+1]/=l;
			vertices[i*3+2]/=l;
		}

		return init(vertices, vertices, indices);
	}

	bool init_as_cylinder(const unsigned int number_of_sides)
	{
		if(good())
		{
			return false;
		}

		if(number_of_sides<3 || number_of_sides>360)
		{
			return false;
		}

		std::vector<GLfloat> vertices;
		std::vector<GLfloat> normals;
		std::vector<GLuint> indices;

		vertices.reserve(number_of_sides*6);
		normals.reserve(number_of_sides*6);
		indices.reserve(number_of_sides*6);

		for(unsigned int i=0;i<number_of_sides;i++)
		{
			const double angle=2.0*glm::pi<double>()*(static_cast<double>(i)/static_cast<double>(number_of_sides));
			const GLfloat x=static_cast<GLfloat>(cos(angle));
			const GLfloat y=static_cast<GLfloat>(sin(angle));

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(0.0f);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(1.0f);

			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(0.0f);
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(0.0f);

			const unsigned int j=((i+1)<number_of_sides ? i+1 : 0u);

			indices.push_back(i*2+0);
			indices.push_back(i*2+1);
			indices.push_back(j*2+0);
			indices.push_back(i*2+1);
			indices.push_back(j*2+0);
			indices.push_back(j*2+1);
		}

		return init(vertices, normals, indices);
	}

	bool good() const
	{
		return (number_of_vertices_>=3 && number_of_indices_>=3);
	}

	bool draw()
	{
		if(good())
		{
			refresh();
			if(number_of_instances_>0)
			{
				glBindVertexArray(vao_);
				glDrawElementsInstanced(GL_TRIANGLES, number_of_indices_, GL_UNSIGNED_INT, 0, number_of_instances_);
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

	bool object_register(unsigned int id)
	{
		if(good())
		{
			cached_map_iterator_=map_of_records_.insert(cached_map_iterator_, std::make_pair(id, Record()));
			change_of_visibility_=true;
			return true;
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
				change_of_visibility_=change_of_visibility_ || (it->second.visible && it->second.enabled);
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
				change_of_visibility_=change_of_visibility_ || ((visible && !record.visible && record.enabled) || (!visible && record.visible && record.enabled));
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
				change_of_visibility_=change_of_visibility_ || ((enabled && !record.enabled && record.visible) || (!enabled && record.enabled && record.visible));
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
				Utilities::calculate_color_from_integer(rgb, &record.color[0]);
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
				record.adjunct[0]=a;
				record.adjunct[1]=b;
				record.adjunct[2]=c;
				change_of_adjuncts_=true;
				return true;
			}
		}
		return false;
	}

	bool object_set_transformation_as_for_sphere(unsigned int id, const float radius, const glm::vec3& position)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				record.transformation.reset();
				record.transformation.add_scaling(radius);
				record.transformation.add_translation(position);
				change_of_transformations_=true;
				return true;
			}
		}
		return false;
	}

	bool object_set_transformation_as_for_cylinder(unsigned int id, const float radius, const glm::vec3& position_start, const glm::vec3& position_end)
	{
		if(good())
		{
			std::map<unsigned int, Record>::iterator it=find_in_map_of_records(id);
			if(it!=map_of_records_.end())
			{
				Record& record=it->second;
				record.transformation.reset();
				record.transformation.add_scaling(radius, radius, glm::distance(position_start, position_end));
				{
					const glm::vec3 p1(0.0f, 0.0f, 1.0f);
					const glm::vec3 p2=glm::normalize(position_end-position_start);
					const float dot_product=glm::dot(p1, p2);
					if(dot_product<1.0f)
					{
						if(dot_product>-1.0f)
						{
							const glm::vec3 axis=glm::cross(p1, p2);
							float angle=asin(glm::length(axis));
							if(dot_product<0.0f)
							{
								angle=glm::pi<float>()-angle;
							}
							record.transformation.add_rotation(angle, glm::normalize(axis));
						}
						else
						{
							record.transformation.add_rotation(glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
						}
					}
				}
				record.transformation.add_translation(position_start);
				change_of_transformations_=true;
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
			change_of_visibility_=true;
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
			change_of_visibility_=true;
		}
	}

private:
	struct Record
	{
		unsigned int instance;
		bool visible;
		bool enabled;
		glm::vec3 color;
		glm::vec3 adjunct;
		TransformationMatrixController transformation;

		Record() : instance(0), visible(true), enabled(true), color(1.0f, 1.0f, 1.0f), adjunct(0.0f, 0.0f, 0.0f)
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

	bool refresh()
	{
		if(change_of_visibility_)
		{
			buffer_for_colors_for_selection_.clear();
			buffer_for_colors_for_display_.clear();
			buffer_for_adjuncts_.clear();
			buffer_for_transformations_0_.clear();
			buffer_for_transformations_1_.clear();
			buffer_for_transformations_2_.clear();
			buffer_for_transformations_3_.clear();
			number_of_instances_=0;
			for(std::map<unsigned int, Record>::iterator it=map_of_records_.begin();it!=map_of_records_.end();++it)
			{
				const unsigned int id=it->first;
				Record& record=it->second;
				if(record.visible && record.enabled)
				{
					record.instance=number_of_instances_++;
					for(int i=0;i<3;i++)
					{
						buffer_for_colors_for_selection_.push_back(1.0f);
					}
					Utilities::calculate_color_from_integer(id, &buffer_for_colors_for_selection_[record.instance*3]);
					for(int i=0;i<3;i++)
					{
						buffer_for_colors_for_display_.push_back(record.color[i]);
					}
					for(int i=0;i<3;i++)
					{
						buffer_for_adjuncts_.push_back(record.adjunct[i]);
					}
					for(int i=0;i<4;i++)
					{
						buffer_for_transformations_0_.push_back(record.transformation.matrix()[0][i]);
						buffer_for_transformations_1_.push_back(record.transformation.matrix()[1][i]);
						buffer_for_transformations_2_.push_back(record.transformation.matrix()[2][i]);
						buffer_for_transformations_3_.push_back(record.transformation.matrix()[3][i]);
					}
				}
			}
			if(number_of_instances_>0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_selection_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_selection_.size()*sizeof(GLfloat), buffer_for_colors_for_selection_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_display_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_colors_for_display_.size()*sizeof(GLfloat), buffer_for_colors_for_display_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_adjuncts_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_adjuncts_.size()*sizeof(GLfloat), buffer_for_adjuncts_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_0_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_0_.size()*sizeof(GLfloat), buffer_for_transformations_0_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_1_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_1_.size()*sizeof(GLfloat), buffer_for_transformations_1_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_2_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_2_.size()*sizeof(GLfloat), buffer_for_transformations_2_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_3_);
				glBufferData(GL_ARRAY_BUFFER, buffer_for_transformations_3_.size()*sizeof(GLfloat), buffer_for_transformations_3_.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			change_of_visibility_=false;
			change_of_colors_for_display_=false;
			change_of_adjuncts_=false;
			change_of_transformations_=false;
			return true;
		}
		else if(change_of_colors_for_display_ || change_of_adjuncts_ || change_of_transformations_)
		{
			for(std::map<unsigned int, Record>::const_iterator it=map_of_records_.begin();it!=map_of_records_.end();++it)
			{
				const Record& record=it->second;
				if(record.visible && record.enabled)
				{
					if(change_of_colors_for_display_)
					{
						for(int i=0;i<3;i++)
						{
							buffer_for_colors_for_display_[record.instance*3+i]=record.color[i];
						}
					}
					if(change_of_adjuncts_)
					{
						for(int i=0;i<3;i++)
						{
							buffer_for_adjuncts_[record.instance*3+i]=record.adjunct[i];
						}
					}
					if(change_of_transformations_)
					{
						for(int i=0;i<4;i++)
						{
							buffer_for_transformations_0_[record.instance*4+i]=record.transformation.matrix()[0][i];
							buffer_for_transformations_1_[record.instance*4+i]=record.transformation.matrix()[1][i];
							buffer_for_transformations_2_[record.instance*4+i]=record.transformation.matrix()[2][i];
							buffer_for_transformations_3_[record.instance*4+i]=record.transformation.matrix()[3][i];
						}
					}
				}
			}
			if(change_of_colors_for_display_)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_colors_for_display_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_colors_for_display_.size()*sizeof(GLfloat)), buffer_for_colors_for_display_.data());
			}
			if(change_of_adjuncts_)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_adjuncts_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_adjuncts_.size()*sizeof(GLfloat)), buffer_for_adjuncts_.data());
			}
			if(change_of_transformations_)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_0_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_transformations_0_.size()*sizeof(GLfloat)), buffer_for_transformations_0_.data());

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_1_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_transformations_1_.size()*sizeof(GLfloat)), buffer_for_transformations_1_.data());

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_2_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_transformations_2_.size()*sizeof(GLfloat)), buffer_for_transformations_2_.data());

				glBindBuffer(GL_ARRAY_BUFFER, vbo_for_transformations_3_);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), static_cast<GLsizeiptr>(buffer_for_transformations_3_.size()*sizeof(GLfloat)), buffer_for_transformations_3_.data());
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			change_of_colors_for_display_=false;
			change_of_adjuncts_=false;
			change_of_transformations_=false;
			return true;
		}
		return false;
	}

	unsigned int number_of_vertices_;
	unsigned int number_of_indices_;
	unsigned int number_of_instances_;
	GLuint vao_;
	GLuint vbo_for_vertices_;
	GLuint vbo_for_normals_;
	GLuint vbo_for_indices_;
	GLuint vbo_for_colors_for_selection_;
	GLuint vbo_for_colors_for_display_;
	GLuint vbo_for_adjuncts_;
	GLuint vbo_for_transformations_0_;
	GLuint vbo_for_transformations_1_;
	GLuint vbo_for_transformations_2_;
	GLuint vbo_for_transformations_3_;
	bool change_of_visibility_;
	bool change_of_colors_for_display_;
	bool change_of_adjuncts_;
	bool change_of_transformations_;
	std::map<unsigned int, Record> map_of_records_;
	std::map<unsigned int, Record>::iterator cached_map_iterator_;
	std::vector<GLfloat> buffer_for_colors_for_selection_;
	std::vector<GLfloat> buffer_for_colors_for_display_;
	std::vector<GLfloat> buffer_for_adjuncts_;
	std::vector<GLfloat> buffer_for_transformations_0_;
	std::vector<GLfloat> buffer_for_transformations_1_;
	std::vector<GLfloat> buffer_for_transformations_2_;
	std::vector<GLfloat> buffer_for_transformations_3_;
};

}

#endif /* UV_DRAWING_WITH_INSTANCING_CONTROLLER_H_ */
