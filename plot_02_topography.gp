# Universal Gnuplot script for a single topography
# Call via:
# gnuplot -c plot_02_topography.gp <column> "<rule>" "<description>"

datafile = "c1_c2_topography_fuzzy.dat"
col_num = int(ARG1)
rule_name = ARG2
rule_desc = ARG3

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

# ------------------------------------------------------------
# General plot style
# ------------------------------------------------------------
set title sprintf("Topography Fuzzy Logic - %s (%s)", rule_name, rule_desc)

set xlabel "Lambda"
set ylabel "tbar"
set zlabel "f"

set xyplane 0

set grid
set border lw 1.0
set tics out

set dgrid3d 41,41
set pm3d at s
set mapping cartesian

set view 55, 35
unset key

# Objective values are in [0,1].
# Keeping the same color range for all rule plots improves comparability.
set cbrange [0.1:0.7]
set cblabel "f"

# ------------------------------------------------------------
# Elegant viridis-like color palette
# ------------------------------------------------------------
set palette defined ( \
0.00 "#F7FBFF", \
0.20 "#DEEBF7", \
0.40 "#9ECAE1", \
0.60 "#4292C6", \
0.80 "#08519C", \
1.00 "#08306B" )


# ------------------------------------------------------------
# Interactive view
# ------------------------------------------------------------
splot datafile using 1:2:col_num with pm3d

print sprintf("--- %s is shown ---", rule_name)
print "Press [ENTER] inside console to save picture as PNG and close program ..."
pause -1

# ------------------------------------------------------------
# PNG export
# ------------------------------------------------------------
set terminal pngcairo size 1200,900 enhanced font "Verdana,10"
set output sprintf("c1_c2_topography_fuzzy_rule%d.png", col_num-4)

replot

set output
