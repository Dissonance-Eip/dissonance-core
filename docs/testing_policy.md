# Testing Policy

This document outlines the testing strategy for the `dissonance-core` project.

---

## Goals

- Ensure correctness and robustness of the core algorithm
- Catch regressions early in development
- Maintain high code quality as the project grows

---

## Test Types

| Test Type           | Description |
|---------------------|-------------|
| **Unit Tests**      | Test individual functions in isolation (e.g., audio parsing, math operations) |
| **Integration Tests** | Validate behavior across multiple components (e.g., parser + CLI interface) |
| **System Tests**    | End-to-end behavior of full CLI tool (planned) |
| **Performance Tests** | Measure processing latency and real-time viability (planned) |
| **Security Tests**  | Evaluate effectiveness of perturbations against AI models (manual, research-driven) |

---

## Tools Used

- **Google Test** for C++ unit/integration testing
- **CMake** for build + test orchestration
- **GitHub Actions** for continuous testing on push/PR

---

## Code Coverage

- Goal: Maintain ≥ 70% test coverage across core logic
- Coverage tooling (future): Integration with `gcov` or `lcov`
- Tests will be expanded as algorithms mature

---

## Test Environment

- Linux-based GitHub runner (Ubuntu latest)
- All tests compiled with standard GNU toolchain
- `ctest` is used to run all test binaries in CI

---

## Policies

- All new features must include relevant tests
- CI must pass before merging into `develop` or `main`
- Tests must be deterministic (no randomness without seeding)
- Critical bugs must be covered by regression tests
