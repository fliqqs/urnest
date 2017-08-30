//     -*- mode: c++ ; c-file-style: "xju" ; -*-
//
// Copyright (c) 2008
// Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <hcp/parser.hh>

#include <vector>
#include <algorithm>
#include <sstream>
#include "xju/format.hh"
#include "xju/JoiningIterator.hh"
#include <hcp/translateException.hh>

namespace hcp_parser
{


namespace
{
class FixedCause : public hcp_parser::Exception::Cause
{
public:
  ~FixedCause() throw()
  {
  }
  explicit FixedCause(std::string const& cause) throw():
      cause_(cause)
  {
  }
  virtual std::string str() const throw()
  {
    return cause_;
  }
  std::string const cause_;
};

    
std::string contextReadableRepr(
  std::pair<std::pair<Parser const*, I>, xju::Traced> const& c) throw()
{
  std::ostringstream s;
  s << c.first.second << ": failed to parse " << c.first.first->target();
  return s.str();
}
}
  
std::string readableRepr(Exception const& e) throw()
{
  std::ostringstream s;
  if (e.context_.size()==0) {
    s << e.at_ << ": " << e.cause_->str();
  }
  else {
    std::vector<std::string> context;
    std::transform(e.context_.rbegin(), e.context_.rend(),
                   xju::JoiningIterator<std::string, std::string>(
                     s,
                     " because\n  "),
                   std::ptr_fun(contextReadableRepr));
    s << " because\n  "
      << e.at_ << ": " << e.cause_->str();
  }
  return s.str();
}

namespace
{
class Optional;
class ParseOr;
class ParseAnd;
class ParseNot;
class ParseZeroOrMore;

class Optional : public Parser
{
public:
  PR x_;
  
  virtual ~Optional() throw() {
  }
  explicit Optional(PR x) throw():
    x_(x) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    PV result(IRs(), at);
    ParseResult const r(x_->parse(result.second, options));
    if (!r.failed()) {
      PV const x(*r);
      std::copy(x.first.begin(), x.first.end(),
                std::back_inserter(result.first));
      result.second=x.second;
    }
    return ParseResult(result);
  }

  // Parser::
  virtual std::string target() const throw();
};
class ParseZeroOrMore : public Parser
{
public:
  PR const x_;
  
  virtual ~ParseZeroOrMore() throw() {
  }
  explicit ParseZeroOrMore(PR x) throw():
    x_(x) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    PV result(IRs(), at);
    while(true) {
      ParseResult const r(x_->parse(result.second, options));
      if (!r.failed()) {
        PV const x(*r);
        std::copy(x.first.begin(), x.first.end(),
                  std::back_inserter(result.first));
        result.second=x.second;
      }
      else {
        return ParseResult(result);
      }
    }
  }

  // Parser::
  virtual std::string target() const throw();
};

class ParseAnd : public Parser
{
public:
  std::vector<PR> terms_;
  
  virtual ~ParseAnd() throw() {
  }
  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    ParseResult first(terms_.front()->parse(at, options));
    if (first.failed()) {
      return first;
    }
    PV result(*first);
    for(std::vector<PR>::iterator i = xju::next(terms_.begin()); 
        i != terms_.end();
        ++i) {
      ParseResult br((*i)->parse(result.second, options));
      if (br.failed()) {
        if (options.irsAtEnd_) {
          br.addAtEndIRs(result.first);
        }
        return br;
      }
      else {
        std::copy((*br).first.begin(), (*br).first.end(),
                  std::back_inserter(result.first));
        result.second=(*br).second;
      }
    }
    return ParseResult(result);
  }

  // Parser::
  virtual std::string target() const throw();
};

class ParseOr : public Parser
{
public:
  std::vector<PR> terms_;
  
  virtual ~ParseOr() throw() {
  }

  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    std::multimap<I, Exception> failures;
    for(std::vector<PR>::iterator i = terms_.begin(); i != terms_.end(); ++i) {
      ParseResult r((*i)->parse(at, options));
      if (r.failed()) {
        failures.insert(std::make_pair(r.e().at_, r.e()));
      }
      else
      {
        return r;
      }
    }
    if (options.trace_) {
      std::ostringstream s;
      s << "ParseOr choosing " 
        << (*(*failures.rbegin()).second.context_.rbegin()).first.first->target()
        << " which got to " << (*failures.rbegin()).first;
      hcp_trace::milestone(s.str(), XJU_TRACED);
    }
    return ParseResult((*failures.rbegin()).second);
  }

  // Parser::
  virtual std::string target() const throw();
};

class ParseNot : public Parser
{
  static xju::Shared<Exception::Cause const> const expected_parse_failure;
public:
  explicit ParseNot(PR term) throw():
    term_(term)
  {
  }
  PR term_;
  
  virtual ~ParseNot() throw() {
  }

  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    ParseResult const r(term_->parse(at, options));
    if (r.failed()) {
      return ParseResult(PV(IRs(), at));
    }
    return ParseResult(
      Exception(ParseNot::expected_parse_failure, at, XJU_TRACED));
  }

  // Parser::
  virtual std::string target() const throw();

};
xju::Shared<Exception::Cause const> const ParseNot::expected_parse_failure(
  new FixedCause("expected parse failure"));

std::string Optional::target() const throw()
{
  // bracket lhs/rhs if ambiguous (and/or/times)
  std::string const xt(x_->target());
  std::ostringstream s;
  s << "optional " 
    << ((dynamic_cast<ParseOr const*>(&*x_)||
         dynamic_cast<ParseAnd const*>(&*x_))?
        std::string("(")+xt+std::string(")"):
        xt);
  return s.str();
}

std::string ParseZeroOrMore::target() const throw() 
{
  // bracket lhs/rhs if ambiguous (and/or/times)
  std::string const xt(x_->target());
  std::ostringstream s;
  s << "zero or more occurrances of " 
    << ((dynamic_cast<ParseOr const*>(&*x_)||
         dynamic_cast<ParseAnd const*>(&*x_))?
        std::string("(")+xt+std::string(")"):
        xt);
  return s.str();
}

std::string ParseAnd::target() const throw() 
{
  std::vector<std::string> x;
  for(std::vector<PR>::const_iterator i=terms_.begin(); i!=terms_.end(); ++i) {
    // bracket lhs/rhs if ambiguous (and/or/times)
    std::string const at((*i)->target());
    std::ostringstream s;
    s << ((dynamic_cast<ParseOr const*>(&**i)||
           ((xju::next(i)!=terms_.end())&&
            dynamic_cast<ParseZeroOrMore const*>(&**i)))?
          std::string("(")+at+std::string(")"):
          at);
    x.push_back(s.str());
  }
  return xju::format::join(x.begin(), x.end(), " then ");
}

std::string ParseOr::target() const throw() {
  std::vector<std::string> x;
  for(std::vector<PR>::const_iterator i=terms_.begin(); i!=terms_.end(); ++i) {
    // bracket term if ambiguous (and/or/times)
    std::string const at((*i)->target());
    std::ostringstream s;
    s << ((dynamic_cast<ParseAnd const*>(&**i)||
           ((xju::next(i)!=terms_.end())&&
            dynamic_cast<ParseZeroOrMore const*>(&**i)))?
          std::string("(")+at+std::string(")"):
          at);
    x.push_back(s.str());
  }
  return xju::format::join(x.begin(), x.end(), " or ");
}

