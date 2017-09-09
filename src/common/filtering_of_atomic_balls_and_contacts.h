#ifndef COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_
#define COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"
#include "matching_utilities.h"

namespace common
{

class FilteringOfAtomicBallsAndContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	class test_id
	{
	public:
		std::set<std::size_t> allowed_ids;
		std::string name_of_allowed_ids;

		virtual bool operator()(const std::size_t id) const
		{
			if(allowed_ids.empty() || allowed_ids.count(id)>0)
			{
				return true;
			}
			return false;
		}

		bool load_allowed_ids_by_name(const std::map< std::string, std::set<std::size_t> >& map_of_names_to_allowed_ids)
		{
			if(!name_of_allowed_ids.empty())
			{
				std::map< std::string, std::set<std::size_t> >::const_iterator it=map_of_names_to_allowed_ids.find(name_of_allowed_ids);
				if(it!=map_of_names_to_allowed_ids.end())
				{
					allowed_ids=it->second;
					return true;
				}
			}
			return false;
		}
	};

	class test_atom : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		std::string match_crad;
		std::string match_crad_not;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;

		test_atom(const std::vector<Atom>* atoms_ptr=0) :
			atoms_ptr(atoms_ptr)
		{
		}

		bool operator()(const std::size_t id) const
		{
			if(atoms_ptr!=0 && id<atoms_ptr->size())
			{
				return (test_id::operator()(id) && (this->operator()((*atoms_ptr)[id])));
			}
			return false;
		}

		bool operator()(const Atom& atom) const
		{
			if(
					MatchingUtilities::match_crad(atom.crad, match_crad, match_crad_not) &&
					MatchingUtilities::match_set_of_tags(atom.value.props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(atom.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
			)
			{
				return true;
			}
			return false;
		}
	};

	class test_contact_between_atoms : public test_id
	{
	public:
		const std::vector<Atom>* atoms_ptr;
		const std::vector<Contact>* contacts_ptr;
		double match_min_area;
		double match_max_area;
		double match_min_dist;
		double match_max_dist;
		int match_min_sequence_separation;
		int match_max_sequence_separation;
		bool no_solvent;
		bool no_same_chain;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;
		test_atom test_atom_a;
		test_atom test_atom_b;

		test_contact_between_atoms(const std::vector<Atom>* atoms_ptr=0, const std::vector<Contact>* contacts_ptr=0) :
			atoms_ptr(atoms_ptr),
			contacts_ptr(contacts_ptr),
			match_min_area(std::numeric_limits<double>::min()),
			match_max_area(std::numeric_limits<double>::max()),
			match_min_dist(std::numeric_limits<double>::min()),
			match_max_dist(std::numeric_limits<double>::max()),
			match_min_sequence_separation(ChainResidueAtomDescriptor::null_num()),
			match_max_sequence_separation(ChainResidueAtomDescriptor::null_num()),
			no_solvent(false),
			no_same_chain(false)
		{
		}

		bool operator()(const std::size_t id) const
		{
			if(contacts_ptr!=0 && id<contacts_ptr->size())
			{
				return (test_id::operator()(id) && (this->operator()((*contacts_ptr)[id])));
			}
			return false;
		}

		bool operator()(const Contact& contact) const
		{
			if(atoms_ptr!=0)
			{
				return (this->operator()(*atoms_ptr, contact));
			}
			return false;
		}

		bool operator()(const std::vector<Atom>& atoms, const Contact& contact) const
		{
			if(
					contact.ids[0]<atoms.size() &&
					contact.ids[1]<atoms.size() &&
					contact.value.area>=match_min_area &&
					contact.value.area<=match_max_area &&
					contact.value.dist>=match_min_dist &&
					contact.value.dist<=match_max_dist &&
					(!no_solvent || !contact.solvent())
			)
			{
				const Atom& atom_a=atoms[contact.ids[0]];
				const Atom& atom_b=atoms[contact.ids[1]];
				const ChainResidueAtomDescriptor& crad_a=atom_a.crad;
				const ChainResidueAtomDescriptor& crad_b=(contact.solvent() ? ChainResidueAtomDescriptor::solvent() : atom_b.crad);
				if(
						(!no_same_chain || crad_a.chainID!=crad_b.chainID) &&
						ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crad_a, crad_b, match_min_sequence_separation, match_max_sequence_separation, true) &&
						MatchingUtilities::match_set_of_tags(contact.value.props.tags, match_tags, match_tags_not) &&
						MatchingUtilities::match_map_of_adjuncts(contact.value.props.adjuncts, match_adjuncts, match_adjuncts_not)
				)
				{
					if(
							(test_atom_a(atom_a) && test_atom_b(atom_b)) ||
							(test_atom_a(atom_b) && test_atom_b(atom_a))
					)
					{
						return true;
					}
				}
			}
			return false;
		}
	};

	template<typename Container, typename Tester>
	static std::set<std::size_t> select(const Container& container, const Tester& tester)
	{
		std::set<std::size_t> result;
		if(!container.empty())
		{
			std::set<std::size_t>::iterator pos=result.begin();
			for(std::size_t i=0;i<container.size();i++)
			{
				if(tester(container[i]))
				{
					pos=result.insert(pos, i);
				}
			}
		}
		return result;
	}

	template<typename Container, typename Tester>
	static std::set<std::size_t> select(const Container& container, const std::set<std::size_t>& restriction, const Tester& tester)
	{
		std::set<std::size_t> result;
		if(!container.empty() || restriction.empty())
		{
			std::set<std::size_t>::iterator pos=result.begin();
			for(std::set<std::size_t>::const_iterator it=restriction.begin();it!=restriction.end();++it)
			{
				const std::size_t i=(*it);
				if(i<container.size() && tester(container[i]))
				{
					pos=result.insert(pos, i);
				}
			}
		}
		return result;
	}
};

}

#endif /* COMMON_FILTERING_OF_ATOMIC_BALLS_AND_CONTACTS_H_ */
