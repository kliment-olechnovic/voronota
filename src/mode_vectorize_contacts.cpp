#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::map< std::string, std::map<CRADsPair, double> > read_maps_of_contacts()
{
	std::map< std::string, std::map<CRADsPair, double> > maps_of_contacts;
	const std::set<std::string> input_files=auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin);
	for(std::set<std::string>::const_iterator it=input_files.begin();it!=input_files.end();++it)
	{
		const std::string& filename=(*it);
		const std::map<CRADsPair, double> map_of_contacts=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(filename);
		if(!map_of_contacts.empty())
		{
			maps_of_contacts[filename]=map_of_contacts;
		}
	}
	return maps_of_contacts;
}

const std::map<CRADsPair, std::size_t> collect_crads_ids(const std::map< std::string, std::map<CRADsPair, double> >& maps_of_contacts)
{
	std::map<CRADsPair, std::size_t> crads_ids;
	for(std::map< std::string, std::map<CRADsPair, double> >::const_iterator it=maps_of_contacts.begin();it!=maps_of_contacts.end();++it)
	{
		const std::map<CRADsPair, double>& map_of_contacts=it->second;
		for(std::map<CRADsPair, double>::const_iterator jt=map_of_contacts.begin();jt!=map_of_contacts.end();++jt)
		{
			crads_ids[jt->first]=0;
		}
	}
	{
		std::size_t i=0;
		for(std::map<CRADsPair, std::size_t>::iterator it=crads_ids.begin();it!=crads_ids.end();++it)
		{
			it->second=i;
			i++;
		}
	}
	return crads_ids;
}

std::map< std::string, std::vector<double> > collect_map_of_areas_vectors(
		const std::map< std::string, std::map<CRADsPair, double> >& maps_of_contacts,
		const std::map<CRADsPair, std::size_t>& crads_ids)
{
	std::map< std::string, std::vector<double> > map_of_areas_vectors;
	for(std::map< std::string, std::map<CRADsPair, double> >::const_iterator it=maps_of_contacts.begin();it!=maps_of_contacts.end();++it)
	{
		const std::map<CRADsPair, double>& map_of_contacts=it->second;
		std::vector<double>& areas_vector=map_of_areas_vectors[it->first];
		areas_vector.resize(crads_ids.size());
		for(std::map<CRADsPair, double>::const_iterator jt=map_of_contacts.begin();jt!=map_of_contacts.end();++jt)
		{
			std::map<CRADsPair, std::size_t>::const_iterator crads_ids_it=crads_ids.find(jt->first);
			if(crads_ids_it!=crads_ids.end())
			{
				areas_vector[crads_ids_it->second]=jt->second;
			}
		}
	}
	return map_of_areas_vectors;
}

void print_map_of_areas_vectors(
		const std::map<CRADsPair, std::size_t>& crads_ids,
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors)
{
	std::cout << "title";
	for(std::map<CRADsPair, std::size_t>::const_iterator it=crads_ids.begin();it!=crads_ids.end();++it)
	{
		const CRADsPair& crads=it->first;
		std::cout << " " << crads.a.str() << "__" << crads.b.str();
	}
	std::cout << "\n";
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		std::cout << it->first;
		const std::vector<double>& areas_vector=it->second;
		for(std::size_t i=0;i<areas_vector.size();i++)
		{
			std::cout << " " << areas_vector[i];
		}
		std::cout << "\n";
	}
}

double calc_configurable_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b, const bool symmetric)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum_diffs=0.0;
	double sum_all=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		const double ref=(symmetric ? std::max(a[i], b[i]) : a[i]);
		sum_diffs+=std::min(fabs(a[i]-b[i]), ref);
		sum_all+=ref;
	}
	return (1.0-(sum_diffs/sum_all));
}

double calc_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	return calc_configurable_cadscore_of_two_vectors(a, b, true);
}

double calc_euclidean_distance_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		sum+=((a[i]-b[i])*(a[i]-b[i]));
	}
	return sqrt(sum);
}

