#ifndef SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_
#define SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ListVirtualFiles
{
public:
	struct Result
	{
		std::map<std::string, std::size_t> map_of_names_to_bytes;
		std::size_t total_count;
		std::size_t total_bytes;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			for(std::map<std::string, std::size_t>::const_iterator it=map_of_names_to_bytes.begin();it!=map_of_names_to_bytes.end();++it)
			{
				VariantObject info;
				info.value("name")=it->first;
				info.value("bytes")=it->second;
				heterostorage.variant_object.objects_array("files").push_back(info);
			}
			heterostorage.variant_object.value("total_count")=total_count;
			heterostorage.variant_object.value("total_bytes")=total_bytes;
			return (*this);
		}
	};

	ListVirtualFiles()
	{
	}

	ListVirtualFiles& init(CommandInput&)
	{
		return (*this);
	}

	Result run() const
	{
		Result result;
		for(std::map<std::string, std::string>::const_iterator it=VirtualFileStorage::files().begin();it!=VirtualFileStorage::files().end();++it)
		{
			result.map_of_names_to_bytes[it->first]=it->second.size();
		}
		result.total_count=VirtualFileStorage::files().size();
		result.total_bytes=VirtualFileStorage::count_bytes();
		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_ */
