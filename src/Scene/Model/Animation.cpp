#include "Animation.hpp"
#include "parsing.hpp"
#include <iostream>
#include <list>
#include <cmath>
#include <glm/gtx/string_cast.hpp>
#include "../../config.hpp"
#include <glm/glm.hpp>

static const std::string tokRange = "range";
static const std::string tokNumChannels = "numchannels";
static const std::string tokChannel = "channel";
static const std::string tokAnimation = "animation";
static const std::string tokOpenBrace = "{";
static const std::string tokCloseBrace = "}";
static const std::string tokExtrapolate = "extrapolate";
static const std::string tokExtrapolateConstant = "constant";
static const std::string tokExtrapolateLinear = "linear";
static const std::string tokExtrapolateCycle = "cycle";
static const std::string tokExtrapolateCycleOffset = "cycle_offset";
static const std::string tokExtrapolateBounce = "bounce";
static const std::string tokKeys = "keys";
static const std::string tokTangentFlat = "flat";
static const std::string tokTangentLinear = "linear";
static const std::string tokTangentSmooth = "smooth";

dmp::Channel::Channel(const ChannelData & cd)
{
  mData = cd;
}

void dmp::Channel::precompute()
{
  computeTangents();
  computeCubicCoefficients();

  //std::cerr << "opengl stuff" << std::endl;

  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);

  expectNoErrors("Gen vao/vbo");

  std::vector<ChannelVertex> verts(0);

  for (float t = -5.0f; t < 4.9f; t = t + 0.02f)
  {
    verts.push_back({glm::vec2(t , evaluate(t)), CURVE_TYPE});
    verts.push_back({glm::vec2(t + 0.02f, evaluate(t + 0.02f)), CURVE_TYPE});
  }

  //for (const auto & curr : verts)
  // {
  //   std::cerr << glm::to_string(curr.pos) << std::endl;
       //expect("curr x in clip space", curr.pos.x >= -1.0f && curr.pos.x <= 1.0f);
       //expect("curr y in clip space", curr.pos.y >= -1.0f && curr.pos.y <= 1.0f);
  // }


  drawCount = (GLsizei) verts.size();
  glBindVertexArray(mVAO);
  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (verts.size() * sizeof(ChannelVertex)),
               verts.data(), GL_STATIC_DRAW);

  expectNoErrors("Fill VBO");

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ChannelVertex),
                        (GLvoid *) 0);

  glEnableVertexAttribArray(1);
  glVertexAttribIPointer(1,
                         1,
                         GL_INT,
                         sizeof(ChannelVertex),
                         (GLvoid *) offsetof(ChannelVertex, type));

  expectNoErrors("Set vertex attributes");

  for (auto & curr : mData.keyframes)
    {
      std::vector<ChannelVertex> verts(4);
      //std::cerr << "tangents" << std::endl;
      verts[0] = {glm::vec2((curr.time-0.1f),
                            curr.value + curr.getTangentIn() * -0.1f), TAN_IN_TYPE};
      //std::cerr << glm::to_string(verts[0].pos) << std::endl;
      verts[1] = {glm::vec2(curr.time,
                            (curr.value)), TAN_IN_TYPE};
      //std::cerr << glm::to_string(verts[1].pos) << std::endl;
      verts[2] = {glm::vec2(curr.time,
                            (curr.value)), TAN_OUT_TYPE};
      //std::cerr << glm::to_string(verts[2].pos) << std::endl;
      verts[3] = {glm::vec2((curr.time+0.1f),
                            curr.value + curr.getTangentOut() * 0.1f), TAN_OUT_TYPE};
      //std::cerr << glm::to_string(verts[3].pos) << std::endl;

       // for (auto & v : verts)
       //   {
       //     v.pos = v.pos / 5.0f;
       //   }

      expect("|verts| = 4", verts.size() == 4);
      curr.drawCount = (GLsizei) verts.size();
      glGenVertexArrays(1, &curr.mVAO);
      glGenBuffers(1, &curr.mVBO);

      expectNoErrors("Gen vao/vbo");

      glBindVertexArray(curr.mVAO);
      glBindBuffer(GL_ARRAY_BUFFER, curr.mVBO);
      glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (verts.size() * sizeof(ChannelVertex)),
                   verts.data(), GL_STATIC_DRAW);

      expectNoErrors("Fill VBO");

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0,
                            2,
                            GL_FLOAT,
                            GL_FALSE,
                            sizeof(ChannelVertex),
                            (GLvoid *) 0);

      glEnableVertexAttribArray(1);
      glVertexAttribIPointer(1,
                             1,
                             GL_INT,
                             sizeof(ChannelVertex),
                             (GLvoid *) offsetof(ChannelVertex, type));

      expectNoErrors("Set vertex attributes");
    }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void parseChannel(dmp::ChannelData & cd,
                         dmp::TokenIterator & b,
                         dmp::TokenIterator & e)
{
  if (*b != tokChannel) return;

  safeIncr(b, e); // swallow channel
  safeIncr(b, e); // swallow {

  using namespace dmp;
  auto parseExtrapolate = [&cd](auto & beg, auto & end)
    {
      if (*beg == tokExtrapolate)
        {
          safeIncr(beg, end);
          expect("beg != end after \"extrapolate\"", beg != end);

          if(*beg == tokExtrapolateBounce)
            {
              cd.extrapIn = Extrapolation::bounce;
            }
          else if(*beg == tokExtrapolateConstant)
            {
              cd.extrapIn = Extrapolation::constant;
            }
          else if(*beg == tokExtrapolateCycle)
            {
              cd.extrapIn = Extrapolation::cycle;
            }
          else if(*beg == tokExtrapolateCycleOffset)
            {
              cd.extrapIn = Extrapolation::cycleOffset;
            }
          else if(*beg == tokExtrapolateLinear)
            {
              cd.extrapIn = Extrapolation::linear;
            }
          else impossible("Invalid extrapolation in literal");

          safeIncr(beg, end);
          expect("beg != end after extrapIn", beg != end);

          if(*beg == tokExtrapolateBounce)
            {
              cd.extrapOut = Extrapolation::bounce;
            }
          else if(*beg == tokExtrapolateConstant)
            {
              cd.extrapOut = Extrapolation::constant;
            }
          else if(*beg == tokExtrapolateCycle)
            {
              cd.extrapOut = Extrapolation::cycle;
            }
          else if(*beg == tokExtrapolateCycleOffset)
            {
              cd.extrapOut = Extrapolation::cycleOffset;
            }
          else if(*beg == tokExtrapolateLinear)
            {
              cd.extrapOut = Extrapolation::linear;
            }
          else impossible("Invalid extrapolation in literal");

          safeIncr(beg, end);
        }
    };

  auto parseKeys = [&cd](auto & b, auto & e)
    {
      if (*b == tokKeys)
        {
          auto t = [](auto & data,
                      auto & beg,
                      auto & end)
          {
            auto size = stoi(*beg);
            expect("size not negative", size >= 0);
            data.keyframes.resize((size_t) size);
          };

          auto f = [](auto & data,
                      auto & beg,
                      auto & end)
          {
            for (auto & curr : data.keyframes)
              {
                float time, value;
                boost::variant<float, Tangent> tangentIn, tangentOut;
                parseVec2(time, value, beg, end);

                auto parseFlatIn = [&tangentIn](auto & b, auto & e)
                {
                  if (*b == tokTangentFlat)
                    {
                      tangentIn = Tangent::flat;
                      safeIncr(b, e);
                    }
                };

                auto parseFlatOut = [&tangentOut](auto & b, auto & e)
                {
                  if (*b == tokTangentFlat)
                    {
                      tangentOut = Tangent::flat;
                      safeIncr(b, e);
                    }
                };

                auto parseLinearIn = [&tangentIn](auto & b, auto & e)
                {
                  if (*b == tokTangentLinear)
                    {
                      tangentIn = Tangent::linear;
                      safeIncr(b, e);
                    }
                };

                auto parseLinearOut = [&tangentOut](auto & b, auto & e)
                {
                  if (*b == tokTangentLinear)
                    {
                      tangentOut = Tangent::linear;
                      safeIncr(b, e);
                    }
                };

                auto parseSmoothIn = [&tangentIn](auto & b, auto & e)
                {
                  if (*b == tokTangentSmooth)
                    {
                      tangentIn = Tangent::smooth;
                      safeIncr(b, e);
                    }
                };

                auto parseSmoothOut = [&tangentOut](auto & b, auto & e)
                {
                  if (*b == tokTangentSmooth)
                    {
                      tangentOut = Tangent::smooth;
                      safeIncr(b, e);
                    }
                };

                auto parseValueIn = [&tangentIn](auto & b, auto & e)
                {
                  float v;
                  parseFloat(v, b, e);
                  tangentIn = v;
                };

                auto parseValueOut = [&tangentOut](auto & b, auto & e)
                {
                  float v;
                  parseFloat(v, b, e);
                  tangentOut = v;
                };

                orParse({parseFlatIn, parseLinearIn, parseSmoothIn, parseValueIn},
                        beg, end);

                orParse({parseFlatOut, parseLinearOut, parseSmoothOut, parseValueOut},
                        beg, end);
                curr = {time, value, tangentIn, tangentOut};
              }
          };

          parseField<ChannelData>(cd, b, e, tokKeys.c_str(), t, f);
        }
    };

  allParse({parseExtrapolate, parseKeys}, b, e);
  safeIncr(b, e); // swallow }
}