std::string ParseNot::target() const throw() {
  return "!"+term_->target();
}

namespace
{
  xju::Shared<Exception::Cause const> const end_of_input(
    new FixedCause("end of input"));
  Exception EndOfInput(I at, xju::Traced const& trace) throw()
  {
    return Exception(end_of_input, at, trace, true);
  }
}

class ParseAnyChar : public Parser
{
public:
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    if (at.atEnd()) {
      return ParseResult(EndOfInput(at, XJU_TRACED));
    }
    return ParseResult(
      std::make_pair(IRs(1U, IR(new hcp_ast::String(at, xju::next(at)))), 
                     xju::next(at)));
  }
  virtual std::string target() const throw() {
    return "any char";
  }
};

// escape newline, tab
std::string printChar(char const c) throw() {
  std::ostringstream s;
  switch(c) {
  case '\t': s << "\\t"; break;
  case '\n': s << "\\n"; break;
  default:
    s << c;
  }
  return s.str();
}


class ParseOneOfChars : public Parser
{
public:
  std::set<char> const chars_;
  
  ~ParseOneOfChars() throw()
  {
  }
  
  explicit ParseOneOfChars(std::string const& chars) throw():
    chars_(chars.begin(), chars.end()) {
  }
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw()
  {
    if (at.atEnd()) {
      return ParseResult(EndOfInput(at, XJU_TRACED));
    }
    if (chars_.find(*at) == chars_.end()) {
      return ParseResult(
        Exception(
          xju::Shared<Exception::Cause const>(new UnexpectedChar(at, chars_)), 
          at, XJU_TRACED));
    }
    return ParseResult(
      std::make_pair(
        IRs(1U, new hcp_ast::String(at, xju::next(at))), xju::next(at)));
  }
  // Parser::
  virtual std::string target() const throw()
  {
    std::vector<std::string> x;
    std::transform(chars_.begin(), chars_.end(), 
                   std::back_inserter(x), 
                   printChar);
    std::ostringstream s;
    s << "one of chars [" << xju::format::join(x.begin(), x.end(), "") << "]";
    return s.str();
  }

  class UnexpectedChar : public Exception::Cause
  {
  public:
    UnexpectedChar(I const at, std::set<char> const& chars) throw():
        at_(at),
        chars_(chars) {
    }
    ~UnexpectedChar() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "'" << (*at_) << "'" << " is not one of chars [" 
        << xju::format::join(chars_.begin(), chars_.end(), "") << "]";
      return s.str();
    }
    I const at_;
    std::set<char> const chars_;
  };

};

class ParseAnyCharExcept : public Parser
{
public:
  std::set<char> const chars_;
  
  ~ParseAnyCharExcept() throw()
  {
  }
  
  explicit ParseAnyCharExcept(std::string const& chars) throw():
    chars_(chars.begin(), chars.end()) {
  }
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw()
  {
    if (at.atEnd()) {
      return ParseResult(EndOfInput(at, XJU_TRACED));
    }
    if (chars_.find(*at) != chars_.end()) {
      return ParseResult(
        Exception(
          xju::Shared<Exception::Cause const>(new UnexpectedChar(at,chars_)), 
          at, XJU_TRACED));
    }
    return ParseResult(
      std::make_pair(
        IRs(1U, new hcp_ast::String(at, xju::next(at))), xju::next(at)));
  }
  // Parser::
  virtual std::string target() const throw()
  {
    std::vector<std::string> x;
    std::transform(chars_.begin(), chars_.end(), 
                   std::back_inserter(x), 
                   printChar);
    std::ostringstream s;
    s << "any char except [" 
      << xju::format::join(x.begin(), x.end(), "") << "]";
    return s.str();
  }

  class UnexpectedChar : public Exception::Cause
  {
  public:
    UnexpectedChar(I const at, std::set<char> const& chars) throw():
        at_(at),
        chars_(chars) {
    }
    ~UnexpectedChar() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "'" << (*at_) << "'" << " is one of [" 
        << xju::format::join(chars_.begin(), chars_.end(), "") << "]";
      return s.str();
    }
    I const at_;
    std::set<char> const chars_;
  };

};

class ParseCharInRange : public Parser
{
public:
  char const min_;
  char const max_;
  
  explicit ParseCharInRange(char const min, char const max) throw():
    min_(min),
    max_(max) {
  }
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    if (at.atEnd()) {
      return ParseResult(EndOfInput(at, XJU_TRACED));
    }
    if (((*at) < min_) || ((*at) > max_)) {
      return ParseResult(
        Exception(
          xju::Shared<Exception::Cause const>(
            new CharNotInRange(at, min_, max_)),
          at, XJU_TRACED));
    }
    return ParseResult(
      std::make_pair(
        IRs(1U, new hcp_ast::String(at, xju::next(at))), xju::next(at)));
  }
  // Parser::
  virtual std::string target() const throw()
  {
    std::ostringstream s;
    s << "one of chars '" << printChar(min_) <<"'..'" << printChar(max_) <<"'";
    return s.str();
  }
  class CharNotInRange : public Exception::Cause
  {
  public:
    CharNotInRange(I const at, char const min, char const max) throw():
        at_(at),
        min_(min),
        max_(max) {
    }
    ~CharNotInRange() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "'" << printChar(*at_) << "'" << " is not one of chars '" 
        << printChar(min_) <<"'..'" << printChar(max_) <<"'";
      return s.str();
    }
    I const at_;
    char const min_;
    char const max_;
  };

};

class ParseUntil : public Parser
{
public:
  PR const x_;
  
  explicit ParseUntil(PR const x) throw():
    x_(x) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    I end(at);
    for(; !end.atEnd(); ++end) {
      ParseResult const r(x_->parse_(end, o));
      if (!r.failed()) {
        xju::Shared<hcp_ast::String> item(new hcp_ast::String(at, end));
        return ParseResult(std::make_pair(IRs(1U, item), end));
      }
    }
    return ParseResult(EndOfInput(end, XJU_TRACED));
  }
  virtual std::string target() const throw() {
    std::ostringstream s;
    s << "up to but not including " << x_->target();
    return s.str();
  }
};

class ParseSpecificUntil : public Parser
{
public:
  PR const match_;
  PR const x_;
  
  explicit ParseSpecificUntil(PR match, PR const x) throw():
    match_(match),
    x_(x) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& options) throw() 
  {
    PV result(IRs(), at);
    while(true) {
      ParseResult const re(x_->parse_(result.second, options));
      if (!re.failed()) {
        return ParseResult(result);
      }
      ParseResult r(match_->parse_(result.second, options));
      if (r.failed()) {
        if (options.irsAtEnd_) {
          r.addAtEndIRs(result.first);
        }
        return r;
      }
      PV const x(*r);
      std::copy(x.first.begin(), x.first.end(),
                std::back_inserter(result.first));
      result.second=x.second;
    }
  }
  virtual std::string target() const throw() {
    std::ostringstream s;
    s << match_->target() << "'s up to but not including " << x_->target();
    return s.str();
  }
};

class ParseLiteral : public Parser
{
public:
  std::string const x_;
  
  virtual ~ParseLiteral() throw() {
  }
  
