#ifndef DMP_SKELETON_HPP
#define DMP_SKELETON_HPP

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "../Object.hpp"
#include "Pose.hpp"

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

  class Bone
  {
  public:
    Bone() = delete;
    Bone(const Bone &) = delete;
    Bone & operator=(const Bone &) = delete;
    Bone(Bone &&) = default;
    Bone & operator=(Bone &&) = default;

    Bone(Balljoint * bj,
         bool * dirty,
         std::vector<dmp::Object *> & objs,
         std::vector<dmp::Object *> & objCache,
         size_t matIdx,
         size_t texIdx,
         std::vector<glm::mat4>::iterator & invBBegin,
         std::vector<glm::mat4>::iterator & invBEnd);

    void update(float deltaT, glm::mat4 M, bool dirty,
                std::vector<glm::mat4> & Ms);
  private:
    void initBone(Balljoint * bj,
                  bool * dirty,
                  std::vector<dmp::Object *> & objs,
                  std::vector<dmp::Object *> & objCache,
                  size_t matIdx,
                  size_t texIdx,
                  std::vector<glm::mat4>::iterator & invBBegin,
                  std::vector<glm::mat4>::iterator & invBEnd);

    std::unique_ptr<Object> mObj;
    std::vector<std::unique_ptr<Bone>> mChildren;

    glm::mat4 mInvB;
    glm::mat4 mOffset;
    std::function<bool(glm::mat4 &, float)> mTransformFn;
    glm::mat4 mRotation;
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

    void makeBones(std::vector<Object *> & objs,
                   size_t matIdx,
                   size_t texIdx,
                   std::vector<glm::mat4>::iterator & invBBegin,
                   std::vector<glm::mat4>::iterator & invBEnd);

    ~Skeleton() {}

    Balljoint * getAST()
    {
      expect("Skeleton AST not null", mRoot);
      return mAST.get();
    }

    void show();
    void hide();

    void update(float deltaT, glm::mat4 M, bool dirty);

    bool isDirty() const {return mDirty;}
    static std::function<bool(glm::mat4 &, float)> makeXformFn(dmp::Balljoint * bj,
                                                               bool * dirty);
    const std::vector<glm::mat4> & getMs() const;
    void applyPose(const Pose & p);
  private:
    std::unique_ptr<Bone> mRoot;
    std::unique_ptr<Balljoint> mAST;
    void initSkeleton(std::string skelPath);

    std::unique_ptr<Balljoint> parse(std::string currName,
                                     std::string currCtor,
                                     std::vector<std::string>::iterator & begin,
                                     std::vector<std::string>::iterator & end);

    std::vector<Object *> mSkeletonObjects;
    std::vector<glm::mat4> mMs;
    bool mDirty = true;
  };
}

#endif
