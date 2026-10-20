# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

@0x8180e964216c1d8e;

# Snapshot manifest revision. Readers must check it before decoding payloads.
# Increment when an older reader would silently lose new semantics.
const schemaMajor :UInt32 = 0;
const schemaMinor :UInt32 = 3;
const schemaRevision :UInt32 = 0;

struct DesignReference {
  dbID      @0 : UInt8;
  libraryID @1 : UInt16;
  designID  @2 : UInt32;
}

struct PropertyValue {
  union {
    text    @0 : Text;
    bool    @1 : Bool;
    uint64  @2 : UInt64;
  }
}

struct Property {
  name    @0 : Text;
  values  @1 : List(PropertyValue);
}

# HDL/netlist attribute, independent of generic properties and RTL source metadata.
# Attribute lists preserve insertion order and duplicate names. A missing list
# means no attributes. Numeric values retain their exact textual representation
# (including width, radix and unknown bits), without machine-integer conversion.
struct Attribute {
  name @0 : Text;
  union {
    none   @1 : Void; # Attribute without a value (the default).
    string @2 : Text; # Includes the explicitly empty string.
    number @3 : Text;
  }
}

# RTL source metadata (see naja's SNLRTLInfos.h).
struct SourceLoc {
  file      @0 : Text;    # source file path
  line      @1 : UInt32;
  endLine   @2 : UInt32;
  column    @3 : UInt16;
  endColumn @4 : UInt16;
}

# Language-neutral range: opaque path, 1-based byte columns, inclusive ends.
# Zero coordinates and empty paths denote unknown values.
struct SourceRange {
  file      @0 : Text;
  line      @1 : UInt32;
  endLine   @2 : UInt32;
  column    @3 : UInt32;
  endColumn @4 : UInt32;
}

struct SourceReference {
  range          @0 : SourceRange;
  provider       @1 : Text;
  language       @2 : Text;
  representation @3 : Text;
}

# One arbitrary extra key/value info.
struct RTLInfo {
  name  @0 : Text;
  value @1 : Text;
}

# Per-object RTL infos: an optional source location plus an optional
# set of extra key/value infos. sourceLoc is a pointer field, so it reads
# back null (hasSourceLoc() == false) when absent; infos is empty when none.
struct RTLInfos {
  sourceLoc @0 : SourceLoc;
  infos     @1 : List(RTLInfo);
  # Independent legacy slot with full-width columns. Prefer over sourceLoc
  # when present; retain sourceLoc's original wire layout for old payloads.
  sourceLoc32 @2 : SourceRange;
  declaration @3 : SourceReference;
  # Ordered, exact-deduplicated upstream origins; qualifiers may be unknown.
  origins     @4 : List(SourceReference);
}
