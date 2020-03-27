#ifndef COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_
#define COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_

#include "conversion_of_descriptors.h"

namespace voronota
{

namespace common
{

class ConstructionOfContactEffectGroupings
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;

	class ContactEffectGrouping
	{
	public:
		std::vector< std::set<CRAD> > related_crads;
		std::set<CRAD> collapsed_related_crads;
		std::vector< std::set<CRADsPair> > related_crads_pairs;
		std::set<CRADsPair> collapsed_related_crads_pairs;

		static std::size_t calc_number_of_inter_residue_layers(const std::size_t number_of_residue_layers)
		{
			return (number_of_residue_layers*2-2);
		}

		static std::map<CRAD, ContactEffectGrouping> construct_map_of_contact_effect_groupings(const std::set<CRADsPair>& raw_all_crads_pairs, const std::size_t number_of_residue_layers)
		{
			if(number_of_residue_layers<2)
			{
				return construct_map_of_contact_effect_groupings(raw_all_crads_pairs, 2);
			}

			std::map<CRAD, ContactEffectGrouping> map_of_contact_effect_groupings;
			std::set<CRADsPair> all_crads_pairs;

			for(std::set<CRADsPair>::const_iterator it=raw_all_crads_pairs.begin();it!=raw_all_crads_pairs.end();++it)
			{
				const CRADsPair crads_pair=it->without_some_info(true, true, false, false);
				if(crads_pair.a!=crads_pair.b && crads_pair.a!=CRAD::solvent() && crads_pair.b!=CRAD::solvent() && CRAD::match_with_sequence_separation_interval(crads_pair.a, crads_pair.b, 1, CRAD::null_num(), true))
				{
					all_crads_pairs.insert(crads_pair);
					map_of_contact_effect_groupings[crads_pair.a].init(number_of_residue_layers, crads_pair.a, crads_pair.b);
					map_of_contact_effect_groupings[crads_pair.b].init(number_of_residue_layers, crads_pair.b, crads_pair.a);
				}
			}

			for(std::size_t top_layer_id=2;top_layer_id<number_of_residue_layers;top_layer_id++)
			{
				const std::size_t prev_layer_id=(top_layer_id-1);
				for(std::map<CRAD, ContactEffectGrouping>::iterator ceg_it=map_of_contact_effect_groupings.begin();ceg_it!=map_of_contact_effect_groupings.end();++ceg_it)
				{
					ContactEffectGrouping& ceg=ceg_it->second;
					for(std::set<CRAD>::const_iterator it=ceg.related_crads[prev_layer_id].begin();it!=ceg.related_crads[prev_layer_id].end();++it)
					{
						const std::set<CRAD>& candidate_crads=map_of_contact_effect_groupings[*it].related_crads[1];
						for(std::set<CRAD>::const_iterator jt=candidate_crads.begin();jt!=candidate_crads.end();++jt)
						{
							ceg.update_related_crads(top_layer_id, *jt);
						}
					}
				}
			}

			for(std::map<CRAD, ContactEffectGrouping>::iterator ceg_it=map_of_contact_effect_groupings.begin();ceg_it!=map_of_contact_effect_groupings.end();++ceg_it)
			{
				ContactEffectGrouping& ceg=ceg_it->second;
				for(std::size_t layer_id=1;layer_id<number_of_residue_layers;layer_id++)
				{
					for(std::set<CRAD>::const_iterator it=ceg.related_crads[layer_id].begin();it!=ceg.related_crads[layer_id].end();++it)
					{
						if(layer_id>1)
						{
							for(std::set<CRAD>::const_iterator jt=ceg.related_crads[layer_id-1].begin();jt!=ceg.related_crads[layer_id-1].end();++jt)
							{
								ceg.update_related_crads_pairs(all_crads_pairs, (layer_id*2-2), CRADsPair(*it, *jt));
							}
						}

						{
							std::set<CRAD>::const_iterator jt=it;
							++jt;
							for(;jt!=ceg.related_crads[layer_id].end();++jt)
							{
								ceg.update_related_crads_pairs(all_crads_pairs, (layer_id*2-1), CRADsPair(*it, *jt));
							}
						}
					}
				}
			}

			return map_of_contact_effect_groupings;
		}

