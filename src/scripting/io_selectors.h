#ifndef SCRIPTING_IO_SELECTORS_H_
#define SCRIPTING_IO_SELECTORS_H_

#include <iostream>
#include <sstream>

#include "virtual_file_storage.h"

namespace voronota
{

namespace scripting
{

class InputSelector
{
public:
	enum LocationType
	{
		VIRTUAL_FILE_STORAGE,
		DISK,
		STDIN
	};

	explicit InputSelector(const std::string& filename) : location_type_(VIRTUAL_FILE_STORAGE)
	{
		if(filename=="_stdin")
		{
			location_type_=STDIN;
		}
		else if(VirtualFileStorage::file_exists(filename))
		{
			memory_stream_.str(VirtualFileStorage::get_file(filename));
		}
		else
		{
			location_type_=DISK;
			disk_stream_.open(filename.c_str(), std::ios::in | std::ios::binary);
		}
	}

	LocationType location_type() const
	{
		return location_type_;
	}

	std::istream& stream()
	{
		if(location_type_==DISK)
		{
			return disk_stream_;
		}
		if(location_type_==STDIN)
		{
			return std::cin;
		}
		return memory_stream_;
	}

private:
	LocationType location_type_;
	std::istringstream memory_stream_;
	std::ifstream disk_stream_;
};

class OutputSelector
{
public:
	enum LocationType
	{
		VIRTUAL_FILE_STORAGE,
		DISK,
		TEMPORARY_MEMORY,
		STDOUT
	};

	explicit OutputSelector(const std::string& filename) : location_type_(VIRTUAL_FILE_STORAGE), filename_(filename)
	{
		if(filename=="_dump")
		{
			location_type_=TEMPORARY_MEMORY;
		}
		else if(filename=="_stdout")
		{
			location_type_=STDOUT;
		}
		else if(!VirtualFileStorage::filename_is_valid(filename))
		{
			location_type_=DISK;
			disk_stream_.open(filename.c_str(), std::ios::out | std::ios::binary);
		}
		else
		{
			VirtualFileStorage::assert_writable();
		}
	}

	~OutputSelector()
	{
		if(location_type_==VIRTUAL_FILE_STORAGE)
		{
			if(VirtualFileStorage::writable())
			{
				VirtualFileStorage::set_file(filename_, memory_stream_.str());
			}
		}
	}

	LocationType location_type() const
	{
		return location_type_;
	}

	std::string str() const
	{
		return memory_stream_.str();
	}

	std::ostream& stream()
	{
		if(location_type_==DISK)
		{
			return disk_stream_;
		}
		if(location_type_==STDOUT)
		{
			return std::cout;
		}
		if(location_type_==VIRTUAL_FILE_STORAGE)
		{
			VirtualFileStorage::assert_writable();
		}
		return memory_stream_;
	}

private:
	LocationType location_type_;
	std::string filename_;
	std::ostringstream memory_stream_;
	std::ofstream disk_stream_;
};

class StandardOutputRedirector
{
public:
	StandardOutputRedirector() :
		cout_buf_(std::cout.rdbuf()),
		cerr_buf_(std::cerr.rdbuf())
	{
		std::cout.rdbuf(cout_out_.rdbuf());
		std::cerr.rdbuf(cerr_out_.rdbuf());
	}

	~StandardOutputRedirector()
	{
		std::cout.rdbuf(cout_buf_);
		std::cerr.rdbuf(cerr_buf_);
	}

	std::string cout_output() const
	{
		return cout_out_.str();
	}

	std::string cerr_output() const
	{
		return cerr_out_.str();
	}

private:
	std::streambuf* cout_buf_;
	std::streambuf* cerr_buf_;
	std::ostringstream cout_out_;
	std::ostringstream cerr_out_;
};

class StandardOutputMockup
{
public:
	StandardOutputMockup()
	{
	}

	std::ostream& cout()
	{
		return cout_out_;
	}

	std::ostream& cerr()
	{
		return cerr_out_;
	}

	std::string cout_output() const
	{
		return cout_out_.str();
	}

	std::string cerr_output() const
	{
		return cerr_out_.str();
	}

private:
	std::ostringstream cout_out_;
	std::ostringstream cerr_out_;
};

}

}

#endif /* SCRIPTING_IO_SELECTORS_H_ */
