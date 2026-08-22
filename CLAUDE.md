# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test

This project uses Conan 2 + CMake 3.30+. The recommended workflow:

```bash
# Full build via Conan (installs deps, configures, builds, tests)
conan install . --build=missing
conan build .

# Or manual CMake (after conan install has run):
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo [-DINSTRUCTION_SET=SSE|NEON|AVX|AVX-512]
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure

# Run a single test executable directly:
./build/test/<test-name>
```

`INSTRUCTION_SET` controls SIMD code paths. The default build enables assertions in `RelWithDebInfo` (via `release-debug-flags.cmake`). Tests require Catch2 3.x (pulled via Conan).

## Architecture

### Two-target structure

- **`utils-micro-dev`** — interface/header-only target (`include/dev/utils-micro/`). Contains all templates. Consumers link this for full generic access.
- **`utils-micro`** — shared library target (`src/utils-micro.cpp`). Wraps the dev library with a **C FFI** whose concrete types are fixed in `include/utils-micro-config.h` (`ScalarT = float`, `InternalMatrixT = NestedMatrix<float, std::vector>`).

The public C API (`include/utils-micro.h`) is the only stable ABI surface. Template internals are implementation detail.

### Matrix module (`include/dev/utils-micro/matrix/`)

Four storage strategies, each with an **immutable view** and a **mutable view** sibling:

| Type | Height | Width | Storage |
|------|--------|-------|---------|
| `FixedShapeMatrix` | compile-time | compile-time | flat contiguous |
| `FixedWidthMatrix` | runtime | compile-time | flat contiguous |
| `FlatMatrix` | runtime | runtime | flat contiguous |
| `NestedMatrix` | runtime | runtime | vector of row containers |

`NestedMatrix` allows independent row allocation (useful for external data ownership and non-uniform alignment). The other three use flat contiguous storage for SIMD friendliness.

`Traits<MatrixT>` maps each matrix type to its corresponding `*View` and `Mutable*View` types. Type-safe dimension wrappers (`Height`, `Width`, `Row`, `Column`, `Coordinates`, `Shape`) prevent silent dimension transpositions.

Non-owning views use `span<T>` (`std::span` with a C++17 nonstd fallback via `detail/nonstd/span.tpp`).

### Fourier module (`include/dev/utils-micro/fourier.hpp`)

`FastFourierTransform<MAX_BITS, RingArithmetic>` — the ring arithmetic parameter makes it work on both real floats and complex numbers. `TwoDimensionalTransform` is a CRTP base that decomposes a 2D FFT into row + column 1D passes. SSE4.1-optimised complex arithmetic lives in `arithmetic/simd-arithmetic.tpp`.

### Memory module (`include/dev/utils-micro/memory.hpp`)

`AllocatorWithAlignmentAdapter` wraps any standard allocator to enforce a minimum alignment (needed for SIMD loads). `ContainerBase` is the base for alignment-aware owning containers.

### Assertion system (`include/dev/utils-micro/assertion-handler.hpp`)

`AssertionType` enum: `OutOfBounds`, `SanityCheckFailed`, `ShapeMismatch`, `OtherPreconditionFailure`. Assertions are no-ops unless `UTILS_MICRO_ASSERT` is defined; when defined, behaviour (log+exit vs. trap) is set at runtime via the handler callback.

## Conventions

- C++17 throughout; no C++20 features in public headers (span is polyfilled).
- `SizeT` is `int` (not `size_t`) — signed arithmetic is intentional.
- Detail headers under `detail/` are internal; never include them directly in public API.
- SIMD paths are gated by `INSTRUCTION_SET` preprocessor defines set by CMake; don't assume a specific ISA at the source level.

## Custom

When navigating code, prefer LSP queries (find references, go to definition, hover) over text search.