  explicit ParseLiteral(std::string const& x) throw():
    x_(x) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw()
  {
    try {
      std::pair<std::string::const_iterator, I> x(
        std::mismatch(x_.begin(), x_.end(), at));
      if (x.first != x_.end()) {
        return ParseResult(
          Exception(
            xju::Shared<Exception::Cause const>(
              new Mismatch(*x.second, *x.first)),
            x.second, XJU_TRACED));
      }
      return ParseResult(
        std::make_pair(IRs(1U, IR(new hcp_ast::String(at, x.second))),
                       x.second));
    }
    catch(I::EndOfInput const& e) {
      return ParseResult(EndOfInput(e.at_, XJU_TRACED));
    }
  }
  
  
  virtual std::string target() const throw() {
    std::ostringstream s;
    s << "\"" << x_ << "\"";
    return s.str();
  }

  class Mismatch : public Exception::Cause
  {
  public:
    Mismatch(char const got, char const wanted) throw():
        got_(got),
        wanted_(wanted)
    {
    }
    ~Mismatch() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "expected '" << printChar(wanted_) << "'" 
        << " but found '" << printChar(got_) << "'";
      return s.str();
    }
    char const got_;
    char const wanted_;
  };

};

class ParseHash : public Parser
{
public:
  static xju::Shared<Exception::Cause const> not_at_column_1;
  
  virtual ~ParseHash() throw() {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw()
  {
    if (at.atEnd()) {
      return ParseResult(EndOfInput(at, XJU_TRACED));
    }
    if (at.column_ != 1) {
      return ParseResult(
        Exception(ParseHash::not_at_column_1, at, XJU_TRACED));
    }
    if ((*at) != '#') {
      return ParseResult(
        Exception(
          xju::Shared<Exception::Cause>(
            new NotHash(*at)), at, XJU_TRACED));
    }
    I const nowAt(xju::next(at));
    return ParseResult(
      std::make_pair(IRs(1U, IR(new hcp_ast::String(at, nowAt))), nowAt));
  }
  
  virtual std::string target() const throw() {
    std::ostringstream s;
    s << "'#' at start of line";
    return s.str();
  }

  class NotHash : public Exception::Cause
  {
  public:
    NotHash(char const got) throw():
        got_(got)
    {
    }
    ~NotHash() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "line starts with '" << printChar(got_) << "', not '#'";
      return s.str();
    }
    char const got_;
  };
    

};
xju::Shared<Exception::Cause const> ParseHash::not_at_column_1(
  new FixedCause("not at column 1"));

PR oneChar() throw()
{
  static PR oneChar(new ParseAnyChar);
  return oneChar;
}


PR octalDigit() throw()
{
  static PR octalDigit(charInRange('0', '7'));
  return octalDigit;
}

  
PR hexDigit() throw()
{
  static PR hexDigit(charInRange('0','9')|
                     charInRange('a','f')|
                     charInRange('A','F'));
  return hexDigit;
}

  
PR stringEscapeSequence() throw()
{
  static PR stringEscapeSequence(
    parseLiteral("\\")+(
      parseOneOfChars("'\"?\\abfnrtv")|
      (octalDigit()+octalDigit()+octalDigit())|
      (octalDigit()+octalDigit())|
      octalDigit()|
      (parseLiteral("x")+atLeastOne(hexDigit()))));
  return stringEscapeSequence;
}


PR s_char() throw()
{
  static PR s_char(
    parseAnyCharExcept("\\\"\n")|
    stringEscapeSequence());
  return s_char;
}

PR c_char() throw()
{
  static PR s_char(
    parseAnyCharExcept("\\'\n")|
    stringEscapeSequence());
  return s_char;
}


class ParseBalanced : public Parser
{
public:
  PR const until_;
  bool angles_;
  
  explicit ParseBalanced(PR const until, bool angles) throw():
    until_(until),
    angles_(angles) {
  }
  
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    xju::Shared<hcp_ast::String> item(new hcp_ast::String(at, at));
    I end(at);
    while(true) {
      ParseResult const r1(until_->parse_(end, o));
      if (!r1.failed()) {
        item->end_=end;
        return ParseResult(std::make_pair(IRs(1U, item), end));
      }
      if (end.atEnd()) {
        return ParseResult(EndOfInput(end, XJU_TRACED));
      }
      switch(*end) {
      case '\'':
      {
        ParseResult const r2(
          (parseOneOfChars("'")+c_char()+parseOneOfChars("'"))->parse_(
            end, o));
        if (r2.failed()) {
          return r2;
        }
        end=(*r2).second;
      }
      break;
      case '"':
      {
        ParseResult const r2(stringLiteral()->parse_(end, o));
        if (r2.failed()) {
          return r2;
        }
        end=(*r2).second;
      }
      break;
      case '{':
      {
        ParseResult const r2(ParseBalanced(parseOneOfChars("}"), angles_).parse_(xju::next(end),o));
        if (r2.failed()) {
          return r2;
        }
        end=xju::next((*r2).second);
      }
      break;
      case '<':
        if (angles_) {
          ParseResult const r2(
            ParseBalanced(parseOneOfChars(">"), angles_).parse_(
              xju::next(end),o));
          if (r2.failed()) {
            return r2;
          }
          end=xju::next((*r2).second);
        }
        else {
          ++end;
        }
        break;
      case '[':
      {
        ParseResult const r2(
          ParseBalanced(parseOneOfChars("]"), angles_).parse_(
            xju::next(end),o));
        if (r2.failed()) {
          return r2;
        }
        end=xju::next((*r2).second);
      }
      break;
      case '(':
      {
        ParseResult const r2(
          ParseBalanced(parseOneOfChars(")"), angles_).parse_(
            xju::next(end),o));
        if (r2.failed()) {
          return r2;
        }
        end=xju::next((*r2).second);
      }
      break;
      case '/':
      {
        ParseResult const r2(
          comments()->parse_(end,o));
        if (!r2.failed()) {
          end=(*r2).second;
        }
        else
        {
          ++end;
        }
      }
      break;
      default:
        ++end;
      }
    }
  }
  virtual std::string target() const throw() {
    std::ostringstream s;
    s << "parse text, balancing (), [], {}, <>, stringLiteral, up to but "
      << "not including " << until_->target();
    return s.str();
  }
};

class AnonParser : public NamedParser_
{
public:
  std::string const name_;
  PR x_;
  
  virtual ~AnonParser() throw() {
  }
  
  explicit AnonParser(std::string const& name, PR const x) throw():
    name_(name),
    x_(x) {
  }

  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    std::unique_ptr<hcp_trace::Scope> scope;
    if (o.trace_) {
      std::ostringstream s;
      s << "parse " << target() << " at " << at;
      scope = std::unique_ptr<hcp_trace::Scope>(
        new hcp_trace::Scope(s.str(), XJU_TRACED));
    }
    ParseResult r(x_->parse(at, o));
    if (!r.failed()) {
      return r;
    }
    if (o.trace_) {
      scope->fail();
    }
    return r;
  }
  // Parser::
  virtual std::string target() const throw() {
    return name_;
  }
};


}

ZeroOrMore zeroOrMore() throw()
{
  static ZeroOrMore zeroOrMore;
  return zeroOrMore;
}

PR operator*(ZeroOrMore const, PR const b) throw()
{
  return PR(new ParseZeroOrMore(b));
}

