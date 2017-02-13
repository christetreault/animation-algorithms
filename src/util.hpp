#ifndef DMP_UTIL_HPP
#define DMP_UTIL_HPP

#include <exception>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <limits>
#include <GL/glew.h>
#include <limits>
#include <math.h>
#include <map>
#include <boost/assert.hpp>

// Exectue a statement IFF built in release mode (NDEBUG is definend)
// define ifRelease
#ifndef ifRelease // if (not defined ifRelease)

#ifdef NDEBUG    // if (not debug defined)
#define ifRelease(_e)                           \
  {                                             \
    _e;                                         \
  }
#else
#define ifRelease(_e) {}
#endif // endif (not debug defined)

#else
#error ifRelease already defined!
#endif // endif (not defined ifRelease)
// end define ifRelease

// Execute a statement IFF built in debug mode (NDEBUG not defined)
// define ifDebug
#ifndef ifDebug // if (not defined ifDebug)

#ifndef NDEBUG    // if (not debug not defined)
#define ifDebug(_e)                             \
  {                                             \
    _e;                                       \
  }
#else
#define ifDebug(_e) {}
#endif // endif (not debug not defined)

#else
#error ifDebug already defined!
#endif // endif (not defined ifDebug)
// end define ifDebug

#ifndef expect
#define expect(_msg, _e)                                                \
  {                                                                     \
    if(!(_e)) throw dmp::InvariantViolation("Truth Assertion Failed: " _msg, __FILE__, __LINE__); \
  }
#else
#error expect already defined!
#endif

#ifndef safeIncr
#define safeIncr(_begin, _end)                                          \
  {                                                                     \
    auto & _evalBegin = (_begin);                                       \
    auto & _evalEnd = (_end);                                           \
    expect("Iterator not equal to end", _evalBegin != _evalEnd);        \
    ++_evalBegin;                                                       \
  }
#else
#error safeIncr already defined!
#endif

#ifndef unreachable
#define unreachable(_msg)                                                \
  {                                                                      \
    throw dmp::InvariantViolation("Unreachable code executed, this should be impossible: " _msg, __FILE__, __LINE__); \
  }
#else
#error unreachable already defined!
#endif

#ifndef todo
#define todo(_msg)                                                \
  {                                                                          \
    throw dmp::InvariantViolation("Feature not implemented! This should never happen: " _msg, __FILE__, __LINE__); \
  }
#else
#error todo already defined!
#endif

#ifndef impossible
#define impossible(_msg)                                                \
  {                                                                          \
    throw dmp::InvariantViolation("This should never happen: " _msg, __FILE__, __LINE__); \
  }
#else
#error impossible already defined!
#endif

namespace dmp
{
  class InvariantViolation : public std::exception
  {
  public:
    InvariantViolation(std::string msg, const char * file, int line)
    {
      mMsg = std::string(msg
                         + "\n\nIn File: "
                         + file
                         + "\nAt Line: "
                         + std::to_string(line));
    }

    InvariantViolation(const std::vector<InvariantViolation> & vs)
    {
      mMsg = "Multiple Violations!\n";

      for (const auto & curr : vs)
        {
          mMsg += curr.mMsg + "\n";
        }
    }

    InvariantViolation(const std::exception & e)
    {
      mMsg = "Rethrowing std::exception: "
        + std::string(e.what());
    }

    InvariantViolation(std::string msg)
    {
      mMsg = msg;
    }

    const char * what() const noexcept
    {
      return mMsg.c_str();
    }

    std::string mMsg;
  };

  // 1) filter unsuitable elements of elems using suitablePred
  // 2) pick most ideal element of the filtered elems using
  // idealPreds where idealPreds is a list of predicates sorted
  // from most to least desirable
  // 3) if none of the idealPreds match, an arbitrary element from
  // the filtered elems is selected
  template <typename T>
  T select(std::vector<T> elems,
           std::function<bool(T &)> suitablePred,
           std::vector<std::function<bool(T &)>> idealPreds = {})
  {
    auto unsuitablePred = [&](T & t) {return !(suitablePred(t));};
    // eliminate unsuitable elements from the vector
    elems.erase(std::remove_if(elems.begin(),
                               elems.end(),
                               unsuitablePred),
                elems.end());

    // if elems is now empty, then the invariant that elems must
    // contain at least one suitable element was violated
    expect("There were no suitable elements in elems!",
           elems.size() > 0);

    // see if there is an ideal element, if so pick it
    for (const auto & curr : idealPreds)
      {
        auto attempt = std::find_if(elems.begin(),
                                    elems.end(),
                                    curr);

        if (attempt != elems.end()) return *attempt;
      }

    // otherwise, just pick whatever element. It wasn't unsuitable,
    // so by definition it should be acceptable
    return elems[0];
  }

