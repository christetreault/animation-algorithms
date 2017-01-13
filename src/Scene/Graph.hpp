#ifndef DMP_SCENE_GRAPH_HPP
#define DMP_SCENE_GRAPH_HPP

#include <boost/variant.hpp>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include "Object.hpp"
#include "Camera.hpp"

namespace dmp
{
  class Node
  {
  public:
    virtual ~Node() {}
    void update(float deltaT = 0.0f,
                glm::mat4 M = glm::mat4(),
                bool dirty = false) {updateImpl(deltaT, M, dirty);}
  private:
    virtual void updateImpl(float deltaT, glm::mat4 M, bool dirty) = 0;
  };

  class Branch;
  class Container;
  class Transform;

  class ContainerVisitor : public boost::static_visitor<>
  {
  public:
    ContainerVisitor(float deltaT, glm::mat4 M, bool dirty)
      : mDeltaT(deltaT), mM(M), mDirty(dirty) {}

    void operator()(Object & obj) const;
    void operator()(CameraPos & cam) const;
    void operator()(CameraFocus & cam) const;
    void operator()(Light & lit) const;

    float mDeltaT;
    glm::mat4 mM;
    bool mDirty;
  };

  class Container : public Node
  {
  public:
    Container() = delete;
    Container(Object & obj) : mValue(obj) {}
    Container(CameraPos & cam) : mValue(cam) {}
    Container(CameraFocus & cam) : mValue(cam) {}
    Container(Light & lit) : mValue(lit) {}

  private:
    void updateImpl(float deltaT, glm::mat4 M, bool dirty) override;
    boost::variant<Object &, CameraPos &, CameraFocus &, Light &> mValue;
  };

  static auto noTransform = [] (glm::mat4 &, float) {return false;};

  class Transform : public Node
  {
  public:
    // CONTRACT: If the update function does not make a change, then it
    // MUST return false. If it makes a change, then it MUST return true.
    std::function<bool(glm::mat4 &, float)> mUpdateFn = noTransform;
    glm::mat4 mTransform;
    std::unique_ptr<Node> mChild = nullptr;

    Container * insert(Object & o);
    Container * insert(Light & l);
    Container * insert(CameraPos & c);
    Container * insert(CameraFocus & c);
    Node * insert(std::unique_ptr<Node> & n);
    Transform * insert(std::unique_ptr<Transform> & t);
    Branch * insert(std::unique_ptr<Branch> & b);
    Container * insert(std::unique_ptr<Container> & c);

    Transform * transform();
    Transform * transform(glm::mat4);
    Transform * transform(std::function<bool(glm::mat4 &, float)>);
    Transform * transform(glm::mat4, std::function<bool(glm::mat4 &, float)>);
    Branch * branch();
  private:
    void updateImpl(float deltaT, glm::mat4 M, bool inDirty) override
    {
      // If outDirty == false, then it must be the case that mTransform is not
      // changed.
      bool outDirty = mUpdateFn(mTransform, deltaT) | inDirty;
      glm::mat4 outM = M * mTransform;
      if (mChild) mChild->update(deltaT, outM, outDirty);
    }
  };

  class Branch : public Node
  {
  public:
    std::vector<std::unique_ptr<Node>> mChildren;

    Node * insert(std::unique_ptr<Node> & n);
    Transform * insert(std::unique_ptr<Transform> & t);
    Branch * insert(std::unique_ptr<Branch> & b);
    Container * insert(std::unique_ptr<Container> & c);
    Container * insert(Object & o);
    Container * insert(Light & l);
    Container * insert(CameraPos & c);
    Container * insert(CameraFocus & c);

    Transform * transform();
    Transform * transform(glm::mat4);
    Transform * transform(std::function<bool(glm::mat4 &, float)>);
    Transform * transform(glm::mat4, std::function<bool(glm::mat4 &, float)>);
  private:
    void updateImpl(float deltaT, glm::mat4 M, bool dirty) override
    {
      for (auto & curr : mChildren)
        {
          curr->update(deltaT, M, dirty);
        }
    }
  };
}

#endif
