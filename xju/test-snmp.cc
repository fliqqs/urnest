// Copyright (c) 2014 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include "snmp.hh"

#include <iostream>
#include <xju/assert.hh>
#include <typeinfo>
#include "xju/snmp/NullValue.hh"
#include "xju/snmp/IntValue.hh"
#include "xju/snmp/StringValue.hh"
#include "xju/snmp/OidValue.hh"

namespace xju
{
namespace snmp
{

void test1() throw() {
  Oid x(".1.3.6.1.4.364");
  xju::assert_equal(x.toString(), ".1.3.6.1.4.364");
  std::vector<uint32_t> b;
  b.push_back(1);
  b.push_back(3);
  b.push_back(6);
  b.push_back(1);
  b.push_back(4);
  b.push_back(364);
  xju::assert_equal(x.components(),b);
  Oid y(b);
  xju::assert_equal(y.toString(), x.toString());
  xju::assert_equal(x,y);
  std::vector<uint32_t> c(b);
  c.push_back(3);
  Oid a(c);
  xju::assert_not_equal(x, a);
  xju::assert_less(x, a);

  Oid a2(x+Oid(".3"));
  xju::assert_equal(a2, a);
}

void test2()
{
  try {
    Oid x("");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \"\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\n\"\" does not start with '.'.");
  }
  try {
    Oid x("34.53");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \"34.53\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\n\"34.53\" does not start with '.'.");
  }
  try {
    Oid x(".34.");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \".34.\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\nfailed to parse component \"\" (at offset 4) because\nfailed to convert \"\" to a base-10 unsigned integer because\n\"\" is null.");
  }
  try {
    Oid x(".ac");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \".ac\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\nfailed to parse component \"ac\" (at offset 1) because\nfailed to convert \"ac\" to a base-10 unsigned integer because\ncharacter 1 ('a') of \"ac\" unexpected.");
  }
  
}

void test3() throw()
{
  // NullValue encoding
  {
    NullValue const x;
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),2U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x05,0});
  }
   
  // IntValue encoding
  // 0
  {
    IntValue const x(0);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0});
  }
 
  // -1, 1
  {
    IntValue const x(-1);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0xff});
  }
  {
    IntValue const x(1);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x01});
  }
  // -128,127
  {
    IntValue const x(-128);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x80});
  }
  {
    IntValue const x(127);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x07f});
  }
  // -129,128
  {
    IntValue const x(-129);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0xff,0x7f});
  }
  {
    IntValue const x(128);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x00,0x80});
  }
  // -32768,32767
  {
    IntValue const x(-32768);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x80,0x00});
  }
  {
    IntValue const x(32767);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x7f,0xff});
  }
  // -32769,32768
  {
    IntValue const x(-32769);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,3,0xff,0x7f,0xff});
  }
  {
    IntValue const x(32768);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,3,0x00,0x80,0x00});
  }
  // INT32_MAX,INT32_MIN
  {
    IntValue const x(0x7fffffff);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,4,0x7f,0xff,0xff,0xff});
  }
  {
    int32_t xx(0x80000000);
    IntValue const x(xx);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,4,0x80,0x00,0x00,0x00});
  }
  // INT64_MAX,INT64_MIN
  {
    IntValue const x(0x7fffffffffffffff);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),10U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,8,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff});
  }
  {
    IntValue const x(0x8000000000000000);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),10U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,8,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
  }

  // StringValue encoding
  // null string
  {
    StringValue const x("");
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),2U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x04,0});
  }
  // fred
  {
    StringValue const x("fred");
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x04,4,'f','r','e','d'});
  }
  // 127 byte string
  {
    StringValue const x(std::string(127U,'x'));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),129U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y[0],0x04);
    xju::assert_equal(y[1],127);
    xju::assert_equal(std::string(y.begin()+2,y.end()),
                      std::string(127U,'x'));
  }
  // 128 byte string
  {
    StringValue const x(std::string(128U,'x'));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),131U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y[0],0x04);
    xju::assert_equal(y[1],0x81);
    xju::assert_equal(y[2],128);
    xju::assert_equal(std::string(y.begin()+3,y.end()),
                      std::string(128U,'x'));
  }

  // OidValue encoding
  // .1.3
  {
    OidValue const x(Oid(".1.3"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,1,40*1+3});
  }
  // .1.3.4
  {
    OidValue const x(Oid(".1.3.4"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,2,40*1+3,0x04});
  }
  // .1.3.127
  {
    OidValue const x(Oid(".1.3.127"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,2,40*1+3,0x7f});
  }
  // .1.3.128
  {
    OidValue const x(Oid(".1.3.128"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,3,40*1+3,0x81,0x00});
  }
  // .1.3.128.4
  {
    OidValue const x(Oid(".1.3.128.4"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,4,40*1+3,0x81,0x00,0x04});
  }

}