PR operator+(PR a, PR b) throw()
{
  xju::Shared<ParseAnd> result(new ParseAnd);
  if (a->isA<ParseAnd>()) {
    ParseAnd const& x(a->asA<ParseAnd>());
    std::copy(x.terms_.begin(),
              x.terms_.end(),
              std::back_inserter(result->terms_));
  }
  else {
    result->asA<ParseAnd>().terms_.push_back(a);
  }
  if (b->isA<ParseAnd>()) {
    ParseAnd const& x(b->asA<ParseAnd>());
    std::copy(x.terms_.begin(),
              x.terms_.end(),
              std::back_inserter(result->terms_));
  }
  else {
    result->asA<ParseAnd>().terms_.push_back(b);
  }
  return result;
}

PR operator|(PR a, PR b) throw()
{
  xju::Shared<ParseOr> result(new ParseOr);
  if (a->isA<ParseOr>()) {
    ParseOr const& x(a->asA<ParseOr>());
    std::copy(x.terms_.begin(), x.terms_.end(),
              std::back_inserter(result->terms_));
  }
  else {
    result->terms_.push_back(a);
  }
  if (b->isA<ParseOr>()) {
    ParseOr const& x(b->asA<ParseOr>());
    std::copy(x.terms_.begin(), x.terms_.end(),
              std::back_inserter(result->terms_));
  }
  else {
    result->terms_.push_back(b);
  }
  return result;
}

PR operator!(PR x) throw()
{
  return xju::Shared<ParseNot>(new ParseNot(x));
}

PR anon(std::string const& name, PR const x) throw()
{
  return PR(new AnonParser(name,x));
}

PR atLeastOne(PR b) throw()
{
  return anon(
    "at least one occurrance of "+b->target(),
    b+zeroOrMore()*b);
}

PR optional(PR x) throw()
{
  return PR(new Optional(x));
}

PR parseAnyChar() throw()
{
  static PR parseAnyChar(new ParseAnyChar);
  return parseAnyChar;
}

PR parseOneOfChars(std::string const& chars) throw()
{
  return PR(new ParseOneOfChars(chars));
}

PR parseAnyCharExcept(std::string const& chars) throw()
{
  return PR(new ParseAnyCharExcept(chars));
}

PR charInRange(char const min, char const max) throw()
{
  return PR(new ParseCharInRange(min, max));
}

PR parseLiteral(std::string const& x) throw()
{
  return PR(new ParseLiteral(x));
}

PR parseUntil(PR match, PR const x) throw()
{
  return PR(new ParseSpecificUntil(match, x));
}

PR parseUntil(PR const x) throw()
{
  return PR(new ParseUntil(x));
}

PR balanced(PR until, bool angles) throw()
{
  return PR(new ParseBalanced(until, angles));
}

PR whitespaceChar() throw()
{
  static PR whitespaceChar(parseOneOfChars(" \t\n"));
  return whitespaceChar;
}

PR parseHash() throw()
{
  static PR parseHash(new ParseHash);
  return parseHash;
}


PR lineComment() throw()
{
  static PR lineComment(new NamedParser<hcp_ast::LineComment>(
                          "line comment",
                          parseLiteral("//")+
                          parseUntil(parseOneOfChars("\n"))+
                          (zeroOrMore()*whitespaceChar())));
  return lineComment;
}

  
PR blockComment() throw()
{
  static PR blockComment(new NamedParser<hcp_ast::BlockComment>(
                           "block comment",
                           parseLiteral("/*")+
                           parseUntil(parseLiteral("*/"))+
                           parseLiteral("*/")+
                           (zeroOrMore()*whitespaceChar())));
  return blockComment;
}

  
PR comments() throw()
{
  static PR comments(new NamedParser<hcp_ast::Comments>(
                       "comments",
                       atLeastOne(lineComment()|blockComment())));
  return comments;
}

  
// matches nothing or something
PR eatWhite() throw()
{
  static PR eatWhite(zeroOrMore()*(whitespaceChar()|comments()));
  return eatWhite;
}

PR nonBackslashDoubleQuote() throw()
{
  static PR nonBackslashDoubleQuote(new ParseUntil(parseOneOfChars("\\\"")));
  return nonBackslashDoubleQuote;
}

PR nonDoubleQuote() throw()
{
  static PR nonDoubleQuote(new ParseUntil(parseOneOfChars("\"")));
  return nonDoubleQuote;
}

  
PR doubleQuote() throw()
{
  static PR doubleQuote(parseOneOfChars("\""));
  return doubleQuote;
}

PR lessThan() throw()
{
  static PR result(parseOneOfChars("<"));
  return result;
}
PR greaterThan() throw()
{
  static PR result(parseOneOfChars(">"));
  return result;
}

PR doubleColon() throw()
{
  static PR result(parseLiteral("::")+!parseOneOfChars(":"));
  return result;
}

PR backslash() throw()
{
  static PR backslash(parseOneOfChars("\\"));
  return backslash;
}

PR s_chars() throw()
{
  static PR s_chars(new NamedParser<hcp_ast::S_Chars>(
                      "string literal characters",
                      parseUntil(s_char(), doubleQuote())));
  return s_chars;
}


PR stringLiteral() throw()
{
  static PR stringLiteral(new NamedParser<hcp_ast::StringLiteral>(
                            "string literal",
                            atLeastOne(doubleQuote()+
                                       s_chars()+
                                       doubleQuote()+eatWhite())));
  return stringLiteral;
}

//
// to be able to split a combined .h and .cpp (ie a .hcp) file into
// .h and .cpp parts, we need to choose whether each #include goes
// into the .h or the .cpp; we use a simple convention of 
// adding //impl to the #include line, eg
//   #include <x.h> //impl
// ... which indicates #include <x.h> should go in the .cpp not the .h
//
PR hashIncludeImplMarker() throw()
{
  static PR hashIncludeImplMarker(
    parseLiteral("//")+
    (zeroOrMore()*parseOneOfChars(" \t"))+
    parseLiteral("impl")+
    (zeroOrMore()*parseOneOfChars(" \t"))+
    parseLiteral("\n"));
  return hashIncludeImplMarker;
}


PR hashIncludeCommon() throw()
{
  static PR hashIncludeCommon(
    parseHash()+
    (zeroOrMore()*parseOneOfChars(" \t"))+
    parseLiteral("include")+(zeroOrMore()*parseOneOfChars(" \t"))+
    ((lessThan()+PR(new NamedParser<hcp_ast::TargetOfHashInclude>(
                      "target of #include",parseUntil(greaterThan())))+
      greaterThan())|
     (doubleQuote()+PR(new NamedParser<hcp_ast::TargetOfHashInclude>(
                         "target of #include",parseUntil(doubleQuote())))+
      doubleQuote()))+
    parseUntil(parseLiteral("\n")|hashIncludeImplMarker()));
  return hashIncludeCommon;
}


PR hashInclude() throw()
{
  static PR hashInclude(new NamedParser<hcp_ast::HashInclude>(
                          "#include",
                          hashIncludeCommon()+
                          parseOneOfChars("\n")+
                          eatWhite()));
  return hashInclude;
}

  
PR hashIncludeImpl() throw()
{
  static PR hashIncludeImpl(new NamedParser<hcp_ast::HashIncludeImpl>(
                              "#include with //impl marker",
                              hashIncludeCommon()+
                              hashIncludeImplMarker()+
                              eatWhite()));
  return hashIncludeImpl;
}

  
PR hash() throw()
{
  static PR hash(new NamedParser<hcp_ast::OtherPreprocessor>(
                   "other preprocessor directive",
                   parseHash()+
                   parseUntil(parseAnyCharExcept("\\")+parseOneOfChars("\n"))+
                   parseAnyChar()+
                   eatWhite()));
  return hash;
}

  
PR whitespace() throw()
{
  static PR whitespace(new NamedParser<hcp_ast::Whitespace>(
                         "some whitespace",
                         atLeastOne(whitespaceChar())));
  return whitespace;
}

