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

# =============================================================
# FuzzyGoal - Makefile with Gnuplot integration
# =============================================================

ifeq ($(origin CXX), default)
CXX := c++
endif

CXX_CMD := $(firstword $(CXX))

CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra -pedantic
CPPFLAGS ?= -I.
LDFLAGS  ?=

BUILD_DIR := build
BIN_DIR   := bin

LIB_SRC := fuzzygoal.cpp
LIB_HDR := fuzzygoal.h
LIB_OBJ := $(BUILD_DIR)/fuzzygoal.o

EXAMPLE_DIR  := examples
EXAMPLE_SRCS := $(wildcard $(EXAMPLE_DIR)/*.cpp)
EXAMPLE_NAMES := $(basename $(notdir $(EXAMPLE_SRCS)))

TEST_DIR := tests
TEST_SRC := $(TEST_DIR)/test_fuzzygoal.cpp
TEST_BIN := $(BIN_DIR)/test_fuzzygoal$(EXEEXT)

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
else
EXEEXT :=
endif

EXAMPLE_BINS := $(addprefix $(BIN_DIR)/,$(addsuffix $(EXEEXT),$(EXAMPLE_NAMES)))

.PHONY: all check-compiler dirs run test clean help run_basic run_topography run_equality

all: check-compiler dirs $(EXAMPLE_BINS)
	@echo
	@echo "Build finished."
	@echo "Executables are in: $(BIN_DIR)"
	@echo
	@echo "Run individual tests and plots with:"
	@echo "  make run_basic"
	@echo "  make run_topography"
	@echo "  make run_equality"
	@echo

check-compiler:
	@command -v "$(CXX_CMD)" >/dev/null 2>&1 || \
	{ \
		echo "Error: no C++ compiler found."; \
		echo; \
		echo "Tried compiler: $(CXX)"; \
		echo; \
		echo "Please install a C++ compiler and try again."; \
		echo "See documentation, section: Installing a compiler."; \
		echo; \
		exit 1; \
	}
	@echo "Using compiler: $$($(CXX) --version | head -n 1)"

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(LIB_OBJ): $(LIB_SRC) $(LIB_HDR) | dirs
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

$(BIN_DIR)/%$(EXEEXT): $(EXAMPLE_DIR)/%.cpp $(LIB_OBJ) $(LIB_HDR) | dirs
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< $(LIB_OBJ) -o $@ $(LDFLAGS)

$(TEST_BIN): $(TEST_SRC) $(LIB_OBJ) $(LIB_HDR) | dirs
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TEST_SRC) $(LIB_OBJ) -o $(TEST_BIN) $(LDFLAGS)

# -------------------------------------------------------------
# Spezifische Run & Plot Targets
# -------------------------------------------------------------

run_basic: $(BIN_DIR)/01_basic_two_criteria$(EXEEXT)
	@echo "Running Basic Example..."
	@./$(BIN_DIR)/01_basic_two_criteria$(EXEEXT)
	@command -v gnuplot >/dev/null 2>&1 && gnuplot plot_01_basic.gp || echo "Gnuplot not found. Please install as described in the documantation."

run_topography: $(BIN_DIR)/02_extended_testcase1_topography$(EXEEXT)
	@echo "Running Topography Example..."
	@./$(BIN_DIR)/02_extended_testcase1_topography$(EXEEXT)
	@if command -v gnuplot >/dev/null 2>&1; then \
	echo "\n=== Starting plot sequence ==="; \
	gnuplot -c plot_02_topography.gp 5 "Rule1" "Desirable AND Desirable"; \
	gnuplot -c plot_02_topography.gp 6 "Rule2" "Desirable AND Tolerable"; \
	gnuplot -c plot_02_topography.gp 7 "Rule3" "Desirable OR Undesirable"; \
	echo "All graphics successfully done."; \
	else \
		echo "Gnuplot not found. Please install as described in the documantation."; \
		fi

run_equality: $(BIN_DIR)/03_equality_constraint$(EXEEXT)
	@echo "Running Equality Constraint Example..."
	@./$(BIN_DIR)/03_equality_constraint$(EXEEXT)
	@command -v gnuplot >/dev/null 2>&1 && gnuplot plot_03_equality.gp || echo "Gnuplot not found. Please install as described in the documantation."

# Führt weiterhin alle Programme nacheinander aus (ohne Plot-Stopps)
run: all
	@echo
	@echo "Running all examples..."
	@echo
	@for exe in $(EXAMPLE_BINS); do \
		echo "------------------------------------------------------------"; \
		echo "$$exe"; \
		echo "------------------------------------------------------------"; \
		./$$exe || exit 1; \
		echo; \
	done

test: check-compiler dirs $(TEST_BIN)
	@echo
	@echo "Running FuzzyGoal tests..."
	@./$(TEST_BIN)

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@rm -f *.dat *.png
	@echo "Cleaned build files, .dat files and .png images."

help:
	@echo "Available targets:"
	@echo
	@echo "  make                  build all examples"
	@echo "  make run_basic        run test 1 and plot with gnuplot"
	@echo "  make run_topography   run test 2 and plot with gnuplot"
	@echo "  make run_equality     run test 3 and plot with gnuplot"
	@echo "  make run              run all examples (no plotting)"
	@echo "  make test             build and run the test suite"
	@echo "  make clean            remove build files, .dat and .png files"
