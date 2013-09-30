#ifndef AUXILIARIES_CLOG_REDIRECTOR_H_
#define AUXILIARIES_CLOG_REDIRECTOR_H_

#include <iostream>
#include <sstream>
#include <fstream>

namespace auxiliaries
{

class CLogRedirector
{
public:
	CLogRedirector(const std::string& filename) : filename_(filename), buffer_backup_(0)
	{
		if(!filename_.empty())
		{
			buffer_backup_=std::clog.rdbuf(string_stream_.rdbuf());
		}
	}

	~CLogRedirector()
	{
		if(buffer_backup_!=0)
		{
			const std::string text=string_stream_.str();
			if(!text.empty())
			{
				std::ofstream output(filename_.c_str(), std::ios::out);
				if(output.good())
				{
					output << text;
				}
				else
				{
					std::cerr << "Failed to redirect log to '" << filename_ << "', redirecting it to stderr.\n";
					std::cerr << text;
				}
			}
			std::clog.rdbuf(buffer_backup_);
		}
	}

private:
	std::string filename_;
	std::ostringstream string_stream_;
	std::streambuf* buffer_backup_;
};

}

#endif /* AUXILIARIES_CLOG_REDIRECTOR_H_ */
