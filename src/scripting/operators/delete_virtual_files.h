#ifndef SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_
#define SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class DeleteVirtualFiles
{
public:
	struct Result
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::vector<std::string> filenames;

	DeleteVirtualFiles()
	{
	}

	DeleteVirtualFiles& init(CommandInput& input)
	{
		filenames=input.get_list_of_unnamed_values();
		input.mark_all_unnamed_values_as_used();
		return (*this);
	}

	Result run(void*&) const
	{
		if(filenames.empty())
		{
			VirtualFileStorage::clear();
		}
		else
		{
			for(std::size_t i=0;i<filenames.size();i++)
			{
				VirtualFileStorage::assert_file_exists(filenames[i]);
			}

			for(std::size_t i=0;i<filenames.size();i++)
			{
				VirtualFileStorage::delete_file(filenames[i]);
			}
		}

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_ */