void dmp::Channel::computeCubicCoefficients()
{
  // INVARIANT: computeTangents must be called before this!

  static const glm::mat4 hermite =
    {
        2.0f, -2.0f,  1.0f,  1.0f,
       -3.0f,  3.0f, -2.0f, -1.0f,
        0.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f
    };

  auto & kf = mData.keyframes;
  for (size_t i = 0; i < (kf.size() - 1); ++i)
    {
      auto deltaT = kf[i + 1].time - kf[i].time;
      expect("time delta not 0", deltaT != 0.0f);
      expect("tangentOut[i] rule evaluated", kf[i].tangentOut.which() == 0);
      expect("tangentIn[i+1] rule evaluated", kf[i+1].tangentIn.which() == 0);
      glm::vec4 rhs =
        {
          kf[i].value,
          kf[i + 1].value,
          deltaT * boost::get<float>(kf[i].tangentOut),
          deltaT * boost::get<float>(kf[i+1].tangentIn)
        };
      kf[i].cubicCoefficients = glm::transpose(hermite) * rhs;
      kf[i].invLerpRhs = 1.0f / deltaT;
    }

  // INVARIANT: the last tangent's cubicCoefficients and invLerpRhs are not
  // not valid and should never be used
}

void dmp::Channel::computeTangents()
{
  auto & kf = mData.keyframes;

  if (kf.size() == 1)
    {
      if (kf[0].tangentIn.which() == 1) kf[0].tangentIn = 0.0f;
      if (kf[0].tangentOut.which() == 1) kf[0].tangentOut = 0.0f;
      return; // that was easy
    }

  auto computeLinearIn = [](const Keyframe & prev,
                            const Keyframe & curr)
  {
    auto rhs = curr.time - prev.time;
    expect("time delta not 0", rhs != 0.0f);
    return ((curr.value - prev.value)/rhs);
  };

  auto computeLinearOut = [&computeLinearIn](const Keyframe & curr,
                                             const Keyframe & next)
    {
      return computeLinearIn(curr, next);
    };

  auto computeSmooth = [&computeLinearIn](const Keyframe & prev,
                                          const Keyframe & next)
    {
      return computeLinearIn(prev, next);
    };

  for (size_t i = 1; i < (kf.size() - 1); ++i) // do all interior keyframes
    { // There should be no special cases in here
      auto & in = kf[i].tangentIn;
      auto & out = kf[i].tangentOut;

      if (in.which() == 1)
        {
          switch (boost::get<Tangent>(in))
            {
            case Tangent::flat:
              in = 0.0f;
              break;
            case Tangent::smooth:
              in = computeSmooth(kf[i - 1], kf[i + 1]);
              break;
            case Tangent::linear:
              in = computeLinearIn(kf[i - 1], kf[i]);
              break;
            }
          expect("tangent now a value",
                 mData.keyframes[i].tangentIn.which() == 0);
        }

      if (out.which() == 1)
        {
          switch (boost::get<Tangent>(out))
            {
            case Tangent::flat:
              out = 0.0f;
              break;
            case Tangent::smooth:
              out = computeSmooth(kf[i - 1], kf[i + 1]);
              break;
            case Tangent::linear:
              out = computeLinearOut(kf[i], kf[i + 1]);
              break;
            }
          expect("tangent now a value",
                 mData.keyframes[i].tangentOut.which() == 0);
        }
    }

  // handle special cases first and last keyframes

  if (kf[0].tangentOut.which() == 1)
    {
      auto out = boost::get<Tangent>(kf[0].tangentOut);
      size_t i = 0;
      switch (out)
        {
        case Tangent::flat:
          kf[i].tangentOut = 0.0f;
          break;
        case Tangent::smooth:
        case Tangent::linear:
          kf[i].tangentOut = computeLinearOut(kf[i], kf[i+1]);
          break;
        }
      expect("tangent now a value",
             mData.keyframes[i].tangentOut.which() == 0);
    }

  if (kf[0].tangentIn.which() == 1)
    { // extrapolate
      auto in = boost::get<Tangent>(kf[0].tangentIn);
      size_t i = 0;
      auto kfp = kf[kf.size() - 1];
      switch (in)
        {
        case Tangent::flat:
          kf[i].tangentIn = 0.0f;
          break;
        case Tangent::smooth:
        case Tangent::linear:
          switch (mData.extrapIn)
            {
            case Extrapolation::constant:
              kf[i].tangentIn = 0.0f;
              break;
            case Extrapolation::linear:
              expect("tangentOut has been evaluated", kf[i].tangentOut.which() == 0);
              kf[i].tangentIn = kf[i].tangentOut;
              break;
            case Extrapolation::cycle:
              kf[i].tangentIn = computeLinearIn(kf[kf.size() - 1], kf[i]);
              //kf[i].tangentIn = kf[i].tangentOut;
              break;
            case Extrapolation::cycleOffset:
              kfp.value = kfp.value - kf[i].value;
              kf[i].tangentIn = computeLinearIn(kfp, kf[i]);
              //kf[i].tangentIn = kf[i].tangentOut;
              break;
            case Extrapolation::bounce:
              kf[i].tangentIn = -(boost::get<float>(kf[i].tangentOut));
              break;
            }
          break;
        }
      expect("tangent now a value",
             mData.keyframes[i].tangentIn.which() == 0);
    }



  if (kf[kf.size() - 1].tangentIn.which() == 1)
    {
      auto in = boost::get<Tangent>(kf[kf.size() - 1].tangentIn);
      size_t i = kf.size() - 1;
      switch (in)
        {
        case Tangent::flat:
          kf[i].tangentIn = 0.0f;
          break;
        case Tangent::smooth:
        case Tangent::linear:
          kf[i].tangentIn = computeLinearIn(kf[i - 1], kf[i]);
          break;
        }
      expect("tangent now a value",
             mData.keyframes[i].tangentIn.which() == 0);
        }

  if (kf[kf.size() - 1].tangentOut.which() == 1)
    { // extrapolate
      auto out = boost::get<Tangent>(kf[kf.size() - 1].tangentOut);
      size_t i = kf.size() - 1;
      auto kfn = kf[0];
      switch (out)
        {
        case Tangent::flat:
          kf[i].tangentOut = 0.0f;
          break;
        case Tangent::smooth:
        case Tangent::linear:;
          switch (mData.extrapOut)
            {
            case Extrapolation::constant:
              kf[i].tangentOut = 0.0f;
              break;
            case Extrapolation::linear:
              expect("tangentIn has been evaluated", kf[i].tangentIn.which() == 0);
              kf[i].tangentOut = kf[i].tangentIn;
              break;
            case Extrapolation::cycle:
              kf[i].tangentOut = computeLinearOut(kf[i], kf[0]);
              //kf[i].tangentOut = kf[i].tangentIn;
              break;
            case Extrapolation::cycleOffset:
              kfn.value = kfn.value + kf[i].value;
              kf[i].tangentOut = computeLinearOut(kf[i], kfn);
              //kf[i].tangentOut = kf[i].tangentIn;
              break;
            case Extrapolation::bounce:
              kf[i].tangentOut = -(boost::get<float>(kf[i].tangentIn));
              break;
            }
          break;
        }
      expect("tangent now a value",
             mData.keyframes[i].tangentOut.which() == 0);
    }

  for (size_t i = 0; i < kf.size(); ++i)
    { // Sure, why not? Obviously if this were not just some academic thing I'd
      // not basically waste a bunch of time spinning here...
      expect("kf[i].out evaluated", mData.keyframes[i].tangentOut.which() == 0);
      expect("kf[i].in evaluated", mData.keyframes[i].tangentIn.which() == 0);
    }
}

