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

# Gnuplot script for equality constraints
set title "Equality Constraint Example"
set xlabel "x"
set ylabel "f"
set grid

plot "equality_constraint_example.dat" using 1:2 title "f_{equality}" with lines lw 2, \
"equality_constraint_example.dat" using 1:3 title "f_{equality-plus-inequality}" with lines lw 2

print "Press [ENTER] inside console to save picture as PNG and close program ..."
pause -1

# PNG-Export
set terminal pngcairo size 1200,900 enhanced
set output "03_equality_constraint_example.png"
replot
set output
