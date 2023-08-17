#ifndef SCRIPTING_BASIC_TYPES_H_
#define SCRIPTING_BASIC_TYPES_H_

#include "../common/construction_of_atomic_balls.h"
#include "../common/construction_of_contacts.h"

#include "figure.h"
#include "figure_of_text.h"

namespace voronota
{

namespace scripting
{

typedef common::ConstructionOfAtomicBalls::AtomicBall Atom;
typedef common::ConstructionOfContacts::Contact Contact;

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
};

struct SummaryOfTriangulation
{
	std::size_t number_of_input_balls;
	std::size_t number_of_all_balls;
	std::size_t number_of_quadruples;
	std::size_t number_of_voronoi_vertices;

	SummaryOfTriangulation() :
		number_of_input_balls(0),
		number_of_all_balls(0),
		number_of_quadruples(0),
		number_of_voronoi_vertices(0)
	{
	}

	explicit SummaryOfTriangulation(const common::ConstructionOfTriangulation::BundleOfTriangulationInformation& triangulation_info) :
		number_of_input_balls(triangulation_info.number_of_input_spheres),
		number_of_all_balls(triangulation_info.spheres.size()),
		number_of_quadruples(triangulation_info.quadruples_map.size()),
		number_of_voronoi_vertices(0)
	{
		for(apollota::Triangulation::QuadruplesMap::const_iterator it=triangulation_info.quadruples_map.begin();it!=triangulation_info.quadruples_map.end();++it)
		{
			number_of_voronoi_vertices+=it->second.size();
		}
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
};

}

}

#endif /* SCRIPTING_BASIC_TYPES_H_ */
