#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_ALT_CONTACTS_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_ALT_CONTACTS_H_

#include "../operators_common.h"

#include "../../apollota/constrained_contact_contour_radicalized.h"
#include "../../apollota/constrained_contact_contour_radicalized_simplified.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfAltContacts : public OperatorBase<AddFigureOfAltContacts>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int total_count;
		double total_area;
		int total_complexity;
		int total_collisions;

		Result() : total_count(0), total_area(0.0), total_complexity(0), total_collisions(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("total_count")=total_count;
			heterostorage.variant_object.value("total_area")=total_area;
			heterostorage.variant_object.value("total_complexity")=total_complexity;
			heterostorage.variant_object.value("total_collisions")=total_collisions;
		}
	};

	std::vector<std::string> figure_name;
	bool radicalized;
	bool simplified;
	bool all_inter_residue;
	bool only_summarize;
	bool only_count_collisions;

	AddFigureOfAltContacts() : radicalized(false), simplified(false), all_inter_residue(false), only_summarize(false), only_count_collisions(false)
	{
	}

	void initialize(CommandInput& input)
	{
		figure_name=input.get_value_vector<std::string>("figure-name");
		radicalized=input.get_flag("radicalized");
		simplified=input.get_flag("simplified");
		all_inter_residue=input.get_flag("all-inter-residue");
		only_summarize=input.get_flag("only-summarize");
		only_count_collisions=input.get_flag("only-count-collisions");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
		doc.set_option_decription(CDOD("force", CDOD::DATATYPE_BOOL, "flag to use radicalized procedure"));
		doc.set_option_decription(CDOD("all-inter-residue", CDOD::DATATYPE_BOOL, "flag construct all inter-residue contacts"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::vector<apollota::SimpleSphere> spheres;
		spheres.reserve(data_manager.atoms().size());

		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const Atom& atom=data_manager.atoms()[i];
			spheres.push_back(apollota::SimpleSphere(atom.value, atom.value.r+1.4));
		}

		Result result;

		apollota::BoundingSpheresHierarchy bsh(spheres, 3.0, 1);

		std::vector< std::vector<std::size_t> > map_of_neighbors(spheres.size());

		for(std::size_t i=0;i<spheres.size();i++)
		{
			const std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
			for(std::size_t j=0;j<collisions.size();j++)
			{
				const std::size_t collision_id=collisions[j];
				if(i!=collision_id)
				{
					map_of_neighbors[i].push_back(collision_id);
					result.total_collisions++;
				}
			}
		}

		if(only_count_collisions)
		{
			return result;
		}

		Figure figure;
		if(!only_summarize)
		{
			figure.name=LongName(figure_name);
		}

		for(std::size_t i=0;i<map_of_neighbors.size();i++)
		{
			for(std::size_t j=0;j<map_of_neighbors[i].size();j++)
			{
				const std::size_t a_id=i;
				const std::size_t b_id=map_of_neighbors[i][j];
				if(a_id<b_id)
				{
					const Atom& atom_a=data_manager.atoms()[a_id];
					const Atom& atom_b=data_manager.atoms()[b_id];
					if(atom_a.crad.chainID!=atom_b.crad.chainID || (all_inter_residue && atom_a.crad.resSeq!=atom_b.crad.resSeq))
					{
						if(radicalized)
						{
							if(simplified)
							{
								apollota::ConstrainedContactContourRadicalizedSimplified::Contour contour;
								if(apollota::ConstrainedContactContourRadicalizedSimplified::construct_contact_contours_for_expanded_spheres_without_tessellation(spheres, a_id, b_id, map_of_neighbors[a_id], map_of_neighbors[b_id], 0.2, contour))
								{
									result.total_count++;
									const apollota::SimplePoint center=apollota::ConstrainedContactContourRadicalizedSimplified::calc_contour_center(contour);
									const apollota::SimplePoint normal=apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit();
									for(std::size_t e=0;e<contour.size();e++)
									{
										const std::size_t e2=(((e+1)<contour.size()) ? (e+1) : 0);
										result.total_area+=apollota::triangle_area(center, contour[e].p, contour[e2].p);
										if(!only_summarize)
										{
											figure.add_triangle(center, contour[e].p, contour[e2].p, normal);
										}
									}
									result.total_complexity+=static_cast<int>(contour.size());
								}
							}
							else
							{
								const std::list<apollota::ConstrainedContactContourRadicalized::Contour> contours=apollota::ConstrainedContactContourRadicalized::construct_contact_contours_for_expanded_spheres_without_tessellation(spheres, a_id, b_id, map_of_neighbors[a_id], map_of_neighbors[b_id], 0.2);
								if(!contours.empty())
								{
									result.total_count++;
									for(std::list<apollota::ConstrainedContactContourRadicalized::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
									{
										const apollota::ConstrainedContactContourRadicalized::ContourAreaDescriptor d=apollota::ConstrainedContactContourRadicalized::construct_contour_area_descriptor(*contours_it);
										const apollota::SimplePoint normal=apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit();
										for(std::size_t e=0;e<d.outline.size();e++)
										{
											const std::size_t e2=(((e+1)<d.outline.size()) ? (e+1) : 0);
											result.total_area+=apollota::triangle_area(d.center, d.outline[e], d.outline[e2]);
											if(!only_summarize)
											{
												figure.add_triangle(d.center, d.outline[e], d.outline[e2], normal);
											}
										}
										result.total_complexity+=static_cast<int>(d.outline.size());
									}
								}
							}
						}
						else
						{
							const std::list<apollota::ConstrainedContactContour::Contour> contours=apollota::ConstrainedContactContour::construct_contact_contours_for_expanded_spheres_without_tessellation(spheres, a_id, b_id, map_of_neighbors[a_id], map_of_neighbors[b_id], 0.2, 5, false);
							if(!contours.empty())
							{
								result.total_count++;
								for(std::list<apollota::ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
								{
									const apollota::ConstrainedContactContour::ContourAreaDescriptor d=apollota::ConstrainedContactContour::construct_contour_area_descriptor(*contours_it, spheres[a_id], spheres[b_id], false);
									const apollota::SimplePoint normal=apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit();
									for(std::size_t e=0;e<d.outline.size();e++)
									{
										const std::size_t e2=(((e+1)<d.outline.size()) ? (e+1) : 0);
										result.total_area+=apollota::triangle_area(d.center, d.outline[e], d.outline[e2]);
										if(!only_summarize)
										{
											figure.add_triangle(d.center, d.outline[e], d.outline[e2], normal);
										}
									}
									result.total_complexity+=static_cast<int>(d.outline.size());
								}
							}
						}
					}
				}
			}
		}

		if(!only_summarize)
		{
			data_manager.add_figure(figure);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_ALT_CONTACTS_H_ */
