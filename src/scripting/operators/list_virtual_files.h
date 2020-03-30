#ifndef SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_
#define SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ListVirtualFiles : public OperatorBase<ListVirtualFiles>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::map<std::string, std::size_t> map_of_names_to_bytes;
		std::size_t total_count;
		std::size_t total_bytes;

		Result() : total_count(0), total_bytes(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
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
		}
	};

	ListVirtualFiles()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(void*) const
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

}

#endif /* SCRIPTING_OPERATORS_LIST_VIRTUAL_FILES_H_ */
