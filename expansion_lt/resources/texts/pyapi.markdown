# Using Voronota-LT Python bindings

## Compiling Python bindings

Python bindings of Voronota-LT can be built using SWIG, in the "expansion_lt/swig" directory:

    swig -python -c++ voronotalt_python.i
    
    g++ -fPIC -shared -O3 -fopenmp voronotalt_python_wrap.cxx -o _voronotalt_python.so $(python3-config --includes)

This produces "_voronotalt_python.so" and "voronotalt_python.py" that are needed to call Voronota-LT from Python code.

## Using Python bindings

When "_voronotalt_python.so" and "voronotalt_python.py" are generated, the "voronotalt_python" module can be made findable by python by adding its dyrectory to the PYTHONPATH environmental variable:

    export PYTHONPATH="${PYTHONPATH}:/path/to/voronota/expansion_lt/swig"

Then Voronota-LT can be used in Python code as in the following example:

    import voronotalt_python as voronotalt
    
    balls = []
    balls.append(voronotalt.Ball(0, 0, 2, 1))
    balls.append(voronotalt.Ball(0, 1, 0, 0.5))
    balls.append(voronotalt.Ball(0.38268343236509, 0.923879532511287, 0, 0.5))
    balls.append(voronotalt.Ball(0.707106781186547, 0.707106781186548, 0, 0.5))
    balls.append(voronotalt.Ball(0.923879532511287, 0.38268343236509, 0, 0.5))
    balls.append(voronotalt.Ball(1, 0, 0, 0.5))
    balls.append(voronotalt.Ball(0.923879532511287, -0.38268343236509, 0, 0.5))
    balls.append(voronotalt.Ball(0.707106781186548, -0.707106781186547, 0, 0.5))
    balls.append(voronotalt.Ball(0.38268343236509, -0.923879532511287, 0, 0.5))
    balls.append(voronotalt.Ball(0, -1, 0, 0.5))
    balls.append(voronotalt.Ball(-0.38268343236509, -0.923879532511287, 0, 0.5))
    balls.append(voronotalt.Ball(-0.707106781186547, -0.707106781186548, 0, 0.5))
    balls.append(voronotalt.Ball(-0.923879532511287, -0.38268343236509, 0, 0.5))
    balls.append(voronotalt.Ball(-1, 0, 0, 0.5))
    balls.append(voronotalt.Ball(-0.923879532511287, 0.38268343236509, 0, 0.5))
    balls.append(voronotalt.Ball(-0.707106781186548, 0.707106781186547, 0, 0.5))
    balls.append(voronotalt.Ball(-0.38268343236509, 0.923879532511287, 0, 0.5))
    
    rt = voronotalt.RadicalTessellation(balls, probe=1.0)
    
    contacts=list(rt.contacts)
    
    print("contacts:")
    
    for contact in contacts:
        print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length);
    
    cells=list(rt.cells)
    
    print("cells:")
    
    for i, cell in enumerate(cells):
        print("cell", i, cell.sas_area, cell.volume);
    
