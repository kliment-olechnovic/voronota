import voronotalt

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

for i, ball in enumerate(balls):
    print("ball", i, ball.x, ball.y, ball.z, ball.r);

rt = voronotalt.RadicalTessellation(balls, probe=1.0)

contacts=list(rt.contacts)

print("contacts:")

for contact in contacts:
    print("contact", contact.index_a, contact.index_b, contact.area, contact.arc_length);

cells=list(rt.cells)

print("cells:")

for i, cell in enumerate(cells):
    print("cell", i, cell.sas_area, cell.volume);

