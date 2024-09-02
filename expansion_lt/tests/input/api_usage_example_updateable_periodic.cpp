#include <iostream>

#include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path

//user-defined function to print input spheres
void print_spheres(const std::vector<voronotalt::SimpleSphere>& spheres)
{
	std::cout << "spheres (sphere id x y z r):\n";
	for(std::size_t i=0;i<spheres.size();i++)
	{
		const voronotalt::SimpleSphere& sphere=spheres[i];
		std::cout << "sphere "<< i << " " << sphere.p.x << " " << sphere.p.y << " " << sphere.p.z << " " << sphere.r << "\n";
	}
	std::cout << "\n";
}

//user-defined function to print tessellation result contacts and cells
void print_tessellation_result_contacts_and_cells(const voronotalt::UpdateableRadicalTessellation::Result& result)
{
	std::cout << "contacts (contact id_a id_b area arc_length):\n";
	for(std::size_t i=0;i<result.contacts_summaries.size();i++)
	{
		for(std::size_t j=0;j<result.contacts_summaries[i].size();j++)
		{
			const voronotalt::RadicalTessellation::ContactDescriptorSummary& contact=result.contacts_summaries[i][j];
			if(contact.id_a==i)
			{
				std::cout << "contact " << contact.id_a << " " << contact.id_b << " " << contact.area << " " << contact.arc_length << "\n";
			}
		}
	}
	std::cout << "\n";

	std::cout << "cells (cell id area volume):\n";
	for(std::size_t i=0;i<result.cells_summaries.size();i++)
	{
		const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& cell=result.cells_summaries[i];
		std::cout << "cell " << cell.id << " " << cell.sas_area << " " << cell.sas_inside_volume << "\n";
	}
	std::cout << "\n";
}

//user-defined function to print tessellation result summary
void print_tessellation_result_summary(const voronotalt::UpdateableRadicalTessellation::ResultSummary& result_summary)
{
	std::cout << "result_summary (summary contacts_area contacts_count cells_sas_area cells_volume):\n";
	std::cout << "summary " << result_summary.total_contacts_summary.area << " " << result_summary.total_contacts_summary.count << " ";
	std::cout << result_summary.total_cells_summary.sas_area << " " << result_summary.total_cells_summary.sas_inside_volume << "\n";
	std::cout << "\n";
}

int main(const int, const char**)
{
	//Input raw balls

	std::vector<voronotalt::SimpleSphere> input_spheres;

	input_spheres.push_back(voronotalt::SimpleSphere(0, 0, 2, 1));
	input_spheres.push_back(voronotalt::SimpleSphere(0, 1, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.382683, 0.92388, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.707107, 0.707107, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.92388, 0.382683, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(1, 0, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.92388, -0.382683, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.707107, -0.707107, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0.382683, -0.92388, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(0, -1, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.382683, -0.92388, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.707107, -0.707107, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.92388, -0.382683, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-1, 0, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.92388, 0.382683, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.707107, 0.707107, 0, 0.5));
	input_spheres.push_back(voronotalt::SimpleSphere(-0.382683, 0.92388, 0, 0.5));

	//Prepare input spheres by augmenting the radii of the raw balls

	const double probe=1.0;

	for(std::size_t i=0;i<input_spheres.size();i++)
	{
		input_spheres[i].r+=probe;
	}

	//Print prepared input spheres

	std::cout << "Input:\n\n";

	print_spheres(input_spheres);

	//Initialize a periodic box description

	std::vector<voronotalt::SimplePoint> periodic_box_corners;
	periodic_box_corners.push_back(voronotalt::SimplePoint(-1.6, -1.6, -0.6));
	periodic_box_corners.push_back(voronotalt::SimplePoint(1.6, 1.6, 3.1));

	//Initialize an updateable tessellation controller object with automatic backup enabled

	const bool backup_enabled=true;
	voronotalt::UpdateableRadicalTessellation updateable_tessellation(backup_enabled);

	//Compute a tessellation from the input spheres

	if(updateable_tessellation.init(input_spheres, voronotalt::PeriodicBox::create_periodic_box_from_corners(periodic_box_corners)))
	{
		std::cout << "Initialized tessellation." << std::endl;
	}
	else
	{
		std::cerr << "Failed to construct tessellation." << std::endl;
		return 1;
	}

	//Save the tessellation result summary after init

	std::vector<voronotalt::UpdateableRadicalTessellation::ResultSummary> result_summaries;

	result_summaries.push_back(updateable_tessellation.result_summary());

	//Print the tessellation results

	std::cout << "\nResults after init:\n\n";

	print_tessellation_result_contacts_and_cells(updateable_tessellation.result());

	//Iteratively change the input spheres and update the tessellation

	for(int n=1;n<=5;n++)
	{
		//Specify the updated indices of spheres

		std::vector<voronotalt::UnsignedInt> ids_to_update;
		ids_to_update.push_back(0);
		ids_to_update.push_back(1);

		//Update the coordinated of the chosen input spheres

		for(const voronotalt::UnsignedInt& id : ids_to_update)
		{
			input_spheres[id].p.x+=0.1;
		}

		//Update the tessellation

		if(updateable_tessellation.update(input_spheres, ids_to_update))
		{
			std::cout << "Updated tessellation." << std::endl;
		}
		else
		{
			std::cerr << "Failed to update tessellation." << std::endl;
			return 1;
		}

		//Save the tessellation result summary after update

		result_summaries.push_back(updateable_tessellation.result_summary());
	}

	//Print the tessellation results

	std::cout << "\nResults after last update:\n\n";

	print_tessellation_result_contacts_and_cells(updateable_tessellation.result());

	//Print all the save tessellation result summaries

	std::cout << "\nResult summaries for all stages:\n\n";

	for(std::size_t i=0;i<result_summaries.size();i++)
	{
		const voronotalt::UpdateableRadicalTessellation::ResultSummary& rs=result_summaries[i];
		print_tessellation_result_summary(rs);
	}

	//Restore the tessellation from the last backup, i.e. cancel the last update

	if(updateable_tessellation.restore_from_backup())
	{
		//Print the tessellation result summary after restoring the tessellation

		std::cout << "\nResult summary after restoring from backup:\n\n";
		print_tessellation_result_summary(updateable_tessellation.result_summary());
	}
	else
	{
		std::cerr << "Results were not restored from backup because ";
		if(updateable_tessellation.in_sync_with_backup())
		{
			std::cerr << "results are already in sync with backup";
		}
		else
		{
			std::cerr << "backup was not enabled";
		}
		std::cerr << std::endl;
	}

	return 0;
}

