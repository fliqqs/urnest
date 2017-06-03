#include <xju/file/read.hh>
#line 1 "/home/xju/urnest/xju/file/read.hcp"
#line 14
#include "xju/AutoFd.hh" //impl
#include "xju/syscall.hh" //impl
#include <fcntl.h> //impl
#include <sstream> //impl
#include <unistd.h> //impl
#include <string> //impl
#include "xju/fcntl.hh" //impl
#include "xju/unistd.hh" //impl

namespace xju
{
namespace file
{
std::string read(
  std::pair<xju::path::AbsolutePath,xju::path::FileName> const& file) throw(
    xju::Exception)
{
  try {
    xju::AutoFd fd(xju::syscall(xju::open, XJU_TRACED)(
                     xju::path::str(file).c_str(), O_RDONLY|O_CLOEXEC, 0777));
    std::ostringstream result;
    char buffer[1024];
    while(1)
    {
      size_t const x(
        xju::syscall(xju::read, XJU_TRACED)(
          fd.value_, buffer, 1024));
      if (x == 0)
      {
        return result.str();
      }
      result << std::string(buffer, buffer+x);
    }
  }
  catch(xju::Exception& e) {
    std::ostringstream s;
    s << "read file " << xju::path::str(file);
    e.addContext(s.str(),XJU_TRACED);
    throw;
  }
}


}
}
