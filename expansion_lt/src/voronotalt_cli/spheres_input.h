#ifndef VORONOTALT_SPHERES_INPUT_H_
#define VORONOTALT_SPHERES_INPUT_H_

#include <string>
#include <vector>
#include <map>

#include "../voronotalt/basic_types_and_functions.h"
#include "../voronotalt/time_recorder.h"

#include "io_utilities.h"

namespace voronotalt
{

class SpheresInput
{
public:
	struct SphereLabel
	{
		std::string chain_id;
		std::string residue_id;
		std::string atom_name;
	};

	struct Result
	{
		std::vector<SimpleSphere> spheres;
		std::vector<SphereLabel> sphere_labels;
		std::vector<int> grouping_by_chain;
		std::vector<int> grouping_by_residue;
		int label_size;
		int number_of_chain_groups;
		int number_of_residue_groups;

		Result() noexcept : label_size(0), number_of_chain_groups(0), number_of_residue_groups(0)
		{
		}
	};

	static bool read_labeled_or_unlabeled_spheres_from_stream(
			std::istream& input_stream,
			const Float probe,
			Result& result,
			std::ostream& error_message_output_stream,
			TimeRecorder& time_recorder) noexcept
	{
		time_recorder.reset();

		result=Result();

		{
			std::vector<std::string> string_ids;
			std::vector<double> values;
			if(read_string_ids_and_double_values_from_text_stream(4, input_stream, string_ids, values))
			{
				const std::size_t N=(values.size()/4);
				const std::size_t label_size=(string_ids.size()/N);
				const bool labels_tailing=(!string_ids.empty() && string_ids.front()=="#");
				if(!(label_size<=3 || (labels_tailing && label_size==8)) || string_ids.size()!=N*label_size)
				{
					error_message_output_stream << "Error: invalid input format, must be exactly 0, 1, 2, or 3 front string IDs or 8 tailing string IDs per line\n";
					return false;
				}
				result.spheres.resize(N);
				for(std::size_t i=0;i<N;i++)
				{
					SimpleSphere& sphere=result.spheres[i];
					sphere.p.x=static_cast<Float>(values[i*4+0]);
					sphere.p.y=static_cast<Float>(values[i*4+1]);
					sphere.p.z=static_cast<Float>(values[i*4+2]);
					sphere.r=static_cast<Float>(values[i*4+3])+probe;
				}
				if(label_size>0)
				{
					result.sphere_labels.resize(N);
					for(std::size_t i=0;i<N;i++)
					{
						SphereLabel& sphere_label=result.sphere_labels[i];
						if(label_size==1)
						{
							sphere_label.chain_id=string_ids[i];
						}
						else if(label_size==2)
						{
							sphere_label.chain_id=string_ids[i*label_size+0];
							sphere_label.residue_id=string_ids[i*label_size+1];
						}
						else if(label_size==3)
						{
							sphere_label.chain_id=string_ids[i*label_size+0];
							sphere_label.residue_id=string_ids[i*label_size+1];
							sphere_label.atom_name=string_ids[i*label_size+2];
						}
						else if(label_size==8)
						{
							sphere_label.chain_id=string_ids[i*label_size+2];
							sphere_label.residue_id=string_ids[i*label_size+3];
							sphere_label.atom_name=string_ids[i*label_size+5];
							if(string_ids[i*label_size+7]!=".")
							{
								sphere_label.residue_id+=std::string("/")+string_ids[i*label_size+7];
							}
							if(string_ids[i*label_size+4]!=".")
							{
								sphere_label.residue_id+=std::string("|")+string_ids[i*label_size+4];
							}
						}
						result.label_size=std::min(static_cast<int>(label_size), 3);
					}
				}
			}
			else
			{
				error_message_output_stream << "Error: invalid data in stdin, must be a text table with exactly 0, 1, 2, or 3 string IDs and exactly 4 floating point values (x, y, z, r) per line\n";
				return false;
			}
		}

		time_recorder.record_elapsed_miliseconds_and_reset("read spheres data from stream");

		if(result.label_size>1)
		{
			result.number_of_chain_groups=assign_groups_to_sphere_labels_by_chain(result.sphere_labels, result.grouping_by_chain);

			result.number_of_residue_groups=assign_groups_to_sphere_labels_by_residue(result.sphere_labels, result.grouping_by_residue);

			time_recorder.record_elapsed_miliseconds_and_reset("assign groups based on labels");
		}

		return true;
	}

private:
	static int assign_groups_to_sphere_labels_by_chain(const std::vector<SphereLabel>& sphere_labels, std::vector<int>& groups) noexcept
	{
		groups.clear();
		groups.resize(sphere_labels.size(), 0);

		std::map<std::string, int> map_of_chains_to_groups;
		for(std::size_t i=0;i<sphere_labels.size();i++)
		{
			const SphereLabel& sl=sphere_labels[i];
			std::map<std::string, int>::const_iterator it=map_of_chains_to_groups.find(sl.chain_id);
			if(it==map_of_chains_to_groups.end())
			{
				groups[i]=static_cast<int>(map_of_chains_to_groups.size());
				map_of_chains_to_groups[sl.chain_id]=groups[i];
			}
			else
			{
				groups[i]=it->second;
			}
		}

		if(map_of_chains_to_groups.size()<2)
		{
			groups.clear();
		}

		return static_cast<int>(map_of_chains_to_groups.size());
	}

	static int assign_groups_to_sphere_labels_by_residue(const std::vector<SphereLabel>& sphere_labels, std::vector<int>& groups) noexcept
	{
		groups.clear();
		groups.resize(sphere_labels.size(), 0);

		std::map< std::pair<std::string, std::string>, int> map_of_residues_to_groups;
		for(std::size_t i=0;i<sphere_labels.size();i++)
		{
			const SphereLabel& sl=sphere_labels[i];
			const std::pair<std::string, std::string> chain_residue_id(sl.chain_id, sl.residue_id);
			std::map< std::pair<std::string, std::string>, int>::const_iterator it=map_of_residues_to_groups.find(chain_residue_id);
			if(it==map_of_residues_to_groups.end())
			{
				groups[i]=static_cast<int>(map_of_residues_to_groups.size());
				map_of_residues_to_groups[chain_residue_id]=groups[i];
			}
			else
			{
				groups[i]=it->second;
			}
		}

		if(map_of_residues_to_groups.size()<2)
		{
			groups.clear();
		}

		return static_cast<int>(map_of_residues_to_groups.size());
	}
};

}

#endif /* VORONOTALT_SPHERES_INPUT_H_ */
