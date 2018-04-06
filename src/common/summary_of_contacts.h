#ifndef COMMON_SUMMARY_OF_CONTACTS_H_
#define COMMON_SUMMARY_OF_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"

namespace common
{

class SummaryOfContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;

	class FullKey
	{
	public:
		CRADsPair crads;
		std::set<std::string> conditions;

		FullKey()
		{
		}

		FullKey(const CRADsPair& crads, const std::set<std::string>& conditions) :
			crads(crads),
			conditions(conditions)
		{
		}

		bool operator==(const FullKey& v) const
		{
			return (crads==v.crads && conditions==v.conditions);
		}

		bool operator!=(const FullKey& v) const
		{
			return (!((*this)==v));
		}

		bool operator<(const FullKey& v) const
		{
			if(crads<v.crads)
			{
				return true;
			}
			else if(crads==v.crads)
			{
				return (conditions<v.conditions);
			}
			return false;
		}
	};

	typedef CRAD AtomTypeKey;
	typedef std::set<std::string> ConditionsKey;

	typedef std::map<FullKey, double> FullMap;
	typedef std::map<AtomTypeKey, double> AtomTypeMap;
	typedef std::map<ConditionsKey, double> ConditionsMap;

	struct DerivedSums
	{
		double sum_of_solvent_areas;
		double sum_of_nonsolvent_areas;
		AtomTypeMap atom_type_map;
		ConditionsMap conditions_map;

		DerivedSums() :
			sum_of_solvent_areas(0.0),
			sum_of_nonsolvent_areas(0.0)
		{
		}
	};

	SummaryOfContacts() : far_seq_sep_min_(6)
	{
	}

	const FullMap& get_full_map_of_areas() const
	{
		return full_map_of_areas_;
	}

	void add(const CRAD& crad_a, const CRAD& crad_b, const std::set<std::string>& conditions, const double area)
	{
		if(crad_a==CRAD::solvent() && crad_b==CRAD::solvent())
		{
			return;
		}

		if(crad_a==CRAD::any() || crad_b==CRAD::any())
		{
			return;
		}

		if(CRAD::match_with_sequence_separation_interval(crad_a, crad_b, 0, 1, false))
		{
			return;
		}

		FullKey key(ChainResidueAtomDescriptorsPair(crad_a.without_numbering(), crad_b.without_numbering()), conditions);

		if(crad_b==CRAD::solvent() || crad_a==CRAD::solvent())
		{
			key.conditions.insert("solvent");
		}

		if(key.conditions.count("far")==0 && key.conditions.count("near")==0 && key.conditions.count("solvent")==0)
		{
			if(CRAD::match_with_sequence_separation_interval(crad_a, crad_b, far_seq_sep_min_, CRAD::null_num(), true))
			{
				key.conditions.insert("far");
			}
			else
			{
				key.conditions.insert("near");
			}
		}

		full_map_of_areas_[key]+=area;
	}

	void add(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts, const std::size_t contact_id)
	{
		if(contact_id>=contacts.size())
		{
			return;
		}

		const Contact& contact=contacts[contact_id];

		if(contact.ids[0]>=atoms.size() || contact.ids[1]>=atoms.size())
		{
			return;
		}

		if(contact.solvent())
		{
			add(atoms[contact.ids[0]].crad, CRAD::solvent(), contact.value.props.tags, contact.value.area);
		}
		else
		{
			add(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad, contact.value.props.tags, contact.value.area);
		}
	}

	void add(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts)
	{
		for(std::size_t i=0;i<contacts.size();i++)
		{
			add(atoms, contacts, i);
		}
	}

	DerivedSums get_derived_sums() const
	{
		DerivedSums result;
		for(FullMap::const_iterator it=full_map_of_areas_.begin();it!=full_map_of_areas_.end();++it)
		{
			const FullKey& key=it->first;
			const double area=it->second;
			if(key.conditions.count("solvent")>0)
			{
				result.sum_of_solvent_areas+=area;
			}
			else
			{
				result.sum_of_nonsolvent_areas+=area;
			}
			result.atom_type_map[key.crads.a]+=area;
			result.atom_type_map[key.crads.b]+=area;
			result.conditions_map[key.conditions]+=area;
		}
		return result;
	}

private:
	int far_seq_sep_min_;
	FullMap full_map_of_areas_;
};

inline std::ostream& operator<<(std::ostream& output, const SummaryOfContacts::FullKey& value)
{
	output << value.crads << " ";
	if(value.conditions.empty())
	{
		output << ".";
	}
	else
	{
		auxiliaries::IOUtilities(';').write_set(value.conditions, output);
	}
	return output;
}

inline std::istream& operator>>(std::istream& input, SummaryOfContacts::FullKey& value)
{
	ChainResidueAtomDescriptorsPair crads;
	std::string conditions_str;
	input >> crads >> conditions_str;
	if(!input.fail())
	{
		value.crads=crads;
		std::set<std::string> conditions;
		if(!conditions_str.empty() && conditions_str[0]!='.')
		{
			auxiliaries::IOUtilities(';').read_string_lines_to_set(conditions_str, conditions);
		}
		value.conditions=conditions;
	}
	return input;
}

}

#endif /* COMMON_SUMMARY_OF_CONTACTS_H_ */