template<typename Functor>
void print_similarity_matrix(const std::map< std::string, std::vector<double> >& map_of_areas_vectors, const std::string& output_file, Functor functor)
{
	if(map_of_areas_vectors.empty() || output_file.empty())
	{
		return;
	}
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		output << (it==map_of_areas_vectors.begin() ? "" : " ") << it->first;
	}
	output << "\n";
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		for(std::map< std::string, std::vector<double> >::const_iterator jt=map_of_areas_vectors.begin();jt!=map_of_areas_vectors.end();++jt)
		{
			output << (jt==map_of_areas_vectors.begin() ? "" : " ") << functor(it->second, jt->second);
		}
		output << "\n";
	}
}

template<typename Functor>
std::vector< std::vector<double> > calc_similarity_matrix(const std::map< std::string, std::vector<double> >& map_of_areas_vectors, Functor functor)
{
	std::vector< std::vector<double> > matrix(map_of_areas_vectors.size(), std::vector<double>(map_of_areas_vectors.size(), 0.0));
	int i=0;
	for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
	{
		int j=0;
		for(std::map< std::string, std::vector<double> >::const_iterator jt=map_of_areas_vectors.begin();jt!=map_of_areas_vectors.end();++jt)
		{
			matrix[i][j]=functor(it->second, jt->second);
			j++;
		}
		i++;
	}
	return matrix;
}

std::vector< std::vector<std::size_t> > calc_clusters_using_similarity_matrix(
		const std::vector< std::vector<double> >& matrix,
		const double threshold,
		const bool eliminate_false_singletons)
{
	std::vector< std::vector<std::size_t> > clusters;
	std::set<std::size_t> unclustered_ids;
	for(std::size_t i=0;i<matrix.size();i++)
	{
		unclustered_ids.insert(i);
	}
	bool end=false;
	while(!end)
	{
		std::pair<int, std::size_t> max_neighbours_value_id(-1, 0);
		for(std::set<std::size_t>::const_iterator it=unclustered_ids.begin();it!=unclustered_ids.end();++it)
		{
			const std::size_t i=(*it);
			int neighbours=0;
			for(std::set<std::size_t>::const_iterator jt=unclustered_ids.begin();jt!=unclustered_ids.end();++jt)
			{
				const std::size_t j=(*jt);
				if(matrix[i][j]>=threshold)
				{
					neighbours++;
				}
			}
			if(neighbours>max_neighbours_value_id.first)
			{
				max_neighbours_value_id.first=neighbours;
				max_neighbours_value_id.second=i;
			}
		}
		if(max_neighbours_value_id.first>=0)
		{
			const std::size_t i=max_neighbours_value_id.second;
			std::vector<std::size_t> cluster(1, i);
			cluster.reserve(max_neighbours_value_id.first+1);
			for(std::set<std::size_t>::const_iterator jt=unclustered_ids.begin();jt!=unclustered_ids.end();++jt)
			{
				const std::size_t j=(*jt);
				if(j!=i && matrix[i][j]>=threshold)
				{
					cluster.push_back(j);
				}
			}
			for(std::vector<std::size_t>::const_iterator it=cluster.begin();it!=cluster.end();++it)
			{
				unclustered_ids.erase(*it);
			}
			clusters.push_back(cluster);
		}
		else
		{
			end=true;
		}
	}
	if(eliminate_false_singletons)
	{
		std::vector<std::size_t> non_singletons;
		std::vector<std::size_t> singletons;
		for(std::size_t l=0;l<clusters.size();l++)
		{
			if(clusters[l].size()==1)
			{
				singletons.push_back(l);
			}
			else if(clusters[l].size()>1)
			{
				non_singletons.push_back(l);
			}
		}
		std::vector<std::size_t> true_singletons;
		for(std::size_t l=0;l<singletons.size();l++)
		{
			const std::size_t i=clusters[singletons[l]].front();
			std::pair<std::size_t, double> max_similarity_value_id(0, 0.0);
			for(std::size_t m=0;m<non_singletons.size();m++)
			{
				const std::vector<std::size_t>& cluster=clusters[non_singletons[m]];
				for(std::size_t n=0;n<cluster.size();n++)
				{
					const std::size_t j=cluster[n];
					if(matrix[i][j]>max_similarity_value_id.second)
					{
						max_similarity_value_id.first=non_singletons[m];
						max_similarity_value_id.second=matrix[i][j];
					}
				}
			}
			if(max_similarity_value_id.second>=threshold)
			{
				clusters[max_similarity_value_id.first].push_back(i);
			}
			else
			{
				true_singletons.push_back(singletons[l]);
			}
		}
		std::vector<std::size_t> survived_clusters=non_singletons;
		survived_clusters.insert(survived_clusters.end(), true_singletons.begin(), true_singletons.end());
		std::multimap<std::size_t, std::size_t> survived_clusters_sizes;
		for(std::size_t l=0;l<survived_clusters.size();l++)
		{
			survived_clusters_sizes.insert(std::make_pair(clusters[survived_clusters[l]].size(), survived_clusters[l]));
		}
		std::vector< std::vector<std::size_t> > refined_clusters;
		refined_clusters.reserve(survived_clusters.size());
		for(std::multimap<std::size_t, std::size_t>::const_reverse_iterator it=survived_clusters_sizes.rbegin();it!=survived_clusters_sizes.rend();++it)
		{
			refined_clusters.push_back(clusters[it->second]);
		}
		return refined_clusters;
	}
	else
	{
		return clusters;
	}
}

