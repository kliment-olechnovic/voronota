#ifndef SCRIPTING_VIRTUAL_FILE_STORAGE_H_
#define SCRIPTING_VIRTUAL_FILE_STORAGE_H_

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>

namespace voronota
{

namespace scripting
{

class VirtualFileStorage
{
public:
	class TemporaryFile
	{
	public:
		TemporaryFile() : filename_(VirtualFileStorage::get_unused_filename())
		{
		}

		explicit TemporaryFile(const std::string& suffix) : filename_(VirtualFileStorage::get_unused_filename(suffix))
		{
		}

		~TemporaryFile()
		{
			VirtualFileStorage::delete_file(filename_);
		}

		const std::string& filename() const
		{
			return filename_;
		}

	private:
		std::string filename_;
	};

	class AutodeleterOfFiles
	{
	public:
		AutodeleterOfFiles()
		{
		}

		AutodeleterOfFiles(const std::vector<std::string>& filenames) : filenames_(filenames)
		{
		}

		AutodeleterOfFiles(const std::string& filename) : filenames_(std::vector<std::string>(1, filename))
		{
		}

		~AutodeleterOfFiles()
		{
			for(std::size_t i=0;i<filenames_.size();i++)
			{
				VirtualFileStorage::delete_file(filenames_[i]);
			}
		}

	private:
		std::vector<std::string> filenames_;
	};

	static bool writable()
	{
		return writable_mutable();
	}

	static void set_writable(const bool status)
	{
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageSetWritable)
#endif
		{
			writable_mutable()=status;
		}
	}

	static void assert_writable()
	{
		if(!writable())
		{
			throw std::runtime_error(std::string("Virtual file storage is set to read-only."));
		}
	}

	static const std::string& prefix()
	{
		static std::string str="_virtual";
		return str;
	}

	static std::string validate_filename(const std::string& filename)
	{
		if(filename_is_valid(filename))
		{
			return filename;
		}
		return (prefix()+filename);
	}

	static bool filename_is_valid(const std::string& filename)
	{
		return (filename.rfind(prefix(), 0)==0);
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

	static const std::set<std::string>& locks()
	{
		return locks_mutable();
	}

	static bool file_exists(const std::string& filename)
	{
		return (VirtualFileStorage::files().count(filename)>0);
	}

	static bool file_locked(const std::string& filename)
	{
		return (VirtualFileStorage::locks().count(filename)>0);
	}

	static void assert_file_exists(const std::string& filename)
	{
		if(!file_exists(filename))
		{
			throw std::runtime_error(std::string("No virtual file '")+filename+"'.");
		}
	}

	static void assert_file_not_locked(const std::string& filename)
	{
		if(file_locked(filename))
		{
			throw std::runtime_error(std::string("Locked virtual file '")+filename+"'.");
		}
	}

	static void clear()
	{
		assert_writable();
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageClear)
#endif
		{
			files_mutable().clear();
			locks_mutable().clear();
		}
	}

	static void clear_not_locked()
	{
		assert_writable();
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageClearNotLocked)
#endif
		{
			std::map<std::string, std::string>::iterator it=files_mutable().begin();
			while(it!=files_mutable().end())
			{
				if(file_locked(it->first))
				{
					++it;
				}
				else
				{
					files_mutable().erase(it++);
				}
			}
		}
	}

	static void delete_file(const std::string& filename)
	{
		if(!file_exists(filename))
		{
			return;
		}
		assert_writable();
		assert_file_not_locked(filename);
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageDeleteFile)
#endif
		{
			files_mutable().erase(filename);
		}
	}

	static void set_file(const std::string& filename, const std::string& data)
	{
		assert_writable();
		assert_file_not_locked(filename);
		assert_filename_is_valid(filename);
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageSetFile)
#endif
		{
			files_mutable()[filename]=data;
		}
	}

	static void set_file(const std::string& filename, const std::string& data, const bool locked)
	{
		set_file(filename, data);
		set_lock(filename, locked);
	}

	static void set_lock(const std::string& filename, const bool locked)
	{
		assert_writable();
		if(locked)
		{
			assert_file_exists(filename);
		}
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageSetLock)
#endif
		{
			if(locked)
			{
				locks_mutable().insert(filename);
			}
			else
			{
				locks_mutable().erase(filename);
			}
		}
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

	static std::string get_unused_filename(const std::string& suffix)
	{
		static long id=1000000;
		std::string result;
#ifdef _OPENMP
		#pragma omp critical(VirtualFileStorageGetUnusedFilename)
#endif
		{
			while(result.empty())
			{
				++id;
				std::ostringstream output;
				output << prefix() << "_file_" << id << suffix;
				std::string candidate=output.str();
				if(!file_exists(candidate))
				{
					result=candidate;
				}
			}
		}
		return result;
	}

	static std::string get_unused_filename()
	{
		return get_unused_filename(std::string());
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

	static std::set<std::string>& locks_mutable()
	{
		static std::set<std::string> set;
		return set;
	}
};

}

}

#endif /* SCRIPTING_VIRTUAL_FILE_STORAGE_H_ */