float dmp::Channel::evaluateImpl(float t)
{
  Keyframe kf;
  bool foundSuitable = false;

  if (mData.keyframes.size() == 1)
    {
      expect("time is ~ keyframe time", roughEq(mData.keyframes[0].time, t));
      return mData.keyframes[0].value;
    }

  //std::cerr << " t = " << t << " curr.t = " << mData.keyframes[0].time << std::endl;
  for (size_t i = 1; i < mData.keyframes.size(); ++i)
    {
      //std::cerr << " t = " << t << " curr.t = " << mData.keyframes[i].time << std::endl;
      if (roughEq(mData.keyframes[i].time, t))
        {
          return mData.keyframes[i].value;
        }
      else if ( mData.keyframes[i].time > t)
        {
          expect("found larger time. This should not be index 0", i != 0);
          kf = mData.keyframes[i-1];
          foundSuitable = true;
          break;
        }
    }
  expect("found a suitable keyframe", foundSuitable);
  expect("current keyframe has had coefficients evaluated",
         kf.invLerpRhs != 0.0f);

  auto u = kf.invLerpRhs * (t - kf.time);
  const auto & cc = kf.cubicCoefficients;

  return cc.w + u * (cc.z + u * (cc.y + u * (cc.x)));
}

static float mod(float lhs, float rhs)
{
  if (rhs == 0.0f) return rhs;
  auto m = fmodf(lhs, rhs);
  if (m < 0.0f) m += rhs;
  return m;
}

