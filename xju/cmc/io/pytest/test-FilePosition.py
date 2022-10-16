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

from xju.cmc.io import FilePosition
from xju.assert_ import Assert
from xju.xn import readable_repr
from xju.misc import ByteCount
from typing import cast

p1=FilePosition(10)
Assert(str(p1))=='10'
Assert(repr(p1))=='10'
Assert(ByteCount(10)+p1)==FilePosition(20)
Assert(p1-FilePosition(3))==ByteCount(7)
Assert(p1-ByteCount(3))==FilePosition(7)
Assert(p1!=FilePosition(12))==True
Assert(p1)<FilePosition(12)
Assert(p1)<=FilePosition(12)
Assert(p1)<=p1
Assert(FilePosition(12))>p1
Assert(p1)>=p1

try:
    p1-cast(ByteCount,'fred')
except Exception as e:
    Assert("cannot subtract fred of type <class 'str'> from FilePosition").isIn(readable_repr(e))
else:
    assert False
    pass

Assert(p1)!=FilePosition(7)
try:
    p1=='fred'
except Exception as e:
    Assert("Failed to compare 10 to fred because\n'fred' is not a FilePosition it is a str.").isIn(
        readable_repr(e))
else:
    assert False
    pass
