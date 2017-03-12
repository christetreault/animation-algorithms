#include "Program.hpp"

#include <iostream>
#include <unistd.h>
#include "config.hpp"
#include "util.hpp"
#include "Quaternion.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

static const std::string HORIZONTAL = "horizontal";
static const std::string VERTICAL = "vertical";
static const std::string DISTANCE = "distance";

static void errorCb(int error, const char * description)
{
  std::cerr << "GLFW Error "
            << error
            << ": "
            << description
            << std::endl;
}

dmp::Program::Program(int width, int height,
                      const char * title, const CommandLine & file)
  : mWindow(width, height, title),
    mRenderer((GLsizei) mWindow.getFramebufferWidth(),
              (GLsizei) mWindow.getFramebufferHeight(),
              basicShader),
    mTimer()
{
  mWindow.windowSizeFn = [&](GLFWwindow * w,
                             int width,
                             int height)
    {
      int fbWidth, fbHeight;
      glfwGetFramebufferSize(w, &fbWidth, &fbHeight);
      mRenderer.resize((GLsizei) fbWidth, (GLsizei) fbHeight);
    };

  mWindow.windowFrameBufferResizeFn = [&](GLFWwindow * w,
                                          int width,
                                          int height)
    {
      mRenderer.resize((GLsizei) width, (GLsizei) height);
    };

  glfwSetErrorCallback(errorCb);

  mCameraState[HORIZONTAL] = 0.0f;
  mCameraState[VERTICAL] = 0.0f;
  mCameraState[DISTANCE] = 5.0f;

  auto cameraFn = [&mCameraState=mCameraState] (glm::mat4 & M, Quaternion &, float)
    {
      static float prevHorz = std::numeric_limits<float>::infinity();
      static float prevVert = std::numeric_limits<float>::infinity();
      static float prevDist = std::numeric_limits<float>::infinity();
      auto horz = mCameraState[HORIZONTAL];
      auto vert = mCameraState[VERTICAL];
      auto dist = mCameraState[DISTANCE];

      if (roughEq(prevHorz, horz)
          && roughEq(prevVert, vert)
          && roughEq(prevDist, dist)) return false;


      auto hRot = glm::rotate(glm::mat4(),
                              horz,
                              glm::vec3(0.0f, 1.0f, 0.0f));
      auto vRot = glm::rotate(glm::mat4(),
                              vert,
                              glm::vec3(1.0f, 0.0f, 0.0f));
      auto zoom = glm::translate(glm::mat4(),
                                 glm::vec3(0.0f, 0.0f, dist));

      M = hRot * vRot * zoom;

      prevHorz = horz;
      prevVert = vert;
      prevDist = dist;

      return true;
    };



  auto lightFn = [&l=mLightCoeff](glm::mat4 & M, Quaternion &, float deltaT)
    {
      M = glm::rotate(M,
                      ((float) l) * (deltaT / 3.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));

      return true;
    };

  auto staticQuatFn = [&](glm::mat4 & M, Quaternion & q, float t)
    {
      auto twoPi = 2.0f * glm::pi<float>();
      auto x = glm::mix(-4.0f, 4.0f, t);
      auto y = 0.8f * glm::cos(glm::mix(-twoPi, twoPi, t));
      auto z = 0.4f * glm::cos(glm::mix(-twoPi, twoPi, t));

      M = glm::translate(glm::mat4(), {x, y, z});

      if (mUseCatmullRom)
        {
          if (t < 0.25f) q = catmullRom(t * 4.0f, q4, q0, q1, q2, mForceShortPath);
          else if (t < 0.5f) q = catmullRom((t - 0.25f) * 4.0f, q0, q1, q2, q3, mForceShortPath);
          else if (t < 0.75f) q = catmullRom((t - 0.5f) * 4.0f, q1, q2, q3, q4, mForceShortPath);
          else q = catmullRom((t - 0.75f) * 4.0f, q2, q3, q4, q0, mForceShortPath);
        }
      else
        {
          if (t < 0.25f) q = slerp(t * 4.0f, q0, q1, mForceShortPath);
          else if (t < 0.5f) q = slerp((t - 0.25f) * 4.0f, q1, q2, mForceShortPath);
          else if (t < 0.75f) q = slerp((t - 0.5f) * 4.0f, q2, q3, mForceShortPath);
          else q = slerp((t - 0.75f) * 4.0f, q3, q4, mForceShortPath);
        }

      return true;
    };

  auto quatFn = [&mTimer=mTimer,
                 &mTimeScale=mTimeScale,
                 &mUseCatmullRom=mUseCatmullRom,
                 &mForceShortPath=mForceShortPath,
                 &q0=q0, &q1=q1, &q3=q3, &q4=q4,
                 staticQuatFn](glm::mat4 & M, Quaternion & q, float deltaT)
    {
      auto t = mod(mTimeScale * mTimer.time() / 5.0f, 2.0f);
      if (t > 1)
        {
          t = t - 1.0f;
          float pi = glm::pi<float>();
          float x;
          float y = 0.8f;
          float z = 0.4f - (4.0f * glm::sin(glm::mix(0.0f, pi, t)));

          float xRight = 4.0f + glm::sin(glm::mix(0.0f, pi,
                                                  glm::clamp(t, 0.0f, 0.5f) * 2.0f)) * 4.0f;
          float xLeft = -4.0f + glm::sin(glm::mix(0.0f, pi,
                                                  glm::clamp(t, 0.5f, 1.0f) * 2.0f)) * 4.0f;

          x = glm::mix(xRight, xLeft, t);

          M = glm::translate(glm::mat4(), {x, y, z});

          if (mUseCatmullRom)
            {
              q = catmullRom(t, q3, q4, q0, q1, mForceShortPath);
            }
          else
            {
              q = slerp(t, q4, q0, mForceShortPath);
            }
          return true;
        }
      else
        {
          return staticQuatFn(M, q, t);
        }
    };

  mScene.build(cameraFn, lightFn, quatFn, staticQuatFn, file);

  Keybind esc((GLFWwindow *) mWindow,
              [&](Keybind & k)
              {
                glfwSetWindowShouldClose(k.window, true);
              },
              GLFW_KEY_ESCAPE);
  Keybind up(mWindow,
             [&](Keybind &)
             {
               mCameraState[VERTICAL]
                 = glm::clamp(mCameraState[VERTICAL] - rotInc,
                              minElev,
                              maxElev);
             },
             GLFW_KEY_UP);
  Keybind down(mWindow,
               [&](Keybind &)
               {
                 mCameraState[VERTICAL]
                   = glm::clamp(mCameraState[VERTICAL] + rotInc,
                                minElev,
                                maxElev);
               },
               GLFW_KEY_DOWN);
  Keybind right(mWindow,
                [&](Keybind &)
                {
                  mCameraState[HORIZONTAL]
                    = glm::clamp(mCameraState[HORIZONTAL] + rotInc,
                                 minHorz,
                                 maxHorz);
                },
                GLFW_KEY_RIGHT);
  Keybind left(mWindow,
               [&](Keybind &)
               {
                 mCameraState[HORIZONTAL]
                   = glm::clamp(mCameraState[HORIZONTAL] - rotInc,
                                minHorz,
                                maxHorz);
               },
               GLFW_KEY_LEFT);

  Keybind pageUp(mWindow,
                 [&](Keybind &)
                 {
                   mCameraState[DISTANCE]
                     = glm::clamp(mCameraState[DISTANCE] - zoomInc,
                                  minZoom,
                                  maxZoom);
                 },
                 GLFW_KEY_PAGE_UP);
  Keybind pageDown(mWindow,
                   [&](Keybind &)
                   {
                     mCameraState[DISTANCE]
                       = glm::clamp(mCameraState[DISTANCE] + zoomInc,
                                    minZoom,
                                    maxZoom);
                   },
                   GLFW_KEY_PAGE_DOWN);
  Keybind w(mWindow,
            [&](Keybind &)
            {
              mRenderOptions.drawWireframe = !(mRenderOptions.drawWireframe);
            },
            GLFW_KEY_W);
  Keybind n(mWindow,
            [&](Keybind &)
            {
              mRenderOptions.drawNormals = !(mRenderOptions.drawNormals);
            },
            GLFW_KEY_N);
  Keybind comma(mWindow,
                [&](Keybind &)
                {
                  if (mTimeScale < 0.1f) return;
                  else if (mTimeScale < 2.0f) mTimeScale = mTimeScale - 0.1f;
                  else mTimeScale = mTimeScale - 1.0f;
                },
                GLFW_KEY_COMMA);
  Keybind period(mWindow,
                 [&](Keybind &)
                 {
                   if (mTimeScale > 10.0f) return;
                   else if (mTimeScale > 0.9f) mTimeScale = mTimeScale + 1.0f;
                   else mTimeScale = mTimeScale + 0.1f;
                 },
                 GLFW_KEY_PERIOD);
  Keybind l(mWindow,
            [&](Keybind &)
            {
              if (mLightCoeff == 0) mLightCoeff = 1;
              else if (mLightCoeff == 1) mLightCoeff = -1;
              else mLightCoeff = 0;
            },
            GLFW_KEY_L);

  Keybind one(mWindow,
               [&](Keybind &)
               {
                 mSelectedQuat = 0;
               },
               GLFW_KEY_1);

  Keybind two(mWindow,
              [&](Keybind &)
              {
                mSelectedQuat = 1;
              },
              GLFW_KEY_2);

  Keybind three(mWindow,
              [&](Keybind &)
              {
                mSelectedQuat = 2;
              },
              GLFW_KEY_3);

  Keybind four(mWindow,
                [&](Keybind &)
                {
                  mSelectedQuat = 3;
                },
                GLFW_KEY_4);

  Keybind five(mWindow,
               [&](Keybind &)
               {
                 mSelectedQuat = 4;
               },
               GLFW_KEY_5);

  Keybind i(mWindow,
               [&](Keybind &)
               {
                 float delta = mIncrementQuatPos ? 0.1f : -0.1f;
                 Quaternion q(delta, RotationAxis::X);
                 switch (mSelectedQuat)
                   {
                   case 0:
                     q0 = q0 * q;
                     return;
                   case 1:
                     q1 = q1 * q;
                     return;
                   case 2:
                     q2 = q2 * q;
                     return;
                   case 3:
                     q3 = q3 * q;
                     return;
                   case 4:
                     q4 = q4 * q;
                     return;
                   default:
                     return;
                   }
               },
               GLFW_KEY_I);

  Keybind j(mWindow,
               [&](Keybind &)
               {
                 float delta = mIncrementQuatPos ? 0.1f : -0.1f;
                 Quaternion q(delta, RotationAxis::Y);
                 switch (mSelectedQuat)
                   {
                   case 0:
                     q0 = q0 * q;
                     return;
                   case 1:
                     q1 = q1 * q;
                     return;
                   case 2:
                     q2 = q2 * q;
                     return;
                   case 3:
                     q3 = q3 * q;
                     return;
                   case 4:
                     q4 = q4 * q;
                     return;
                   default:
                     return;
                   }
               },
               GLFW_KEY_J);

  Keybind k(mWindow,
               [&](Keybind &)
               {
                 float delta = mIncrementQuatPos ? 0.1f : -0.1f;
                 Quaternion q(delta, RotationAxis::Z);
                 switch (mSelectedQuat)
                   {
                   case 0:
                     q0 = q0 * q;
                     return;
                   case 1:
                     q1 = q1 * q;
                     return;
                   case 2:
                     q2 = q2 * q;
                     return;
                   case 3:
                     q3 = q3 * q;
                     return;
                   case 4:
                     q4 = q4 * q;
                     return;
                   default:
                     return;
                   }
               },
               GLFW_KEY_K);

  Keybind tab(mWindow,
               [&](Keybind &)
               {
                 mIncrementQuatPos = !mIncrementQuatPos;
               },
               GLFW_KEY_TAB);

  Keybind c(mWindow,
               [&](Keybind &)
               {
                 mUseCatmullRom = !mUseCatmullRom;
                 std::cerr << "catmull rom enabled? " << mUseCatmullRom << std::endl;
               },
               GLFW_KEY_C);

  Keybind f(mWindow,
               [&](Keybind &)
               {
                 mForceShortPath = !mForceShortPath;
                 std::cerr << "force short path? " << mForceShortPath << std::endl;
               },
               GLFW_KEY_F);

  Keybind s(mWindow,
               [&](Keybind &)
               {
                 mShowDynBox = !mShowDynBox;
                 if (mShowDynBox) mScene.dynamicBox->show();
                 else mScene.dynamicBox->hide();
               },
               GLFW_KEY_S);

  mKeybinds = {esc, up, down, right, left, pageUp, pageDown,
               w, n, l, comma, period, one, two, three, four, five,
               i, j, k, tab, c, f, s};

  mWindow.keyFn = [&mKeybinds=mKeybinds](GLFWwindow * w,
                                         int key,
                                         int scancode,
                                         int action,
                                         int mods)
    {
      expect("keybinds not empty", !mKeybinds.empty());
      Keybind got = {};
      got.window = w;
      got.key = key;
      got.scancode = scancode;
      got.action = action;
      got.mods = mods;

      auto res = mKeybinds.find(got);

      if (res != mKeybinds.end())
        {
          res->fn(got);
        }
    };
}

static void updateFPS(dmp::Window & window, const dmp::Timer & timer, float scale)
{
  static size_t fps = 0;
  static float timeElapsed = 0.0f;
  float runTime = timer.time();

  ++fps;

  if ((runTime - timeElapsed) >= 1.0f)
    {
      window.updateFPS(fps, 1000/fps, scale);

      fps = 0;
      timeElapsed += 1.0f;
    }
}

int dmp::Program::run()
{
  mTimer.reset();
  mTimer.unpause();

  while (!mWindow.shouldClose())
    {
      // time marches on...
      mTimer.tick();

      updateFPS(mWindow, mTimer, mTimeScale);

      // do actual work

      if (mTimer.isPaused())
        {
          sleep(100);
        }
      else
        {
          mScene.update(mTimer.deltaTime() * mTimeScale);
          mRenderer.render(mScene, mTimer, mRenderOptions);
          mDOFWindow->pollEvents();
          mWindow.swapBuffer();
        }

      // poll window system events

      mWindow.pollEvents();
    }
  return EXIT_SUCCESS;
}

dmp::Program::~Program()
{
  mScene.free();
}
