# naja-if

![Ubuntu Build](https://github.com/najaeda/naja-if/actions/workflows/ubuntu-cpp-build.yml/badge.svg)
![MacOS Build](https://github.com/najaeda/naja-if/actions/workflows/macos-cpp-build.yml/badge.svg)

## Introduction

Naja-if is a Cap'n Proto based Interchange format for EDA data serialization and streaming.

## Source metadata and snapshot revision

`schema/naja_common.capnp` owns the snapshot revision constants (currently
0.2.0). Snapshot readers must validate this revision before decoding payloads.
Existing field ordinals and primitive types must never be reinterpreted.

`RTLInfos` keeps the legacy location independent from a loaded-input declaration
and ordered upstream origins. `SourceReference` carries a range and optional
provider, language and representation strings. Empty strings mean unknown;
filenames never determine language or representation. Ranges use opaque paths,
1-based byte columns and inclusive endpoints; zero coordinates mean unknown.
`sourceLoc32` preserves the legacy slot with full-width coordinates and takes
precedence over the original 16-bit `sourceLoc` when present. Sparse metadata
records also cover bus-term bits and instance terminals. Source text and ASTs
are not required.

The 0.2.0 revision intentionally prevents older readers from silently discarding
new source roles or wide columns. Naja currently requires exact revision and
producer identity matches; snapshots from older revisions must be regenerated.
Source consumers select the schema revision through their naja-if checkout.
Registry consumers select a separately published release.


## Building from sources

CMake and Bazel are supported build paths for development and package builds.
Both generate C++ from the same `schema/` files in the selected checkout; neither
requires a naja-if registry release. Package builders should pin that checkout
and their build dependencies for reproducibility.

### CMake

Install CMake (3.21 or newer), a C++20 compiler, and Cap'n Proto with its CMake
package and code-generation tools. From the repository root:

```sh
cmake -S c++ -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The standalone build fetches GoogleTest and runs the existing serialization and
source-metadata tests. A parent project or package build can instead use:

```cmake
add_subdirectory(path/to/naja-if/c++ naja-if-build)
target_link_libraries(my_target PRIVATE naja_if)
```

This exposes the generated headers through `naja_if` and does not fetch GoogleTest
or add standalone tests when naja-if is a subproject. CMake package builders
provide Cap'n Proto through `find_package(CapnProto)` (for example using
`CMAKE_PREFIX_PATH`) and link the `naja_if` target; the project does not currently
provide install/export or CPack rules.

### Bazel

Use Bazelisk with the checked-in `.bazelversion`, or the corresponding Bazel
version, and a C++ compiler. From the repository root:

```sh
bazel build //:naja_common_capnp //:naja_nl_interface_capnp //:naja_nl_implementation_capnp
```

These public targets are the same native targets previously on `bazel-support`.
They generate and compile the schemas, with Cap'n Proto libraries, `capnp`, and
`capnpc-c++` supplied by the `capnp-cpp` Bazel module. They do not invoke CMake or
search for installed Cap'n Proto tools. Compiler/platform toolchain selection
remains the responsibility of the Bazel build environment. The CMake standalone
test executable is not exposed as a Bazel target.

A consuming project's `MODULE.bazel` declares the dependency:

```starlark
bazel_dep(name = "naja-if")
```

Build the consumer with the checkout selected explicitly:

```sh
bazel build --override_module=naja-if=/absolute/path/to/naja-if //path/to:target
```

The override also works if the consumer already declares a registry version.
For Naja source and package builds, select the same naja-if submodule checkout
that CMake uses. A BCR version of that checkout is not needed. Configuration of
Naja's builds and dependency synchronization policy is handled in Naja itself.

Consumer C++ targets depend on one or more of:

- `@naja-if//:naja_common_capnp`
- `@naja-if//:naja_nl_interface_capnp`
- `@naja-if//:naja_nl_implementation_capnp`

The generated headers remain at `schema/naja_common.capnp.h`,
`schema/naja_nl_interface.capnp.h`, and `schema/naja_nl_implementation.capnp.h`
within the Bazel output repository. The exported `schema` include directory also
supports unprefixed includes such as `#include "naja_common.capnp.h"`.
Package builds use these same targets and dependency resolution; there is no
separate development mode or registry requirement.

## Independent BCR publication

BCR publication distributes an immutable snapshot; it is independent of source
builds and can lag `main`. Schema changes and source/package builds must not wait
for a BCR release. The snapshot schema revision (currently 0.2.0) and the Bazel
module release version describe different things.

Follow the [BCR contribution guide](https://github.com/bazelbuild/bazel-central-registry/blob/main/docs/README.md)
and [Bazel module versioning guidance](https://bazel.build/external/faq):

1. Select and validate a release commit containing the native Bazel files and
   schemas. Create a new release tag and a stable source archive containing them
   and the licenses. Do not move an already published tag or replace its archive.
2. Submit a separate BCR change for a new `naja-if` version. Supply maintainer
   metadata, the registry `MODULE.bazel` with that release version and the source
   dependency declarations, and `source.json` with the archive URL, integrity
   hash, and appropriate strip prefix. The source `MODULE.bazel` deliberately
   omits a version so source overrides do not claim to be a released version.
3. Add the BCR presubmit configuration for supported Bazel versions/platforms.
   Build all three public targets and exercise a consuming C++ module with the
   generated headers. Validate the archive through BCR's tests before merging.
   Native build files in the archive must suffice without consumer patches or
   build-file overlays.
4. Preserve published versions and archives. Publish a new version for changes;
   do not rewrite an existing version to follow `main`. Consumers of the registry
   opt into new versions separately from consumers of source checkouts.

Release automation and the BCR entry/presubmits are follow-up work, maintained
separately from source-build CI. No registry publication is performed by these
builds. Once this native support is merged into `main` and source consumers use
that checkout, `bazel-support` can be retired. Retain its historical commits for
consumers still pinned to them; new changes need no parallel branch maintenance.
