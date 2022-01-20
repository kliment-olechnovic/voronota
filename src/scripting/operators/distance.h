#ifndef SCRIPTING_OPERATORS_DISTANCE_H_
#define SCRIPTING_OPERATORS_DISTANCE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class Distance : public OperatorBase<Distance>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		double distance_min;
		double distance_max;
		std::size_t total_count;
		std::vector<double> histogram_breaks;
		std::vector<std::size_t> histogram_values;

		Result() :
			distance_min(std::numeric_limits<double>::max()),
			distance_max((-std::numeric_limits<double>::max())),
			total_count(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("distance_min")=distance_min;
			heterostorage.variant_object.value("distance_max")=distance_max;
			heterostorage.variant_object.value("total_count")=total_count;

			{
				std::vector<VariantValue>& array=heterostorage.variant_object.values_array("histogram_breaks");
				for(std::size_t i=0;i<histogram_breaks.size();i++)
				{
					array.push_back(VariantValue(histogram_breaks[i]));
				}
			}

			{
				std::vector<VariantValue>& array=heterostorage.variant_object.values_array("histogram_values");
				for(std::size_t i=0;i<histogram_values.size();i++)
				{
					array.push_back(VariantValue(histogram_values[i]));
				}
			}
		}
	};

	std::string selection_expresion_for_atoms_a;
	std::string selection_expresion_for_atoms_b;
	bool inter_chain;
	std::vector<double> histogram_breaks;
	std::string adjunct_min;
	std::string adjunct_max;

	Distance() : inter_chain(false)
	{
	}

	void initialize(CommandInput& input)
	{
		selection_expresion_for_atoms_a=input.get_value<std::string>("atoms-first");
		selection_expresion_for_atoms_b=input.get_value<std::string>("atoms-second");
		inter_chain=input.get_flag("inter-chain");
		histogram_breaks=input.get_value_vector_or_default<double>("histogram-breaks", std::vector<double>());
		adjunct_min=input.get_value_or_default<std::string>("adjunct-min", "");
		adjunct_max=input.get_value_or_default<std::string>("adjunct-max", "");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("atoms-first", CDOD::DATATYPE_STRING, "selection expression for the first group of atoms"));
		doc.set_option_decription(CDOD("atoms-second", CDOD::DATATYPE_STRING, "selection expression for the second group of atoms"));
		doc.set_option_decription(CDOD("inter-chain", CDOD::DATATYPE_BOOL, "flag to only calculate inter-chain distances"));
		doc.set_option_decription(CDOD("histogram-breaks", CDOD::DATATYPE_FLOAT_ARRAY, "values of histogram breaks", ""));
		doc.set_option_decription(CDOD("adjunct-min", CDOD::DATATYPE_STRING, "name of output adjunct for min distance", ""));
		doc.set_option_decription(CDOD("adjunct-max", CDOD::DATATYPE_STRING, "name of output adjunct for max distance", ""));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(adjunct_min, true);
		assert_adjunct_name_input(adjunct_max, true);

		const std::set<std::size_t> atom_ids_a=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_a, false));

		if(atom_ids_a.empty())
		{
			throw std::runtime_error(std::string("No first atoms selected."));
		}

		const std::set<std::size_t> atom_ids_b=data_manager.selection_manager().select_atoms(SelectionManager::Query(selection_expresion_for_atoms_b, false));

		if(atom_ids_b.empty())
		{
			throw std::runtime_error(std::string("No second atoms selected."));
		}

		Result result;

		result.histogram_breaks=histogram_breaks;
		std::sort(result.histogram_breaks.begin(), result.histogram_breaks.end());
		result.histogram_values.resize(result.histogram_breaks.size()+1, 0);

		for(std::set<std::size_t>::const_iterator it_a=atom_ids_a.begin();it_a!=atom_ids_a.end();++it_a)
		{
			const Atom& atom_a=data_manager.atoms()[*it_a];
			for(std::set<std::size_t>::const_iterator it_b=atom_ids_b.begin();it_b!=atom_ids_b.end();++it_b)
			{
				const Atom& atom_b=data_manager.atoms()[*it_b];
				if(!inter_chain || atom_a.crad.chainID!=atom_b.crad.chainID)
				{
					const double distance=apollota::distance_from_point_to_point(atom_a.value, atom_b.value);

					if(distance<result.distance_min)
					{
						result.distance_min=distance;
					}
					if(distance>result.distance_max)
					{
						result.distance_max=distance;
					}

					result.total_count++;

					if(result.histogram_values.size()==1)
					{
						result.histogram_values[0]++;
					}
					else
					{
						bool found_bin=false;
						for(std::size_t i=0;i<result.histogram_values.size() && !found_bin;i++)
						{
							if(i==0)
							{
								found_bin=(distance<=result.histogram_breaks[i]);
							}
							else if((i+1)==result.histogram_values.size())
							{
								found_bin=(distance>result.histogram_breaks[i-1]);
							}
							else
							{
								found_bin=((distance<=result.histogram_breaks[i]) && (distance>result.histogram_breaks[i-1]));
							}

							if(found_bin)
							{
								result.histogram_values[i]++;
							}
						}
					}

					if(!adjunct_min.empty() || !adjunct_max.empty())
					{
						std::map<std::string, double>& map_a=data_manager.atom_adjuncts_mutable(*it_a);
						std::map<std::string, double>& map_b=data_manager.atom_adjuncts_mutable(*it_b);
						if(!adjunct_min.empty())
						{
							if(map_a.count(adjunct_min)<1 || map_a[adjunct_min]>distance)
							{
								map_a[adjunct_min]=distance;
							}
							if(map_b.count(adjunct_min)<1 || map_b[adjunct_min]>distance)
							{
								map_b[adjunct_min]=distance;
							}
						}
						if(!adjunct_max.empty())
						{
							if(map_a.count(adjunct_max)<1 || map_a[adjunct_max]<distance)
							{
								map_a[adjunct_max]=distance;
							}
							if(map_b.count(adjunct_max)<1 || map_b[adjunct_max]<distance)
							{
								map_b[adjunct_max]=distance;
							}
						}
					}
				}
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DISTANCE_H_ */
