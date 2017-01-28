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

void ContainerVisitor::operator()(Model & mod) const
{
  mod.update(mDeltaT, mM, mDirty);
}

// -----------------------------------------------------------------------------
// Container
// -----------------------------------------------------------------------------

void Container::updateImpl(float deltaT, glm::mat4 M, bool dirty)
{
  if (dirty)
    {
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

Model * Transform::insert(Model m)
{
  mChild = std::make_unique<Container>(std::move(m));
  return &(boost::get<Model>(((Container *) mChild.get())->mValue));
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

Transform * Transform::transform(std::function<bool(glm::mat4 &, float)> f)
{
  auto p = std::make_unique<Transform>();
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Transform::transform(glm::mat4 t,
                                 std::function<bool(glm::mat4 &, float)> f)
{
  auto p = std::make_unique<Transform>();
  p->mTransform = t;
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

Transform * Branch::transform(std::function<bool(glm::mat4 &, float)> f)
{
  auto p = std::make_unique<Transform>();
  p->mUpdateFn = f;
  return insert(p);
}

Transform * Branch::transform(glm::mat4 t,
                              std::function<bool(glm::mat4 &, float)> f)
{
  auto p = std::make_unique<Transform>();
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

Model * Branch::insert(Model m)
{
  mChildren.push_back(std::make_unique<Container>(std::move(m)));
  return &(boost::get<Model>(((Container *) mChildren.back().get())->mValue));
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
