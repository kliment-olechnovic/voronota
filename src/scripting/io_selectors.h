#ifndef SCRIPTING_IO_SELECTORS_H_
#define SCRIPTING_IO_SELECTORS_H_

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
		DISK
	};

	explicit InputSelector(const std::string& filename) : location_type_(VIRTUAL_FILE_STORAGE)
	{
		if(VirtualFileStorage::file_exists(filename))
		{
			memory_stream_.str(VirtualFileStorage::get_file(filename));
		}
		else
		{
			location_type_=DISK;
			disk_stream_.open(filename.c_str(), std::ios::in);
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
		TEMPORARY_MEMORY
	};

	explicit OutputSelector(const std::string& filename) : location_type_(VIRTUAL_FILE_STORAGE), filename_(filename)
	{
		if(filename=="_dump")
		{
			location_type_=TEMPORARY_MEMORY;
		}
		else if(!VirtualFileStorage::filename_is_valid(filename))
		{
			location_type_=DISK;
			disk_stream_.open(filename.c_str(), std::ios::out);
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

}

}

#endif /* SCRIPTING_IO_SELECTORS_H_ */
