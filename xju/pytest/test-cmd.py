# Copyright (c) 2022 Trevor Taylor
# coding: utf-8
# 
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that all
# copyright notices and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

from xju.cmd import do_cmd,CmdFailed

from typing import cast
from xju.xn import readable_repr
from xju.assert_ import Assert

Assert(do_cmd('/bin/echo fred'.split()))==(b'fred\n','')
try:
    do_cmd('lsssss')
except Exception as e:
    Assert(readable_repr(e))=="Failed to do non-shell command 'lsssss' because\nl not found on PATH /home/xju/.cargo/bin:/home/xju/gcc-9.3.0-run/bin:/usr/bin:/bin."
else:
    assert False
    pass

try:
    do_cmd('/etc/passwd')
except Exception as e:
    Assert(readable_repr(e))=="Failed to do non-shell command '/etc/passwd' because\n[Errno 13] Permission denied: '/etc/passwd'."
else:
    assert False
    pass

try:
    do_cmd('/bin/ls /dev/non-existent'.split())
except CmdFailed as e:
    Assert(e.argv)==['/bin/ls','/dev/non-existent']
    Assert(e.status)!=0
    Assert(e.stderr)=="/bin/ls: cannot access '/dev/non-existent': No such file or directory\n"
    Assert(readable_repr(e))=="Failed to do non-shell command ['/bin/ls', '/dev/non-existent'] because\nnon-shell command ['/bin/ls', '/dev/non-existent'] failed with exit status 2 and stderr /bin/ls: cannot access '/dev/non-existent': No such file or directory\n."
else:
    assert False
    pass
