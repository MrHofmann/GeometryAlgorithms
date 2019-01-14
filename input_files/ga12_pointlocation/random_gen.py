#!/usr/bin/env python

import sys
import scipy as sp
import numpy as np
from scipy.spatial import Delaunay

x_range = [30, 850]
y_range = [30, 450]

if len(sys.argv) < 3:
    sys.stderr.write("Usage: generator.py num_points output_filename")
n_points = int(sys.argv[1])
points = []

for i in xrange(n_points):
    x = np.random.randint(x_range[0], x_range[1])
    y = np.random.randint(y_range[0], y_range[1])
    points.append([x, y])

points = np.array(points)
tri = Delaunay(points)

file = open(sys.argv[2], "w")

for t in tri.simplices:
    file.write("%f %f %f %f\n" % (points[t[0]][0], points[t[0]][1], points[t[1]][0], points[t[1]][1]))
    file.write("%f %f %f %f\n" % (points[t[1]][0], points[t[1]][1], points[t[2]][0], points[t[2]][1]))
    file.write("%f %f %f %f\n" % (points[t[2]][0], points[t[2]][1], points[t[0]][0], points[t[0]][1]))

file.close()
    
