---
title: 'FuzzyGoal: A C++ Library for Fuzzy-Logic-Based Objective Functions'
tags:
  - C++
  - fuzzy logic
  - multi-criteria optimization
  - objective functions
  - scalarization
  - engineering optimization
authors:
  - name: Olaf Frommann
    affiliation: 1
affiliations:
  - name: Institute for Aerospace Technology, Hochschule Bremen
    index: 1
date: 2026-06-08
bibliography: paper.bib
---

# Summary

FuzzyGoal is a lightweight C++ library for constructing and evaluating
fuzzy-logic-based scalar objective functions for multi-criteria optimization.
It provides a compact implementation of fuzzy criteria, membership functions,
rule-based aggregation, equality-constraint handling, and defuzzification to a
single objective value in the interval `[0, 1]`.

The library is intended for engineering and scientific optimization problems in
which several criteria have to be combined into a scalar objective function,
but where classical weighted sums are difficult to interpret or insufficiently
expressive. FuzzyGoal allows users to describe preferences through qualitative
statements such as desirable, tolerable, and undesirable criterion ranges, and
through explicit fuzzy rules linking multiple criteria.

The implementation deliberately consists of only one header file and one source
file. It has no external runtime dependencies and is designed to be copied,
compiled, and linked directly into existing simulation or optimization codes.
This keeps the entry barrier low for users who need a customizable objective
function but do not want to adopt a larger optimization or fuzzy-logic
framework.

# Statement of need

Many practical optimization problems involve several competing criteria. A
typical engineering design may require low structural mass, low aerodynamic
drag, acceptable stability characteristics, and additional constraints. In such
cases, a Pareto front describes possible compromises but does not by itself
define which solution should be selected [@Ehrgott2005; @Deb2002]. A scalar
objective function is therefore still required whenever a single optimization
target or a final design decision is needed.

The weighted-sum method is one of the most common scalarization approaches
[@MarlerArora2010]. It is simple and computationally convenient, but its
interpretation can be difficult when criteria have different meanings,
nonlinear trade-offs, non-convex Pareto fronts, or additional penalty terms
[@DasDennis1997]. Small changes in weights can lead to large changes in the
selected optimum, and some Pareto-optimal regions may be unreachable by linear
scalarization.

Fuzzy logic provides an alternative way to express gradual preferences
[@Zadeh1965; @BellmanZadeh1970; @Zimmermann1978]. Instead of assigning only
numerical weights to criteria, a user can define which values are desirable,
still tolerable, or undesirable, and can combine these assessments using
explicit rules. Such rule-based formulations can be closer to how engineering
requirements are often stated in practice.

FuzzyGoal addresses the need for a small, transparent, and easily embeddable
C++ implementation of this idea. A central design goal of FuzzyGoal is to minimize
the practical entry barrier. In many engineering optimization workflows,
the objective function is embedded directly in existing simulation, analysis,
or design codes. Introducing a large external framework can be impractical in
such settings, especially when the objective-function formulation itself is the
experimental component. FuzzyGoal therefore provides the fuzzy-objective
construction as a small, self-contained C++ component that can be integrated
with minimal changes to existing code. General fuzzy-logic libraries exist,
but they are often broader in scope and not specifically designed for the
construction of scalar objective functions in optimization workflows. FuzzyGoal
focuses on this use case and provides a minimal public API for defining criteria,
rules, aggregation operators, and objective-function evaluations.

# Software functionality

FuzzyGoal supports the construction of objective functions from multiple fuzzy
criteria. Each criterion is defined by an interval, a reference value, a
preference direction, and a membership-function type. The library supports both
minimization-like and maximization-like criteria through `SmallerIsBetter` and
`LargerIsBetter`.

The available membership-function variants include linear, parabolic, and
Gaussian-based formulations. Criteria are classified into desirable, tolerable,
and undesirable membership values. Equality constraints can be represented by
internally evaluating absolute deviations from a target value.

Users may add explicit fuzzy rules connecting two criteria through logical
AND/OR semantics. Several aggregation operators are implemented, including
min/max aggregation, product/probabilistic-sum aggregation, Soft-Min/Soft-Max
aggregation, and p-norm-based smooth aggregation. The resulting rule strengths
are defuzzified into a scalar objective value in `[0, 1]`, where lower values
represent more desirable designs.

The library provides detailed status codes for criterion creation, rule
creation, and evaluation. This is intended to make integration into automated
optimization workflows robust and diagnosable.

The public API is intentionally compact. A typical use consists of creating a
`FuzzyGoal` object, adding criteria, optionally adding fuzzy rules, and calling
`evaluate` with the current criterion values. This structure allows the library
to be used as a drop-in objective-function component inside optimization loops,
including custom optimizers, simulation-driven parameter studies, or external
optimization frameworks.

No ownership of the optimization process is assumed by the library: FuzzyGoal
only evaluates the scalar objective value. The choice of optimizer, simulation
workflow, parameterization, and stopping criteria remains entirely with the
user.

# Examples and documentation

The repository includes three example programs:

1. A basic two-criteria example illustrating a conflicting trade-off and
   rule-based objective construction.

2. A two-dimensional topography example comparing objective-function
   formulations on a parameter domain.

3. An equality-constraint example showing how equality constraints can be
   incorporated through absolute deviations.

The examples generate data files that can be visualized using the included
gnuplot scripts. A user manual provides installation instructions, build
instructions, explanations of the examples, troubleshooting information, and a
public API reference.

# Testing and reproducibility

FuzzyGoal includes a small regression and API test suite. The tests cover
criterion creation, rule creation, status-code handling, membership evaluation,
equality constraints, defuzzification anchor values, and regression cases based
on the example problems.

The tests can be run with:

```bash
make test
```

The User Manual documents how to build and run the test suite. Continuous
integration is configured through GitHub Actions and builds and tests the
project on Linux and Windows. This provides automated checks for the core
library and example programs.

The software is archived on Zenodo and released under the GNU Lesser General
Public License v3.0 or later.

# Theoretical background

The mathematical motivation and a detailed discussion of weighted sums,
fuzzy-logic-based objective functions, Pareto-front interpretation, and
solution-space topography are provided in the accompanying technical report
[@Frommann2026OFMCO]. The present paper focuses on the software implementation
and its use in optimization workflows.

# AI usage disclosure

Generative AI assistance (GPT 5.5) was used for language editing, documentation
structuring, consistency checks, and drafting support. The author reviewed and
validated the final text, code, tests, mathematical formulations, and scientific
claims, and remains responsible for the software and the contents of this
submission.

# Acknowledgements

The author thanks Hochschule Bremen for providing the academic environment in
which this work was developed.

# References
