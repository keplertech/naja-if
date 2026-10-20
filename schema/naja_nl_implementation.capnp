# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

@0xd0ba693613b57951;

using NajaCommon = import "naja_common.capnp";

struct DBImplementation {
  id                      @0 : UInt8 = 1;
  libraryImplementations  @1 : List(LibraryImplementation);

  struct NetComponentReference {
    union {
      termReference     @0 : TermReference;
      instTermReference @1 : InstTermReference;
    }
  }

  struct TermReference {
    termID  @0 : UInt32;
    bit     @1 : UInt32;    
  }

  struct InstTermReference {
    instanceID  @0 : UInt32;
    termID      @1 : UInt32;
    bit         @2 : UInt32;    
  }

  struct LibraryImplementation {
    id                        @0 : UInt16 = 0;
    libraryImplementations    @1 : List(LibraryImplementation);
    snlDesignImplementations  @2 : List(SNLDesignImplementation);

    struct SNLDesignImplementation {
      id        @0 : UInt32 = 0;
      instances @1 : List(Instance);
      nets      @2 : List(Net);

      struct Instance {
        struct InstParameter {
          name  @0 : Text;
          value @1 : Text;
        }

        id              @0 : UInt32;
        name            @1 : Text;
        attributes      @2 : List(NajaCommon.Attribute);
        modelReference  @3 : NajaCommon.DesignReference;
        instParameters  @4 : List(InstParameter);
        # Sparse terminal records may carry source metadata, attributes, or both.
        termInfos       @5 : List(InstTermInfo);
        rtlInfos        @6 : NajaCommon.RTLInfos;

        struct InstTermInfo {
          termID      @0 : UInt32;
          bit         @1 : Int32;
          attributes  @2 : List(NajaCommon.Attribute);
          rtlInfos    @3 : NajaCommon.RTLInfos;
        }
      }

      struct Net {
        union {
          scalarNet @0 : ScalarNet;
          busNet    @1 : BusNet;
        }
      }

      enum NetType {
        standard @0;
        supply0  @1;
        supply1  @2;
        assign0  @3;
        assign1  @4;
        assignX  @5;
        assignZ  @6;
      }

      struct ScalarNet {
        id          @0 : UInt32 = 0;
        name        @1 : Text;
        attributes  @2 : List(NajaCommon.Attribute);
        type        @3 : NetType;
        components  @4 : List(NetComponentReference);
        rtlInfos    @5 : NajaCommon.RTLInfos;
      }

      struct BusNet {
        id          @0 : UInt32 = 0;
        name        @1 : Text;
        attributes  @2 : List(NajaCommon.Attribute);
        msb         @3 : Int32;
        lsb         @4 : Int32;
        bits        @5 : List(BusNetBit);
        rtlInfos    @6 : NajaCommon.RTLInfos;
      }

      struct BusNetBit {
        bit         @0 : UInt32;
        attributes  @1 : List(NajaCommon.Attribute);
        destroyed   @2 : Bool;
        type        @3 : NetType;
        components  @4 : List(NetComponentReference);
        rtlInfos    @5 : NajaCommon.RTLInfos;
      }
    }

    struct PNLDesignImplementation {
      id        @0 : UInt32 = 0;
      instances @1 : List(Instance);
      nets      @2 : List(Net);

      struct Point {
        x @0 : Int32;
        y @1 : Int32;
      }

      struct Instance {
        id              @0 : UInt32;
        name            @1 : Text;
        modelReference  @2 : NajaCommon.DesignReference;
        origin          @3 : Point;
        placementStatus @4 : PlacementStatus;

        enum PlacementStatus {
          unplaced @0;
          placed   @1;
          fixed    @2;
        }
      }

      struct Net {
        union {
          scalarNet @0 : ScalarNet;
          busNet    @1 : BusNet;
        }
      }

      struct ScalarNet {
        id          @0 : UInt32 = 0;
        name        @1 : Text;
        components  @2 : List(NetComponentReference);
      }

      struct BusNet {
        id        @0 : UInt32 = 0;
        name      @1 : Text;
        msb       @2 : Int32; 
        lsb       @3 : Int32;
        bits      @4 : List(BusNetBit);
      }

      struct BusNetBit {
        bit         @0 : UInt32;    
        destroyed   @1 : Bool;
        components  @2 : List(NetComponentReference);
      }
    }
  }
}
