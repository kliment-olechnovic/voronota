#ifndef ENVUTIL_H_
#define ENVUTIL_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace envutil
{

class OpenMPUtilities
{
public:
	static bool openmp_enabled()
	{
#ifdef _OPENMP
		return true;
#else
		return false;
#endif
	}

	static unsigned int openmp_setup(const unsigned int max_number_of_processors)
	{
#ifdef _OPENMP
		omp_set_num_threads(max_number_of_processors);
		omp_set_max_active_levels(1);
		return max_number_of_processors;
#else
		return 1;
#endif
	}
};

class FileSystemUtilities
{
public:
	struct FileInfo
	{
		std::string path;
		std::string name;

		explicit FileInfo(const std::filesystem::path& p) : path(p.lexically_normal().string()), name(p.filename().string())
		{
		}

		bool operator==(const FileInfo& v) const
		{
			return (path==v.path);
		}

		bool operator!=(const FileInfo& v) const
		{
			return (path!=v.path);
		}

		bool operator<(const FileInfo& v) const
		{
			return path<v.path;
		}
	};

	static std::set<FileInfo> collect_file_descriptors(const std::vector<std::string>& input_paths, bool recursive)
	{
		std::set<FileInfo> file_descriptors;
		for(const std::string& input_path : input_paths)
		{
			try
			{
				std::filesystem::path p(input_path);
				if(std::filesystem::is_regular_file(p))
				{
					file_descriptors.emplace(FileInfo(p));
				}
				else if(std::filesystem::is_directory(p))
				{
					const std::filesystem::directory_options options=std::filesystem::directory_options::skip_permission_denied;
					if(recursive)
					{
						for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(p, options))
						{
							if(entry.is_regular_file())
							{
								file_descriptors.emplace(FileInfo(entry.path()));
							}
						}
					}
					else
					{
						for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(p, options))
						{
							if(entry.is_regular_file())
							{
								file_descriptors.emplace(FileInfo(entry.path()));
							}
						}
					}
				}
			}
			catch(const std::filesystem::filesystem_error& e)
			{
				std::cerr << "Skipping path '" << input_path << "' due to filesystem error: " << e.what() << "\n";
			}
		}
		return file_descriptors;
	}

	static bool create_directory(const std::string& directory_path)
	{
		try
		{
			std::filesystem::create_directories(directory_path);
		}
		catch(const std::filesystem::filesystem_error& e)
		{
			std::cerr << "Failed to create directory '" << directory_path << "' due to filesystem error: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	static bool create_parent_directory(const std::string& full_file_path)
	{
		std::filesystem::path file_path=full_file_path;
		std::filesystem::path dir_path=file_path.parent_path();
		if(!dir_path.empty())
		{
			try
			{
				std::filesystem::create_directories(dir_path);
			}
			catch(const std::filesystem::filesystem_error& e)
			{
				std::cerr << "Failed to create parent directory '" << dir_path << "' due to filesystem error: " << e.what() << "\n";
				return false;
			}
		}
		return true;
	}

	static bool write_file(const std::string& full_file_path, const std::string& output_string)
	{
		if(output_string.empty())
		{
			std::cerr << "Error: empty contents provided to write to file '" << full_file_path << "'.\n";
			return false;
		}
		if(!create_parent_directory(full_file_path))
		{
			std::cerr << "Error: failed to create parent directory for file '" << full_file_path << "'.\n";
			return false;
		}
		std::ofstream output_file_stream(full_file_path.c_str(), std::ios::out);
		if(!output_file_stream.good())
		{
			std::cerr << "Error: failed to open file '" << full_file_path << "' to output table of global scores.\n";
			return false;
		}
		output_file_stream.write(output_string.data(), static_cast<std::streamsize>(output_string.size()));
		return true;
	}
};

}

#endif /* ENVUTIL_H_ */
