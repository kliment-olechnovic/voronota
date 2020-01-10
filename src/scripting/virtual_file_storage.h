#ifndef SCRIPTING_VIRTUAL_FILE_STORAGE_H_
#define SCRIPTING_VIRTUAL_FILE_STORAGE_H_

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdexcept>

namespace voronota
{

namespace scripting
{

class VirtualFileStorage
{
public:
	static bool writable()
	{
		return writable_mutable();
	}

	static void set_writable(const bool status)
	{
		writable_mutable()=status;
	}

	static void assert_writable()
	{
		if(!writable())
		{
			throw std::runtime_error(std::string("Virtual file storage is set to read-only."));
		}
	}

	static bool filename_is_valid(const std::string& filename)
	{
		return (filename.rfind("_virtual", 0)==0);
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
		assert_writable();
		files_mutable().clear();
	}

	static void delete_file(const std::string& filename)
	{
		assert_writable();
		files_mutable().erase(filename);
	}

	static void set_file(const std::string& filename, const std::string& data)
	{
		assert_writable();
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
	static bool& writable_mutable()
	{
		static bool status=true;
		return status;
	}

	static std::map<std::string, std::string>& files_mutable()
	{
		static std::map<std::string, std::string> map;
		return map;
	}
};

}

}

#endif /* SCRIPTING_VIRTUAL_FILE_STORAGE_H_ */