  // returns true if there is at least 1 element in l and one element
  // in r for which cmp(l, r) returns true.
  template <typename T>
  bool hasCommonElement(std::vector<T> l,
                        std::vector<T> r,
                        std::function<bool(T, T)> cmp = std::equal_to<T>())
  {
    for (const auto & lhs : l)
      {
        for (const auto & rhs : r)
          {
            if (cmp(lhs, rhs)) return true;
          }
      }

    return false;
  }

  //queries all opengl errors. Blows up if there are any errors.
  inline void expectNoErrors(std::string header = "")
  {
    ifRelease(return);

    std::vector<std::string>  msgs = {"Attempting: " + header + "\n"};;
    for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
      {
        switch(err)
          {
          case GL_NO_ERROR:
            unreachable("GL_NO_ERROR encountered in expectNoErrors loop!");
            break;
          case GL_INVALID_ENUM:
            msgs.push_back("GL_INVALID_ENUM");
            break;
          case GL_INVALID_VALUE:
            msgs.push_back("GL_INVALID_VALUE");
            break;
          case GL_INVALID_OPERATION:
            msgs.push_back("GL_INVALID_OPERATION");
            break;
          case GL_STACK_OVERFLOW:
            msgs.push_back("GL_STACK_OVERFLOW");
            break;
          case GL_STACK_UNDERFLOW:
            msgs.push_back("GL_STACK_UNDERFLOW");
            break;
          case GL_OUT_OF_MEMORY:
            msgs.push_back("GL_OUT_OF_MEMORY");
            break;
          case GL_INVALID_FRAMEBUFFER_OPERATION:
            msgs.push_back("GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
          case GL_CONTEXT_LOST:
            msgs.push_back("GL_CONTEXT_LOST");
            break;
          case GL_TABLE_TOO_LARGE:
            msgs.push_back("GL_TABLE_TOO_LARGE");
            break;
          default:
            std::string unkErr = "Unknown OpenGL error type occurred: ";
            unkErr = unkErr + std::to_string(err);
            msgs.push_back(unkErr);
          }
      }

    if (msgs.size() == 1) return;

    std::string msg = "OpenGL error(s) occurred: \n";
    for (const auto & curr : msgs)
      {
        msg += curr;
        msg += "\n";
      }

    throw dmp::InvariantViolation(msg);
  }

  inline bool roughEq(float lhs, float rhs,
                      float epsilon = std::numeric_limits<float>::epsilon())
  {
    return ((float) fabs((double)(lhs - rhs))
            < epsilon);
  }

  template <typename K, typename T>
  void mapUnion(const std::map<K, T> & lhs,
                const std::map<K, T> & rhs,
                std::function<T(const T & l, const T & r)> conflictFn,
                std::map<K, T> & resMap)
  {
    for (const auto & curr : lhs)
      {
        if (rhs.find(curr.first) == rhs.end())
          {
            // lhs element is not in rhs. Take it
            resMap.insert(curr);
          }
        else
          {
            // rhs map has an element at the same key as this lhs
            // element. Call the conflictFn
            resMap.insert(std::make_pair(curr.first,
                                         conflictFn(curr.second,
                                                    rhs.at(curr.first))));
          }
      }

    // At this point, resMap contains:
    // - all elements of lhs that are not in rhs
    // - all elements that are in both lhs and rhs, resolved by conflictFn

    for (const auto & curr : rhs)
      {

        if (resMap.find(curr.first) != resMap.end())
          {
            // if resMap contains the current key, that means that this element
            // had a corresponding element in lhs, and the conflictFn
            // was called, skip this element
            continue;
          }
        else
          {
            // If resMap does not contain the current key, that means that this
            // key only appears in rhs. Take it.
            resMap.insert(curr);
          }
      }

    // Union should be complete at this point
  }
}

#define BOOST_ENABLE_ASSERT_HANDLER
namespace boost
{
  inline void assertion_failed(char const * expr,
                        char const * function,
                        char const * file,
                        long line)
  {
    std::string msg = std::string("Boost assert failure! Expression: "
                                  + std::string(expr)
                                  + " Function: " + std::string(function));
    throw dmp::InvariantViolation(msg, file, (int) line);
  }
}

#endif