void test4() throw()
{
  // encode(SnmpV1GetRequest)
  SnmpV1GetRequest r(
    Community("private"),
    RequestId(1),
    {Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0")});
  std::vector<uint8_t> x(encode(r));
  xju::assert_equal(
    x,
    std::vector<uint8_t>({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA0,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
          }));
}

void test5() throw()
{
  // decodeSnmpV1Response
  std::vector<uint8_t> x({
      0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
  
  SnmpV1Response y(decodeSnmpV1Response(x));
  xju::assert_equal(y.responseType_,0xA2);
  xju::assert_equal(y.community_,Community("private"));
  xju::assert_equal(y.id_,RequestId(1));
  xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
  xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
  xju::assert_equal(y.values_.size(),1);
  xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
  xju::assert_not_equal(dynamic_cast<NullValue const*>(&*y.values_[0].second),(NullValue const*)0);
}

void test6() throw()
{
  // decodeSnmpV1Response variations
  
  // definite long length eg long string
  try {
    std::vector<uint8_t> x({
        0x30,0x81,0xb0,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x81,0xA1,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x81,0x95,0x30,0x81,0x92,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a'
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  // indefinite length s1
  try {
    std::vector<uint8_t> x({
        0x30,0x80,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x81,0xA1,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x81,0x95,0x30,0x81,0x92,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0,0
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  // indefinite length s2
  try {
    std::vector<uint8_t> x({
        0x30,0x81,0xb1,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x80,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x81,0x95,0x30,0x81,0x92,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0,0
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  // indefinite length s3
  try {
    std::vector<uint8_t> x({
        0x30,0x81,0xb1,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x81,0xA2,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x80,0x30,0x81,0x92,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0,0
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  // indefinite length s4
  try {
    std::vector<uint8_t> x({
        0x30,0x81,0xb1,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x81,0xA2,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x81,0x96,0x30,0x80,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0,0
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  // indefinite length s4 and s3
  try {
    std::vector<uint8_t> x({
        0x30,0x81,0xb2,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x81,0xA3,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x80,0x30,0x80,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x04,0x81,128,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0,0,0,0
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second),(StringValue const*)0);
    xju::assert_equal(dynamic_cast<StringValue const*>(&*y.values_[0].second)->val_,std::string(128,'a'));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  // multi-byte int value
  try {
    std::vector<uint8_t> x({
        0x30,0x2D,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1F,0x02,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(0x100));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),1);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<NullValue const*>(&*y.values_[0].second),(NullValue const*)0);
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
    
  // oid value
  try {
    std::vector<uint8_t> x({
        0x30,53,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,39,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,28,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00,0x30,0x07,0x06,0x01,0x2B,0x06,0x02,0x2B,0x07
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_equal(y.responseType_,0xA2);
    xju::assert_equal(y.community_,Community("private"));
    xju::assert_equal(y.id_,RequestId(1));
    xju::assert_equal(y.error_,SnmpV1Response::ErrorStatus::NO_ERROR);
    xju::assert_equal(y.errorIndex_,SnmpV1Response::ErrorIndex(0));
    xju::assert_equal(y.values_.size(),2);
    xju::assert_equal(y.values_[0].first,Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"));
    xju::assert_not_equal(dynamic_cast<NullValue const*>(&*y.values_[0].second),(NullValue const*)0);
    xju::assert_equal(y.values_[1].first,Oid(".1.3"));
    xju::assert_not_equal(dynamic_cast<OidValue const*>(&*y.values_[1].second),(OidValue const*)0);
    xju::assert_equal(dynamic_cast<OidValue const*>(&*y.values_[1].second)->val_,Oid(".1.3.7"));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
    
}

void test7() throw()
{
  // decodeSnmpV1Response failures

  try {
    std::vector<uint8_t> x({
        0x31,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
          });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data because\nexpected sequence type byte 0x30, got 0x31 at offset 0.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x03,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes because\nfailed to decode integer at offset 2 because\ntype is 0x03 not 0x02.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x01,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes because\nexpected integer 0 (SNMP V1), got integer 1, at offset 2.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x02,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2 because\nfailed to decode string at offset 5 because\ntype is 0x02 not 0x04.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0xa2,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14 because\nfailed to decode integer at offset 16 because\ntype is 0xa2 not 0x02.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x01,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14, request id 1 at offset 16 because\nfailed to decode integer at offset 19 because\ntype is 0x01 not 0x02.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x09,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14, request id 1 at offset 16, error 0x0000000000000000 at offset 19 because\nfailed to decode integer at offset 22 because\ncan only handle 8-byte integers, not 9.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x30,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x22,0x02,0x01,0x01,0x02,0x05,0x01,0x00,0x00,0x00,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 50 bytes of data having successfully decoded sequence type 0x30 and length 48 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 34 bytes at offset 14, request id 1 at offset 16, error 0x0000000100000000 at offset 19, error index 0 at offset 26, 3rd sequence type 0x30 and length 19 bytes at offset 29 because\nerror status 4294967296 exceeds maximimum supported (2147483647).");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x01,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14, request id 1 at offset 16, error 0x0000000000000000 at offset 19 because\nfailed to decode integer at offset 22 because\ntype is 0x01 not 0x02.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x03,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14, request id 1 at offset 16, error 0x0000000000000000 at offset 19, error index 0 at offset 22, 3rd sequence type 0x30 and length 19 bytes at offset 25 because\nfailed to decode param oid and value sequence at offset offset 27 because\nfailed to decode oid at offset 29 because\ntype is 0x03 not 0x06.");
  }

  try {
    std::vector<uint8_t> x({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA2,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x30,0x00
        });
    
    SnmpV1Response y(decodeSnmpV1Response(x));
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to decode snmp v1 response from 46 bytes of data having successfully decoded sequence type 0x30 and length 44 bytes, snmp version 1 at offset 2, community \"private\" at offset 5, 2nd sequence type 0xa2 and length 30 bytes at offset 14, request id 1 at offset 16, error 0x0000000000000000 at offset 19, error index 0 at offset 22, 3rd sequence type 0x30 and length 19 bytes at offset 25 because\nfailed to decode param oid and value sequence at offset offset 27 because\nfailed to decode one int/string/oid/null etc value at offset 44 because\ndecoding of type 0x30 is not implemented.");
  }

}

void test8() throw()
{
  // validateResponse
  std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
    {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new IntValue(3)}}
  };
  auto x=validateResponse(
    SnmpV1GetRequest(Community("dje"),
                     RequestId(23),
                     std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
    SnmpV1Response(0xA2,
                   Community("dd2"),
                   RequestId(23),
                   SnmpV1Response::ErrorStatus(0),
                   SnmpV1Response::ErrorIndex(0),
                   values));
    
  xju::assert_equal(x.size(),2U);
  xju::assert_equal(x[Oid(".1.3.3")]->operator std::string(),"fred");
  xju::assert_equal(x[Oid(".1.3.9.3333")]->operator int(),3);

  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
      {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
      {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new IntValue(3)}}
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA0,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(ResponseTypeMismatch const& e) {
    xju::assert_equal(e.got_,0xa0);
    xju::assert_equal(e.expected_,0xa2);
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa0, community dd2, id 23, error status 0, error index 0, values .1.3.3: \"fred\", .1.3.9.3333: 3 to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nexpected response of type 0xa2 but got response of type 0xa0.");
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
      {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
      {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new IntValue(3)}}
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(24),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(ResponseIdMismatch const& e) {
    xju::assert_equal(e.got_,RequestId(24));
    xju::assert_equal(e.expected_,RequestId(23));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 24, error status 0, error index 0, values .1.3.3: \"fred\", .1.3.9.3333: 3 to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nexpected response with id 23 but got response of id 24.");
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
      {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
      {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new NullValue}}
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(2),
                     SnmpV1Response::ErrorIndex(2),
                     values));
    
    xju::assert_never_reached();
  }
  catch(NoSuchName const& e) {
    xju::assert_equal(e.param_,Oid(".1.3.9.3333"));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 2, error index 2, values .1.3.3: \"fred\", .1.3.9.3333: null to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nserver has no object with oid .1.3.9.3333.");
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(1),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(TooBig const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 1, error index 0, values  to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nSNMP response would have exceeded server internal limit.");
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
      {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
      {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new NullValue}}
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(5),
                     SnmpV1Response::ErrorIndex(1),
                     values));
    
    xju::assert_never_reached();
  }
  catch(GenErr const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 5, error index 1, values .1.3.3: \"fred\", .1.3.9.3333: null to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\ngeneral error for oid .1.3.3.");
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }

  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    };
    auto x=validateResponse(
      SnmpV1GetRequest(Community("dje"),
                       RequestId(23),
                       std::set<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 0, error index 0, values  to request community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nvalue not reported for oid(s) .1.3.3, .1.3.9.3333.");
  }
}

void test9() throw()
{
  // encode(SnmpV1SetRequest)
  SnmpV1SetRequest r(
    Community("private"),
    RequestId(1),
    {{Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"),
          std::shared_ptr<Value const>(new NullValue)}});
  std::vector<uint8_t> x(encode(r));
  xju::assert_equal(
    x,
    std::vector<uint8_t>({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA3,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
          }));
}

void test10() throw()
{
  // validateResponse
  std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
    {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new IntValue(3)}}
  };
  std::map<Oid, std::shared_ptr<Value const> > requestValues(
    values.begin(),values.end());
  
  validateResponse(
    SnmpV1SetRequest(Community("dje"),
                     RequestId(23),
                     requestValues),
    SnmpV1Response(0xA2,
                   Community("dd2"),
                   RequestId(23),
                   SnmpV1Response::ErrorStatus(0),
                   SnmpV1Response::ErrorIndex(0),
                   values));
    
  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA0,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(ResponseTypeMismatch const& e) {
    xju::assert_equal(e.got_,0xa0);
    xju::assert_equal(e.expected_,0xa2);
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa0, community dd2, id 23, error status 0, error index 0, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nexpected response of type 0xa2 but got response of type 0xa0.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(24),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(ResponseIdMismatch const& e) {
    xju::assert_equal(e.got_,RequestId(24));
    xju::assert_equal(e.expected_,RequestId(23));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 24, error status 0, error index 0, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nexpected response with id 23 but got response of id 24.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(2),
                     SnmpV1Response::ErrorIndex(2),
                     values));
    
    xju::assert_never_reached();
  }
  catch(NoSuchName const& e) {
    xju::assert_equal(e.param_,Oid(".1.3.9.3333"));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 2, error index 2, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nserver has no object with oid .1.3.9.3333.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(3),
                     SnmpV1Response::ErrorIndex(2),
                     values));
    
    xju::assert_never_reached();
  }
  catch(BadValue const& e) {
    xju::assert_equal(e.param_,Oid(".1.3.9.3333"));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 3, error index 2, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nvalue of .1.3.9.3333 is invalid.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(4),
                     SnmpV1Response::ErrorIndex(2),
                     values));
    
    xju::assert_never_reached();
  }
  catch(ReadOnly const& e) {
    xju::assert_equal(e.param_,Oid(".1.3.9.3333"));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 4, error index 2, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nobject oid .1.3.9.3333 is read-only.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(1),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(TooBig const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 1, error index 0, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nSNMP response would have exceeded server internal limit.");
  }

  try {
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(5),
                     SnmpV1Response::ErrorIndex(1),
                     values));
    
    xju::assert_never_reached();
  }
  catch(GenErr const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 5, error index 1, values .1.3.3: \"fred\", .1.3.9.3333: 3 to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\ngeneral error for oid .1.3.3.");
  }
  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    };
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 0, error index 0, values  to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nresponse did not return oids .1.3.3, .1.3.9.3333.");
  }
  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
      {Oid(".1.3.4"), std::shared_ptr<Value const>{new StringValue("fred")}},
    };
    validateResponse(
      SnmpV1SetRequest(Community("dje"),
                       RequestId(23),
                       requestValues),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     values));
    
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 0, error index 0, values .1.3.4: \"fred\" to SnmpV1SetRequest community dje, id 23, values .1.3.3: \"fred\", .1.3.9.3333: 3 because\nresponse did not return oids .1.3.3, .1.3.9.3333 and response returned unrequested oids .1.3.4.");
  }
}

