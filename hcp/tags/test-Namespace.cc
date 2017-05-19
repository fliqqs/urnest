// Copyright (c) 2017 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <hcp/tags/Namespace.hh>

#include <iostream>
#include <xju/assert.hh>

namespace hcp
{
namespace tags
{

void test1() {
  Namespace x;
  std::vector<Location> const l1(
    1U,
    Location(AbsolutePath("/"),
             FileName("fred.hh"),
             LineNumber(2)));
  
  x.addSymbol(std::vector<NamespaceName>{},
              UnqualifiedSymbol("fred"),
              l1);
  
  xju::assert_equal(x.lookup(std::vector<NamespaceName>{},
                             std::vector<NamespaceName>{},
                             UnqualifiedSymbol("fred")),
                    l1);
  xju::assert_equal(x.lookup({NamespaceName("a")},
                             std::vector<NamespaceName>{},
                             UnqualifiedSymbol("fred")),
                    l1);
  std::vector<Location> const l2(
    1U,
    Location(AbsolutePath("/n1"),
             FileName("jock.hh"),
             LineNumber(2)));
  
  x.addSymbol({NamespaceName("n1")},
              UnqualifiedSymbol("jock"),
              l2);

  xju::assert_equal(x.lookup({NamespaceName("n1")},
                             std::vector<NamespaceName>{},
                             UnqualifiedSymbol("jock")),
                    l2);
  xju::assert_equal(x.lookup({NamespaceName("a")},
                             {NamespaceName("n1")},
                             UnqualifiedSymbol("jock")),
                    l2);
  
  std::vector<Location> const l3(
    1U,
    Location(AbsolutePath("/a"),
             FileName("fred.hh"),
             LineNumber(2)));
  
  x.addSymbol({NamespaceName("a")},
              UnqualifiedSymbol("fred"),
              l3);

  xju::assert_equal(x.lookup({NamespaceName("a")},
                             std::vector<NamespaceName>{},
                             UnqualifiedSymbol("fred")),
                    l3);
  xju::assert_equal(x.lookup({},
                             std::vector<NamespaceName>{},
                             UnqualifiedSymbol("fred")),
                    l1);
  xju::assert_equal(x.lookup(std::vector<NamespaceName>{},
                             {NamespaceName("a")},
                             UnqualifiedSymbol("fred")),
                    l3);

  try {
    x.lookup({NamespaceName("")},
             {NamespaceName("n2")},
             UnqualifiedSymbol("jock"));
    xju::assert_never_reached();
  }
  catch(Namespace::UnknownNamespace const& e) {
    xju::assert_equal(readableRepr(e),"Failed to lookup locations of n2::jock when it is referenced from :: because\nfailed to find namespace n2 because\nunknown namespace n2, not one of a,n1.");
  }
  
  try {
    x.lookup({},
             {},
             UnqualifiedSymbol("anne"));
    xju::assert_never_reached();
  }
  catch(Namespace::UnknownSymbol const& e) {
    xju::assert_equal(readableRepr(e),"Failed to lookup locations of ::anne when it is referenced from :: because\nfailed to find symbol anne amongst fred because\nunknown symbol.");
  }
  
}

}
}

using namespace hcp::tags;

int main(int argc, char* argv[])
{
  unsigned int n(0);
  test1(), ++n;
  std::cout << "PASS - " << n << " steps" << std::endl;
  return 0;
}

