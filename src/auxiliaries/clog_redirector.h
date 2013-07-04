#ifndef AUXILIARIES_CLOG_REDIRECTOR_H_
#define AUXILIARIES_CLOG_REDIRECTOR_H_

#include <fstream>

namespace auxiliaries
{

class CLogRedirector
{
public:
	CLogRedirector() : buffer_backup_(0)
	{
	}

	bool init(const std::string& filename)
	{
		if(!buffer_backed_up() && !filename.empty())
		{
			file_stream_.open(filename.c_str());
			if(file_stream_.good())
			{
				buffer_backup_=std::clog.rdbuf(file_stream_.rdbuf());
			}
			else
			{
				buffer_backup_=0;
				file_stream_.close();
			}
		}
		return buffer_backed_up();
	}

	bool buffer_backed_up() const
	{
		return (buffer_backup_!=0);
	}

	~CLogRedirector()
	{
		if(buffer_backed_up())
		{
			std::clog.rdbuf(buffer_backup_);
		}
	}

private:
	std::ofstream file_stream_;
	std::streambuf* buffer_backup_;
};

}

#endif /* AUXILIARIES_CLOG_REDIRECTOR_H_ */