	private:
		void init(const std::size_t number_of_layers_of_crads, const CRAD& central_crad, const CRAD& related_crad)
		{
			if(number_of_layers_of_crads<2)
			{
				return;
			}
			if(central_crad==related_crad)
			{
				return;
			}
			if(related_crads.size()!=number_of_layers_of_crads)
			{
				related_crads.resize(number_of_layers_of_crads);
			}
			if(related_crads_pairs.size()!=calc_number_of_inter_residue_layers(number_of_layers_of_crads))
			{
				related_crads_pairs.resize(calc_number_of_inter_residue_layers(number_of_layers_of_crads));
			}
			related_crads[0].insert(central_crad);
			collapsed_related_crads.insert(central_crad);
			related_crads[1].insert(related_crad);
			collapsed_related_crads.insert(related_crad);
			const CRADsPair related_crads_pair(central_crad, related_crad);
			related_crads_pairs[0].insert(related_crads_pair);
			collapsed_related_crads_pairs.insert(related_crads_pair);
		}

		void update_related_crads(const std::size_t layer_id, const CRAD& related_crad)
		{
			if(layer_id<related_crads.size() && collapsed_related_crads.count(related_crad)<1)
			{
				related_crads[layer_id].insert(related_crad);
				collapsed_related_crads.insert(related_crad);
			}
		}

		void update_related_crads_pairs(const std::set<CRADsPair>& valid_crads_pairs, const std::size_t layer_id, const CRADsPair& related_crads_pair)
		{
			if(layer_id<related_crads_pairs.size() && collapsed_related_crads_pairs.count(related_crads_pair)<1 && valid_crads_pairs.count(related_crads_pair)>0)
			{
				related_crads_pairs[layer_id].insert(related_crads_pair);
				collapsed_related_crads_pairs.insert(related_crads_pair);
			}
		}
	};

	struct BundleOfContactEffectGroupings
	{
		std::size_t number_of_residue_layers;
		std::size_t number_of_inter_residue_layers;
		std::map< CRAD, std::set<std::size_t> > map_of_atom_ids;
		std::map< CRAD, std::set<std::size_t> > map_of_solvent_contact_ids;
		std::map< CRADsPair, std::set<std::size_t> > map_of_inter_atom_contact_ids;
		std::map< CRAD, ContactEffectGrouping > map_of_contact_effect_groupings;
	};

	static BundleOfContactEffectGroupings construct_bundle_of_contact_effect_groupings(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts, const std::size_t number_of_residue_layers)
	{
		if(number_of_residue_layers<2)
		{
			return construct_bundle_of_contact_effect_groupings(atoms, contacts, 2);
		}

		BundleOfContactEffectGroupings bundle;

		bundle.number_of_residue_layers=number_of_residue_layers;
		bundle.number_of_inter_residue_layers=ContactEffectGrouping::calc_number_of_inter_residue_layers(bundle.number_of_residue_layers);

		for(std::size_t i=0;i<atoms.size();i++)
		{
			const CRAD crad=atoms[i].crad.without_some_info(true, true, false, false);
			bundle.map_of_atom_ids[crad].insert(i);
		}

		std::set<CRADsPair> all_crads_pairs;

		for(std::size_t i=0;i<contacts.size();i++)
		{
			const Contact& contact=contacts[i];
			if(contact.solvent())
			{
				const CRAD crad=atoms[contact.ids[0]].crad.without_some_info(true, true, false, false);
				bundle.map_of_solvent_contact_ids[crad].insert(i);
			}
			else
			{
				const CRADsPair crads_pair=ConversionOfDescriptors::get_contact_descriptor(atoms, contact).without_some_info(true, true, false, false);
				if(crads_pair.a!=crads_pair.b && CRAD::match_with_sequence_separation_interval(crads_pair.a, crads_pair.b, 1, CRAD::null_num(), true))
				{
					bundle.map_of_inter_atom_contact_ids[crads_pair].insert(i);
					all_crads_pairs.insert(crads_pair);
				}
			}
		}

		bundle.map_of_contact_effect_groupings=ContactEffectGrouping::construct_map_of_contact_effect_groupings(all_crads_pairs, bundle.number_of_residue_layers);

		return bundle;
	}

	class ContactEffectGroupingEnergyProfile
	{
	public:
		struct ResidueAttributes
		{
			int atom_count_sum;
			double atom_volume_sum;
			double atom_quality_score_sum;
			double solvent_contact_area_sum;
			double solvent_contact_energy_sum;
		};

		struct InterResidueAttributes
		{
			std::vector<double> inter_atom_contact_area_split_sum;
			std::vector<double> inter_atom_contact_energy_split_sum;

			InterResidueAttributes() :
				inter_atom_contact_area_split_sum(number_of_seq_sep_groups(), 0.0),
				inter_atom_contact_energy_split_sum(number_of_seq_sep_groups(), 0.0)
			{
			}