void test11() throw()
{
  // encode(SnmpV1GetNexRequest)
  SnmpV1GetNextRequest r(
    Community("private"),
    RequestId(1),
    {Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0")});
  std::vector<uint8_t> x(encode(r));
  xju::assert_equal(
    x,
    std::vector<uint8_t>({
        0x30,0x2c,0x02,0x01,0x00,0x04,0x07,0x70,0x72,0x69,0x76,0x61,0x74,0x65,0xA1,0x1E,0x02,0x01,0x01,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x13,0x30,0x11,0x06,0x0D,0x2B,0x06,0x01,0x04,0x01,0x94,0x78,0x01,0x02,0x07,0x03,0x02,0x00,0x05,0x00
          }));
}

void test12() throw()
{
  // validateResponse(SnmpV1GetNextRequest,SnmpV1Response)

  std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values {
    {Oid(".1.3.3"), std::shared_ptr<Value const>{new StringValue("fred")}},
    {Oid(".1.3.9.3333"),std::shared_ptr<Value const>{new IntValue(3)}}
  };
  std::vector<std::pair<Oid, std::shared_ptr<Value const> > > nextValues {
    {Oid(".1.3.4"), std::shared_ptr<Value const>{new StringValue("jock")}},
    {Oid(".1.3.9.3334"),std::shared_ptr<Value const>{new IntValue(5)}}
  };
  try
  {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                           RequestId(23),
                           std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
  
    xju::assert_equal(x.size(),2U);
    xju::assert_equal(x[0].first,Oid(".1.3.4"));
    xju::assert_equal(x[0].second->operator std::string(),"jock");
    xju::assert_equal(x[1].first,Oid(".1.3.9.3334"));
    xju::assert_equal(x[1].second->operator int(),5);
  }
  catch(xju::Exception const& e)
  {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));
  }
  
  try {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA0,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(ResponseTypeMismatch const& e) {
    xju::assert_equal(e.got_,0xa0);
    xju::assert_equal(e.expected_,0xa2);
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa0, community dd2, id 23, error status 0, error index 0, values .1.3.4: \"jock\", .1.3.9.3334: 5 to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nexpected response of type 0xa2 but got response of type 0xa0.");
  }

  try {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(24),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(ResponseIdMismatch const& e) {
    xju::assert_equal(e.got_,RequestId(24));
    xju::assert_equal(e.expected_,RequestId(23));
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 24, error status 0, error index 0, values .1.3.4: \"jock\", .1.3.9.3334: 5 to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nexpected response with id 23 but got response of id 24.");
  }

  try {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(2),
                     SnmpV1Response::ErrorIndex(2),
                     nextValues));

    xju::assert_equal(x.size(),2U);
    xju::assert_equal(x[0].first,Oid(".1.3"));
    xju::assert_equal(x[1].first,Oid(".1.3"));
  }
  catch(xju::Exception const& e) {
    xju::assert_not_equal(readableRepr(e),readableRepr(e));//never reached
  }

  try {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(1),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(TooBig const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 1, error index 0, values .1.3.4: \"jock\", .1.3.9.3334: 5 to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nSNMP response would have exceeded server internal limit.");
  }

  try {
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(5),
                     SnmpV1Response::ErrorIndex(1),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(GenErr const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 5, error index 1, values .1.3.4: \"jock\", .1.3.9.3334: 5 to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\ngeneral error for oid .1.3.3.");
  }
  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > nextValues {
    };
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 0, error index 0, values  to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nresponse has less values than request.");
  }
  try {
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > nextValues {
      {Oid(".1.3.4"), std::shared_ptr<Value const>{new StringValue("jock")}},
      {Oid(".1.3.4.1"), std::shared_ptr<Value const>{new StringValue("sal")}},
      {Oid(".1.3.9.3334"),std::shared_ptr<Value const>{new IntValue(5)}}
    };
    auto x=validateResponse(
      SnmpV1GetNextRequest(Community("dje"),
                       RequestId(23),
                       std::vector<Oid>({Oid(".1.3.3"),Oid(".1.3.9.3333")})),
      SnmpV1Response(0xA2,
                     Community("dd2"),
                     RequestId(23),
                     SnmpV1Response::ErrorStatus(0),
                     SnmpV1Response::ErrorIndex(0),
                     nextValues));
    
    xju::assert_never_reached();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e),"Failed to validate response type 0xa2, community dd2, id 23, error status 0, error index 0, values .1.3.4: \"jock\", .1.3.4.1: \"sal\", .1.3.9.3334: 5 to SnmpV1GetNextRequest community dje, id 23, oids .1.3.3, .1.3.9.3333 because\nresponse has more values than request.");
  }
}

