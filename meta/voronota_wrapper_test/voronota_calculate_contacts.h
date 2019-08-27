#include "voronota_1.19.2352/src/common/construction_of_contacts.h"

namespace voronota
{

struct Contact
{
	std::size_t ids[2];
	double area;

	Contact() : area(0)
	{
		ids[0]=0;
		ids[1]=0;
	}

	Contact(const std::size_t id_a, const std::size_t id_b, const double area) : area(area)
	{
		ids[0]=id_a;
		ids[1]=id_b;
	}

	Contact(const std::size_t id, const double area) : area(area)
	{
		ids[0]=id;
		ids[1]=id;
	}

	bool solvent() const
	{
		return (ids[0]==ids[1]);
	}
};

template<typename Points, typename Radii>
inline std::vector<Contact> calculate_contacts(
		const Points& ball_centers,
		const Radii& ball_radii,
		const double probe)
{
	common::ConstructionOfContacts::ParametersToConstructBundleOfContactInformation parameters_to_construct_contacts;
	parameters_to_construct_contacts.probe=probe;

	if(ball_centers.size()!=ball_radii.size())
	{
		throw std::runtime_error(std::string("Centers and radii vectors are not equally sized."));
	}

	if(ball_centers.size()<4)
	{
		throw std::runtime_error(std::string("Number of balls is smaller than 4."));
	}

	std::vector<apollota::SimpleSphere> balls;
	balls.reserve(ball_centers.size());
	for(std::size_t i=0;i<ball_centers.size();i++)
	{
		balls.push_back(apollota::SimpleSphere(ball_centers[i][0], ball_centers[i][1], ball_centers[i][2], ball_radii[i]));
	}

	common::ConstructionOfTriangulation::BundleOfTriangulationInformation bundle_of_triangulation_information;
	common::ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

	if(!common::ConstructionOfContacts::construct_bundle_of_contact_information(
			parameters_to_construct_contacts,
			balls,
			bundle_of_triangulation_information,
			bundle_of_contact_information))
	{
		throw std::runtime_error(std::string("Failed to construct contacts."));
	}

	std::vector<Contact> contacts;
	contacts.reserve(bundle_of_contact_information.contacts.size());
	for(std::size_t i=0;i<bundle_of_contact_information.contacts.size();i++)
	{
		const common::ConstructionOfContacts::Contact& c=bundle_of_contact_information.contacts[i];
		contacts.push_back(Contact(c.ids[0], c.ids[1], c.value.area));
	}

	return contacts;
}

template<typename Points, typename Radii>
inline bool calculate_contacts(
		const Points& ball_centers,
		const Radii& ball_radii,
		const double probe,
		std::vector<Contact>& contacts,
		std::string& error_message)
{
	contacts.clear();
	try
	{
		contacts=calculate_contacts(ball_centers, ball_radii, probe);
		return true;
	}
	catch(const std::exception& e)
	{
		error_message=e.what();
		return false;
	}
	return false;
}

}
