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

		std::map< std::pair<std::string, std::string>, std::vector<apollota::SimpleSphere> > substructures;

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
				std::vector<apollota::SimpleSphere>& spheres=substructures[substructure_id];
				spheres.reserve(loading_result.atoms.size());
				for(std::size_t j=0;j<loading_result.atoms.size();j++)
				{
					const Atom& atom=loading_result.atoms[j];
					spheres.push_back(apollota::SimpleSphere(atom.value.x, atom.value.y, atom.value.z, atom.value.r+probe));
				}
			}
			else
			{
				for(std::size_t j=0;j<loading_result.atoms.size();j++)
				{
					const Atom& atom=loading_result.atoms[j];
					substructure_id.second=atom.crad.chainID;
					std::vector<apollota::SimpleSphere>& spheres=substructures[substructure_id];
					spheres.push_back(apollota::SimpleSphere(atom.value.x, atom.value.y, atom.value.z, atom.value.r+probe));
				}
			}
		}

		std::map< std::pair<std::string, std::string>, apollota::SimpleSphere > bounding_spheres;

		for(std::map< std::pair<std::string, std::string>, std::vector<apollota::SimpleSphere> >::const_iterator it=substructures.begin();it!=substructures.end();++it)
		{
			apollota::SimplePoint center;
			const std::vector<apollota::SimpleSphere>& spheres=it->second;
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
			bounding_spheres[it->first]=boundary;
		}

		std::map< std::pair<std::string, std::string>, apollota::BoundingSpheresHierarchy > search_hierarchies;

		std::vector< std::pair< std::pair<std::string, std::string>, std::pair<std::string, std::string> > > pairs_of_interacting_substructures;

		for(std::map< std::pair<std::string, std::string>, apollota::SimpleSphere >::const_iterator it1=bounding_spheres.begin();it1!=bounding_spheres.end();++it1)
		{
			std::map< std::pair<std::string, std::string>, apollota::SimpleSphere >::const_iterator it2=it1;
			++it2;
			for(;it2!=bounding_spheres.end();++it2)
			{
				if(apollota::sphere_intersects_sphere(it1->second, it2->second))
				{
					std::pair<std::string, std::string> id1=it1->first;
					std::pair<std::string, std::string> id2=it2->first;
					apollota::SimpleSphere bs1=it1->second;
					apollota::SimpleSphere bs2=it2->second;
					std::map< std::pair<std::string, std::string>, std::vector<apollota::SimpleSphere> >::const_iterator it_spheres1=substructures.find(id1);
					std::map< std::pair<std::string, std::string>, std::vector<apollota::SimpleSphere> >::const_iterator it_spheres2=substructures.find(id2);
					if(it_spheres1!=substructures.end() && it_spheres2!=substructures.end())
					{
						std::map< std::pair<std::string, std::string>, apollota::BoundingSpheresHierarchy >::iterator it_bsh1=search_hierarchies.find(id1);
						std::map< std::pair<std::string, std::string>, apollota::BoundingSpheresHierarchy >::iterator it_bsh2=search_hierarchies.find(id2);
						if((it_bsh1!=search_hierarchies.end() && it_bsh2==search_hierarchies.end()) || ((it_bsh2==search_hierarchies.end() || (it_bsh1!=search_hierarchies.end() && it_bsh2!=search_hierarchies.end())) && it_spheres1->second.size()<it_spheres2->second.size()))
						{
							std::swap(id1, id2);
							std::swap(bs1, bs2);
							std::swap(it_spheres1, it_spheres2);
							std::swap(it_bsh1, it_bsh2);
						}
						const std::vector<apollota::SimpleSphere>& spheres1=it_spheres1->second;
						const std::vector<apollota::SimpleSphere>& spheres2=it_spheres2->second;
						apollota::BoundingSpheresHierarchy& bsh2=(it_bsh2!=search_hierarchies.end() ? (it_bsh2->second) : search_hierarchies[id2]);
						if(bsh2.leaves_spheres().empty())
						{
							bsh2=apollota::BoundingSpheresHierarchy(spheres2, 3.5+probe, 1);
						}
						bool found_collision=false;
						if(!apollota::SearchForSphericalCollisions::find_any_collision(bsh2, bs1).empty())
						{
							for(std::vector<apollota::SimpleSphere>::const_iterator sphere_it=spheres1.begin();sphere_it!=spheres1.end() && !found_collision;++sphere_it)
							{
								const apollota::SimpleSphere& sphere=(*sphere_it);
								if(apollota::sphere_intersects_sphere(bs2, sphere) && !apollota::SearchForSphericalCollisions::find_any_collision(bsh2, sphere).empty())
								{
									found_collision=true;
								}
							}
						}
						if(found_collision)
						{
							pairs_of_interacting_substructures.push_back(std::make_pair(it1->first, it2->first));
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
				const std::pair< std::pair<std::string, std::string>, std::pair<std::string, std::string> >& p=pairs_of_interacting_substructures[i];
				foutput << p.first.first << " " << p.second.first;
				if(consider_chains)
				{
					foutput << " " << p.first.second << " " << p.second.second;
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
