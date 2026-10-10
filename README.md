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
Schema publication must update the CMake submodule pin and the descendant Bazel
support pin together in consumers.
