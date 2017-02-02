#include "parsing.hpp"
#include <fstream>
#include <iostream>

void dmp::parseVec3(float & x, float & y, float & z,
                    TokenIterator & iter,
                    TokenIterator & end)
{
  x = stof(*iter);
  safeIncr(iter, end); // advance to y
  y = stof(*iter);
  safeIncr(iter, end); // advance to z
  z = stof(*iter);
  safeIncr(iter, end); // advance to x or }
}

void dmp::parseVec2(float & x, float & y,
                    TokenIterator & iter,
                    TokenIterator & end)
{
  x = stof(*iter);
  safeIncr(iter, end); // advance to y
  y = stof(*iter);
  safeIncr(iter, end); // advance to x or }
}

void dmp::parseInt(int & i,
                   TokenIterator & iter,
                   TokenIterator & end)
{
  i = stoi(*iter);
  safeIncr(iter, end);
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
