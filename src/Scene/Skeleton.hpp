#ifndef DMP_SKELETON_HPP
#define DMP_SKELETON_HPP

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "Object.hpp"
#include "Graph.hpp"

namespace dmp
{
  struct Balljoint
  {
    std::string name;

    float offsetx = 0.0f;
    float offsety = 0.0f;
    float offsetz = 0.0f;
    float boxminx = -0.1f;
    float boxminy = -0.1f;
    float boxminz = -0.1f;
    float boxmaxx = 0.1f;
    float boxmaxy = 0.1f;
    float boxmaxz = 0.1f;
    float rotxmin = -std::numeric_limits<float>::infinity();
    float rotymin = -std::numeric_limits<float>::infinity();
    float rotzmin = -std::numeric_limits<float>::infinity();
    float rotxmax = std::numeric_limits<float>::infinity();
    float rotymax = std::numeric_limits<float>::infinity();
    float rotzmax = std::numeric_limits<float>::infinity();
    float posex = 0.0f;
    float posey = 0.0f;
    float posez = 0.0f;

    std::vector<std::unique_ptr<Balljoint>> children;

    bool rotateDirty = true;
  };

  class Skeleton
  {
  public:
    Skeleton() = delete;
    Skeleton(const Skeleton &) = delete;
    Skeleton & operator=(const Skeleton &) = delete;
    Skeleton(Skeleton &&) = default;
    Skeleton & operator=(Skeleton &&) = default;

    Skeleton(std::string skelPath)
    {
      initSkeleton(skelPath);
    }

    void insertInScene(Branch * graph,
                       std::vector<Object *> & objs,
                       size_t matIdx,
                       size_t texIdx);

    ~Skeleton() {}

    Balljoint * getAST()
    {
      expect("Skeleton AST not null", mRoot);
      return mRoot.get();
    }

  private:
    static void insertInSceneImpl(dmp::Balljoint * bj,
                                  dmp::Branch * graph,
                                  std::vector<dmp::Object *> & objs,
                                  size_t matIdx,
                                  size_t texIdx);
    std::unique_ptr<Balljoint> mRoot;
    void initSkeleton(std::string skelPath);

    std::unique_ptr<Balljoint> parse(std::string currName,
                                     std::string currCtor,
                                     std::vector<std::string>::iterator & begin,
                                     std::vector<std::string>::iterator & end);
    static std::function<bool(glm::mat4 &, float)> makeXformFn(dmp::Balljoint * bj);
  };
}

#endif
