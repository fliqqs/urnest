# coding: utf-8

# Copyright (c) 2018 Trevor Taylor
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

import urllib
from xn import inContext
from xn import firstLineOf as l1

def parseHeaders(mimePart):
    try:
        h,rest=mimePart.split('\r\n\r\n', 1)
        headers=h.split('\r\n',1)
        headers=[_.split(':',1) for _ in headers]
        headers=[(_[0], _[1].strip()) for _ in headers]
        return dict(headers), rest
    except:
        mps=mimePart[0:256]
        raise inContext('parse headers from mime part %(mps)s...'%vars())
    pass

def parseQuoted(x):
    assert x.startswith('"'), x
    assert x.endswith('"'), x
    return x[1:-1].replace('\"','"')

def parseDisposition(dispositionValue):
    '''Parse Content-Disposition value %(dispositionValue)s'''
    try:
        assert dispositionValue.strip().startswith('form-data;'), dispositionValue
        v=dispositionValue.split(';')[1:]
        v=[_.strip().split('=',1) for _ in v]
        v=[(_[0],parseQuoted(_[1])) for _ in v]
        return dict(v)
    except:
        raise inContext(parseDisposition.__doc__ % vars())
    pass

class FileVar(object):
    def __init__(self, filename, content):
        self.filename=filename
        self.content=content
        pass
    pass

def getVariablesFromWSGIenviron(wsgiEnv):
    '''parse query string and wsgi.input into a dictionary from WSGI environ {wsgiEnv}'''
    '''like { varName : str or FileVar or list of str or FileVar }, e.g.:'''
    '''  { "id":"883999", "colours":["red","blue"] }'''
    '''... note list is only used where var has multiple values'''
    try:
        result={
        }
        d=[]
        if wsgiEnv.get('QUERY_STRING',''):
            x=[_.split('=') for _ in wsgiEnv['QUERY_STRING'].split('&')]
            d.extend(
                [ (_[0], urllib.parse.unquote(_[1].replace('+',' ')))
                  for _ in x])
            pass
        if wsgiEnv['REQUEST_METHOD']!='GET':
            ct=[_.strip() for _ in wsgiEnv['CONTENT_TYPE'].split(';')]
            encoding='iso-8859-1'
            if 'charset=' in ct:
                encoding=ct.split('charset=')[1].split(';')[0]
                pass
            if 'application/x-www-form-urlencoded' in ct:
                l=int(wsgiEnv.get('CONTENT_LENGTH',-1))
                y=wsgiEnv['wsgi.input'].read(l).decode(encoding)
                x=[_.split('=') for _ in y.split('&')]
                d.extend(
                    [ (_[0], urllib.parse.unquote(_[1].replace('+',' ')))
                      for _ in x])
            elif 'multipart/form-data' in ct:
                assert ct[1].startswith('boundary='), ct
                # Firefox client adds extra '--' in front of all occurrances
                # of bondary (and after last too), not sure
                # if it's standard or not
                boundary='\r\n--'+ct[1].split('=',1)[1]
                rest='\r\n'+wsgiEnv['wsgi.input'].read(
                    wsgiEnv.get('CONTENT_LENGTH',None)).decode(encoding)
                all=rest.split(boundary)[1:-1]
                for x,i in zip(all,range(0,len(all))):
                    try:
                        assert x.startswith('\r\n'), x[0:256]
                        x=x[2:]
                        headers,rest=parseHeaders(x)
                        disposition=parseDisposition(
                            headers['Content-Disposition'])
                        if disposition.has_key('filename'):
                            d.append( (disposition['name'], 
                                       FileVar(disposition['filename'], rest)))
                        else:
                            d.append( (disposition['name'],rest) )
                            pass
                    except:
                        raise inContext('parse part %(i)s' % vars())
                    pass
                pass
            else:
                raise Exception(
                    'unimplemented CONTENT_TYPE: '+wsgiEnv['CONTENT_TYPE'])
            pass
        for key, value in d:
            if key in result:
                v=result[key]
                if isinstance(v,list):
                    v.append(value)
                else:
                    result[key]=[v,value]
                    pass
                pass
            else:
                result[key]=value
                pass
            pass
        return result
    except:
        raise inContext(l1(getVariablesFromWSGIenviron.__doc__).format(**vars()))
    pass

def getCookiesFromWSGIenviron(environ):
    '''get cookies from WSGI environ {environ} as dictionary'''
    '''like { cookieName : str or [ str ] }, e.g.:'''
    '''  { "emailAddress" : "fred@dot.com", '''
    '''    "preferredSizes" : [ "L", "XL" ] '''
    try:
        result={}
        e=environ.get('HTTP_COOKIE',None)
        if e: result=dict([_.strip().split('=') for _ in e.split(';')])
        return result
    except:
        raise inContext(l1(getCookiesFromWSGIenviron.__doc__).format(**vars()))
    pass


def getHTTPHeadersFromWSGIenviron(environ):
    '''get HTTP_ headers from WSGI environ {environ!r} as dictionary'''
    '''like { headerName : str }, e.g.:'''
    try:
        return dict([(name,value) for name,value in environ.items()
                     if name.startswith('HTTP_')])
    except:
        raise inContext(l1(getHTTPHeadersFromWSGIenviron.__doc__).format(**vars()))
    pass