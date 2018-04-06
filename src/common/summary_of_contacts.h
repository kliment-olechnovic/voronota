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

	struct DerivedSumsOfAreas
	{
		double solvent;
		double nonsolvent;
		AtomTypeMap atom_type_map;
		ConditionsMap conditions_map;

		DerivedSumsOfAreas() :
			solvent(0.0),
			nonsolvent(0.0)
		{
		}
	};

	struct DerivedObservedProbabilities
	{
		double full_sum;
		double atom_type_sum;
		double conditions_sum;
		FullMap full_map;
		AtomTypeMap atom_type_map;
		ConditionsMap conditions_map;

		DerivedObservedProbabilities() :
			full_sum(0.0),
			atom_type_sum(0.0),
			conditions_sum(0.0)
		{
		}
	};

	static CRAD generalize_crad(const CRAD& input_crad)
	{
		CRAD crad=input_crad.without_numbering();
		if(crad.resName=="ARG" && (crad.name=="NH1" || crad.name=="NH2"))
		{
			crad.name="NH1";
		}
		else if(crad.resName=="ASP" && (crad.name=="OD1" || crad.name=="OD2"))
		{
			crad.name="OD1";
		}
		else if(crad.resName=="GLU" && (crad.name=="OE1" || crad.name=="OE2"))
		{
			crad.name="OE1";
		}
		else if(crad.resName=="PHE" && (crad.name=="CD1" || crad.name=="CD2"))
		{
			crad.name="CD1";
		}
		else if(crad.resName=="PHE" && (crad.name=="CE1" || crad.name=="CE2"))
		{
			crad.name="CE1";
		}
		else if(crad.resName=="TYR" && (crad.name=="CD1" || crad.name=="CD2"))
		{
			crad.name="CD1";
		}
		else if(crad.resName=="TYR" && (crad.name=="CE1" || crad.name=="CE2"))
		{
			crad.name="CE1";
		}
		else if(crad.name=="OXT")
		{
			crad.name="O";
		}
		else if(crad.resName=="MSE")
		{
			crad.resName="MET";
			if(crad.name=="SE")
			{
				crad.name="SD";
			}
		}
		else if(crad.resName=="SEC")
		{
			crad.resName="CYS";
			if(crad.name=="SE")
			{
				crad.name="SG";
			}
		}
		return crad;
	}

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

		FullKey key(ChainResidueAtomDescriptorsPair(generalize_crad(crad_a), generalize_crad(crad_b)), conditions);

		if(crad_b==CRAD::solvent() || crad_a==CRAD::solvent())
		{
			key.conditions.insert("solvent");
		}
		else
		{
			key.conditions.erase("solvent");
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

	DerivedSumsOfAreas get_derived_sums_of_areas() const
	{
		DerivedSumsOfAreas sums_of_areas;
		for(FullMap::const_iterator it=full_map_of_areas_.begin();it!=full_map_of_areas_.end();++it)
		{
			const FullKey& key=it->first;
			const double area=it->second;
			if(key.conditions.count("solvent")>0)
			{
				sums_of_areas.solvent+=area;
			}
			else
			{
				sums_of_areas.nonsolvent+=area;
			}
			sums_of_areas.atom_type_map[key.crads.a]+=area;
			sums_of_areas.atom_type_map[key.crads.b]+=area;
			sums_of_areas.conditions_map[key.conditions]+=area;
		}
		return sums_of_areas;
	}

	DerivedObservedProbabilities get_derived_observed_probabilities() const
	{
		const DerivedSumsOfAreas sums_of_areas=get_derived_sums_of_areas();
		DerivedObservedProbabilities observed_probabilities;
		for(FullMap::const_iterator it=full_map_of_areas_.begin();it!=full_map_of_areas_.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+sums_of_areas.nonsolvent);
			observed_probabilities.full_sum+=value;
			observed_probabilities.full_map[it->first]=value;
		}
		for(AtomTypeMap::const_iterator it=sums_of_areas.atom_type_map.begin();it!=sums_of_areas.atom_type_map.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+(sums_of_areas.nonsolvent*2.0));
			observed_probabilities.atom_type_sum+=value;
			observed_probabilities.atom_type_map[it->first]=value;
		}
		for(ConditionsMap::const_iterator it=sums_of_areas.conditions_map.begin();it!=sums_of_areas.conditions_map.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+sums_of_areas.nonsolvent);
			observed_probabilities.conditions_sum+=value;
			observed_probabilities.conditions_map[it->first]=value;
		}
		return observed_probabilities;
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
