#
# Copyright (C) 2026 Olaf Frommann
# * This file is part of the FuzzyGoal toolkit.
#
# FuzzyGoal is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# FuzzyGoal is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

# Gnuplot script for a simple test case
set title "Basic Two Criteria Fuzzy Example"
set xlabel "x"
set ylabel "Membership / Objective Value"
set grid

# Interactive view (showed using default wx, qt oder x11 window)
plot "basic_fuzzy_example.dat" using 1:2 title "c1" with lines lw 2, \
"basic_fuzzy_example.dat" using 1:3 title "c2" with lines lw 2, \
"basic_fuzzy_example.dat" using 1:10 title "f_{base}" with lines lw 3 lc rgb "black", \
"basic_fuzzy_example.dat" using 1:11 title "f_{rule}" with lines lw 3 lc rgb "red"

print "Press [ENTER] inside console to save picture as PNG and close program ..."
pause -1

# PNG export after key press
set terminal pngcairo size 1200,900 enhanced font "Verdana,10"
set output "01_basic_fuzzy_example.png"
replot
set output