float dmp::Channel::evaluate(float t)
{
  auto startTime = mData.keyframes.front().time;
  auto endTime = mData.keyframes.back().time;
  auto rangeTime = endTime - startTime;


  if (t >= startTime && t <= endTime)
    { // In range, just evaluate
      return evaluateImpl(t);
    }

  else if (t < startTime)
    {// extrapolateIn
      float tPrime = mod(t-startTime, rangeTime) + startTime;
      size_t distance = 0;
      float acc = t;
      float startVal = evaluateImpl(startTime);
      float endVal = evaluateImpl(endTime);

      expect ("tPrime in in range",
              startTime <= tPrime && tPrime <= endTime);

      switch (mData.extrapIn)
        {
        case Extrapolation::constant:
          return startVal;
        case Extrapolation::linear:
          return startVal
            * mData.keyframes.front().getTangentOut();
        case Extrapolation::cycle:
          return evaluateImpl(tPrime);
          return 0.0f;
        case Extrapolation::cycleOffset:
          while (true)
            {
              acc = acc + rangeTime;
              ++distance;
              if (acc >= startTime && acc <= endTime) break;
            }
          return ((((float) distance) * startVal)
          + (evaluateImpl(tPrime)));
        case Extrapolation::bounce:
          while (true)
            {
              acc = acc + rangeTime;
              if (acc >= startTime && acc <= endTime) break;
              ++distance;
            }
          if (distance % 2) return evaluateImpl(endTime - tPrime); // odd
          else return evaluateImpl(tPrime);
        }
    }
  else
    { // extrapolateOut
      float tPrime = mod(t-startTime, rangeTime) + startTime;
      size_t distance = 0;
      float acc = t;
      float startVal = evaluateImpl(startTime);
      float endVal = evaluateImpl(endTime);

      expect ("tPrime out in range",
              startTime <= tPrime && tPrime <= endTime);

      switch (mData.extrapOut)
        {
        case Extrapolation::constant:
          return endVal;
        case Extrapolation::linear:
          return endVal
            * mData.keyframes.back().getTangentIn();
          //return 0.0f;
        case Extrapolation::cycle:
          return evaluateImpl(tPrime);
          //return 0.0f;
        case Extrapolation::cycleOffset:
          while (true)
            {
              acc = acc - rangeTime;
              ++distance;
              if (acc >= startTime && acc <= endTime) break;
            }
          return ((((float) distance) * endVal)
          + (evaluateImpl(fabsf(acc))));
        case Extrapolation::bounce:
          while (true)
            {
              acc = acc - rangeTime;
              ++distance;
              if (acc >= startTime && acc <= endTime) break;
            }
          if (distance % 2) return evaluateImpl(endTime - tPrime); // odd
          else return evaluateImpl(tPrime);
        }
    }
  impossible("channel::evaluate if/then/else didn't return");
}

