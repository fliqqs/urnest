// Copyright (c) 2015 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include "GenErr.hh"

namespace xju
{
namespace snmp
{

GenErr::GenErr(Oid const& param, const xju::Traced& trace) throw():
    InvalidParam("general error for oid "+param.toString(),param,trace)
{
}


}
}

