#ifndef VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_
#define VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_

#include <string>
#include <map>

namespace voronotalt
{

class MolecularRadiiAssignment
{
public:
	static double get_atom_radius(const std::string& resName, const std::string& name) noexcept
	{
		const double default_radius=1.7;
		static const std::string wcany="*";

		if(name.empty() || name==wcany)
		{
			return default_radius;
		}

		const std::map<Descriptor, double>& mrd=initialized_radii_by_descriptors();

		for(int v=0;v<2;v++)
		{
			{
				std::map<Descriptor, double>::const_iterator it=mrd.find(Descriptor((v==0 ? resName : wcany), name));
				if(it!=mrd.end())
				{
					return it->second;
				}
			}

			for(std::size_t i=0;i<name.size();i++)
			{
				std::map<Descriptor, double>::const_iterator it=mrd.find(Descriptor((v==0 ? resName : wcany), name.substr(0, name.size()-i)+wcany));
				if(it!=mrd.end())
				{
					return it->second;
				}
			}
		}

		return default_radius;
	}

private:
	struct Descriptor
	{
		std::string resName_pattern;
		std::string name_pattern;

		Descriptor(const std::string& resName_pattern, const std::string& name_pattern) noexcept : resName_pattern(resName_pattern), name_pattern(name_pattern)
		{
		}

		bool operator<(const Descriptor& d) const noexcept
		{
			return ( resName_pattern<d.resName_pattern || (resName_pattern==d.resName_pattern && (name_pattern<d.name_pattern)) );
		}
	};

	static std::map<Descriptor, double>& radii_by_descriptors() noexcept
	{
		static std::map<Descriptor, double> map_of_radii_by_descriptors;
		return map_of_radii_by_descriptors;
	}

	static std::map<Descriptor, double>& initialized_radii_by_descriptors() noexcept
	{
		std::map<Descriptor, double>& mrd=radii_by_descriptors();
		if(mrd.empty())
		{
			mrd[Descriptor("*", "C*")]=1.70;
			mrd[Descriptor("*", "N*")]=1.55;
			mrd[Descriptor("*", "O*")]=1.52;
			mrd[Descriptor("*", "S*")]=1.80;
			mrd[Descriptor("*", "P*")]=1.80;
			mrd[Descriptor("*", "H*")]=1.20;
		}
		return mrd;
	}
};

}

#endif /* VORONOTALT_CLI_MOLECULAR_RADII_ASSIGNMENT_H_ */
