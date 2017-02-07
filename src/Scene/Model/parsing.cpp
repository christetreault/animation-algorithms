#include "parsing.hpp"
#include <fstream>
#include <iostream>

void dmp::parseVec3(float & x, float & y, float & z,
                    TokenIterator & iter,
                    TokenIterator & end)
{
  try
    {
      expect("iter not end", iter != end);
      x = stof(*iter);
      safeIncr(iter, end); // advance to y
      y = stof(*iter);
      safeIncr(iter, end); // advance to z
      z = stof(*iter);
      safeIncr(iter, end); // advance to x or }
    }
  catch (std::invalid_argument & e)
    {
      throw InvariantViolation(e);
    }
}

void dmp::parseVec2(float & x, float & y,
                    TokenIterator & iter,
                    TokenIterator & end)
{
  try
    {
      expect("iter not end", iter != end);
      x = stof(*iter);
      safeIncr(iter, end); // advance to y
      y = stof(*iter);
      safeIncr(iter, end); // advance to x or }
    }
  catch (std::invalid_argument & e)
    {
      throw InvariantViolation(e);
    }
}

void dmp::parseInt(int & i,
                   TokenIterator & iter,
                   TokenIterator & end)
{
  try
    {
      expect("iter not end", iter != end);
      i = stoi(*iter);
      safeIncr(iter, end);
    }
  catch (std::invalid_argument & e)
    {
      throw InvariantViolation(e);
    }
}

void dmp::parseString(std::string & s,
                      TokenIterator & iter,
                      TokenIterator & end)
{
  expect("iter not end", iter != end);
  s = *iter;
  safeIncr(iter, end);
}

void dmp::parseToken(std::string & out,
                     const std::string & toMatch,
                     TokenIterator & iter,
                     TokenIterator & end)
{
  expect("iter not end", iter != end);
  if (*iter == toMatch)
    {
      out = toMatch;
      safeIncr(iter, end);
    }
}

void dmp::tryParse(std::function<void(TokenIterator &, TokenIterator &)> p,
                   TokenIterator & beg,
                   TokenIterator & end)
{
  auto bc = beg;
  auto ec = end;
  try
    {
      p(bc, ec); // try the parse
      beg = bc; // if successful, commit beg and end
      end = ec;
      return;
    }
  catch (InvariantViolation & e)
    {
      return; // if unsuccessful, do not consume input
    }
}

void dmp::orParse(std::list<std::function<void(TokenIterator &,
                                               TokenIterator &)>> ps,
                  TokenIterator & beg,
                  TokenIterator & end)
{
  auto bc = beg;
  for (auto curr : ps)
    {
      curr(beg, end);
      if (bc != beg) return;
    }
}

void dmp::allParse(std::list<std::function<void(TokenIterator &,
                                                TokenIterator &)>> ps,
                   TokenIterator & beg,
                   TokenIterator & end)
{
  auto bc = beg;
  while (!ps.empty())
    {
      auto psBeg = ps.begin();
      auto psEnd = ps.end();
      for(;psBeg != psEnd; ++psBeg)
        {
          tryParse(*psBeg, beg, end);
          if (bc != beg)
            {
              ps.erase(psBeg);
              bc = beg;
              break;
            }
        }
      expect("All parsers succeed", ps.empty())
        }
}

std::list<std::function<void(dmp::TokenIterator &,
                             dmp::TokenIterator &)>>
dmp::someParse(std::list<std::function<void(TokenIterator &,
                                            TokenIterator &)>> ps,
               TokenIterator & beg,
               TokenIterator & end)
{
  auto bc = beg;
  while (!ps.empty())
    {
      auto psBeg = ps.begin();
      auto psEnd = ps.end();
      for(;psBeg != psEnd; ++psBeg)
        {
          tryParse(*psBeg, beg, end);
          if (bc != beg)
            {
              ps.erase(psBeg);
              break;
            }
        }
      if (bc != beg) // a parser succeeded
        {
          bc = beg;
          continue;
        }
      break; // no more parsers will succeed
    }
  return ps;
}

// -----------------------------------------------------------------------------
// read a file. TODO: this probalby doesn't belong here
// -----------------------------------------------------------------------------

void dmp::readFile(const std::string & path,
                   std::string & buffer)
{
  std::ifstream fin(path, std::ios::in);
  fin.seekg(0, std::ios_base::end);
  size_t endPos = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  std::vector<char> data(endPos);
  fin.read(data.data(), endPos);
  fin.close();

  buffer = std::string(data.data(), endPos);
  return;
}