PR identifierStartChar() throw()
{
  static PR result(charInRange('a', 'z')|
                   charInRange('A', 'Z')|
                   parseOneOfChars("_"));
  return result;
}
  
PR identifierContChar() throw()
{
  static PR identifierContChar(charInRange('a', 'z')|
                               charInRange('A', 'Z')|
                               charInRange('0', '9')|
                               parseOneOfChars("_"));
  return identifierContChar;
}


PR identifier() throw()
{
  static PR result(
    identifierStartChar()+
    zeroOrMore()*identifierContChar());
  return result;
}

PR class_struct_union_literal() throw()
{
  static PR class_struct_union_literal(
    (parseLiteral("class")|parseLiteral("struct")|parseLiteral("union"))+
    !identifierContChar());
  return class_struct_union_literal;
}


PR scope_ref() throw(){
  static PR result(
    optional(class_struct_union_literal())+eatWhite()+
    optional(doubleColon()+eatWhite())+
    zeroOrMore()*(
      identifier()+eatWhite()+optional(
        parseOneOfChars("<")+
        balanced(parseOneOfChars(">"), true)+
        parseOneOfChars(">")+eatWhite())+
      doubleColon()+eatWhite()));
  return result;
}

PR typename_keyword() throw()
{
  PR result(
    parseLiteral("typename")+!identifierContChar()+eatWhite());
  return result;
}

PR scoped_name() throw(){ //see also scoped_function_name
  static PR result(
    optional(typename_keyword())+
    scope_ref()+
    identifier()+eatWhite()+optional(
        parseOneOfChars("<")+
        balanced(parseOneOfChars(">"), true)+
        parseOneOfChars(">")+eatWhite()));
  return result;
}

PR bracketed(PR x) throw()
{
  return parseLiteral("(")+eatWhite()+x+parseLiteral(")")+eatWhite();
}

PR typedef_keyword() throw()
{
  static PR typedef_keyword(parseLiteral("typedef")+!identifierContChar());
  return typedef_keyword;
}

PR const_keyword() throw()
{
  static PR result{parseLiteral("const")+!identifierContChar()};
  return result;
}

PR volatile_keyword() throw()
{
  static PR result{parseLiteral("volatile")+!identifierContChar()};
  return result;
}

PR cv() throw()
{
  static PR result {
    anon("const/volatile qualifiers",
         zeroOrMore()*((const_keyword()|volatile_keyword())+eatWhite()))};
  return result;
}

PR type_qual() throw()
{
  static PR result{
    anon("const/volatile/*/& type qualifier",
         (const_keyword()|volatile_keyword()|parseOneOfChars("*&"))+eatWhite())
      };
  return result;
}

PR type_ref() throw()
{
  static PR result{
    anon("type reference",
         cv() +
         type_name() +eatWhite() +
         zeroOrMore()*type_qual()+ eatWhite()+
         (!parseLiteral(".")|(parseLiteral("...")+eatWhite())))
      };
  return result;
}

PR defined_type() throw(){
  static PR result(
    new NamedParser<hcp_ast::DefinedType>(
      "\"defined type\"",
      identifier())+eatWhite());
  return result;
}

PR typedef_non_fp() throw()
{
  static PR typedef_statement(
    typedef_keyword()+
    whitespace()+
    type_ref()+defined_type()+parseOneOfChars(";")+
    eatWhite());
  return typedef_statement;
}

PR using_keyword() throw()
{
  static PR using_keyword(parseLiteral("using")+!identifierContChar());
  return using_keyword;
}


PR using_statement() throw()
{
  static PR using_statement(
    new NamedParser<hcp_ast::Using>(
      "using statement",
      using_keyword()+
      whitespace()+
      balanced(parseOneOfChars(";"))+
      parseOneOfChars(";")+
      eatWhite()));
  return using_statement;
}

  
PR enum_name() throw()
{
  static PR enum_name(
    new NamedParser<hcp_ast::EnumName>(
      "\"enum name\"",
      identifier()));
  return enum_name;
}


PR enum_keyword() throw()
{
  static PR enum_keyword(parseLiteral("enum")+!identifierContChar());
  return enum_keyword;
}


PR scoped_enum_def() throw()
{
  static PR scoped_enum_def(
    new NamedParser<hcp_ast::EnumDef>(
      "scoped enum definition",
      enum_keyword()+whitespace()+(parseLiteral("struct")|
                                   parseLiteral("class"))+
      whitespace()+
      optional(enum_name())+eatWhite()+parseLiteral("{")+
      balanced(parseOneOfChars("}"))+
      parseOneOfChars("}")+eatWhite()+
      parseOneOfChars(";")+
      eatWhite()));
  return scoped_enum_def;
}

PR enum_def() throw()
{
  static PR enum_def(
    new NamedParser<hcp_ast::EnumDef>(
      "enum definition",
      enum_keyword()+
      whitespace()+
      optional(enum_name())+eatWhite()+parseLiteral("{")+
      balanced(parseOneOfChars("}"))+
      parseOneOfChars("}")+eatWhite()+
      parseOneOfChars(";")+
      eatWhite()));
  return enum_def;
}

  
PR bracketed() throw()
{
  static PR result(parseLiteral("(")+balanced(parseLiteral(")"))+parseLiteral(")")+eatWhite());
  return result;
}


PR unqualifiedTypeName() throw()
{
  static PR unqualifiedTypeName(
    identifier()+
    zeroOrMore()*(parseOneOfChars("<")+
                  balanced(parseOneOfChars(">"), true)+
                  parseOneOfChars(">")));
  return unqualifiedTypeName;
}

PR operator_keyword() throw()
{
  static PR operator_keyword(
    parseLiteral("operator")+!identifierContChar()+eatWhite());
  return operator_keyword;
}

PR operator_name() throw()
{
  static PR operator_name(
    anon(
      "operator name",
      scope_ref()+
      operator_keyword()+
      (parseLiteral("()")|
       parseLiteral("<<=")|
       parseLiteral("<<")|
       parseLiteral(">>")|
       parseLiteral(">>=")|
       parseLiteral("==")|
       parseLiteral("!=")|
       parseLiteral("<=")|
       parseLiteral(">=")|
       parseLiteral("<")|
       parseLiteral(">")|
       parseLiteral("++")|
       parseLiteral("+=")|
       parseLiteral("--")|
       parseLiteral("-=")|
       parseLiteral("->")|
       parseLiteral("+")|
       parseLiteral("-")|
       parseLiteral("|=")|
       parseLiteral("&=")|
       parseLiteral("|")|
       parseLiteral("&")|
       parseLiteral("[]")|
       parseLiteral("!")|
       parseLiteral("%=")|
       parseLiteral("%")|
       parseLiteral("=")|
       parseLiteral("*")|
       parseLiteral("*=")|
       parseLiteral("~")|
       parseLiteral("~=")|
       parseLiteral("/")|
       parseLiteral("/=")|
       parseLiteral("new"))+
      eatWhite()));
  return operator_name;
}

