#ifndef DMP_PARSING_HPP
#define DMP_PARSING_HPP

#include <boost/tokenizer.hpp>
#include <functional>
#include <list>
#include "../../util.hpp"

namespace dmp
{
  typedef boost::token_iterator_generator<boost::char_separator<char>>::type TokenIterator;
  static const boost::char_separator<char> whitespaceSeparator(" \t\r\n");

  void parseVec3(float & x, float & y, float & z,
                 TokenIterator & iter,
                 TokenIterator & end);

  void parseVec2(float & x, float & y,
                 TokenIterator & iter,
                 TokenIterator & end);

  void parseInt(int & i,
                TokenIterator & iter,
                TokenIterator & end);

  void parseFloat(float & f,
                  TokenIterator & iter,
                  TokenIterator & end);

  void parseString(std::string & s,
                   TokenIterator & iter,
                   TokenIterator & end);

  template <typename T>
  void parseField(T & data,
                  TokenIterator & iter,
                  TokenIterator & end,
                  const char * fieldName,
                  std::function<void(T &,
                                     TokenIterator &,
                                     TokenIterator &)> tagFn,
                  std::function<void(T &,
                                     TokenIterator &,
                                     TokenIterator &)> parseFn)
  {
    static const char * const tokOpenBrace = "{";
    static const char * const tokCloseBrace = "}";
    expect("iterator not equal to end", iter != end);
    expect("parsing correct field", *iter == fieldName);

    safeIncr(iter, end); // swallow fieldName
    tagFn(data, iter, end);
    safeIncr(iter, end); // advance to {
    expect("field tag followed by opening brace", *iter == tokOpenBrace);
    safeIncr(iter, end); // swallow {

    while (*iter != tokCloseBrace)
      {
        parseFn(data, iter, end);
      }

  safeIncr(iter, end); // swallow }
  }

  // Try to parse using p. If fail, swallow error and do not consume input
  void tryParse(std::function<void(TokenIterator &, TokenIterator &)> p,
                TokenIterator & beg,
                TokenIterator & end);

  // Try parsers until one either consumes input or throws. Throws if
  // no parser succeeds
  void orParse(std::list<std::function<void(TokenIterator &,
                                            TokenIterator &)>> ps,
               TokenIterator & beg,
               TokenIterator & end);

  // Try all parsers until they either all succeed, or one throws
  void allParse(std::list<std::function<void(TokenIterator &,
                                             TokenIterator &)>> ps,
                TokenIterator & beg,
                TokenIterator & end);

  // Repeatedly run p until it stops consuming input or beg == end
  void manyParse(std::function<void(TokenIterator &, TokenIterator &)> p,
                 TokenIterator & beg,
                 TokenIterator & end);

  // Try all parsers until no more succeed. Return list contains failures
  std::list<std::function<void(TokenIterator &,
                               TokenIterator &)>>
  someParse(std::list<std::function<void(TokenIterator &,
                                TokenIterator &)>> ps,
            TokenIterator & beg,
            TokenIterator & end);

  void parseToken(std::string & out,
                  const std::string & toMatch,
                  TokenIterator & beg,
                  TokenIterator & end);

  // read the file at path into buffer.
  // contents of buffer will be clobbered.
  void readFile(const std::string & path,
                std::string & buffer);
}

#endif