dmp::Animation::Animation(const std::string & path)
{
  initAnimation(path);
}



void dmp::Animation::initAnimation(const std::string & path)
{
  using namespace boost;
  std::string data;
  readFile(path, data);

  auto sep = whitespaceSeparator;
  tokenizer<char_separator<char>> tokens(data, sep);

  auto beg = tokens.begin();
  auto end = tokens.end();

  auto parseRange = [&rb=mRangeBegin,
                     &re=mRangeEnd](TokenIterator & b,
                                    TokenIterator & e)
    {
      if (b != e && *b == tokRange)
        {
          safeIncr(b, e);
          parseVec2(rb, re, b, e);
        }
    };
  auto parseNumChans = [&chans=mChannels](auto & b, auto & e)
    {
      // if current node is "numchannels" swallow "channels, and reserve space
      // in the channels vector
      if (b != e && *b == tokNumChannels)
        {
          safeIncr(b, e);
          int numChans;
          // If this fails, the input file is malformed
          parseInt(numChans, b, e);
          chans.reserve((size_t) numChans);
        }
    };
  auto parseChannels = [&chans=mChannels](auto & b, auto & e)
    {
      auto f = [&chans](auto & beg, auto & end)
      {
        if (beg != end && *beg == tokChannel)
          {
            ChannelData cd;
            // if the current token is "channel", and parseChannel
            // fails, then the input is malformed
            parseChannel(cd, beg, end);
            chans.emplace_back(cd);
          }
      };
      manyParse(f, b, e);
    };
  auto parseAnimationOpen = [](auto & b, auto & e)
    {
      if (*b == tokAnimation)
        {
          safeIncr(b, e);
          expect("after animation is {", *b == tokOpenBrace);
          safeIncr(b, e);
        }
    };
  auto parseAnimationClose = [](auto & b, auto & e)
    {
      if (*b == tokCloseBrace)
        {
          safeIncr(b, e);
          expect("No input follows Animation", b == e);
        }
    };

  std::list<std::function<void(dmp::TokenIterator &,
                               dmp::TokenIterator &)>> ps = {parseAnimationOpen,
                                                             parseRange,
                                                             parseNumChans,
                                                             parseChannels,
                                                             parseAnimationClose};

  // assuming the order of tokens specified in the writeup is fixed, this could
  // very well be empty
  ps = someParse(ps, beg, end);
  expect("no parse failures. TODO: can there be failures?", ps.empty());

  for (auto & curr : mChannels)
    {
      curr.precompute();
    }

  //printAnimation();

  auto vertName = channelShader + std::string(".vert");
  auto fragName = channelShader + std::string(".frag");

  mShaderProg.initShader(vertName.c_str(),
                         nullptr, nullptr, nullptr,
                         fragName.c_str());

  expectNoErrors("load channel shader");
}