PR destructor_name() throw()
{
  static PR destructor_name(
    anon(
      "destructor name",
      scope_ref()+parseLiteral("~")+eatWhite()+identifier()));
  return destructor_name;
}

PR name() throw()
{
  static PR name(
    optional(typename_keyword())+
    optional(doubleColon())+eatWhite()+
    zeroOrMore()*(
      unqualifiedTypeName()+eatWhite()+doubleColon()+eatWhite())+
    (operator_name()|destructor_name()|unqualifiedTypeName())+
    eatWhite());
  return name;
}


PR class_name() throw()
{
  static PR result(
    optional(typename_keyword())+
    optional(doubleColon())+eatWhite()+
    zeroOrMore()*(
      unqualifiedTypeName()+eatWhite()+doubleColon()+eatWhite())+
    unqualifiedTypeName());
  return result;
}

PR built_in_type_name() throw()
{
  static PR result(
    atLeastOne(
      (parseLiteral("char")|
       parseLiteral("char16_t")|
       parseLiteral("char32_t")|
       parseLiteral("wchar_t")|
       parseLiteral("short")|
       parseLiteral("int")|
       parseLiteral("long")|
       parseLiteral("float")|
       parseLiteral("double")|
       parseLiteral("signed")|
       parseLiteral("unsigned"))+!identifierContChar()+eatWhite()));
  return result;
}

PR type_name() throw()
{
  static PR result(
    anon(
      "type name",
      built_in_type_name()|
      scoped_name()));
  return result;
}

PR conversion_operator_name() throw()
{
  static PR conversion_operator_name(
    scope_ref()+
    operator_keyword()+
    type_ref());
  return conversion_operator_name;
}

  
PR keyword_static() throw()
{
  static PR keyword_static(
    PR(new NamedParser<hcp_ast::KeywordStatic>(
         "\"static\"",
         parseLiteral("static")))+!identifierContChar()+eatWhite());
  return keyword_static;
}

PR keyword_extern() throw()
{
  static PR result(
    PR(new NamedParser<hcp_ast::KeywordExtern>(
         "\"extern\"",
         parseLiteral("extern")))+!identifierContChar()+eatWhite());
  return result;
}


PR keyword_inline() throw()
{
  static PR result(
    parseLiteral("inline")+!identifierContChar()+eatWhite());
  return result;
}

PR keyword_mutable() throw()
{
  static PR result(
    parseLiteral("mutable")+!identifierContChar()+eatWhite());
  return result;
}

PR keyword_friend() throw()
{
  static PR keyword_friend(
    new NamedParser<hcp_ast::KeywordFriend>(
      "\"friend\"",
      parseLiteral("friend"))+!identifierContChar()+eatWhite());
  return keyword_friend;
}

PR keyword_virtual() throw()
{
  static PR keyword_virtual(
    new NamedParser<hcp_ast::KeywordVirtual>(
      "\"virtual\"",
      parseLiteral("virtual"))+!identifierContChar()+eatWhite());
  return keyword_virtual;
}

PR keyword_explicit() throw()
{
  static PR keyword_explicit(
    new NamedParser<hcp_ast::KeywordExplicit>(
      "\"explicit\"",
      parseLiteral("explicit"))+!identifierContChar()+eatWhite());
  return keyword_explicit;
}

PR keyword_override() throw()
{
  static PR result(
    new NamedParser<hcp_ast::KeywordExplicit>(
      "\"override\"",
      parseLiteral("override"))+!identifierContChar()+eatWhite());
  return result;
}

PR keyword_noexcept() throw()
{
  static PR result(
    new NamedParser<hcp_ast::KeywordExplicit>(
      "\"noexcept\"",
      parseLiteral("noexcept"))+!identifierContChar()+eatWhite());
  return result;
}

PR keyword_throw() throw()
{
  static PR result(
    new NamedParser<hcp_ast::KeywordThrow>(
      "\"throw\"",
      parseLiteral("throw"))+!identifierContChar()+eatWhite());
  return result;
}

PR throw_clause() throw()
{
  static PR result(
    (keyword_throw()+bracketed())|
    (keyword_noexcept()+optional(bracketed())));
  return result;
}

PR function_qualifiers() throw()
{
  static PR function_qualifiers(
    new NamedParser<hcp_ast::FunctionQualifiers>(
      "function qualifiers",
      zeroOrMore()*((keyword_virtual()|
                     keyword_explicit()|
                     keyword_friend()|
                     keyword_static()|
                     keyword_inline())+eatWhite())));
  return function_qualifiers;
}


PR function_post_qualifiers() throw()
{
  static PR result(
    new NamedParser<hcp_ast::FunctionQualifiers>(
      "function post-qualifiers",
      cv()+
      zeroOrMore()*(keyword_override()|
                    throw_clause())));
  return result;
}


PR block_open() throw()
{
  static PR result(new NamedParser<hcp_ast::BlockOpen>(
                    "block open",
                    parseLiteral("{")));
  return result;
}

PR block() throw()
{
  static PR block(new NamedParser<hcp_ast::Block>(
                    "block",
                    block_open()+
                    balanced(parseOneOfChars("}"))+
                    parseLiteral("}")));
  return block;
}


PR init_list() throw()
{
  static PR init_list(new NamedParser<hcp_ast::InitList>(
                        "initialiser list",
                        parseLiteral(":")+
                        balanced(parseOneOfChars("{;:"))));
  return init_list;
}

PR keyword_catch() throw()
{
  static PR result(
    new NamedParser<hcp_ast::KeywordCatch>(
      "\"catch\"",
      parseLiteral("catch"))+!identifierContChar()+eatWhite());
  return result;
}

PR catch_block() throw()
{
  static PR catch_block(keyword_catch()+bracketed()+block());
  return catch_block;
}


PR function_impl() throw()
{
  static PR function_impl(
    new NamedParser<hcp_ast::FunctionImpl>(
      "function implementation",
      eatWhite()+
      (zeroOrMore()*(parseLiteral("try")+eatWhite())+
       zeroOrMore()*(init_list()+eatWhite())+
       block()+
       zeroOrMore()*(eatWhite()+catch_block()))));
  return function_impl;
}

                
PR params() throw();

PR template_keyword() throw()
{
  static PR template_keyword(parseLiteral("template")+!identifierContChar()+eatWhite());
  return template_keyword;
}


PR template_empty_preamble() throw()
{
  static PR template_empty_preamble(
    new NamedParser<hcp_ast::TemplateEmptyPreamble>(
      "template empty preamble",
      template_keyword()+
      parseOneOfChars("<")+
      eatWhite()+
      parseOneOfChars(">")+
      eatWhite()));
  return template_empty_preamble;
}


PR template_preamble() throw()
{
  static PR template_preamble(
    new NamedParser<hcp_ast::TemplatePreamble>(
      "template preamble",
      !template_empty_preamble()+(
        template_keyword()+
        parseOneOfChars("<")+
        balanced(parseOneOfChars(">"), true)+
        parseOneOfChars(">")+
        eatWhite())));
  return template_preamble;
}

PR conversion_operator_function_proto() throw()
{
  static PR result(
    function_qualifiers()+
    PR(new NamedParser<hcp_ast::FunctionName>(
         "conversion operator name",
         conversion_operator_name()))+
    eatWhite()+
    bracketed(params()));
  return result;
}

PR typed_function_proto() throw()
{
  static PR result(
    function_qualifiers()+
    type_ref()+
    PR(new NamedParser<hcp_ast::FunctionName>(
         "function name",
         operator_name()|
         scoped_name()))+
    eatWhite()+
    bracketed(params()));
  return result;
}

