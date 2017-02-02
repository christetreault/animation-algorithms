#ifndef DMP_PARSING_HPP
#define DMP_PARSING_HPP

#include <boost/tokenizer.hpp>
#include <functional>
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

  // read the file at path into buffer.
  // contents of buffer will be clobbered.
  void readFile(const std::string & path,
                std::string & buffer);
}

#endif
