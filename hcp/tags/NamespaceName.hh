#ifndef _HCP_TAGS_NAMESPACENAME_HH
#define _HCP_TAGS_NAMESPACENAME_HH
// generated from NamespaceName.idl by omni cxy idl backend specifying 
// xju::Exception from <xju/Exception.hh> as base class for all ipc exceptions

#include <xju/Exception.hh>
#include <string>
#include <xju/Tagged.hh>


namespace hcp
{
namespace tags
{
class NamespaceName_tag {};
typedef ::xju::Tagged<std::string,NamespaceName_tag > NamespaceName;
}
}
#endif