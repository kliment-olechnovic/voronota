# Using Voronota-LT as a C++ library

Voronota-LT can be used as a header-only C++ library.
The need headers are all in "./src/voronotalt" folder.
The only header file needed to be included is "voronotalt.h".

Below is a detailed example:

```cpp
    #include <stdexcept> // this example uses exceptions, but the Voronota-LT code does not
    
    #include "voronotalt.h" // assuming that the "voronotalt" directory is in the include path
    
    //user-defined structure for a ball
    struct Ball
    {
        Ball() : x(0.0), y(0.0), z(0.0), r(0.0) {}
    
        double x;
        double y;
        double z;
        double r;
    };
    
    //user-defined structure for a contact descriptor
    struct Contact
    {
        Contact() : index_a(0), index_b(0), area(0.0), arc_length(0.0) {}
    
        int index_a;
        int index_b;
        double area;
        double arc_length;
    };
    
    //user-defined structure for a cell descriptor
    struct Cell
    {
        Cell() : sas_area(0.0), volume(0.0), included(false) {}
    
        double sas_area;
        double volume;
        bool included;
    };
    
    //user-defined function that uses voronotalt::RadicalTessellation to fill vectors of contact and cell descriptors
    void compute_contact_and_cell_descriptors(
            const std::vector<Ball>& balls,
            const double probe,
            std::vector<Contact>& contacts,
            std::vector<Cell>& cells)
    {
        contacts.clear();
        cells.clear();
    
        if(balls.empty())
        {
            throw std::runtime_error("No balls to compute the tessellation for.");
        }
    
        // computing Voronota-LT radical tessellation results
        voronotalt::RadicalTessellation::Result result;
        voronotalt::RadicalTessellation::construct_full_tessellation(voronotalt::get_spheres_from_balls(balls, probe), result);
    
        if(result.contacts_summaries.empty())
        {
            throw std::runtime_error("No contacts constructed for the provided balls and probe.");
        }
    
        if(result.cells_summaries.empty())
        {
            throw std::runtime_error("No cells constructed for the provided balls and probe.");
        }
    
        // using the result data about contacts
        contacts.resize(result.contacts_summaries.size());
        for(std::size_t i=0;i<result.contacts_summaries.size();i++)
        {
            contacts[i].index_a=result.contacts_summaries[i].id_a;
            contacts[i].index_b=result.contacts_summaries[i].id_b;
            contacts[i].area=result.contacts_summaries[i].area;
            contacts[i].arc_length=result.contacts_summaries[i].arc_length;
        }
    
        // using the result data about cells
        cells.resize(balls.size());
        for(std::size_t i=0;i<result.cells_summaries.size();i++)
        {
            const std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
            cells[index].sas_area=result.cells_summaries[i].sas_area;
            cells[index].volume=result.cells_summaries[i].sas_inside_volume;
            cells[index].included=true;
        }
    }

```