void print_clusters(
		const std::map< std::string, std::vector<double> >& map_of_areas_vectors,
		const std::vector< std::vector<std::size_t> > clusters,
		const std::string& output_file)
{
	std::ofstream output(output_file.c_str(), std::ios::out);
	if(!output.good())
	{
		return;
	}
	std::vector<std::string> titles(map_of_areas_vectors.size());
	{
		std::size_t i=0;
		for(std::map< std::string, std::vector<double> >::const_iterator it=map_of_areas_vectors.begin();it!=map_of_areas_vectors.end();++it)
		{
			titles[i]=(it->first);
			i++;
		}
	}
	for(std::size_t i=0;i<clusters.size();i++)
	{
		for(std::size_t j=0;j<clusters[i].size();j++)
		{
			output << (j==0 ? "" : " ") << titles[clusters[i][j]];
		}
		output << "\n";
	}
}

}

void vectorize_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts files");
	pohw.describe_io("stdout", false, true, "table of contacts vectors");

	const std::string cadscore_matrix_file=poh.argument<std::string>(pohw.describe_option("--CAD-score-matrix", "string", "file path to output CAD-score matrix"), "");
	const std::string distance_matrix_file=poh.argument<std::string>(pohw.describe_option("--distance-matrix", "string", "file path to output euclidean distance matrix"), "");
	const std::string clustering_output_file=poh.argument<std::string>(pohw.describe_option("--clustering-output", "string", "file path to output clusters"), "");
	const double clustering_threshold=poh.argument<double>(pohw.describe_option("--clustering-threshold", "string", "clustering threshold value"), 0.5);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map< std::string, std::map<CRADsPair, double> > maps_of_contacts=read_maps_of_contacts();
	if(maps_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRADsPair, std::size_t> crads_ids=collect_crads_ids(maps_of_contacts);
	const std::map< std::string, std::vector<double> > map_of_areas_vectors=collect_map_of_areas_vectors(maps_of_contacts, crads_ids);

	print_similarity_matrix(map_of_areas_vectors, cadscore_matrix_file, calc_cadscore_of_two_vectors);
	print_similarity_matrix(map_of_areas_vectors, distance_matrix_file, calc_euclidean_distance_of_two_vectors);

	if(!clustering_output_file.empty())
	{
		print_clusters(
				map_of_areas_vectors,
				calc_clusters_using_similarity_matrix(calc_similarity_matrix(map_of_areas_vectors, calc_cadscore_of_two_vectors), clustering_threshold, true),
				clustering_output_file);
	}

	print_map_of_areas_vectors(crads_ids, map_of_areas_vectors);
}
