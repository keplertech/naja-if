# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

@0xc1f73089d4df5f68;

using NajaCommon = import "naja_common.capnp";

enum Direction {
  input     @0;
  output    @1;
  inout     @2;
  undefined @3;
}

enum DesignType {
  standard      @0;
  userBlackbox  @1;
  autoBlackbox  @2;
  primitive     @3;
}

struct SNLDesignInterface {
  id          @0 : UInt32 = 0;
  name        @1 : Text;
  attributes  @2 : List(NajaCommon.Attribute);
  properties  @3 : List(NajaCommon.Property);
  type        @4 : DesignType; # = Standard
  terms       @5 : List(Term);
  parameters  @6 : List(Parameter);
  rtlInfos    @7 : NajaCommon.RTLInfos;

  enum ParameterType {
    decimal @0;
    binary  @1;
    boolean @2;
    string  @3;
  }

  struct Parameter {
    name  @0 : Text;
    type  @1 : ParameterType;
    value @2 : Text;
  }

  struct Term {
    union {
      scalarTerm  @0 : ScalarTerm;
      busTerm     @1 : BusTerm;
    }
  }

  struct ScalarTerm {
    id          @0 : UInt32 = 0;
    name        @1 : Text;
    attributes  @2 : List(NajaCommon.Attribute);
    direction   @3 : Direction;
    rtlInfos    @4 : NajaCommon.RTLInfos;
  }

  struct BusTerm {
    id          @0 : UInt32 = 0;
    name        @1 : Text;
    attributes  @2 : List(NajaCommon.Attribute);
    direction   @3 : Direction;
    msb         @4 : Int32;
    lsb         @5 : Int32;
    # Sparse bit records may carry source metadata, attributes, or both.
    bitInfos    @6 : List(BitInfo);
    rtlInfos    @7 : NajaCommon.RTLInfos;

    struct BitInfo {
      bit         @0 : Int32;
      attributes  @1 : List(NajaCommon.Attribute);
      rtlInfos    @2 : NajaCommon.RTLInfos;
    }
  }
}

struct PNLDesignInterface {
  id          @0 : UInt32 = 0;
  name        @1 : Text;
  properties  @2 : List(NajaCommon.Property);
  type        @3 : DesignType; # = Standard
  terms       @4 : List(Term);

  struct Term {
    union {
      scalarTerm  @0 : ScalarTerm;
      busTerm     @1 : BusTerm;
    }
  }

  struct ScalarTerm {
    id        @0 : UInt32 = 0;
    name      @1 : Text;
    direction @2 : Direction;
  }
  
  struct BusTerm {
    id        @0 : UInt32 = 0;
    name      @1 : Text;
    direction @2 : Direction;
    msb       @3 : Int32; 
    lsb       @4 : Int32;
  }
}

struct DBInterface {
  id                  @0 : UInt8 = 1;
  properties          @1 : List(NajaCommon.Property);
  libraryInterfaces   @2 : List(LibraryInterface);
  topDesignReference  @3 : NajaCommon.DesignReference;

  enum LibraryType {
    standard    @0;
    primitives  @1;
  }

  struct LibraryInterface {
    id                  @0 : UInt16 = 0;
    name                @1 : Text;
    properties          @2 : List(NajaCommon.Property);
    type                @3 : LibraryType;
    libraryInterfaces   @4 : List(LibraryInterface);
    snlDesignInterfaces @5 : List(SNLDesignInterface);
  }
}
