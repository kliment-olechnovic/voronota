#ifndef SCRIPTING_HETEROGENEOUS_STORAGE_H_
#define SCRIPTING_HETEROGENEOUS_STORAGE_H_

#include "basic_types.h"
#include "variant_types.h"

namespace voronota
{

namespace scripting
{

struct HeterogeneousStorage
{
	std::vector<std::string> errors;
	std::map< std::string, SummaryOfAtoms > summaries_of_atoms;
	std::map< std::string, SummaryOfContacts > summaries_of_contacts;
	VariantObject variant_object;
};

}

}

#endif /* SCRIPTING_HETEROGENEOUS_STORAGE_H_ */
