import voronotalt_python as voronotalt

balls = []
balls.append(voronotalt.Ball(0, 0, 2, 1))
balls.append(voronotalt.Ball(0, 1, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, 0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186547, 0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(1, 6.12323399573677e-17, 0, 0.5))
balls.append(voronotalt.Ball(0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(0.707106781186548, -0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(1.22464679914735e-16, -1, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, -0.923879532511287, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186547, -0.707106781186548, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, -0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-1, -1.83697019872103e-16, 0, 0.5))
balls.append(voronotalt.Ball(-0.923879532511287, 0.38268343236509, 0, 0.5))
balls.append(voronotalt.Ball(-0.707106781186548, 0.707106781186547, 0, 0.5))
balls.append(voronotalt.Ball(-0.38268343236509, 0.923879532511287, 0, 0.5))

rt = voronotalt.RadicalTessellation(balls, 1.4)

print(type(rt.contacts));
print(rt.contacts.size());

print(type(rt.contacts[0]));
print(rt.contacts[0].area, rt.contacts[0].arc_length);

print(type(rt.cells));
print(rt.cells.size());

print(type(rt.cells[0]));
print(rt.cells[0].sas_area, rt.cells[0].volume);

sorted_contacts=list(rt.contacts)
sorted_contacts.sort(key = lambda x: x.area, reverse=True)
print(type(sorted_contacts));
print(len(sorted_contacts));
print(sorted_contacts[0].area, sorted_contacts[0].arc_length);
