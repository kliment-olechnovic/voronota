#ifndef COMMON_SCRIPTING_HETEROGENEOUS_STORAGE_H_
#define COMMON_SCRIPTING_HETEROGENEOUS_STORAGE_H_

#include "basic_types.h"
#include "variant_types.h"

namespace common
{

namespace scripting
{

struct HeterogeneousStorage
{
	std::string error;
	std::string text;
	std::set< std::string > flags;
	std::map< std::string, std::vector<std::size_t> > vectors_of_ids;
	std::map< std::string, SummaryOfAtoms > summaries_of_atoms;
	std::map< std::string, SummaryOfContacts > summaries_of_contacts;
	VariantObject variant_object;
};

}

}

#endif /* COMMON_SCRIPTING_HETEROGENEOUS_STORAGE_H_ */
