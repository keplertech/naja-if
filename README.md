# naja-if

![Ubuntu Build](https://github.com/najaeda/naja-if/actions/workflows/ubuntu-cpp-build.yml/badge.svg)
![MacOS Build](https://github.com/najaeda/naja-if/actions/workflows/macos-cpp-build.yml/badge.svg)

## Introduction

Naja-if is a Cap'n Proto based Interchange format for EDA data serialization and streaming.

## Source metadata and snapshot revision

`schema/naja_common.capnp` owns the snapshot revision constants (currently
0.3.0). Snapshot readers must validate this revision before decoding payloads.
The schema is still unstable. Reordering field ordinals is allowed only with a
new snapshot revision; readers must reject unsupported revisions.

`RTLInfos` keeps the legacy location independent from a loaded-input declaration
and ordered upstream origins. `SourceReference` carries a range and optional
provider, language and representation strings. Empty strings mean unknown;
filenames never determine language or representation. Ranges use opaque paths,
1-based byte columns and inclusive endpoints; zero coordinates mean unknown.
`sourceLoc32` preserves the legacy slot with full-width coordinates and takes
precedence over the original 16-bit `sourceLoc` when present. Sparse metadata
records also cover bus-term bits and instance terminals. Source text and ASTs
are not required.

The 0.3.0 revision prevents older readers from silently discarding attributes,
in addition to the source roles and wide columns introduced in 0.2.0. Naja
currently requires exact revision and producer identity matches; snapshots from
older revisions must be regenerated.
Source consumers select the schema revision through their naja-if checkout.
Registry consumers select a separately published release.


## Netlist attributes

`NajaCommon.Attribute` represents an HDL/netlist attribute independently of generic
`Property` values and `RTLInfos`. It has a name and one of three value variants:
`none` (the default, a valueless attribute), `string` (including an empty string),
or `number`. Numbers are stored as text, preserving their representation, width,
radix, arbitrary precision and unknown bits without integer conversion. A numeric
value and a string containing the same text remain distinct.

Ordered `attributes` lists are available on SNL designs, scalar and bus terms,
bus-term bits, instances, instance terminals, scalar and bus nets, and bus-net
bits. Preserve duplicate names and list order; an absent or empty list means no
attributes. Bus-term `bitInfos` and instance `termInfos` are sparse records which
may contain attributes without any `rtlInfos`. Attributes on a bus and those on
an individual bit are independent. Existing PNL records are unchanged.

Revision 0.3.0 groups attributes immediately after object identity fields, before
structural data and source metadata. This reorders existing SNL field ordinals
and is intentionally wire-incompatible with 0.2.0. Readers must validate the
snapshot revision before decoding; regenerate older snapshots rather than
decoding them with the new layout.

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

The standalone build fetches GoogleTest and runs the serialization,
source-metadata and attribute round-trip tests. A parent project or package build
can instead use:

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
bazel test //:naja_if_tests --test_output=errors
```

These public targets are the same native targets previously on `bazel-support`.
They generate and compile the schemas, with Cap'n Proto libraries, `capnp`, and
`capnpc-c++` supplied by the `capnp-cpp` Bazel module. They do not invoke CMake or
search for installed Cap'n Proto tools. Compiler/platform toolchain selection
remains the responsibility of the Bazel build environment. The `naja_if_tests`
target runs the same serialization, source-metadata and attribute round-trip
test sources as the CMake standalone build.

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
for a BCR release. The snapshot schema revision (currently 0.3.0) and the Bazel
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