			std::size_t length() const
			{
				return std::min(inter_atom_contact_area_split_sum.size(), inter_atom_contact_energy_split_sum.size());
			}
		};

		std::vector<ResidueAttributes> layered_residue_attributes;
		std::vector<InterResidueAttributes> layered_inter_residue_attributes;
		std::vector<InterResidueAttributes> layered_inter_residue_attributes_with_solvent;
		bool propagated;

		ContactEffectGroupingEnergyProfile() : propagated(false)
		{
		}

		static std::map<CRAD, ContactEffectGroupingEnergyProfile> construct_map_of_contact_effect_grouping_energy_profiles(
				const std::vector<Atom>& atoms,
				const std::vector<Contact>& contacts,
				const std::size_t number_of_residue_layers,
				const bool propagate,
				const std::string& adjunct_atom_volumes,
				const std::string& adjunct_atom_quality_scores,
				const std::string& adjunct_inter_atom_energy_scores_raw)
		{
			std::map<CRAD, ContactEffectGroupingEnergyProfile> map_of_contact_effect_grouping_energy_profiles;

			BundleOfContactEffectGroupings bundle_of_cegs=construct_bundle_of_contact_effect_groupings(atoms, contacts, number_of_residue_layers);

			for(std::map<CRAD, ContactEffectGrouping>::const_iterator residue_it=bundle_of_cegs.map_of_contact_effect_groupings.begin();residue_it!=bundle_of_cegs.map_of_contact_effect_groupings.end();++residue_it)
			{
				const CRAD& central_crad=residue_it->first;
				const ContactEffectGrouping& ceg=residue_it->second;

				ContactEffectGroupingEnergyProfile cegep;
				cegep.layered_residue_attributes.resize(ceg.related_crads.size());
				cegep.layered_inter_residue_attributes.resize(ceg.related_crads_pairs.size());

				for(std::size_t layer_id=0;layer_id<ceg.related_crads.size();layer_id++)
				{
					ResidueAttributes& ras=cegep.layered_residue_attributes[layer_id];
					for(std::set<CRAD>::const_iterator related_crads_it=ceg.related_crads[layer_id].begin();related_crads_it!=ceg.related_crads[layer_id].end();++related_crads_it)
					{
						{
							const std::set<std::size_t>& atom_ids=bundle_of_cegs.map_of_atom_ids[*related_crads_it];
							for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
							{
								const Atom& atom=atoms[*it];
								ras.atom_count_sum++;
								ras.atom_volume_sum+=get_map_value_safely(atom.value.props.adjuncts, adjunct_atom_volumes, 0.0);
								ras.atom_quality_score_sum+=get_map_value_safely(atom.value.props.adjuncts, adjunct_atom_quality_scores, 0.0);
							}
						}
						{
							const std::set<std::size_t>& solvent_contact_ids=bundle_of_cegs.map_of_solvent_contact_ids[*related_crads_it];
							for(std::set<std::size_t>::const_iterator it=solvent_contact_ids.begin();it!=solvent_contact_ids.end();++it)
							{
								const Contact& contact=contacts[*it];
								ras.solvent_contact_area_sum+=contact.value.area;
								ras.solvent_contact_energy_sum+=get_map_value_safely(contact.value.props.adjuncts, adjunct_inter_atom_energy_scores_raw, 0.0);
							}
						}
					}
				}

				for(std::size_t layer_id=0;layer_id<ceg.related_crads_pairs.size();layer_id++)
				{
					InterResidueAttributes& iras=cegep.layered_inter_residue_attributes[layer_id];
					for(std::set<CRADsPair>::const_iterator related_crads_pairs_it=ceg.related_crads_pairs[layer_id].begin();related_crads_pairs_it!=ceg.related_crads_pairs[layer_id].end();++related_crads_pairs_it)
					{
						const int seq_sep_group=calc_seq_sep_group(*related_crads_pairs_it);
						if(seq_sep_group>=0 && seq_sep_group<static_cast<int>(iras.inter_atom_contact_area_split_sum.size()))
						{
							const std::set<std::size_t>& contact_ids=bundle_of_cegs.map_of_inter_atom_contact_ids[*related_crads_pairs_it];
							for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
							{
								const Contact& contact=contacts[*it];
								iras.inter_atom_contact_area_split_sum[seq_sep_group]+=contact.value.area;
								iras.inter_atom_contact_energy_split_sum[seq_sep_group]+=get_map_value_safely(contact.value.props.adjuncts, adjunct_inter_atom_energy_scores_raw, 0.0);
							}
						}
					}
				}

				if(propagate)
				{
					cegep.propagate();
				}

				map_of_contact_effect_grouping_energy_profiles[central_crad]=cegep;
			}

			return map_of_contact_effect_grouping_energy_profiles;
		}