PR untyped_function_proto() throw()
{
  static PR result(
    function_qualifiers()+
    PR(new NamedParser<hcp_ast::FunctionName>(
         "function name",
         destructor_name()|
         operator_name()|
         scoped_name()))+
    eatWhite()+
    bracketed(params()));
  return result;
}

PR function_initialiser() throw()
{
  static PR result(
    anon("function initialier",
         parseLiteral("=")+balanced(parseLiteral(";"))));
  return result;
}

PR function_proto() throw()
{
  static PR result(
    anon(
      "function proto",
      (conversion_operator_function_proto()|
       typed_function_proto()|
       untyped_function_proto())+
      //balanced((eatWhite()+parseOneOfChars(";:{"))|parseLiteral("try"))
      function_post_qualifiers()+
      (!parseLiteral("=")|function_initialiser())
      ));
  return result;
}

PR function_decl() throw()
{
  static PR function_decl(new NamedParser<hcp_ast::FunctionDecl>(
                            "function declaration",
                            optional(template_empty_preamble()|
                                     template_preamble())+
                            function_proto()+
                            (eatWhite()+parseOneOfChars(";"))+
                            eatWhite()));
  return function_decl;
}

  
PR function_def_unnamed() throw()
{
  static PR result(
    function_proto()+
    function_impl()+
    new NamedParser<hcp_ast::WhiteSpace>("whitespace",eatWhite()));
  return result;
}

PR function_def() throw()
{
  static PR function_def(
    new NamedParser<hcp_ast::FunctionDef>(
      "non-template function definition",
      optional(template_empty_preamble())+
      function_def_unnamed()));
  return function_def;
}

  
PR template_function_def() throw()
{
  static PR template_function_def(
    new NamedParser<hcp_ast::TemplateFunctionDef>(
      "template function definition",
      atLeastOne(template_preamble())+
      function_def_unnamed()));
  return template_function_def;
}

  
PR not_class_struct_union_literal() throw()
{
  static PR not_class_struct_union_literal(
    !class_struct_union_literal());
  return not_class_struct_union_literal;
}


PR class_proto() throw()
{
  static PR class_proto(
    zeroOrMore()*(template_preamble()|template_empty_preamble())+
    optional(keyword_friend())+
    class_struct_union_literal()+
    whitespace()+
    new NamedParser<hcp_ast::ClassName>(
      "class name",
      class_name())+
    eatWhite()+
    balanced(parseOneOfChars("{;")));
  return class_proto;
}


PR class_decl() throw()
{
  static PR class_decl(new NamedParser<hcp_ast::ClassForwardDecl>(
                         "class forward-declaration",
                         class_proto()+
                         parseOneOfChars(";")+
                         eatWhite()));
  return class_decl;
}

  
PR var_name() throw()
{
  static PR var_name(new NamedParser<hcp_ast::VarName>(
                       "var name",
                       identifier())+eatWhite());
  return var_name;
}


PR array_decl() throw()
{
  static PR array_decl(
    parseOneOfChars("[")+
    balanced(parseOneOfChars("]"))+
    parseOneOfChars("]")+
    eatWhite());
  return array_decl;
}


PR var_intro() throw()
{
  static PR var_intro(
    balanced(whitespaceChar()+var_name()+
             optional(array_decl())+eatWhite()+
             parseOneOfChars("=;"))+
    whitespaceChar()+var_name()+optional(array_decl())+eatWhite());
  return var_intro;
}


PR static_var_intro() throw()
{
  static PR static_var_intro(
    keyword_static()+
    eatWhite()+
    var_intro());
  return static_var_intro;
}

PR var_initialiser_open() throw()
{
  static PR result(
    new NamedParser<hcp_ast::VarInitialiserOpen>(
      "variable initialiser '='",
      parseOneOfChars("=")));
  return result;
}
    
PR var_initialiser_1() throw()
{
  static PR var_initialiser(
    new NamedParser<hcp_ast::VarInitialiser>(
      "variable initialiser",
      (var_initialiser_open()+balanced(parseOneOfChars(");,")))));
  return var_initialiser;
}

PR var_initialiser_open_2() throw()
{
  PR result{
    new NamedParser<hcp_ast::VarInitialiserOpen>(
      "variable initialiser '{'",
      parseOneOfChars("{"))};
  return result;
}
    
    
PR var_initialiser_2() throw()
{
  static PR result{
    new NamedParser<hcp_ast::VarInitialiser>(
      "variable initialiser",
      (var_initialiser_open_2()+balanced(parseOneOfChars("}"))+
       parseOneOfChars("}")))};
  return result;
}

PR var_initialiser() throw()
{
  static PR result{
    anon("var initialiser",
         var_initialiser_1()|
         var_initialiser_2())};
  return result;
}

PR var_non_fp() throw()
{
  static PR result{
    anon(
      "non-function pointer var",
      (type_ref()+var_name()+optional(array_decl())+
       (!parseOneOfChars("={")|var_initialiser())))};
  return result;
}

PR var_fp() throw();

struct VarFpBackref : public Parser
{
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    return var_fp()->parse_(at,o);
  }
  virtual std::string target() const throw() {
    return "function pointer var (backref)";
  }
};
PR var_fp_backref() throw()
{
  static PR result{new VarFpBackref};
  return result;
}
    
PR param() throw()
{
  static PR result(
    !parseLiteral(")")+
    (var_non_fp()|
     var_fp_backref()|
     type_ref())+
    eatWhite());
  return result;
}
  
PR params() throw()
{
  static PR result {
    anon(
      "params",
      optional(param()+
               zeroOrMore()*(parseLiteral(",")+eatWhite()+param()))+
      eatWhite())};
  return result;
}
  
PR var_fp() throw()
{
  static PR result{
    anon("function pointer var",
         type_ref()+
         bracketed(
           scope_ref()+cv()+parseLiteral("*")+eatWhite()+cv()+
           var_name())+
         bracketed(params())+
         function_post_qualifiers()+
         (!parseOneOfChars("={")|var_initialiser())+
         eatWhite())};
  return result;
}

PR typedef_fp() throw()
{
  PR result(
    anon("typedef of function pointer",
         typedef_keyword()+
         whitespace()+
         type_ref()+
         bracketed(
           scope_ref()+cv()+parseLiteral("*")+eatWhite()+cv()+
           defined_type())+
         bracketed(params())+
         function_post_qualifiers()+
         eatWhite()+
         parseOneOfChars(";")+eatWhite()));
  return result;
}

PR typedef_statement() throw()
{
  static PR result(
    new NamedParser<hcp_ast::Typedef>(
      "typedef statement",
      typedef_fp()|
      typedef_non_fp()));
  return result;
}


PR var_def() throw()
{
  static PR result{
    optional(keyword_mutable())+
    (var_non_fp()|var_fp())+
    eatWhite()};
  return result;
}
    
PR global_var_def() throw()
{
  static PR global_var_def{
    new NamedParser<hcp_ast::GlobalVarDef>(
      "global variable definition",
      var_def()+
      parseLiteral(";")+
      eatWhite())};
  return global_var_def;
}

PR static_var_def() throw()
{
  static PR static_var_def(new NamedParser<hcp_ast::StaticVarDef>(
                             "static variable definition",
                             keyword_static()+
                             eatWhite()+
                             var_def()+
                             parseLiteral(";")+
                             eatWhite()));
  return static_var_def;
}
  
