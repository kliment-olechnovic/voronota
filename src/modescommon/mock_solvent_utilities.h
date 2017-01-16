#ifndef MOCK_SOLVENT_UTILITIES_H_
#define MOCK_SOLVENT_UTILITIES_H_

#include "../auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

inline std::string mock_solvent_name()
{
	return std::string("w");
}

inline bool identify_mock_solvent(const auxiliaries::ChainResidueAtomDescriptor& crad)
{
	return (crad.name==mock_solvent_name() && crad.resName==mock_solvent_name());
}

}

#endif /* MOCK_SOLVENT_UTILITIES_H_ */
