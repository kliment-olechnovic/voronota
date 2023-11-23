import voronotalt_python as voronotalt

balls = [voronotalt.Ball(1.0, 2.0, 3.0, 1.5), voronotalt.Ball(4.0, 5.0, 6.0, 2.0)]
rt = voronotalt.RadicalTessellation(balls, 2.5)

print(type(rt.contacts));
print(rt.contacts.size());

print(type(rt.contacts[0]));
print(rt.contacts[0].area);
print(rt.contacts[0].arc_length);

print(type(rt.cells));
print(rt.cells.size());

print(type(rt.cells[0]));
print(rt.cells[0].sas_area);
print(rt.cells[0].volume);

