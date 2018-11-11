#ifndef COMMON_SCRIPTING_VIRTUAL_FILE_STORAGE_H_
#define COMMON_SCRIPTING_VIRTUAL_FILE_STORAGE_H_

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdexcept>

namespace common
{

namespace scripting
{

class VirtualFileStorage
{
public:
	class InputSelector
	{
	public:
		InputSelector(const std::string& filename) : on_disk_(false)
		{
			if(VirtualFileStorage::file_exists(filename))
			{
				memory_stream_.str(VirtualFileStorage::get_file(filename));
			}
			else
			{
				on_disk_=true;
				disk_stream_.open(filename.c_str(), std::ios::in);
			}
		}

		bool on_disk() const
		{
			return on_disk_;
		}

		std::istream& stream()
		{
			if(on_disk_)
			{
				return disk_stream_;
			}
			return memory_stream_;
		}

	private:
		bool on_disk_;
		std::istringstream memory_stream_;
		std::ifstream disk_stream_;
	};

	class OutputSelector
	{
	public:
		OutputSelector(const std::string& filename) : on_disk_(false), filename_(filename)
		{
			if(!VirtualFileStorage::filename_is_valid(filename))
			{
				on_disk_=true;
				disk_stream_.open(filename.c_str(), std::ios::out);
			}
		}

		~OutputSelector()
		{
			if(!on_disk_)
			{
				VirtualFileStorage::set_file(filename_, memory_stream_.str());
			}
		}

		bool on_disk() const
		{
			return on_disk_;
		}

		std::ostream& stream()
		{
			if(on_disk_)
			{
				return disk_stream_;
			}
			return memory_stream_;
		}

	private:
		bool on_disk_;
		std::string filename_;
		std::ostringstream memory_stream_;
		std::ofstream disk_stream_;
	};

	static bool filename_is_valid(const std::string& filename)
	{
		return (filename.rfind("vfs/", 0)==0);
	}

	static void assert_filename_is_valid(const std::string& filename)
	{
		if(!filename_is_valid(filename))
		{
			throw std::runtime_error(std::string("Invalid virtual file name '")+filename+"'.");
		}
	}

	static const std::map<std::string, std::string>& files()
	{
		return files_mutable();
	}

	static bool file_exists(const std::string& filename)
	{
		return (VirtualFileStorage::files().count(filename)>0);
	}

	static void assert_file_exists(const std::string& filename)
	{
		if(!file_exists(filename))
		{
			throw std::runtime_error(std::string("No virtual file '")+filename+"'.");
		}
	}

	static void clear()
	{
		files_mutable().clear();
	}

	static void delete_file(const std::string& filename)
	{
		files_mutable().erase(filename);
	}

	static void set_file(const std::string& filename, const std::string& data)
	{
		assert_filename_is_valid(filename);
		files_mutable()[filename]=data;
	}

	static const std::string& get_file(const std::string& filename)
	{
		assert_file_exists(filename);
		return (files().find(filename)->second);
	}

	static std::size_t count_bytes()
	{
		std::size_t sum=0;
		for(std::map<std::string, std::string>::const_iterator it=files().begin();it!=files().end();++it)
		{
			sum+=it->second.size();
		}
		return sum;
	}

private:
	static std::map<std::string, std::string>& files_mutable()
	{
		static std::map<std::string, std::string> map;
		return map;
	}
};

}

}

#endif /* COMMON_SCRIPTING_VIRTUAL_FILE_STORAGE_H_ */
