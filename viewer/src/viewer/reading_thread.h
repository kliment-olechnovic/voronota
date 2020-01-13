#ifndef VIEWER_READING_THREAD_H_
#define VIEWER_READING_THREAD_H_

#ifdef FOR_WEB
#include <string>
namespace voronota
{

namespace viewer
{

class ReadingThread
{
public:
	static bool check_data()
	{
		return false;
	}

	static std::string extract_data()
	{
		return std::string();
	}
};

}

}
#else

#include <iostream>
#include <string>
#include <pthread.h>

namespace voronota
{

namespace viewer
{

class ReadingThread
{
public:
	static bool check_data()
	{
		bool status=false;
		ReadingThread& obj=ReadingThread::instance();
		if(obj.activate())
		{
			obj.lock();
			status=!(obj.data_.empty());
			obj.unlock();
		}
		return status;
	}

	static std::string extract_data()
	{
		std::string result;
		ReadingThread& obj=ReadingThread::instance();
		if(obj.activate())
		{
			obj.lock();
			result=obj.data_;
			obj.data_.clear();
			obj.unlock();
		}
		return result;
	}

private:
	ReadingThread() : active_(false), thread_id_(0)
	{
	}

	~ReadingThread()
	{
		if(active_)
		{
			pthread_cancel(thread_id_);
		}
	}

	static ReadingThread& instance()
	{
		static ReadingThread obj;
		return obj;
	}

	static void* thread_function(void* param)
	{
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
		ReadingThread* obj=static_cast<ReadingThread*>(param);
		while(std::cin.good())
		{
			if(!obj->check_data())
			{
				std::string line;
				std::getline(std::cin, line);
				if(!line.empty())
				{
					obj->lock();
					obj->data_=line;
					obj->unlock();
				}
			}
		}
		return 0;
	}

	bool activate()
	{
		if(active_)
		{
			return true;
		}

		if(pthread_mutex_init(&mutex_, 0)!=0)
		{
			return false;
		}

		if(pthread_create(&thread_id_, 0, thread_function, this)!=0)
		{
			return false;
		}

		pthread_detach(thread_id_);

		active_=true;

		return true;
	}

	void lock()
	{
		pthread_mutex_lock(&mutex_);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex_);
	}

	bool active_;
	pthread_t thread_id_;
	pthread_mutex_t mutex_;
	std::string data_;
};

}

}

#endif /* FOR_WEB */

#endif /* VIEWER_READING_THREAD_H_ */
