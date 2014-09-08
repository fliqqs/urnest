// Copyright (c) 2014 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//


#include "p1.hh"
#include "p1.cref.hh"
//#include "p1.sref.hh"

#include <xju/Exception.hh>
#include <iostream>
#include <string>
#include "xju/format.hh"
#include "xju/stringToInt.hh"
#include <stdlib.h>
#include "xju/mt.hh"

std::string makeURI(int port, std::string const& objectName) throw()
{
  std::ostringstream s;
  s << "corbaloc:iiop:localhost:"<< port << "/" << objectName;
  return s.str();
}

class F_impl : public p1::F
{
public:
  ~F_impl() throw()
  {
  }
  
  virtual void f1() throw(cxy::Exception)
  {
    std::cout << "F::f1()" << std::endl;
  }
};

  
int main(int argc, char* argv[])
{
  try {
    if (argc != 3 || !(std::string("client")==argv[2]||
                       std::string("server")==argv[2]||
                       std::string("both")==argv[2])) {
      std::cerr << "usage:  " 
                << argv[0] << " <ip-port> [client|server|both]" << std::endl;
      return 1;
    }
    
    std::string const OBJECT_NAME("p1");
    
    int const port(xju::stringToInt(argv[1]));
    
    if (argv[2]==std::string("client")) {
      cxy::ORB<cxy::Exception> orb("giop:tcp::");
      cxy::cref<p1::F> ref(orb, makeURI(port, OBJECT_NAME));
      ref->f1();
    }
    // else if (argv[2]==std::string("server")) {
    //   std::string const orbEndPoint="giop:tcp::"+xju::format::str(port);
    //   cxy::ORB orb(orbEndPoint);

    //   F_impl x;
      
    //   cxy::sref<F> const xa(orb, x);
      
    //   orb.run();
    // }
    // else
    // {
    //   cxy::ORB<xju::Exception> orb(orbEndPoint);

    //   F_impl x;
      
    //   cxy::sref<F> const xa(orb, x);
      
    //   xju::mt::Thread<cxy::ORB> server_t(orb, 
    //                                      &cxy::ORB::run, // exceptions?
    //                                      &cxy::ORB::stop);
      
    //   cxy::cref<p1::F> ref(orb, makeURI(port, OBJECT_NAME));
    //   ref->f();
    // }
    
    return 0;
  }
  catch(xju::Exception& e) {
    e.addContext(xju::format::join(argv, argv+argc, " "), XJU_TRACED);
    std::cerr << readableRepr(e) << std::endl;
    return 1;
  }
  catch(cxy::Exception& e) {
    e.addContext(xju::format::join(argv, argv+argc, " "), 
                 std::make_pair(__FILE__, __LINE__));
    std::cerr << readableRepr(e, true, false) << std::endl;
    return 1;
  }
}
