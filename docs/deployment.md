# Deployment Guide

This document describes how the `dissonance-core` repository is set up and how its CI/CD pipeline is configured.

---

## Repository Structure

```
dissonance-core/
├── src/           # Main audio processing logic
├── include/       # Header files
├── tests/         # Unit and integration tests
├── assets/        # Sample or demo audio files
├── CMakeLists.txt # Build configuration
├── .github/workflows/ci.yml # CI pipeline
```

---

## Branch Strategy

We use a lightweight GitFlow-inspired model:

- `main`: Stable, release-ready code
- `dev`: Active development and integration
- `feature/*`: Per-feature short-lived branches from `dev`

All changes are tested via CI before being merged into `main`.

---

## CI/CD: GitHub Actions

CI is defined in `.github/workflows/ci.yml`.

### What It Does:
- Installs dependencies (`cmake`, `g++`, `Google Test`)
- Builds the project using `CMake`
- Runs all tests using `ctest`

### Trigger Rules:
- On every `push` to `main` or `dev`
- On all pull requests targeting `dev`

---

## Deployment

At this pre-alpha stage, we do not deploy artifacts yet. Once public releases begin:
- Binaries will be uploaded to GitHub Releases
- Web/desktop integrations will pull from tagged releases

---