		static void write_map_of_contact_effect_grouping_energy_profiles(const std::map<CRAD, ContactEffectGroupingEnergyProfile>& map_of_residue_energy_profiles, std::ostream& output)
		{
			std::string header;
			std::ostringstream content_output;

			for(std::map<CRAD, ContactEffectGroupingEnergyProfile>::const_iterator it=map_of_residue_energy_profiles.begin();it!=map_of_residue_energy_profiles.end();++it)
			{
				std::ostringstream header_output;

				{
					const CRAD& crad=it->first;

					header_output << "ID";
					content_output << crad;

					const int res_name_number=convert_residue_name_to_number(crad.resName);

					header_output << " category";
					content_output << " " << res_name_number;

					for(int i=0;i<20;i++)
					{
						header_output << " nf" << i;
						content_output << " " << (i==res_name_number ? 1 : 0);
					}
				}

				const ContactEffectGroupingEnergyProfile& cegep=it->second;

				for(std::size_t i=0;i<cegep.layered_residue_attributes.size();i++)
				{
					header_output << " rl" << i << "_ac";
					content_output << " " << cegep.layered_residue_attributes[i].atom_count_sum;

					header_output << " rl" << i << "_av";
					content_output << " " << cegep.layered_residue_attributes[i].atom_volume_sum;

					header_output << " rl" << i << "_aqs";
					content_output << " " << cegep.layered_residue_attributes[i].atom_quality_score_sum;

					header_output << " rl" << i << "_sca";
					content_output << " " << cegep.layered_residue_attributes[i].solvent_contact_area_sum;

					header_output << " rl" << i << "_sce";
					content_output << " " << cegep.layered_residue_attributes[i].solvent_contact_energy_sum;

					header_output << " rl" << i << "_aqs_n";
					content_output << " " << safe_ratio(cegep.layered_residue_attributes[i].atom_quality_score_sum, cegep.layered_residue_attributes[i].atom_count_sum, 0.0);

					header_output << " rl" << i << "_sce_n";
					content_output << " " << safe_ratio(cegep.layered_residue_attributes[i].solvent_contact_energy_sum, cegep.layered_residue_attributes[i].solvent_contact_area_sum, 0.0);
				}

				for(std::size_t i=0;i<cegep.layered_inter_residue_attributes.size();i++)
				{
					for(std::size_t j=0;j<cegep.layered_inter_residue_attributes[i].length();j++)
					{
						header_output << " irl" << i << "_ss" << j << "_iaca";
						content_output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum[j];

						header_output << " irl" << i << "_ss" << j << "_iace";
						content_output << " " << cegep.layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum[j];

						header_output << " irl" << i << "_ss" << j << "_iace_n";
						content_output << " " << safe_ratio(cegep.layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum[j], cegep.layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum[j], 0.0);
					}
				}

				for(std::size_t i=0;i<cegep.layered_inter_residue_attributes_with_solvent.size();i++)
				{
					for(std::size_t j=0;j<cegep.layered_inter_residue_attributes_with_solvent[i].length();j++)
					{
						header_output << " irlws" << i << "_ss" << j << "_iaca";
						content_output << " " << cegep.layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_area_split_sum[j];

						header_output << " irlws" << i << "_ss" << j << "_iace";
						content_output << " " << cegep.layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_energy_split_sum[j];

						header_output << " irlws" << i << "_ss" << j << "_iace_n";
						content_output << " " << safe_ratio(cegep.layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_energy_split_sum[j], cegep.layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_area_split_sum[j], 0.0);
					}
				}

				content_output << "\n";

				if(header.empty())
				{
					header=header_output.str();
				}
				else
				{
					if(header!=header_output.str())
					{
						throw std::runtime_error(std::string("Inconsistent table"));
					}
				}
			}

			output << header << "\n";
			output << content_output.str();
		}

	private:
		static double get_map_value_safely(const std::map<std::string, double>& map, const std::string& key, const double default_value)
		{
			std::map<std::string, double>::const_iterator it=map.find(key);
			if(it!=map.end())
			{
				return it->second;
			}
			return default_value;
		}

