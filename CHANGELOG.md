# Changelog

All notable changes to FuzzyGoal are documented in this file.

The format is inspired by [Keep a Changelog](https://keepachangelog.com/),
and this project uses semantic versioning where applicable.

---

## [1.1.0] - 2026-06-08

### Changed

- Added JOSS paper
- Small extension concerning tests in user manual

## [1.0.3] - 2026-06-08

### Changed

- Updated the README with the DOI of the accompanying technical report:
*Objective Functions in Multi-Criteria Optimization: Weighting, Fuzzy Logic,
and Solution-Space Topography*.
- Updated citation information for the theoretical background.
- Updated the User Manual with the technical report DOI.
- Added CHANGELOG.md

### Fixed

- Restored support for `GaussianRefGaussianTol`.
- Added or updated tests covering all membership-function variants.

---

## [1.0.2] - 2026-06-05

### Changed

- Replaced version-specific software DOI references in the documentation with
the Zenodo Concept DOI where appropriate.
- Updated citation metadata in `CITATION.cff`.
- Updated README badges and citation text.

---

## [1.0.1] - 2026-06-04

### Changed

- Added final Zenodo DOI information after initial archiving.
- Updated README, citation metadata, and User Manual.

---

## [1.0.0] - 2026-06-04

### Added

- Initial public release of FuzzyGoal.
- Core C++ library:
- `fuzzygoal.h`
- `fuzzygoal.cpp`
- Support for fuzzy criteria with:
- linear membership functions,
- parabolic membership functions,
- Gaussian-based membership functions.
- Support for minimization-like and maximization-like criteria:
- `SmallerIsBetter`
- `LargerIsBetter`
- Support for equality constraints via internally evaluated absolute deviations.
- Explicit fuzzy rule creation.
- Multiple aggregation operators:
- min/max,
- product/probabilistic sum,
- Soft-Min/Soft-Max,
- p-norm aggregation.
- Defuzzification to a scalar objective value in `[0, 1]`.
- Example programs:
- basic two-criteria example,
- two-dimensional topography example,
- equality-constraint example.
- Gnuplot scripts for visualizing example outputs.
- User Manual.
- Test suite.
- GitHub Actions CI for Linux and Windows.
- Zenodo archiving.
