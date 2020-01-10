#ifndef COMMON_CONVERSION_OF_DESCRIPTORS_H_
#define COMMON_CONVERSION_OF_DESCRIPTORS_H_

#include "chain_residue_atom_descriptor.h"
#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"

namespace voronota
{

namespace common
{

class ConversionOfDescriptors
{
public:
	static ChainResidueAtomDescriptorsPair get_contact_descriptor(
			const std::vector<ConstructionOfAtomicBalls::AtomicBall>& atoms,
			const ConstructionOfContacts::Contact& contact)
	{
		if(contact.ids[0]<atoms.size() && contact.ids[1]<atoms.size())
		{
			if(contact.solvent())
			{
				return ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad, ChainResidueAtomDescriptor::solvent());
			}
			else
			{
				return ChainResidueAtomDescriptorsPair(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad);
			}
		}
		else
		{
			return ChainResidueAtomDescriptorsPair();
		}
	}
};

}

}

#endif /* COMMON_CONVERSION_OF_DESCRIPTORS_H_ */
