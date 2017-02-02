#include "Morph.hpp"
#include <fstream>
#include <iostream>

static const char * tokPositions = "positions";
static const char * tokNormals = "normals";

void dmp::Morph::parsePositions(TokenIterator & iter,
                                TokenIterator & end)
{
  auto t = [](Morph &,
              TokenIterator &,
              TokenIterator &)
    {
      // do nothing
    };

  auto f = [](Morph & data,
              TokenIterator & iter,
              TokenIterator & end)
    {
      int idx;
      float x, y, z;
      parseInt(idx, iter, end);
      parseVec3(x, y, z, iter, end);
      data.verts.insert(std::make_pair(idx, glm::vec3(x, y, z)));
    };

  dmp::parseField<dmp::Morph>(*this, iter, end, tokPositions, t, f);
}

void dmp::Morph::parseNormals(TokenIterator & iter,
                              TokenIterator & end)
{
  auto t = [](Morph &,
              TokenIterator &,
              TokenIterator &)
    {
      // do nothing
    };

  auto f = [](Morph & data,
              TokenIterator & iter,
              TokenIterator & end)
    {
      int idx;
      float x, y, z;
      parseInt(idx, iter, end);
      parseVec3(x, y, z, iter, end);
      data.normals.insert(std::make_pair(idx, glm::vec3(x, y, z)));
    };

  dmp::parseField<dmp::Morph>(*this, iter, end, tokNormals, t, f);
}

// -----------------------------------------------------------------------------
// End of parsing functions
// -----------------------------------------------------------------------------

void dmp::Morph::initMorph(const std::string & path)
{
  std::cerr << "morph file: " << path << std::endl;
  std::string dataStr;
  readFile(path, dataStr);

  auto sep = whitespaceSeparator;

  boost::tokenizer<boost::char_separator<char>> tokens(dataStr, sep);

  auto iter = tokens.begin();
  auto end = tokens.end();

  while (iter != end)
    {
      if (*iter == tokPositions)
        {
          parsePositions(iter, end);
        }
      else if (*iter == tokNormals)
        {
           parseNormals(iter, end);
        }
      else
        {
          std::cerr << "at top level: " << *iter << std::endl;
          impossible("toplevel token not positions or normals");
        }
    }
  mValid = true;
}

void dmp::Morph::initNullMorph(const std::vector<glm::vec3> & verts,
                               const std::vector<glm::vec3> & normals,
                               Morph * morphs,
                               size_t length)
{
  expect("verts and normals have same length",
         verts.size() == normals.size());

  for (size_t currMorph = 0; currMorph < length; ++currMorph)
    {
      for (size_t i = 0; i < verts.size(); ++i)
        {
          if (this->verts.find(i) != this->verts.end()) continue;

          auto vIter = morphs[currMorph].verts.find(i);
          auto nIter = morphs[currMorph].normals.find(i);

          expect("Both vert and norm found, or neither found",
                 (vIter == morphs[currMorph].verts.end()
                  && nIter == morphs[currMorph].normals.end())
                 || (vIter != morphs[currMorph].verts.end()
                     && nIter != morphs[currMorph].normals.end()));
          if (vIter != morphs[currMorph].verts.end())
            {
              this->verts.insert(std::make_pair(i, verts[i]));
              this->normals.insert(std::make_pair(i, normals[i]));
            }
        }
    }
  mValid = true;
}

dmp::Morph::Morph(const Morph & lhs,
                  const Morph & rhs,
                  float t)
{
  expect("Morph lerp ctor must have 0 <= t <= 1",
         0.0f <= t && t <= 1.0f);

  auto conflictFn = [&](const glm::vec3 & l,
                        const glm::vec3 & r)
    {
      return glm::mix(l, r, t);
    };

  mapUnion<size_t, glm::vec3>(lhs.normals,
                              rhs.normals,
                              conflictFn,
                              normals);

  mapUnion<size_t, glm::vec3>(lhs.verts,
                              rhs.verts,
                              conflictFn,
                              verts);
}
