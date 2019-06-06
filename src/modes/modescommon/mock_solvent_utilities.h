#ifndef MODESCOMMON_MOCK_SOLVENT_UTILITIES_H_
#define MODESCOMMON_MOCK_SOLVENT_UTILITIES_H_

#include "../../common/chain_residue_atom_descriptor.h"

namespace modescommon
{

inline std::string mock_solvent_name()
{
	return std::string("w");
}

inline bool identify_mock_solvent(const common::ChainResidueAtomDescriptor& crad)
{
	return (crad.name==mock_solvent_name() && crad.resName==mock_solvent_name());
}

}

#endif /* MODESCOMMON_MOCK_SOLVENT_UTILITIES_H_ */
