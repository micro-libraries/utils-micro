# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.2] - 2026-09-28

### Fixed

- Lib package `requires` dev package with the same user/channel.

## [0.4.1] - 2026-09-28

### Added

- Separate job for successively creating both Conan dev and wrapper-lib packages - needed because lib package requires
  dev package. Should depend on a previous dev job passing.

### Fixed

- Consistent names for configuration headers.
- Configuration passing in CMake.
- Missing dependency: Conan lib package `requires` dev package.
- Missing `static_cast` in `~NestedMatrix`.

## [0.4.0] - 2026-08-22

### Added

- Homepage in conanfiles. The primary host of micro-libraries is GitGud.io.
- Link-time optimization support for the binary library target `utils-micro`.

### Changed

- Refactored GitLab CI to building blocks usable in other libraries.
- Refactored CMake to functions in new scripts `micro-libraries.cmake` and `tests-with-purpose.cmake`.
- GitHub CI checks made consistent with GitLab CI. Release on GitHub is still omitted from CI.

### Fixed

- Processing options `shared` and `fPIC` in Conan. 

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
