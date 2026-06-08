# Contributing to FuzzyGoal

Contributions are welcome.

Please use GitHub issues for bug reports, questions, feature suggestions, and
discussion of larger changes. Pull requests are welcome for:

- bug fixes,
- documentation improvements,
- additional tests,
- portability improvements,
- carefully discussed extensions of the public API.

For larger changes, please open an issue first to discuss the motivation and
possible impact on the public API.

All contributions should preserve the existing test suite and should add tests
where appropriate. Before submitting a pull request, please run:

```bash
make test
```

FuzzyGoal is intended to remain a small, transparent, and easy-to-integrate C++
library. New features should therefore be evaluated with respect to simplicity,
maintainability, and compatibility with existing optimization workflows.
