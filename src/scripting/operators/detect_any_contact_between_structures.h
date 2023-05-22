#ifndef SCRIPTING_OPERATORS_DETECT_ANY_CONTACT_BETWEEN_STRUCTURES_H_
#define SCRIPTING_OPERATORS_DETECT_ANY_CONTACT_BETWEEN_STRUCTURES_H_

#include "../operators_common.h"
#include "../loading_of_data.h"

#include "../../apollota/search_for_spherical_collisions.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DetectAnyContactBetweenStructures : public OperatorBase<DetectAnyContactBetweenStructures>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string input_file;
	std::string output_file;
	LoadingOfData::Parameters loading_parameters;
	double probe;
	bool consider_chains;
	bool output_header;

	DetectAnyContactBetweenStructures() : probe(1.4), consider_chains(false), output_header(false)
	{
	}

	void initialize(CommandInput& input)
	{
		input_file=input.get_value<std::string>("input-file");
		output_file=input.get_value<std::string>("output-file");
		loading_parameters=LoadingOfData::Parameters();
		loading_parameters.format=input.get_value_or_default<std::string>("format", "");
		loading_parameters.format_fallback=input.get_value_or_default<std::string>("format-fallback", "pdb");
		loading_parameters.forced_include_heteroatoms=input.is_option("include-heteroatoms");
		loading_parameters.forced_include_hydrogens=input.is_option("include-hydrogens");
		loading_parameters.forced_multimodel_chains=input.is_option("as-assembly");
		loading_parameters.forced_same_radius_for_all=input.is_option("same-radius-for-all");
		loading_parameters.include_heteroatoms=input.get_flag("include-heteroatoms");
		loading_parameters.include_hydrogens=input.get_flag("include-hydrogens");
		loading_parameters.multimodel_chains=input.get_flag("as-assembly");
		loading_parameters.same_radius_for_all=input.get_value_or_default<double>("same-radius-for-all", LoadingOfData::Configuration::recommended_default_radius());
		probe=input.get_value_or_default<double>("probe", 1.4);
		consider_chains=input.get_flag("consider-chains");
		output_header=input.get_flag("output-header");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-file", CDOD::DATATYPE_STRING, "path to input list file"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("format", CDOD::DATATYPE_STRING, "input file format", ""));
		doc.set_option_decription(CDOD("format-fallback", CDOD::DATATYPE_STRING, "input file format fallback", ""));
		doc.set_option_decription(CDOD("include-heteroatoms", CDOD::DATATYPE_BOOL, "flag to include heteroatoms"));
		doc.set_option_decription(CDOD("include-hydrogens", CDOD::DATATYPE_BOOL, "flag to include hydrogens"));
		doc.set_option_decription(CDOD("as-assembly", CDOD::DATATYPE_BOOL, "flag import as a biological assembly"));
		doc.set_option_decription(CDOD("same-radius-for-all", CDOD::DATATYPE_FLOAT, "radius to use for all atoms", LoadingOfData::Configuration::recommended_default_radius()));
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "probe radius", 1.4));
		doc.set_option_decription(CDOD("consider-chains", CDOD::DATATYPE_BOOL, "flag to consider chains and detect possible contacts between chains"));
		doc.set_option_decription(CDOD("output-header", CDOD::DATATYPE_BOOL, "flag to start results output with vintage header"));
	}

	Result run(void*) const
	{
		InputSelector finput_selector(input_file);
		std::istream& finput=finput_selector.stream();
		assert_io_stream(input_file, finput);

		std::vector<std::string> input_filenames;

		while(finput.good())
		{
			std::string line;
			std::getline(finput, line);
			if(!line.empty())
			{
				input_filenames.push_back(line);
			}
		}

		if(input_filenames.empty())
		{
			throw std::runtime_error(std::string("No file names provided."));
		}

		if(!consider_chains && input_filenames.size()==1)
		{
			throw std::runtime_error(std::string("Only one file provided with no consideration of chains."));
		}

		std::vector< std::pair<std::string, std::string> > ids_of_substructures;
		std::vector< std::vector<apollota::SimpleSphere> > substructures;

		{
			for(std::size_t i=0;i<input_filenames.size();i++)
			{
				const std::string& input_name=input_filenames[i];
				LoadingOfData::Parameters loading_parameters_to_use=loading_parameters;
				loading_parameters_to_use.file=input_name;
				LoadingOfData::Result loading_result;
				LoadingOfData::construct_result(loading_parameters_to_use, loading_result);

				if(loading_result.atoms.empty())
				{
					throw std::runtime_error(std::string("No atoms read from file '")+input_name+"'.");
				}

				std::pair<std::string, std::string> substructure_id(input_name, std::string());

				if(!consider_chains)
				{
					ids_of_substructures.push_back(substructure_id);
					substructures.push_back(std::vector<apollota::SimpleSphere>());
					std::vector<apollota::SimpleSphere>& spheres=substructures.back();
					spheres.reserve(loading_result.atoms.size());
					for(std::size_t j=0;j<loading_result.atoms.size();j++)
					{
						const Atom& atom=loading_result.atoms[j];
						spheres.push_back(apollota::SimpleSphere(atom.value.x, atom.value.y, atom.value.z, atom.value.r+probe));
					}
				}
				else
				{
					std::map<std::string, std::size_t> map_of_chains_to_indices;
					for(std::size_t j=0;j<loading_result.atoms.size();j++)
					{
						const Atom& atom=loading_result.atoms[j];
						std::map<std::string, std::size_t>::const_iterator index_it=map_of_chains_to_indices.find(atom.crad.chainID);
						std::size_t index=0;
						if(index_it==map_of_chains_to_indices.end())
						{
							index=substructures.size();
							map_of_chains_to_indices[atom.crad.chainID]=index;
							substructure_id.second=atom.crad.chainID;
							ids_of_substructures.push_back(substructure_id);
							substructures.push_back(std::vector<apollota::SimpleSphere>());
						}
						else
						{
							index=index_it->second;
						}
						std::vector<apollota::SimpleSphere>& spheres=substructures[index];
						spheres.push_back(apollota::SimpleSphere(atom.value.x, atom.value.y, atom.value.z, atom.value.r+probe));
					}
				}
			}
		}

		std::vector< apollota::SimpleSphere > bounding_spheres(substructures.size());

		for(std::size_t i=0;i<substructures.size();i++)
		{
			apollota::SimplePoint center;
			const std::vector<apollota::SimpleSphere>& spheres=substructures[i];
			for(std::size_t j=0;j<spheres.size();j++)
			{
				center=center+apollota::custom_point_from_object<apollota::SimplePoint>(spheres[j]);
			}
			center=center*(1.0/static_cast<double>(spheres.size()));
			apollota::SimpleSphere boundary=apollota::custom_sphere_from_point<apollota::SimpleSphere>(center, 0.0);
			for(std::size_t j=0;j<spheres.size();j++)
			{
				boundary.r=std::max(boundary.r, apollota::maximal_distance_from_point_to_sphere(center, spheres[j]));
			}
			bounding_spheres[i]=boundary;
		}

		std::vector<apollota::BoundingSpheresHierarchy> search_hierarchies(substructures.size());

		std::vector< std::pair<std::size_t, std::size_t> > pairs_of_interacting_substructures;

		for(std::size_t i1=0;i1<substructures.size();i1++)
		{
			for(std::size_t i2=(i1+1);i2<substructures.size();i2++)
			{
				if(apollota::sphere_intersects_sphere(bounding_spheres[i1], bounding_spheres[i2]))
				{
					std::size_t id1=i1;
					std::size_t id2=i2;
					{
						{
							const bool bsh_status1=!search_hierarchies[i1].leaves_spheres().empty();
							const bool bsh_status2=!search_hierarchies[i2].leaves_spheres().empty();
							if(bsh_status1 && !bsh_status2)
							{
								id1=i2;
								id2=i1;
							}
							else if(!bsh_status1 && bsh_status2)
							{
								id1=i1;
								id2=i2;
							}
							else if(substructures[i1].size()<substructures[i2].size())
							{
								id1=i2;
								id2=i1;
							}
						}

						if(search_hierarchies[id2].leaves_spheres().empty())
						{
							search_hierarchies[id2]=apollota::BoundingSpheresHierarchy(substructures[id2], 3.5+probe, 1);
						}

						bool found_collision=false;

						if(!apollota::SearchForSphericalCollisions::find_any_collision(search_hierarchies[id2], bounding_spheres[id1]).empty())
						{
							for(std::vector<apollota::SimpleSphere>::const_iterator sphere_it=substructures[id1].begin();sphere_it!=substructures[id1].end() && !found_collision;++sphere_it)
							{
								const apollota::SimpleSphere& sphere=(*sphere_it);
								if(apollota::sphere_intersects_sphere(bounding_spheres[id2], sphere) && !apollota::SearchForSphericalCollisions::find_any_collision(search_hierarchies[id2], sphere).empty())
								{
									found_collision=true;
								}
							}
						}

						if(found_collision)
						{
							pairs_of_interacting_substructures.push_back(std::make_pair(i1, i2));
						}
					}
				}
			}
		}

		{
			OutputSelector foutput_selector(output_file);
			std::ostream& foutput=foutput_selector.stream();
			assert_io_stream(output_file, foutput);

			if(output_header)
			{
				foutput << "pairs_of_interacting_structures\n";
				foutput << pairs_of_interacting_substructures.size() << "\n";
			}

			for(std::size_t i=0;i<pairs_of_interacting_substructures.size();i++)
			{
				const std::pair<std::size_t, std::size_t>& p=pairs_of_interacting_substructures[i];
				foutput << ids_of_substructures[p.first].first << " " << ids_of_substructures[p.second].first;
				if(consider_chains)
				{
					foutput << " " << ids_of_substructures[p.first].second << " " << ids_of_substructures[p.second].second;
				}
				foutput << "\n";
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DETECT_ANY_CONTACT_BETWEEN_STRUCTURES_H_ */
