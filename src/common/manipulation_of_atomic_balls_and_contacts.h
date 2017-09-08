#ifndef COMMON_MANIPULATION_OF_ATOMIC_BALLS_AND_CONTACTS_H_
#define COMMON_MANIPULATION_OF_ATOMIC_BALLS_AND_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"
#include "matching_utilities.h"

namespace common
{

class ManipulationOfAtomicBallsAndContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	class test_atom
	{
	public:
		std::string match_crad;
		std::string match_crad_not;
		std::string match_tags;
		std::string match_tags_not;
		std::string match_adjuncts;
		std::string match_adjuncts_not;

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

	class test_contact_between_atoms
	{
	public:
		const std::vector<Atom>* atoms_ptr;
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

		test_contact_between_atoms() :
			atoms_ptr(0),
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

		bool operator()(const Contact& contact) const
		{
			if(atoms_ptr!=0)
			{
				return ((*this)(*atoms_ptr, contact));
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

	class Selection
	{
		enum Mode
		{
			SELECTION_MODE_SET,
			SELECTION_MODE_UPDATE_WITH_OR,
			SELECTION_MODE_UPDATE_WITH_AND
		};

		template<typename Container, typename Tester>
		static bool select(const Container& container, const Tester& tester, const bool logical_not, const Mode mode, std::vector<bool>& selection)
		{
			if(container.empty())
			{
				return false;
			}

			const bool logical_or=(mode==SELECTION_MODE_UPDATE_WITH_OR);
			const bool logical_and=(mode==SELECTION_MODE_UPDATE_WITH_AND);

			if(selection.size()!=container.size())
			{
				selection.clear();
				selection.resize(container.size(), false);
				if(logical_and)
				{
					return true;
				}
			}

			for(std::size_t i=0;i<container.size();i++)
			{
				if(
						(!logical_or && !logical_and) ||
						(logical_or && !selection[i]) ||
						(logical_and && selection[i])
				)
				{
					const bool result=tester(container[i]);
					selection[i]=(logical_not ? (!result) : result);
				}
			}

			return true;
		}

		template<typename Container, typename Tester>
		static bool select(const Container& container, const Tester& tester, const bool logical_not, std::vector<bool>& selection)
		{
			return select(container, tester, logical_not, SELECTION_MODE_SET, selection);
		}

		template<typename Container>
		static bool select(const Container& container, const bool value, std::vector<bool>& selection)
		{
			if(container.empty())
			{
				return false;
			}
			selection.clear();
			selection.resize(container.size(), value);
			return true;
		}

		static bool select(const std::vector<bool>& reference_selection, const Mode mode, std::vector<bool>& selection)
		{
			return select(reference_selection, test_echo(), false, mode, selection);
		}

		static void invert(std::vector<bool>& selection)
		{
			for(std::size_t i=0;i<selection.size();i++)
			{
				selection[i]=!selection[i];
			}
		}

	private:
		class test_echo
		{
		public:
			bool operator()(const bool value) const
			{
				return value;
			}
		};
	};
};

}

#endif /* COMMON_MANIPULATION_OF_ATOMIC_BALLS_AND_CONTACTS_H_ */
