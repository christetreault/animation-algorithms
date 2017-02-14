#ifndef DMP_ANIMATION_HPP
#define DMP_ANIMATION_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Pose.hpp"
#include "../../util.hpp"
#include "../../Renderer/Shader.hpp"
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

    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLsizei drawCount = 0;
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

  #define CURVE_TYPE 0
  #define TAN_IN_TYPE 1
  #define TAN_OUT_TYPE 2
  struct ChannelVertex
  {
    glm::vec2 pos;
    int type;
  };

  class Channel
  {
  public:
    Channel() = delete;
    Channel(const Channel &) = delete;
    Channel & operator=(const Channel &) = delete;
    Channel(Channel &&) = default;
    Channel & operator=(Channel &&) = default;

    ~Channel()
    {
      if (mVAO != 0) glDeleteVertexArrays(1, &mVAO);
      if (mVBO != 0) glDeleteBuffers(1, &mVBO);

      for (auto & curr : mData.keyframes)
        {
          if (curr.mVAO != 0) glDeleteVertexArrays(1, &curr.mVAO);
          if (curr.mVBO != 0) glDeleteBuffers(1, &curr.mVBO);
        }
    }

    Channel(const ChannelData & cd);
    void precompute();
    float evaluate(float t);
    void printChannel();
    void draw();
  private:
    float evaluateImpl(float t);
    void computeTangents();
    void computeCubicCoefficients();
    ChannelData mData;
    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLsizei drawCount = 0;
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

    int nextCurveIndex(int prev);
    int prevCurveIndex(int next);
    void drawCurveIndex(int idx);
    void printChannel(size_t idx);
  private:
    void printAnimation();
    void initAnimation(const std::string & path);
    float mRangeBegin;
    float mRangeEnd;
    std::vector<Channel> mChannels;
    Shader mShaderProg;
  };
}

#endif
