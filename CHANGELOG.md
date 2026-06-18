# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2026-06-18

### Added

- CMake build scripts to package `utils-micro-dev`.
- Dockerignore and proper gitignore.

## [0.2.0] - 2026-06-16

### Added

- Changelog.
- GitHub CI.
- Extended CI to validate proper versioning, including changelog entry and release tags.
- Deployment to `gitlab` remote.

### Fixed

- Building packages with Conan in GitLab CI.
- GitLab CI now properly uses hosted Docker-in-Docker runners.

## [0.1.0] - 2026-06-07

### Added

- Initial library.
