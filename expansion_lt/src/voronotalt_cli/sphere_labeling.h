#ifndef VORONOTALT_CLI_SPHERE_LABELING_H_
#define VORONOTALT_CLI_SPHERE_LABELING_H_

#include <string>
#include <vector>

namespace voronotalt
{

class SphereLabeling
{
public:
	struct ExpandedResidueID
	{
		bool parsed;
		bool valid;
		int rnum;
		std::string icode;
		std::string rname;

		ExpandedResidueID() noexcept : parsed(false), valid(false), rnum(0)
		{
		}
	};

	struct SphereLabel
	{
		std::string chain_id;
		std::string residue_id;
		std::string atom_name;
		ExpandedResidueID expanded_residue_id;
	};

	static void form_residue_id_string(const std::string& rnum, const std::string& icode, const std::string& rname, std::string& residue_id) noexcept
	{
		residue_id.clear();
		if(!rnum.empty() && rnum!=".")
		{
			residue_id=rnum;
		}
		if(!icode.empty() && icode!=".")
		{
			residue_id+=std::string("/")+icode;
		}
		if(!rname.empty() && rname!=".")
		{
			residue_id+=std::string("|")+rname;
		}
	}

	static void form_residue_id_string(const int rnum, const std::string& icode, const std::string& rname, std::string& residue_id) noexcept
	{
		form_residue_id_string(std::to_string(rnum), icode, rname, residue_id);
	}

	static ExpandedResidueID parse_expanded_residue_id(const std::string& residue_id_str) noexcept
	{
		ExpandedResidueID erid;
		erid.parsed=true;
		if(residue_id_str.empty())
		{
			return erid;
		}
		const std::string::size_type pos_bar=residue_id_str.find('|');
		const std::string::size_type pos_slash=residue_id_str.find('/');
		if(pos_bar==std::string::npos && pos_slash==std::string::npos)
		{
			erid.rnum=std::atoi(residue_id_str.c_str());
			if(erid.rnum!=0 || residue_id_str=="0")
			{
				erid.valid=true;
			}
			return erid;
		}
		else
		{
			const std::string num=residue_id_str.substr(0, (pos_slash!=std::string::npos && (pos_bar==std::string::npos || pos_slash<pos_bar)) ? pos_slash : pos_bar);
			erid.rnum=std::atoi(num.c_str());
			if(erid.rnum!=0 || num=="0")
			{
				erid.valid=true;
				if(pos_slash!=std::string::npos && (pos_bar==std::string::npos || pos_slash<pos_bar))
				{
					erid.icode=residue_id_str.substr(pos_slash+1, (pos_bar==std::string::npos ? std::string::npos : (pos_bar-pos_slash-1)));
				}
				if(pos_bar!=std::string::npos)
				{
					erid.rname=residue_id_str.substr(pos_bar+1);
				}
			}
		}
		return erid;
	}

	static void parse_expanded_residue_ids_in_sphere_labels(std::vector<SphereLabel>& sls) noexcept
	{
		for(std::size_t i=0;i<sls.size();i++)
		{
			SphereLabel& sl=sls[i];
			if(!sl.expanded_residue_id.parsed)
			{
				sl.expanded_residue_id=parse_expanded_residue_id(sl.residue_id);
			}
		}
	}
};

}


#endif /* VORONOTALT_CLI_SPHERE_LABELING_H_ */
