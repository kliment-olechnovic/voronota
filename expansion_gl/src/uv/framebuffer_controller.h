#ifndef UV_FRAMEBUFFER_CONTROLLER_H_
#define UV_FRAMEBUFFER_CONTROLLER_H_

#include "common.h"

namespace voronota
{

namespace uv
{

class FramebufferController : private Noncopyable
{
public:
	FramebufferController() :
		screen_width_(0),
		screen_height_(0),
		multiply_(0),
		fbo_(0),
		rbo_(0),
		texture_color_buffer_(0)
	{
	}

	virtual ~FramebufferController()
	{
		reset();
	}

	bool init(const int screen_width, const int screen_height, const int multiply)
	{
		if(good())
		{
			if(screen_width==screen_width_ && screen_height==screen_height_)
			{
				return true;
			}
			else
			{
				reset();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenFramebuffers(1, &fbo_);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

		glGenTextures(1, &texture_color_buffer_);
		glBindTexture(GL_TEXTURE_2D, texture_color_buffer_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width*multiply, screen_height*multiply, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer_, 0);

		glGenRenderbuffers(1, &rbo_);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screen_width*multiply, screen_height*multiply);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
		{
			reset();
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		screen_width_=screen_width;
		screen_height_=screen_height;
		multiply_=multiply;

		return good();
	}

	bool good() const
	{
		return (screen_width_!=0 && screen_height_!=0 && multiply_!=0 && fbo_!=0 && rbo_!=0 && texture_color_buffer_!=0);
	}

	GLuint framebuffer() const
	{
		return fbo_;
	}

	GLuint texture() const
	{
		return texture_color_buffer_;
	}

	int width() const
	{
		return (screen_width_*multiply_);
	}

	int height() const
	{
		return (screen_height_*multiply_);
	}

private:
	void reset()
	{
		if(rbo_!=0)
		{
			glDeleteRenderbuffers(1, &rbo_);
			rbo_=0;
		}

		if(texture_color_buffer_!=0)
		{
			glDeleteTextures(1, &texture_color_buffer_);
			texture_color_buffer_=0;
		}

		if(fbo_!=0)
		{
			glDeleteFramebuffers(1, &fbo_);
			fbo_=0;
		}

		screen_width_=0;
		screen_height_=0;
	}

	int screen_width_;
	int screen_height_;
	int multiply_;
	GLuint fbo_;
	GLuint rbo_;
	GLuint texture_color_buffer_;
};

}

}

#endif /* UV_FRAMEBUFFER_CONTROLLER_H_ */