void test13() throw()
{
  // SnmpV1Table
  Oid const col1(".1.3.5.20");
  Oid const col2(".1.3.5.21");
  SnmpV1Table t({col1,col2});
  
  xju::assert_equal(t.atEnd(),false);
  xju::assert_equal(t.nextOids().size(),2U);
  xju::assert_equal(t.nextOids(),std::vector<Oid>({col1,col2}));
  typedef std::pair<Oid, std::shared_ptr<Value const> > V;
  t.add({V(Oid(".1.3.5.20.1"),std::shared_ptr<Value const>(new IntValue(3))),
        V(Oid(".1.3.5.21.1"),std::shared_ptr<Value const>(new IntValue(10)))});
  
  xju::assert_equal(t[col1].size(),1U);
  xju::assert_equal(t[col1][0].oid_,Oid(".1.3.5.20.1"));
  xju::assert_equal(t[col1][0].value_->operator int(),3);
  xju::assert_equal(t[col2].size(),1U);
  xju::assert_equal(t[col2][0].oid_,Oid(".1.3.5.21.1"));
  xju::assert_equal(t[col2][0].value_->operator int(),10);
  
  xju::assert_equal(t.atEnd(),false);
  xju::assert_equal(t.nextOids().size(),2U);
    
  xju::assert_equal(t.nextOids()[0],Oid(".1.3.5.20.1"));
  xju::assert_equal(t.nextOids()[1],Oid(".1.3.5.21.1"));

  t.add({V(Oid(".1.3.5.20.2"),std::shared_ptr<Value const>(new IntValue(4))),
        V(Oid(".1.3.5.21.2"),std::shared_ptr<Value const>(new IntValue(11)))});
  
  xju::assert_equal(t[col1].size(),2U);
  xju::assert_equal(t[col1][0].oid_,Oid(".1.3.5.20.1"));
  xju::assert_equal(t[col1][0].value_->operator int(),3);
  xju::assert_equal(t[col1][1].oid_,Oid(".1.3.5.20.2"));
  xju::assert_equal(t[col1][1].value_->operator int(),4);
  xju::assert_equal(t[col2].size(),2U);
  xju::assert_equal(t[col2][0].oid_,Oid(".1.3.5.21.1"));
  xju::assert_equal(t[col2][0].value_->operator int(),10);
  xju::assert_equal(t[col2][1].oid_,Oid(".1.3.5.21.2"));
  xju::assert_equal(t[col2][1].value_->operator int(),11);
  
  xju::assert_equal(t.atEnd(),false);
  xju::assert_equal(t.nextOids().size(),2U);
    
  xju::assert_equal(t.nextOids()[0],Oid(".1.3.5.20.2"));
  xju::assert_equal(t.nextOids()[1],Oid(".1.3.5.21.2"));

  t.add({V(Oid(".1.3"),std::shared_ptr<Value const>(new NullValue)),
        V(Oid(".1.3"),std::shared_ptr<Value const>(new NullValue))});

  xju::assert_equal(t[col1].size(),2U);
  xju::assert_equal(t[col1][0].oid_,Oid(".1.3.5.20.1"));
  xju::assert_equal(t[col1][0].value_->operator int(),3);
  xju::assert_equal(t[col1][1].oid_,Oid(".1.3.5.20.2"));
  xju::assert_equal(t[col1][1].value_->operator int(),4);
  xju::assert_equal(t[col2].size(),2U);
  xju::assert_equal(t[col2][0].oid_,Oid(".1.3.5.21.1"));
  xju::assert_equal(t[col2][0].value_->operator int(),10);
  xju::assert_equal(t[col2][1].oid_,Oid(".1.3.5.21.2"));
  xju::assert_equal(t[col2][1].value_->operator int(),11);
  
  xju::assert_equal(t.atEnd(),true);
}

