#ifndef COMMON_SCRIPTING_BASIC_TYPES_H_
#define COMMON_SCRIPTING_BASIC_TYPES_H_

#include "../construction_of_atomic_balls.h"
#include "../construction_of_contacts.h"

namespace common
{

namespace scripting
{

typedef ConstructionOfAtomicBalls::AtomicBall Atom;
typedef ConstructionOfContacts::Contact Contact;

struct BoundingBox
{
	bool filled;
	apollota::SimplePoint p_min;
	apollota::SimplePoint p_max;

	BoundingBox() : filled(false)
	{
	}

	template<typename Point>
	void update(const Point& p)
	{
		if(!filled)
		{
			p_min=apollota::SimplePoint(p);
			p_max=p_min;
		}
		else
		{
			p_min.x=std::min(p_min.x, p.x);
			p_min.y=std::min(p_min.y, p.y);
			p_min.z=std::min(p_min.z, p.z);
			p_max.x=std::max(p_max.x, p.x);
			p_max.y=std::max(p_max.y, p.y);
			p_max.z=std::max(p_max.z, p.z);
		}
		filled=true;
	}
};

struct SummaryOfAtoms
{
	std::size_t number_total;
	double volume;
	BoundingBox bounding_box;

	SummaryOfAtoms() : number_total(0), volume(0.0)
	{
	}

	explicit SummaryOfAtoms(const std::vector<Atom>& atoms) : number_total(0), volume(0.0)
	{
		for(std::vector<Atom>::const_iterator it=atoms.begin();it!=atoms.end();++it)
		{
			feed(*it);
		}
	}

	SummaryOfAtoms(const std::vector<Atom>& atoms, const std::set<std::size_t>& ids) : number_total(0), volume(0.0)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<atoms.size())
			{
				feed(atoms[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid atom id encountered when summarizing atoms."));
			}
		}
	}

	void feed(const Atom& atom)
	{
		number_total++;
		if(atom.value.props.adjuncts.count("volume")>0)
		{
			volume+=atom.value.props.adjuncts.find("volume")->second;
		}
		bounding_box.update(atom.value);
	}

	void print(std::ostream& output, bool no_brackets=false) const
	{
		output << (no_brackets ? "" : "(");
		output << "count=" << number_total;
		if(volume>0.0)
		{
			output << " volume=" << volume;
		}
		output << (no_brackets ? "" : ")");
	}
};

struct SummaryOfContacts
{
	std::size_t number_total;
	std::size_t number_drawable;
	double area;

	SummaryOfContacts() : number_total(0), number_drawable(0), area(0.0)
	{
	}

	explicit SummaryOfContacts(const std::vector<Contact>& contacts) : number_total(0), number_drawable(0), area(0.0)
	{
		for(std::vector<Contact>::const_iterator it=contacts.begin();it!=contacts.end();++it)
		{
			feed(*it);
		}
	}

	SummaryOfContacts(const std::vector<Contact>& contacts, const std::set<std::size_t>& ids) : number_total(0), number_drawable(0), area(0.0)
	{
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<contacts.size())
			{
				feed(contacts[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid contact id encountered when summarizing contacts."));
			}
		}
	}

	void feed(const Contact& contact)
	{
		number_total++;
		area+=contact.value.area;
		if(!contact.value.graphics.empty())
		{
			number_drawable++;
		}
	}

	void print(std::ostream& output, bool no_brackets=false) const
	{
		output << (no_brackets ? "" : "(");
		output << "count=" << number_total;
		output << " drawable=" << number_drawable;
		output << " area=" << area;
		output << (no_brackets ? "" : ")");
	}
};

}

}

#endif /* COMMON_SCRIPTING_BASIC_TYPES_H_ */
