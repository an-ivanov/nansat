#!/usr/bin/env python
# Name:    pointbrowser_test.py
# Purpose: Tutorial with list of examples
# Authors:      Asuka Yamakawa, Anton Korosov, Knut-Frode Dagestad,
#               Morten W. Hansen, Alexander Myasoyedov,
#               Dmitry Petrenko, Evgeny Morozov
# Created:      29.06.2011
# Copyright:    (c) NERSC 2011 - 2013
# Licence:
# This file is part of NANSAT.
# NANSAT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
# http://www.gnu.org/licenses/gpl-3.0.html
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

import numpy as np
import inspect, os

from nansat import Nansat
from nansat_tools import PointBrowser

''' Pointbrowser class fetch the coordinates of cliked points on the image

    PointBrowser object is created by a numpy array.
    When get_points() is called, an image is shown automatically.
    Then click some points on the image.
    Fetch the coodrdinates of the clicked points.

'''
# Get input and output filenames
from testio import testio
iPath, iFileName, oPath, oFileName, shpFileName = testio()

# Create a Nansat object (n)
n = Nansat(iFileName)
# get numpy array from the Nansat object
array = n[1]

# Create browser object
browser = PointBrowser(array)
# Choose points by clicking the fig
browser.get_points()
# Get coordinates of the clicked points
points = browser.coordinates
# Print coordinates
print '1: Coordinates of Clicked Points ---'
for iPoint in points:
    print iPoint
print ''


# Create browser object setting vmin and vmax of the image
browser = PointBrowser(array, vmin=10.0, vmax=50.0)
# Choose points by clicking the fig
browser.get_points()
# Get coordinates of the clicked points
points = browser.coordinates
# Print coordinates
print '2: Coordinates of Clicked Points ---'
for iPoint in points:
    print iPoint