void test14() throw()
{
  IPv4Address const a({192,168,0,3});
  IPv4Address const b({192,168,0,4});
  
  xju::assert_less(a,b);
  xju::assert_equal(a,a);
  xju::assert_equal(xju::format::str(a),"192.168.0.3");
  xju::assert_equal(a._,(192U<<24)+(168U<<16)+3);
}

void test15() throw()
{
  // encode(SnmpV1Trap)
  SnmpV1Trap t(
    Community("private"),
    Oid(".1.3.6"),
    IPv4Address({192,168,0,3}),
    SnmpV1Trap::GenericType(6),
    SnmpV1Trap::SpecificType(42),
    xju::MicroSeconds(330000),
    {{Oid(".1.3.6.1.4.1.2680.1.2.7.3.2.0"),
          std::shared_ptr<Value const>(new IntValue(2))}});
  std::vector<uint8_t> x(encode(t));
  xju::assert_equal(
    x,
    std::vector<uint8_t>({
        0x30,55,
          0x02,0x01,0x00,//snmp v1
          0x04,0x07,'p','r','i','v','a','t','e',
          0xA4,41,//trap-pdu
          0x06,2,0x2B,6,//oid
          0x40,4,192,168,0,3, //origin
          0x02,1,6, //generic type
          0x02,1,42, //specific type
          0x43,1,33, //timestamp
          0x30,20, // vars:
          0x30,18, // var:
          0x06,0x0D,0x2B,6,1,4,1,0x94,0x78,1,2,7,3,2,0,//oid
          0x02,1,2 //value
          }));
}


}
}


using namespace xju::snmp;

int main(int argc, char* argv[])
{
  unsigned int n(0);
  test1(), ++n;
  test2(), ++n;
  test3(), ++n;
  test4(), ++n;
  test5(), ++n;
  test6(), ++n;
  test7(), ++n;
  test8(), ++n;
  test9(), ++n;
  test10(), ++n;
  test11(), ++n;
  test12(), ++n;
  test13(), ++n;
  test14(), ++n;
  test15(), ++n;

  std::cout << "PASS - " << n << " steps" << std::endl;
  return 0;
}