		static int number_of_seq_sep_groups()
		{
			return 7;
		}

		static int calc_seq_sep_group(const CRADsPair& crads_pair)
		{
			for(int seq_sep_group=(number_of_seq_sep_groups()-1);seq_sep_group>=0;seq_sep_group--)
			{
				int seq_sep=(1+seq_sep_group);
				if(CRAD::match_with_sequence_separation_interval(crads_pair.a, crads_pair.b, seq_sep, CRAD::null_num(), true))
				{
					return seq_sep_group;
				}
			}
			return -1;
		}

		static double safe_ratio(const double a, const double b, const double nan_replacement)
		{
			return (b>0.0 ? (a/b) : nan_replacement);
		}

		static int convert_residue_name_to_number(const std::string& name)
		{
			static const std::map<std::string, int> m=create_map_of_residue_names_to_numbers();
			const std::map<std::string, int>::const_iterator it=m.find(name);
			return (it==m.end() ? -1 : it->second);
		}

		static std::map<std::string, int> create_map_of_residue_names_to_numbers()
		{
			std::map<std::string, int> m;

			m["LEU"]=-1;
			m["VAL"]=-1;
			m["ILE"]=-1;
			m["ALA"]=-1;
			m["PHE"]=-1;
			m["TRP"]=-1;
			m["MET"]=-1;
			m["PRO"]=-1;
			m["ASP"]=-1;
			m["GLU"]=-1;
			m["LYS"]=-1;
			m["ARG"]=-1;
			m["HIS"]=-1;
			m["CYS"]=-1;
			m["SER"]=-1;
			m["THR"]=-1;
			m["TYR"]=-1;
			m["ASN"]=-1;
			m["GLN"]=-1;
			m["GLY"]=-1;

			int id=0;
			for(std::map<std::string, int>::iterator it=m.begin();it!=m.end();++it)
			{
				it->second=id;
				id++;
			}

			return m;
		}

		void propagate()
		{
			if(propagated)
			{
				return;
			}

			for(std::size_t i=1;i<layered_residue_attributes.size();i++)
			{
				const ResidueAttributes& a=layered_residue_attributes[i-1];
				ResidueAttributes& b=layered_residue_attributes[i];
				b.atom_count_sum+=a.atom_count_sum;
				b.atom_volume_sum+=a.atom_volume_sum;
				b.atom_quality_score_sum+=a.atom_quality_score_sum;
				b.solvent_contact_area_sum+=a.solvent_contact_area_sum;
				b.solvent_contact_energy_sum+=a.solvent_contact_energy_sum;
			}

			for(std::size_t i=0;i<layered_inter_residue_attributes.size();i++)
			{
				const int n=static_cast<int>(layered_inter_residue_attributes[i].length());
				for(int j=(n-2);j>=0;j--)
				{
					layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum[j]+=layered_inter_residue_attributes[i].inter_atom_contact_area_split_sum[j+1];
					layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum[j]+=layered_inter_residue_attributes[i].inter_atom_contact_energy_split_sum[j+1];
				}
			}

			for(std::size_t i=1;i<layered_inter_residue_attributes.size();i++)
			{
				const InterResidueAttributes& a=layered_inter_residue_attributes[i-1];
				InterResidueAttributes& b=layered_inter_residue_attributes[i];
				for(std::size_t j=0;j<b.length();j++)
				{
					b.inter_atom_contact_area_split_sum[j]+=a.inter_atom_contact_area_split_sum[j];
					b.inter_atom_contact_energy_split_sum[j]+=a.inter_atom_contact_energy_split_sum[j];
				}
			}

			layered_inter_residue_attributes_with_solvent=layered_inter_residue_attributes;

			for(std::size_t i=0;i<layered_inter_residue_attributes_with_solvent.size();i++)
			{
				const std::size_t residue_layer_id=(i/2);
				if(residue_layer_id<layered_inter_residue_attributes_with_solvent.size())
				{
					for(std::size_t j=0;j<layered_inter_residue_attributes_with_solvent[i].length();j++)
					{
						layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_area_split_sum[j]+=layered_residue_attributes[residue_layer_id].solvent_contact_area_sum;
						layered_inter_residue_attributes_with_solvent[i].inter_atom_contact_energy_split_sum[j]+=layered_residue_attributes[residue_layer_id].solvent_contact_energy_sum;
					}
				}
			}

			propagated=true;
		}
	};
};

}

}

#endif /* COMMON_CONSTRUCTION_OF_CONTACT_EFFECT_GROUPINGS_H_ */
