#include "Graph.hpp"

using namespace dmp;

// -----------------------------------------------------------------------------
// ContainerVisitor
// -----------------------------------------------------------------------------

void ContainerVisitor::operator()(Object & obj) const
{
  obj.setM(mM);
}

void ContainerVisitor::operator()(CameraPos & cam) const
{
  cam.setM(mM);
}

void ContainerVisitor::operator()(CameraFocus & cam) const
{
  cam.setM(mM);
}

void ContainerVisitor::operator()(Light & lit) const
{
  lit.M = mM;
}

void ContainerVisitor::operator()(Cloth & clo) const
{
  clo.update(mM, mDeltaT);
}

// -----------------------------------------------------------------------------
// Container
// -----------------------------------------------------------------------------

void Container::updateImpl(float deltaT, glm::mat4 M, bool dirty)
{
  if (dirty
      || mValue.which() == 1) // mValue.which() == 1 -> Cloth. Should always
    {                         // update because wind, gravity, etc...
      boost::apply_visitor(ContainerVisitor(deltaT, M, dirty), mValue);
    }
}

// -----------------------------------------------------------------------------
// Transform
// -----------------------------------------------------------------------------

Object * Transform::insert(Object o)
{
  mChild = std::make_unique<Container>(o);
  return &(boost::get<Object>(((Container *) mChild.get())->mValue));
}

Cloth * Transform::insert(Cloth c)
{
  mChild = std::make_unique<Container>(std::move(c));
  return &(boost::get<Cloth>(((Container *) mChild.get())->mValue));
}

Light * Transform::insert(Light & l)
{
  mChild = std::make_unique<Container>(l);
  return &(boost::get<Light &>(((Container *) mChild.get())->mValue));
}

CameraPos * Transform::insert(CameraPos & c)
{
  mChild = std::make_unique<Container>(c);
  return &(boost::get<CameraPos &>(((Container *) mChild.get())->mValue));
}

CameraFocus * Transform::insert(CameraFocus & c)
{
  mChild = std::make_unique<Container>(c);
  return &(boost::get<CameraFocus>(((Container *) mChild.get())->mValue));
}

Node * Transform::insert(std::unique_ptr<Node> & n)
{
  mChild = std::move(n);
  return mChild.get();
}

Transform * Transform::insert(std::unique_ptr<Transform> & t)
{
  mChild = std::move(t);
  return (Transform *) mChild.get();
}

Transform * Transform::transform()
{
  auto p = std::make_unique<Transform>();
  return insert(p);
}

Transform * Transform::transform(glm::mat4 t)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mUpdateFn = noTransform;
  return insert(p);
}

Transform * Transform::transform(TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Transform::transform(Quaternion q)
{
  auto p = std::make_unique<Transform>();
  p->mQuatRotation = q;
  return insert(p);
}

Transform * Transform::transform(glm::mat4 t,
                                 TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Transform::transform(Quaternion q,
                                 TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mUpdateFn = f;
  p->mQuatRotation = q;
  return insert(p);
}

Transform * Transform::transform(Quaternion q,
                                 glm::mat4 t)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mQuatRotation = q;
  return insert(p);
}

Transform * Transform::transform(Quaternion q,
                                 glm::mat4 t,
                                 TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mQuatRotation = q;
  p->mUpdateFn = f;
  return insert(p);
}

Branch * Transform::insert(std::unique_ptr<Branch> & b)
{
  mChild = std::move(b);
  return (Branch *) mChild.get();
}

Branch * Transform::branch()
{
  auto p = std::make_unique<Branch>();
  return insert(p);
}

Container * Transform::insert(std::unique_ptr<Container> & c)
{
  mChild = std::move(c);
  return (Container *) mChild.get();
}

void dmp::Transform::updateImpl(float deltaT, glm::mat4 M, bool inDirty)
{
  // If outDirty == false, then it must be the case that mTransform is not
  // changed.
  expect("updateFn not null", mUpdateFn);
  bool outDirty = mUpdateFn(mTransform, mQuatRotation, deltaT) || inDirty;
  glm::mat4 outM = M * mTransform * ((glm::mat4) mQuatRotation);
  if (mChild) mChild->update(deltaT, outM, outDirty);
}

// -----------------------------------------------------------------------------
// Branch
// -----------------------------------------------------------------------------

Node * Branch::insert(std::unique_ptr<Node> & n)
{
  mChildren.push_back(std::move(n));
  return mChildren.back().get();
}

Transform * Branch::insert(std::unique_ptr<Transform> & t)
{
  mChildren.push_back(std::move(t));
  return (Transform *) mChildren.back().get();
}

Transform * Branch::transform(glm::mat4 t)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mUpdateFn = noTransform;
  return insert(p);
}

Transform * Branch::transform(Quaternion q)
{
  auto p = std::make_unique<Transform>();
  p->mQuatRotation = q;
  return insert(p);
}

Transform * Branch::transform(TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Branch::transform(glm::mat4 t,
                              TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Branch::transform(Quaternion q,
                              glm::mat4 t)
{
  auto p = std::make_unique<Transform>();
  p->mQuatRotation = q;
  p->mTransform = t;
  return insert(p);
}

Transform * Branch::transform(Quaternion q,
                              TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mQuatRotation = q;
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Branch::transform(Quaternion q,
                              glm::mat4 t,
                              TransformFn f)
{
  auto p = std::make_unique<Transform>();
  p->mQuatRotation = q;
  p->mTransform = t;
  p->mUpdateFn = f;
  return insert(p);
}

Branch * Branch::insert(std::unique_ptr<Branch> & b)
{
  mChildren.push_back(std::move(b));
  return (Branch *) mChildren.back().get();
}

Container * Branch::insert(std::unique_ptr<Container> & c)
{
  mChildren.push_back(std::move(c));
  return (Container *) mChildren.back().get();
}

Object * Branch::insert(Object o)
{
  mChildren.push_back(std::make_unique<Container>(o));
  return &(boost::get<Object>(((Container *) mChildren.back().get())->mValue));
}

Light * Branch::insert(Light & l)
{
  mChildren.push_back(std::make_unique<Container>(l));
  return &(boost::get<Light &>(((Container *) mChildren.back().get())->mValue));
}

CameraPos * Branch::insert(CameraPos & c)
{
  mChildren.push_back(std::make_unique<Container>(c));
  return &(boost::get<CameraPos &>(((Container *) mChildren.back().get())->mValue));
}

CameraFocus * Branch::insert(CameraFocus & c)
{
  mChildren.push_back(std::make_unique<Container>(c));
  return &(boost::get<CameraFocus &>(((Container *) mChildren.back().get())->mValue));
}

Cloth * Branch::insert(Cloth c)
{
  mChildren.push_back(std::make_unique<Container>(std::move(c)));
  return &(boost::get<Cloth>(((Container *) mChildren.back().get())->mValue));
}
