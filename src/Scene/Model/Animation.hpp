#ifndef DMP_ANIMATION_HPP
#define DMP_ANIMATION_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Pose.hpp"
#include "../../util.hpp"
#include <boost/variant.hpp>

namespace dmp
{
  enum class Tangent
  {
    flat, linear, smooth
  };

  struct Keyframe
  {
  public:
    float time;
    float value;
    boost::variant<float, Tangent> tangentIn;
    boost::variant<float, Tangent> tangentOut;

    glm::vec4 cubicCoefficients = {};
    float invLerpRhs = 0.0f;

    float getTangentIn() const
    {
      expect("tangentIn evaluated", tangentIn.which() == 0);
      return boost::get<float>(tangentIn);
    }

    float getTangentOut() const
    {
      expect("tangentOut evaluated", tangentOut.which() == 0);
      return boost::get<float>(tangentOut);
    }
  };

  enum class Extrapolation
  {
    constant, linear, cycle, cycleOffset, bounce
  };

  struct ChannelData
  {
    Extrapolation extrapIn;
    Extrapolation extrapOut;
    std::vector<Keyframe> keyframes;
  };

  class Channel
  {
  public:
    Channel() = delete;
    Channel(const Channel &) = delete;
    Channel & operator=(const Channel &) = delete;
    Channel(Channel &&) = default;
    Channel & operator=(Channel &&) = default;

    Channel(const ChannelData & cd);
    void precompute();
    float evaluate(float t);
    void printChannel();
  private:
    float evaluateImpl(float t);
    void computeTangents();
    void computeCubicCoefficients();
    ChannelData mData;
  };

  class Animation
  {
  public:
    Animation() = delete;
    Animation(const Animation &) = delete;
    Animation & operator=(const Animation &) = delete;
    Animation(Animation &&) = default;
    Animation & operator=(Animation &&) = default;

    Animation(const std::string & path);

    Pose evaluate(float t);
  private:
    void printAnimation();
    void initAnimation(const std::string & path);
    float mRangeBegin;
    float mRangeEnd;
    std::vector<Channel> mChannels;
  };
}

#endif
