// SPDX-FileCopyrightText: 2025 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

#include <fcntl.h>
#include <filesystem>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "naja_nl_interface.capnp.h"
#include "naja_nl_implementation.capnp.h"

#ifndef NAJA_IF_TEST_PATH
#define NAJA_IF_TEST_PATH "Undefined"
#endif

TEST(NajaIFTest, test0) {
  std::filesystem::path outPath(NAJA_IF_TEST_PATH);
  outPath /= "NajaIFTestTest0.naja";
  if (std::filesystem::exists(outPath)) {
    std::filesystem::remove_all(outPath);
  }

  {
    ::capnp::MallocMessageBuilder message;

    DBInterface::Builder db = message.initRoot<DBInterface>();
    db.setId(0);
    auto libraries = db.initLibraryInterfaces(2);
    auto library0InterfaceBuilder = libraries[0];
    library0InterfaceBuilder.setId(0);
    library0InterfaceBuilder.setName("LIB0");
    auto designs = library0InterfaceBuilder.initSnlDesignInterfaces(1);
    auto design0InterfaceBuilder = designs[0];
    design0InterfaceBuilder.setId(0);
    design0InterfaceBuilder.setName("DESIGN0");
    design0InterfaceBuilder.setType(DesignType::STANDARD);
    auto terms = design0InterfaceBuilder.initTerms(3);
    auto term0InterfaceBuilder = terms[0];
    auto scalarTerm0 = term0InterfaceBuilder.initScalarTerm();
    scalarTerm0.setId(0);
    scalarTerm0.setName("TERM0");
    scalarTerm0.setDirection(Direction::INPUT);
    auto term1InterfaceBuilder = terms[1];
    auto scalarTerm1 = term1InterfaceBuilder.initScalarTerm();
    scalarTerm1.setId(1);
    scalarTerm1.setName("TERM1");
    scalarTerm1.setDirection(Direction::OUTPUT);
    auto term2InterfaceBuilder = terms[2];
    auto busTerm2 = term2InterfaceBuilder.initBusTerm();
    busTerm2.setId(2);
    busTerm2.setName("TERM2");
    busTerm2.setMsb(31);
    busTerm2.setLsb(0);
    busTerm2.setDirection(Direction::INOUT);


    int fd = open(
      outPath.c_str(),
      O_CREAT | O_WRONLY,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  
    writePackedMessageToFd(fd, message);
    close(fd);
  }

  // Read the message back
  int fd = open(outPath.c_str(), O_RDONLY);
  ::capnp::PackedFdMessageReader message(fd);

  DBInterface::Reader dbInterface = message.getRoot<DBInterface>();
  auto dbID = dbInterface.getId();
  EXPECT_EQ(0, dbID);
  EXPECT_EQ(2, dbInterface.getLibraryInterfaces().size());
  auto library0Interface = dbInterface.getLibraryInterfaces()[0];
  EXPECT_EQ(0, library0Interface.getId());
  EXPECT_EQ(std::string("LIB0"), library0Interface.getName());
  EXPECT_EQ(1, library0Interface.getSnlDesignInterfaces().size());
}

namespace {

// Fills an RTLInfos builder with a source location and a couple of infos.
template<typename Builder>
void setRTLInfos(Builder builder) {
  auto rtlInfos = builder.initRtlInfos();
  auto sourceLoc = rtlInfos.initSourceLoc();
  sourceLoc.setFile("design.v");
  sourceLoc.setLine(10);
  sourceLoc.setEndLine(12);
  sourceLoc.setColumn(3);
  sourceLoc.setEndColumn(7);
  auto wide = rtlInfos.initSourceLoc32();
  wide.setFile("wide.v");
  wide.setColumn(70000);
  wide.setEndColumn(UINT32_MAX);
  auto declaration = rtlInfos.initDeclaration();
  declaration.initRange().setFile("loaded.v");
  declaration.setProvider("verilog");
  declaration.setLanguage("verilog");
  declaration.setRepresentation("gate");
  auto origins = rtlInfos.initOrigins(2);
  origins[0].initRange().setFile("C:/rtl/top.sv");
  origins[0].setProvider("yosys");
  origins[1].initRange().setColumn(UINT32_MAX);
  auto infos = rtlInfos.initInfos(2);
  infos[0].setName("key0");
  infos[0].setValue("value0");
  infos[1].setName("key1");
  infos[1].setValue("value1");
}

// Asserts an RTLInfos reader holds what setRTLInfos wrote.
template<typename Reader>
void checkRTLInfos(Reader reader) {
  ASSERT_TRUE(reader.hasRtlInfos());
  auto rtlInfos = reader.getRtlInfos();
  ASSERT_TRUE(rtlInfos.hasSourceLoc());
  auto sourceLoc = rtlInfos.getSourceLoc();
  EXPECT_EQ(std::string("design.v"), sourceLoc.getFile());
  EXPECT_EQ(10, sourceLoc.getLine());
  EXPECT_EQ(12, sourceLoc.getEndLine());
  EXPECT_EQ(3, sourceLoc.getColumn());
  EXPECT_EQ(7, sourceLoc.getEndColumn());
  EXPECT_EQ(0u, SCHEMA_MAJOR);
  EXPECT_EQ(3u, SCHEMA_MINOR);
  EXPECT_EQ(0u, SCHEMA_REVISION);
  ASSERT_TRUE(rtlInfos.hasSourceLoc32());
  EXPECT_EQ(70000u, rtlInfos.getSourceLoc32().getColumn());
  EXPECT_EQ(UINT32_MAX, rtlInfos.getSourceLoc32().getEndColumn());
  ASSERT_TRUE(rtlInfos.hasDeclaration());
  EXPECT_EQ(std::string("loaded.v"), rtlInfos.getDeclaration().getRange().getFile());
  EXPECT_EQ(std::string("verilog"), rtlInfos.getDeclaration().getProvider());
  EXPECT_EQ(std::string("verilog"), rtlInfos.getDeclaration().getLanguage());
  EXPECT_EQ(std::string("gate"), rtlInfos.getDeclaration().getRepresentation());
  ASSERT_EQ(2u, rtlInfos.getOrigins().size());
  EXPECT_EQ(std::string("C:/rtl/top.sv"), rtlInfos.getOrigins()[0].getRange().getFile());
  EXPECT_EQ(std::string("yosys"), rtlInfos.getOrigins()[0].getProvider());
  EXPECT_EQ(0u, rtlInfos.getOrigins()[0].getLanguage().size());
  EXPECT_EQ(UINT32_MAX, rtlInfos.getOrigins()[1].getRange().getColumn());
  auto infos = rtlInfos.getInfos();
  ASSERT_EQ(2, infos.size());
  EXPECT_EQ(std::string("key0"), infos[0].getName());
  EXPECT_EQ(std::string("value0"), infos[0].getValue());
  EXPECT_EQ(std::string("key1"), infos[1].getName());
  EXPECT_EQ(std::string("value1"), infos[1].getValue());
}

}  // namespace

TEST(NajaIFTest, rtlInfos) {
  std::filesystem::path outPath(NAJA_IF_TEST_PATH);
  outPath /= "NajaIFTestRTLInfos.naja";
  if (std::filesystem::exists(outPath)) {
    std::filesystem::remove_all(outPath);
  }

  // Interface side: a design + a scalar term carry rtlInfos, a bus term does not.
  {
    ::capnp::MallocMessageBuilder message;
    auto db = message.initRoot<DBInterface>();
    auto libraries = db.initLibraryInterfaces(1);
    auto designs = libraries[0].initSnlDesignInterfaces(1);
    auto design = designs[0];
    design.setName("DESIGN0");
    setRTLInfos(design);
    auto terms = design.initTerms(2);
    auto scalarTerm = terms[0].initScalarTerm();
    scalarTerm.setName("TERM0");
    setRTLInfos(scalarTerm);
    auto busTerm = terms[1].initBusTerm();
    busTerm.setName("TERM1");  // intentionally no rtlInfos

    int fd = open(outPath.c_str(), O_CREAT | O_WRONLY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    writePackedMessageToFd(fd, message);
    close(fd);
  }
  {
    int fd = open(outPath.c_str(), O_RDONLY);
    ::capnp::PackedFdMessageReader message(fd);
    auto db = message.getRoot<DBInterface>();
    auto design = db.getLibraryInterfaces()[0].getSnlDesignInterfaces()[0];
    checkRTLInfos(design);
    auto terms = design.getTerms();
    checkRTLInfos(terms[0].getScalarTerm());
    // Absent rtlInfos reads back null/empty.
    auto busTerm = terms[1].getBusTerm();
    EXPECT_FALSE(busTerm.hasRtlInfos());
    EXPECT_FALSE(busTerm.getRtlInfos().hasSourceLoc());
    EXPECT_EQ(0, busTerm.getRtlInfos().getInfos().size());
    close(fd);
  }

  std::filesystem::path implPath(NAJA_IF_TEST_PATH);
  implPath /= "NajaIFTestRTLInfosImpl.naja";
  if (std::filesystem::exists(implPath)) {
    std::filesystem::remove_all(implPath);
  }

  // Implementation side: an instance + a scalar net carry rtlInfos.
  {
    ::capnp::MallocMessageBuilder message;
    auto db = message.initRoot<DBImplementation>();
    auto libraries = db.initLibraryImplementations(1);
    auto designs = libraries[0].initSnlDesignImplementations(1);
    auto design = designs[0];
    auto instances = design.initInstances(1);
    auto instance = instances[0];
    instance.setName("INST0");
    setRTLInfos(instance);
    auto nets = design.initNets(1);
    auto scalarNet = nets[0].initScalarNet();
    scalarNet.setName("NET0");
    setRTLInfos(scalarNet);

    int fd = open(implPath.c_str(), O_CREAT | O_WRONLY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    writePackedMessageToFd(fd, message);
    close(fd);
  }
  {
    int fd = open(implPath.c_str(), O_RDONLY);
    ::capnp::PackedFdMessageReader message(fd);
    auto db = message.getRoot<DBImplementation>();
    auto design =
      db.getLibraryImplementations()[0].getSnlDesignImplementations()[0];
    checkRTLInfos(design.getInstances()[0]);
    checkRTLInfos(design.getNets()[0].getScalarNet());
    close(fd);
  }
}
