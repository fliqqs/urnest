// Copyright (c) 2020 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <xju/http/decodeQuotedStringContent.hh>

#include <iostream>
#include <xju/assert.hh>

namespace xju
{
namespace http
{

void test1() {
  xju::assert_equal(decodeQuotedStringContent(""),"");
  xju::assert_equal(decodeQuotedStringContent("\t x"),"\t x");
  xju::assert_equal(decodeQuotedStringContent("\\\\x"),"\\x");
  xju::assert_equal(decodeQuotedStringContent("\\\"\\\t"),"\"\t");
}

}
}

using namespace xju::http;

int main(int argc, char* argv[])
{
  unsigned int n(0);
  test1(), ++n;
  std::cout << "PASS - " << n << " steps" << std::endl;
  return 0;
}

