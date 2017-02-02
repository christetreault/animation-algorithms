#ifndef DMP_MORPH_HPP
#define DMP_MORPH_HPP

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "parsing.hpp"

namespace dmp
{
  class Morph
  {
  public:
    std::map<size_t, glm::vec3> normals;
    std::map<size_t, glm::vec3> verts;

    Morph() = default;
    Morph(const std::string & path) {initMorph(path);}
    Morph(const Morph & lhs,
          const Morph & rhs,
          float t);

    operator bool() {return mValid;}
    void initNullMorph(const std::vector<glm::vec3> & verts,
                       const std::vector<glm::vec3> & normals,
                       Morph * morphs,
                       size_t length);
  private:
    bool mValid = false;
    void initMorph(const std::string & path);

    void parsePositions(TokenIterator & iter,
                        TokenIterator & end);
    void parseNormals(TokenIterator & iter,
                      TokenIterator & end);
  };
}

#endif
