<p align="center">
<img src="FuzzyGoal_Logo.png" alt="FuzzyGoal logo" width="420"/>
</p>

<p align="center">
<strong>A C++ Library for Fuzzy-Logic-Based Objective Functions</strong>
</p>

**FuzzyGoal** is a lightweight C++ library for constructing and evaluating
fuzzy-logic-based objective functions for multi-criteria optimization.

It was developed for engineering and scientific optimization problems where
several criteria have to be combined into a scalar objective value in a
transparent, interpretable, and tunable way.

The library supports fuzzy criteria, equality constraints, rule-based
aggregation, and defuzzification into a scalar objective value within the interval
`[0, 1]`.

---

## Motivation

In many optimization problems, objectives are not simply "good" or "bad".
Instead, design points may be:

- clearly desirable,
- still tolerable,
- or clearly undesirable.

Fuzzy logic provides a natural way to model such gradual transitions. Furthermore,
it allows in a very simple and reliable manner to combine and asses competing and
contradictory criteria. FuzzyGoal implements this idea in a compact C++ library
that can be integrated into existing optimization codes.

The library is intended for users who want to define objective functions based
on qualitative preference formulations such as:

> If criterion A is undesirable or criterion B is undesirable, then the overall
> result should be undesirable.

or:

> If criterion A is desirable and criterion B is tolerable, then the overall
> result should remain tolerable.

---

## Features

- Definition of fuzzy criteria with interval bounds and reference values.
- Support for both minimization-like and maximization-like criteria:
- `SmallerIsBetter`
- `LargerIsBetter`
- Multiple membership-function variants: linear, parabolic, Gaussian-based.
- Support for equality constraints via internally evaluated absolute deviations.
- Explicit fuzzy rule creation.
- Several rule aggregation operators:
- min/max,
- product/probabilistic sum,
- smooth min/max,
- p-norm aggregation.
- Defuzzification to a scalar objective value in `[0, 1]`.
- Detailed status codes for criterion creation, rule creation, and evaluation.
- Small implementation consisting of one header and one source file.
- Example programs and gnuplot scripts for visualization.

---

## Minimal Example

```cpp
#include "fuzzygoal.h"

#include <iostream>

int main()
{
    FuzzyGoal goal;

    auto cost = goal.addCriterion(
        "cost",
        0.0,
        100.0,
        30.0,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    if (cost.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Could not create criterion: "
                  << FuzzyGoal::statusToString(cost.status)
                  << "\n";
        return 1;
    }

    double objective = 0.0;

    auto status = goal.evaluate(
        {
            {cost.id, 40.0}
        },
        objective
    );

    if (status != FuzzyGoal::EvaluationOk)
    {
        std::cerr << "Evaluation failed: "
                  << FuzzyGoal::statusToString(status)
                  << "\n";
        return 1;
    }

    std::cout << "Objective value = " << objective << "\n";

    return 0;
}
```

---

## Example Programs

The repository contains three example programs:

1. **Basic two-criteria example**
Demonstrates two conflicting criteria and compares a base fuzzy objective
with a rule-based objective.

2. **Two-dimensional topography example**
Evaluates different fuzzy rule formulations on a two-dimensional parameter
domain.

3. **Equality constraint example**
Demonstrates equality constraints, which are internally evaluated as
absolute deviations from a target value.

The examples generate data files and plots using `gnuplot`.

Details on compiling, running, and plotting the examples are provided in the
User Manual.

---

## Documentation

The full documentation is provided in the User Manual:

```text
FuzzyGoal_UserManual.pdf
```

The manual contains:

- installation instructions for Windows, Linux, and macOS,
- a quick start guide,
- a detailed explanation of the example programs,
- build and plotting instructions,
- troubleshooting information,
- a full public API reference.

---

## Theoretical Background

The mathematical background and motivation are described in the accompanying
scientific work:

> Olaf Frommann,
> *Objective Functions in Multi-Criteria Optimization: Weighting, Fuzzy Logic,
> and Solution-Space Topography*,
> 2026.

Available:

```text
https://zenodo.org/records/
```

DOI information will be added once available.

---

## Repository Contents

Typical repository contents are:

```text
fuzzygoal.h
fuzzygoal.cpp
Makefile
examples/plot_01_basic.gp
examples/plot_02_topography.gp
examples/plot_03_equality.gp
FuzzyGoal_UserManual.pdf
CITATION.cff
LICENSE
README.md
```

The core library consists only of:

```text
fuzzygoal.h
fuzzygoal.cpp
```

---

## License

FuzzyGoal is licensed under the **GNU Lesser General Public License v3.0 or
later**.

See the `LICENSE` file for details.

SPDX identifier:

```text
LGPL-3.0-or-later
```

---

## How to Cite

If you use FuzzyGoal in scientific work, engineering studies, or publications,
please cite both the software and the accompanying theoretical work.

### Software

```bibtex
@software{frommann2026fuzzygoal,
    author       = {Frommann, Olaf},
    title        = {FuzzyGoal: A C++ Library for Fuzzy-Logic-Based Objective Functions},
    year         = {2026},
    version      = {1.0},
    url          = {https://github.com/of33/FuzzyGoal},
    license      = {LGPL-3.0-or-later},
    doi          = {10.5281/zenodo.20533391}
}
```

If a DOI is assigned to a release, please use the DOI-based citation instead.

### Theoretical Background

```bibtex
@misc{frommann2026ofmco,
    author       = {Frommann, Olaf},
    title        = {Objective Functions in Multi-Criteria Optimization:
        Weighting, Fuzzy Logic, and Solution-Space Topography},
        year         = {2026},
        url          = {https://zenodo.org/records/},
        note         = {DOI identifier to be added}
}
```

---

## Status

FuzzyGoal is currently released as version **1.0**.

The public API is documented in the User Manual. Future changes will be tracked
through release notes and version updates.

---

## Contact

For questions, suggestions, or bug reports, please use the GitHub issue
tracker:

```text
https://github.com/of33/FuzzyGoal/issues
```