PR extern_var_def() throw()
{
  static PR result(new NamedParser<hcp_ast::ExternVarDef>(
                     "extern variable definition",
                     keyword_extern()+
                     eatWhite()+
                     var_def()+
                     parseLiteral(";")+
                     eatWhite()));
  return result;
}
  
PR access_modifier() throw()
{
  static PR access_modifier(new NamedParser<hcp_ast::AccessModifier>(
                              "public/private/protected: marker",
                              (parseLiteral("public")|
                               parseLiteral("private")|
                               parseLiteral("protected"))+
                              eatWhite()+
                              parseOneOfChars(":")+
                              eatWhite()));
  return access_modifier;
}


PR not_typedef_using_enum_keyword() throw()
{
  static PR not_typedef_using_enum_keyword(
    !(typedef_keyword()|using_keyword()|enum_keyword()));
  return not_typedef_using_enum_keyword;
}


namespace
{
class SelfParser : public Parser
{
public:
  Parser& self_;
  
  explicit SelfParser(Parser& self) throw():
    self_(self) {
  }

  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    return self_.parse_(at, o);
  }

  // Parser::
  virtual std::string target() const throw() {
    return self_.target();
  }
};

class ParseClass : public Parser
{
public:
  PR x_;
  PR tp_;
  PR p_;
  
  ParseClass() throw():
  x_(class_proto()+
       parseOneOfChars("{")+
       eatWhite()+
       PR(new NamedParser<hcp_ast::ClassMembers>(
            "class members",
            parseUntil(comments()|
                       access_modifier()|
                       PR(new SelfParser(*this))|
                       class_decl()|
                       (not_class_struct_union_literal()+(
                         typedef_statement()|
                         scoped_enum_def()|
                         enum_def()|
                         (not_typedef_using_enum_keyword()+(
                           function_decl()|
                           template_function_def()|
                           function_def()|
                           static_var_def()|
                           extern_var_def()|
                           global_var_def())))),
                       parseOneOfChars("}"))))+
       parseOneOfChars("}")+
       eatWhite()+
       parseOneOfChars(";")+
       eatWhite()),
    tp_(new NamedParser<hcp_ast::TemplateClassDef>(
      "template class definition",
      template_preamble()+
      x_)),
    p_(new NamedParser<hcp_ast::ClassDef>(
      "non-template class definition",
      x_))
  {
  }
       
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    return (tp_|p_)->parse_(at, o);
  }

  // Parser::
  virtual std::string target() const throw() {
    return "class definition";
  }
  
};
}

PR class_def() throw()
{
  static PR class_def(new ParseClass);
  return class_def;
}

PR not_function_proto_or_template_function_proto() throw()
{
  static PR result(
    anon("not function proto or template function proto",
         !(function_proto()|(template_preamble()+function_proto()))));
  return result;
}

PR namespace_leaf() throw()
{
  static PR namespace_leaf(
    comments()|
    hashIncludeImpl()|
    hashInclude()|
    hash()|
    class_def()| // note recursive
    class_decl()|
    (not_class_struct_union_literal()+(
      typedef_statement()|
      using_statement()|
      scoped_enum_def()|
      enum_def()|
      (not_typedef_using_enum_keyword()+(
        whitespace()| //REVISIT: remove?
        function_decl()| // inc. template
        template_function_def()|
        function_def()|
        (not_function_proto_or_template_function_proto()+
         (static_var_def()|
          extern_var_def()|
          global_var_def())))))));
  return namespace_leaf;
}


PR namespace_keyword() throw()
{
  static PR namespace_keyword(parseLiteral("namespace")+!identifierContChar());
  return namespace_keyword;
}


PR not_namespace_keyword() throw()
{
  static PR not_namespace_keyword(!namespace_keyword());
  return not_namespace_keyword;
}


PR anonymous_namespace() throw()
{
  static PR anonymous_namespace(
    new NamedParser<hcp_ast::AnonymousNamespace>(
      "anonymous namespace",
      namespace_keyword()+
      eatWhite()+
      parseOneOfChars("{")+
      eatWhite()+
      parseUntil(namespace_leaf(), parseOneOfChars("}"))+
      parseOneOfChars("}")+
      eatWhite()));
  return anonymous_namespace;
}

  
namespace
{
class ParseNamespace : public NamedParser<hcp_ast::NamespaceDef>
{
public:
  PR x_;
  
  ParseNamespace() throw():
    NamedParser<hcp_ast::NamespaceDef>(
      "namespace",(
        namespace_keyword()+
        whitespace()+
        new NamedParser<hcp_ast::NamespaceName>(
          "namespace name",
          identifier())+
        eatWhite()+
        parseOneOfChars("{")+
        eatWhite()+
        new NamedParser<hcp_ast::NamespaceMembers>(
          "namespace members",
          parseUntil((PR(new SelfParser(*this))|
                      anonymous_namespace()|
                      (not_namespace_keyword()+namespace_leaf()))+
                     eatWhite(),
                     parseOneOfChars("}")))+
        parseOneOfChars("}")+
        eatWhite())) {
  }
};

}

PR namespace_def() throw()
{
  static PR namespace_def(new ParseNamespace);
  return namespace_def;
}

  
namespace
{
class ParseEndOfFile : public Parser
{
public:
  // Parser::
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    if (!at.atEnd()) {
      return ParseResult(
        Exception(
          xju::Shared<Exception::Cause>(
            new NotEndOfInput(*at)), at, XJU_TRACED));
    }
    return ParseResult(
      std::make_pair(
        IRs(1U, new hcp_ast::EndOfFile(IRs(1U, new hcp_ast::String(at, at)))),
        at));
  }


  // Parser::
  virtual std::string target() const throw() {
    return "end of file";
  }

  class NotEndOfInput : public Exception::Cause
  {
  public:
    NotEndOfInput(char const got) throw():
        got_(got)
    {
    }
    ~NotEndOfInput() throw()
    {
    }
    std::string str() const throw()
    {
      std::ostringstream s;
      s << "expected end of input, not '" << printChar(got_) << "'";
      return s.str();
    }
    char const got_;
  };

};
}

PR endOfFile() throw()
{
  static PR endOfFile(new ParseEndOfFile);
  return endOfFile;
}

  
PR file() throw()
{
  static PR file(new NamedParser<hcp_ast::File>(
                   "file",
                   eatWhite()+
                   parseUntil(namespace_def()|
                              anonymous_namespace()|
                              (not_namespace_keyword()+namespace_leaf()),
                              endOfFile())+
                   endOfFile()));
  return file;
}

I parse(hcp_ast::CompositeItem& parent,
        I const startOfElement,
        xju::Shared<Parser> elementType,
        bool traceToStdout,
        bool irsAtEnd)
  throw(
    // post: parent unmodified
    xju::Exception)
{
  try {
    Options options(traceToStdout,
                    Cache(new hcp_parser::CacheVal()),
                    irsAtEnd);
    ParseResult const r(elementType->parse(startOfElement, options));
    if (r.failed()) {
      throw r.e();
    }
    PV const x(*r);
    std::copy(x.first.begin(), 
              x.first.end(), 
              std::back_inserter(parent.items_));
    return x.second;
  }
  catch(Exception const& e) {
    throw hcp::translateException(e);
  }
}


}
