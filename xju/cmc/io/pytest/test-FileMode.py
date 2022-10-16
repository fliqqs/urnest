#!/usr/bin/env python3

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

from xju.cmc.io import FileMode
from xju.assert_ import Assert
from xju.xn import readable_repr
from typing import cast

p1=FileMode(0o641)
Assert(str(p1))=="0o641"
Assert(repr(p1))=="0o641"
Assert(p1+FileMode(0o002))==FileMode(0o643)
Assert(p1-FileMode(0o002))==p1
Assert(p1-FileMode(0o001))==FileMode(0o640)
Assert(p1)==FileMode(0o641)
Assert(p1!=FileMode(0o640))==True

try:
    y=p1==7
except Exception as e:
    Assert(readable_repr(e))=="7 is of type <class 'int'> not <class 'xju.cmc.io.FileMode'>"
else:
    assert False,y
    pass

try:
    p1!='fred'
except Exception as e:
    Assert(readable_repr(e))=="fred is of type <class 'str'> not <class 'xju.cmc.io.FileMode'>"
else:
    assert False
    pass