dmp::Pose dmp::Animation::evaluate(float t)
{
  Pose retval;

  auto begin = mChannels.begin();
  auto end = mChannels.end();

  auto startTime = mRangeBegin;
  auto endTime = mRangeEnd;
  auto spanTime = endTime - startTime;
  //std::cerr << "start time: " << startTime;
  //std::cerr << " end time: " << endTime;
  //std::cerr << " timeRange: " << spanTime << std::endl;
  float tPrime = startTime + fmodf(t, spanTime); // TODO: probalby needs some fabs()

  float x, y, z;

  x = begin->evaluate(tPrime);
  safeIncr(begin, end);
  y = begin->evaluate(tPrime);
  safeIncr(begin, end);
  z = begin->evaluate(tPrime);
  safeIncr(begin, end);

  retval.translation = {x, y, z};

  while (begin != end)
    {
      x = begin->evaluate(tPrime);
      safeIncr(begin, end);
      y = begin->evaluate(tPrime);
      safeIncr(begin, end);
      z = begin->evaluate(tPrime);
      safeIncr(begin, end);
      retval.rotations.emplace_back(x, y, z);
      //std::cerr << t << " --> " << glm::to_string(retval.rotations.back()) << std::endl;
    }

  return retval;
}

void dmp::Animation::printChannel(size_t idx)
{
  expect("index in range", idx < mChannels.size());

  mChannels[idx].printChannel();
}

