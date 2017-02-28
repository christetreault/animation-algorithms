#include "Program.hpp"

#include <iostream>
#include <unistd.h>
#include "config.hpp"
#include "util.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

static const std::string HORIZONTAL = "horizontal";
static const std::string VERTICAL = "vertical";
static const std::string DISTANCE = "distance";

static const std::string CLOTH_X = "clothX";
static const std::string CLOTH_Y = "clothY";
static const std::string CLOTH_THETA_Y = "clothThetaY";
static const std::string CLOTH_THETA_Z = "clothThetaZ";
static const std::string CLOTH_WIND_CONST = "clothWindConstant";
static const float CLOTH_MAX = 2.0f;
static const float CLOTH_MIN = -1.0f;
static const float CLOTH_STEP = 0.1f;
static const float CLOTH_THETA_MAX = glm::pi<float>();
static const float CLOTH_THETA_MIN = -glm::pi<float>();
static const float CLOTH_THETA_STEP = glm::pi<float>() * 0.05f;
static const float CLOTH_LERP_LENGTH = 0.05f;

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

  auto cameraFn = [&mCameraState=mCameraState] (glm::mat4 & M, float)
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

  auto clothFn = [&] (glm::mat4 & M, float)
    {
      static float prevX = 0.0f;
      static float prevY = 0.0f;
      static float prevThetaY = 0.0f;
      static float prevThetaZ = 0.0f;
      auto x = mClothState[CLOTH_X];
      auto y = mClothState[CLOTH_Y];
      auto thetaY = mClothState[CLOTH_THETA_Y];
      auto thetaZ = mClothState[CLOTH_THETA_Z];

      if (roughEq(prevX, x)
          && roughEq(prevY, y)
          && roughEq(prevThetaY, thetaY)
          && roughEq(prevThetaZ, thetaZ))
        {
          // if prev == curr, then we are not moving. Make sure
          // rot/trans in progress are false and return
          mClothMoveInProgress = false;
          return false;
        }

      auto endTime = (mClothLerpBegin + CLOTH_LERP_LENGTH);
      auto currTime = mTimer.time();
      if (currTime > endTime)
        {
          // if currTime > endTime, then the lerp should be done.
          // clear rot and trans in progress flags so we begin
          // catching keypresses again
          mClothMoveInProgress = false;
          prevX = x;
          prevY = y;
          prevThetaY = thetaY;
          prevThetaZ = thetaZ;
          //std::cerr << "lerp ended" << std::endl;
          return false;
        }

      auto t = endTime / currTime;

      x = glm::mix(prevX, x, t);
      y = glm::mix(prevY, y, t);
      thetaY = glm::mix(prevThetaY, thetaY, t);
      thetaZ = glm::mix(prevThetaZ, thetaZ, t);

      auto trans = glm::translate(glm::mat4(), glm::vec3(x, y, 0.0f));
      auto rotY = glm::rotate(glm::mat4(),
                              thetaY,
                              glm::vec3(0.0f, 1.0f, 0.0f));
      auto rotZ = glm::rotate(glm::mat4(),
                              thetaZ,
                              glm::vec3(0.0f, 0.0f, 1.0f));

      M = trans * rotY * rotZ;

      //std::cerr << "<clothX, clothY, clothTheta> = <"
      //<< x << ", " << y << ", " << theta << ">" << std::endl;

      return true;
    };

  auto lightFn = [&l=mLightCoeff](glm::mat4 & M, float deltaT)
    {
      M = glm::rotate(M,
                      ((float) l) * (deltaT / 3.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));

      return true;
    };

  mScene.build(cameraFn, lightFn, clothFn, file);

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
  Keybind w(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_Y]
                = glm::clamp(mClothState[CLOTH_Y] + CLOTH_STEP,
                             CLOTH_MIN,
                             CLOTH_MAX);
            },
            GLFW_KEY_W);
  Keybind q(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_X]
                = glm::clamp(mClothState[CLOTH_X] - CLOTH_STEP,
                             CLOTH_MIN,
                             CLOTH_MAX);
            },
            GLFW_KEY_Q);
  Keybind s(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_Y]
                = glm::clamp(mClothState[CLOTH_Y] - CLOTH_STEP,
                             CLOTH_MIN,
                             CLOTH_MAX);
            },
            GLFW_KEY_S);
  Keybind e(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_X]
                = glm::clamp(mClothState[CLOTH_X] + CLOTH_STEP,
                             CLOTH_MIN,
                             CLOTH_MAX);
            },
            GLFW_KEY_E);

  Keybind a(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_THETA_Y]
                = glm::clamp(mClothState[CLOTH_THETA_Y] + CLOTH_THETA_STEP,
                             CLOTH_THETA_MIN,
                             CLOTH_THETA_MAX);
            },
            GLFW_KEY_A);

  Keybind d(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_THETA_Y]
                = glm::clamp(mClothState[CLOTH_THETA_Y] - CLOTH_THETA_STEP,
                             CLOTH_THETA_MIN,
                             CLOTH_THETA_MAX);
            },
            GLFW_KEY_D);

    Keybind z(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_THETA_Z]
                = glm::clamp(mClothState[CLOTH_THETA_Z] + CLOTH_THETA_STEP,
                             CLOTH_THETA_MIN,
                             CLOTH_THETA_MAX);
            },
            GLFW_KEY_Z);

  Keybind c(mWindow,
            [&](Keybind &)
            {
              if (mClothMoveInProgress) return;
              else mClothMoveInProgress = true;
              mClothLerpBegin = mTimer.time();
              mClothState[CLOTH_THETA_Z]
                = glm::clamp(mClothState[CLOTH_THETA_Z] - CLOTH_THETA_STEP,
                             CLOTH_THETA_MIN,
                             CLOTH_THETA_MAX);
            },
            GLFW_KEY_C);

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
  Keybind f(mWindow,
            [&](Keybind &)
            {
              mRenderOptions.drawWireframe = !(mRenderOptions.drawWireframe);
            },
            GLFW_KEY_F);
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

    Keybind x(mWindow,
              [&](Keybind &)
              {
                auto & cam = mScene.cameras[0];
                auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
                std::cerr << "set wind = "
                << glm::to_string(windDir) << std::endl;
                mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST]);
              },
              GLFW_KEY_X);

  mClothState[CLOTH_WIND_CONST] = 1.0f;

  Keybind one(mWindow,
            [&](Keybind & k)
            {
              mClothState[CLOTH_WIND_CONST] = 1.0f;
              auto & cam = mScene.cameras[0];
              auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
              std::cerr << "set wind = "
                        << glm::to_string(windDir) << 1.0f <<std::endl;
              mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST]);
            },
            GLFW_KEY_1);
  Keybind two(mWindow,
            [&](Keybind & k)
            {
              mClothState[CLOTH_WIND_CONST] = 2.0f;
              auto & cam = mScene.cameras[0];
              auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
              std::cerr << "set wind = "
                        << glm::to_string(windDir) << 2.0f << std::endl;
              mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST]);
            },
            GLFW_KEY_2);
  Keybind three(mWindow,
            [&](Keybind & k)
            {
              mClothState[CLOTH_WIND_CONST] = 3.0f;
              auto & cam = mScene.cameras[0];
              auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
              std::cerr << "set wind = "
                        << glm::to_string(windDir)  << 3.0f << std::endl;
              mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST]);
            },
            GLFW_KEY_3);
  Keybind four(mWindow,
            [&](Keybind & k)
            {
              mClothState[CLOTH_WIND_CONST] = 4.0f;
              auto & cam = mScene.cameras[0];
              auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
              std::cerr << "set wind = "
                        << glm::to_string(windDir) << 4.0f << std::endl;
              mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST]);
            },
            GLFW_KEY_4);
  Keybind zero(mWindow,
            [&](Keybind & k)
            {
              mClothState[CLOTH_WIND_CONST] = 2.0f;
              auto & cam = mScene.cameras[0];
              auto windDir =
                -glm::normalize(glm::vec3(cam.getE(glm::mat4())));
              std::cerr << "set wind = "
                        << glm::to_string(windDir) << 2.0f << std::endl;
              mScene.cloth->setWind(windDir, mClothState[CLOTH_WIND_CONST], false);
            },
            GLFW_KEY_0);

  mKeybinds = {esc, up, down, right, left, pageUp, pageDown,
               f, n, l, comma, period, w, a, s, d, q, e, z, c,
               x, one, two, three, four, zero};

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
