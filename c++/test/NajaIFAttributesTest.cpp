// SPDX-FileCopyrightText: 2026 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/io.h>

#include "naja_nl_interface.capnp.h"
#include "naja_nl_implementation.capnp.h"

namespace {

// Repeat names and text across variants to catch sorting, deduplication, or
// conversion of strings/numbers during a round trip.
template<typename Builder>
void setAttributes(Builder builder) {
  auto attributes = builder.initAttributes(6);
  attributes[0].setName("tag");
  attributes[0].setString("23");
  attributes[1].setName("tag");
  attributes[1].setNumber("23");
  attributes[2].setName("marker");
  attributes[2].setNone();
  attributes[3].setName("empty");
  attributes[3].setString("");
  attributes[4].setName("wide");
  attributes[4].setNumber("160'h123456789abcdef0123456789abcdef0123456789");
  attributes[5].setName("unknown");
  attributes[5].setNumber("8'b10xz0011");
}

template<typename Reader>
void checkAttributes(Reader reader) {
  ASSERT_TRUE(reader.hasAttributes());
  auto attributes = reader.getAttributes();
  ASSERT_EQ(6u, attributes.size());
  EXPECT_EQ(std::string("tag"), attributes[0].getName());
  ASSERT_TRUE(attributes[0].isString());
  EXPECT_EQ(std::string("23"), attributes[0].getString());
  EXPECT_EQ(std::string("tag"), attributes[1].getName());
  ASSERT_TRUE(attributes[1].isNumber());
  EXPECT_EQ(std::string("23"), attributes[1].getNumber());
  EXPECT_EQ(std::string("marker"), attributes[2].getName());
  EXPECT_TRUE(attributes[2].isNone());
  EXPECT_EQ(std::string("empty"), attributes[3].getName());
  ASSERT_TRUE(attributes[3].isString());
  EXPECT_EQ(0u, attributes[3].getString().size());
  EXPECT_EQ(std::string("wide"), attributes[4].getName());
  ASSERT_TRUE(attributes[4].isNumber());
  EXPECT_EQ(std::string("160'h123456789abcdef0123456789abcdef0123456789"),
            attributes[4].getNumber());
  EXPECT_EQ(std::string("unknown"), attributes[5].getName());
  ASSERT_TRUE(attributes[5].isNumber());
  EXPECT_EQ(std::string("8'b10xz0011"), attributes[5].getNumber());
}

template<typename Root, typename Check>
void roundTrip(capnp::MessageBuilder& message, Check check) {
  kj::VectorOutputStream output;
  capnp::writePackedMessage(output, message);
  kj::ArrayInputStream input(output.getArray());
  capnp::PackedMessageReader reader(input);
  check(reader.getRoot<Root>());
}

}  // namespace

TEST(NajaIFAttributesTest, interfaceRoundTrip) {
  capnp::MallocMessageBuilder message;
  auto db = message.initRoot<DBInterface>();
  auto design = db.initLibraryInterfaces(1)[0].initSnlDesignInterfaces(1)[0];
  design.setName("top");
  setAttributes(design);
  auto terms = design.initTerms(4);
  auto scalar = terms[0].initScalarTerm();
  scalar.setDirection(Direction::OUTPUT);
  setAttributes(scalar);
  auto bus = terms[1].initBusTerm();
  bus.setMsb(3);
  bus.setLsb(-2);
  setAttributes(bus);
  auto bit = bus.initBitInfos(1)[0];
  bit.setBit(-1);
  setAttributes(bit); // No RTLInfos: sparse records can carry only attributes.
  terms[2].initScalarTerm(); // Absent list.
  terms[3].initBusTerm().initAttributes(0); // Explicitly empty list.

  roundTrip<DBInterface>(message, [](auto db) {
    auto design = db.getLibraryInterfaces()[0].getSnlDesignInterfaces()[0];
    EXPECT_EQ(std::string("top"), design.getName());
    checkAttributes(design);
    auto terms = design.getTerms();
    EXPECT_EQ(Direction::OUTPUT, terms[0].getScalarTerm().getDirection());
    checkAttributes(terms[0].getScalarTerm());
    auto bus = terms[1].getBusTerm();
    EXPECT_EQ(3, bus.getMsb());
    EXPECT_EQ(-2, bus.getLsb());
    checkAttributes(bus);
    ASSERT_EQ(1u, bus.getBitInfos().size());
    auto bit = bus.getBitInfos()[0];
    EXPECT_EQ(-1, bit.getBit());
    EXPECT_FALSE(bit.hasRtlInfos());
    checkAttributes(bit);
    EXPECT_FALSE(terms[2].getScalarTerm().hasAttributes());
    EXPECT_EQ(0u, terms[2].getScalarTerm().getAttributes().size());
    EXPECT_EQ(0u, terms[3].getBusTerm().getAttributes().size());
  });
}

TEST(NajaIFAttributesTest, implementationRoundTrip) {
  capnp::MallocMessageBuilder message;
  auto db = message.initRoot<DBImplementation>();
  auto design = db.initLibraryImplementations(1)[0].initSnlDesignImplementations(1)[0];
  auto instances = design.initInstances(2);
  auto instance = instances[0];
  instance.setId(7);
  setAttributes(instance);
  auto terminal = instance.initTermInfos(1)[0];
  terminal.setTermID(4);
  terminal.setBit(-1);
  setAttributes(terminal);
  auto nets = design.initNets(3);
  auto scalar = nets[0].initScalarNet();
  scalar.setId(5);
  setAttributes(scalar);
  auto bus = nets[1].initBusNet();
  setAttributes(bus);
  auto bit = bus.initBits(1)[0];
  bit.setBit(2);
  setAttributes(bit);
  nets[2].initScalarNet();

  roundTrip<DBImplementation>(message, [](auto db) {
    auto design = db.getLibraryImplementations()[0].getSnlDesignImplementations()[0];
    auto instance = design.getInstances()[0];
    EXPECT_EQ(7u, instance.getId());
    checkAttributes(instance);
    ASSERT_EQ(1u, instance.getTermInfos().size());
    auto terminal = instance.getTermInfos()[0];
    EXPECT_EQ(4u, terminal.getTermID());
    EXPECT_EQ(-1, terminal.getBit());
    EXPECT_FALSE(terminal.hasRtlInfos());
    checkAttributes(terminal);
    auto nets = design.getNets();
    EXPECT_EQ(5u, nets[0].getScalarNet().getId());
    checkAttributes(nets[0].getScalarNet());
    auto bus = nets[1].getBusNet();
    checkAttributes(bus);
    ASSERT_EQ(1u, bus.getBits().size());
    EXPECT_EQ(2u, bus.getBits()[0].getBit());
    checkAttributes(bus.getBits()[0]);
    EXPECT_FALSE(design.getInstances()[1].hasAttributes());
    EXPECT_EQ(0u, design.getInstances()[1].getAttributes().size());
    EXPECT_FALSE(nets[2].getScalarNet().hasAttributes());
    EXPECT_EQ(0u, nets[2].getScalarNet().getAttributes().size());
  });
}

TEST(NajaIFAttributesTest, defaultValueAndRevision) {
  capnp::MallocMessageBuilder message;
  message.initRoot<Attribute>();
  roundTrip<Attribute>(message, [](auto attribute) {
    EXPECT_TRUE(attribute.isNone());
    EXPECT_EQ(0u, attribute.getName().size());
  });
  EXPECT_EQ(0u, SCHEMA_MAJOR);
  EXPECT_EQ(3u, SCHEMA_MINOR);
  EXPECT_EQ(0u, SCHEMA_REVISION);
}
