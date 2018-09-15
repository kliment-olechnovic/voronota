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

	void update(const BoundingBox& b)
	{
		if(b.filled)
		{
			update(b.p_min);
			update(b.p_max);
		}
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

	void feed(const SummaryOfAtoms& s)
	{
		number_total+=s.number_total;
		volume+=s.volume;
		bounding_box.update(s.bounding_box);
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

	void feed(const SummaryOfContacts& s)
	{
		number_total+=s.number_total;
		number_drawable+=s.number_drawable;
		area+=s.area;
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

class VariantValue
{
public:
	template<typename T>
	struct Descriptor
	{
		bool assigned;
		T value;

		Descriptor() : assigned(false)
		{
		}

		Descriptor& operator=(const T& v)
		{
			assigned=true;
			value=v;
			return (*this);
		}

		void clear()
		{
			assigned=false;
		}
	};

	VariantValue()
	{
	}

	VariantValue(const VariantValue& value)
	{
		(*this)=value;
	}

	template<typename T>
	explicit VariantValue(const T& value)
	{
		(*this)=value;
	}

	VariantValue& operator=(const VariantValue& value)
	{
		v_bool_=value.v_bool_;
		v_long_=value.v_long_;
		v_double_=value.v_double_;
		v_string_=value.v_string_;
		return (*this);
	}

	VariantValue& operator=(const bool& value)
	{
		clear();
		v_bool_=value;
		return (*this);
	}

	VariantValue& operator=(const long& value)
	{
		clear();
		v_long_=value;
		return (*this);
	}

	VariantValue& operator=(const double& value)
	{
		clear();
		v_double_=value;
		return (*this);
	}

	VariantValue& operator=(const std::string& value)
	{
		clear();
		v_string_=value;
		return (*this);
	}

	const Descriptor<bool>& as_bool() const
	{
		return v_bool_;
	}

	const Descriptor<long>& as_long() const
	{
		return v_long_;
	}

	const Descriptor<double>& as_double() const
	{
		return v_double_;
	}

	const Descriptor<std::string>& as_string() const
	{
		return v_string_;
	}

private:
	void clear()
	{
		v_bool_.clear();
		v_long_.clear();
		v_double_.clear();
		v_string_.clear();
	}

	Descriptor<bool> v_bool_;
	Descriptor<long> v_long_;
	Descriptor<double> v_double_;
	Descriptor<std::string> v_string_;
};

}

}

#endif /* COMMON_SCRIPTING_BASIC_TYPES_H_ */