void dmp::Animation::printAnimation()
{
  std::cerr << "range = " << mRangeBegin << " -> " << mRangeEnd << std::endl;
  std::cerr << "|channels| = " << mChannels.size() << std::endl;

  for (auto & curr : mChannels)
    {
      curr.printChannel();
    }
}

void dmp::Channel::printChannel()
{
  std::cerr << "channel" << std::endl << "{" << std::endl;
  std::cerr << "   extrapolation in = " << (int) mData.extrapIn << std::endl;
  std::cerr << "   extrapolation out = " << (int) mData.extrapOut << std::endl;
  std::cerr << "   |keyframes| = " << mData.keyframes.size() << std::endl;

  for (const auto & curr : mData.keyframes)
    {
      std::cerr << "   keyframe" << std::endl << "   {" << std::endl;
      std::cerr << "      time = " << curr.time << std::endl;
      std::cerr << "      value = " << curr.value << std::endl;
      if (curr.tangentIn.which() == 0)
        {
          std::cerr << "      tangent in value = "
                    << boost::get<float>(curr.tangentIn)
                    << std::endl;
        }
      else
        {
          std::cerr << "      tangent in rule = "
                    << (int) boost::get<Tangent>(curr.tangentIn)
                    << std::endl;
        }

      if (curr.tangentOut.which() == 0)
        {
          std::cerr << "      tangent out value = "
                    << boost::get<float>(curr.tangentOut)
                    << std::endl;
        }
      else
        {
          std::cerr << "      tangent out rule = "
                    << (int) boost::get<Tangent>(curr.tangentOut)
                    << std::endl;
        }
      std::cerr << "   }" << std::endl;
    }

  std::cerr << "}" << std::endl;
}

int dmp::Animation::nextCurveIndex(int prev)
{
  if (prev < 0) return 0;
  else if ((size_t) prev >= mChannels.size() - 1) return -1;
  else return prev + 1;
}

int dmp::Animation::prevCurveIndex(int prev)
{
  if (prev <= 0) return -1;
  else if ((size_t) prev > mChannels.size()) return -1;
  else return prev - 1;
}

void dmp::Animation::drawCurveIndex(int idx)
{
  if (idx < 0) return;
  if ((size_t) idx >= mChannels.size()) return;

  glUseProgram(mShaderProg);
  GLuint pcIdx = glGetUniformBlockIndex(mShaderProg, "PassConstants");
  glUniformBlockBinding(mShaderProg, pcIdx, 1);
  expectNoErrors("set uniform");

  mChannels[idx].draw();
}

void dmp::Channel::draw()
{
  glBindVertexArray(mVAO);
  expectNoErrors("bind VAO");

  glDrawArrays(GL_LINES, 0, drawCount);
  expectNoErrors("draw");

  for (auto & curr : mData.keyframes)
    {
      glBindVertexArray(curr.mVAO);
      expectNoErrors("bind VAO");

      glDrawArrays(GL_LINES, 0, curr.drawCount);
      expectNoErrors("draw");
    }
}
