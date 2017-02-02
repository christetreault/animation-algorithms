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

  auto lightFn = [&l=mLightCoeff](glm::mat4 & M, float deltaT)
    {
      M = glm::rotate(M,
                      ((float) l) * (deltaT / 3.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));

      return true;
    };

  mScene.build(cameraFn, lightFn, file);

  mDOFWindow = std::make_unique<DOFWindow>(mScene.model->getSkeletonAST());

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
  Keybind c(mWindow,
            [&](Keybind &)
            {
              mDOFWindow->show();
            },
            GLFW_KEY_C);
  Keybind l(mWindow,
            [&](Keybind &)
            {
              if (mLightCoeff == 0) mLightCoeff = 1;
              else if (mLightCoeff == 1) mLightCoeff = -1;
              else mLightCoeff = 0;
            },
            GLFW_KEY_L);

  mKeybinds = {esc, up, down, right, left, pageUp, pageDown,
               w, n, c, l};

  // Only the first 10 morphs will be bound
  size_t upperBound = file.morphPaths.size() < 10 ? file.morphPaths.size() : 10;

  for (size_t i = 0; i <= upperBound ; ++i)
    {
      mKeybinds.insert(Keybind(mWindow,
                               [&, i](Keybind &)
                               {
                                 expect("Model not null", mScene.model);
                                 std::cerr << "Apply morph "
                                           << i << std::endl;
                                 mScene.model->applyMorph(i, mTimer.time());
                               },
                               GLFW_KEY_0 + ((int) i)));
    }

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

static void updateFPS(dmp::Window & window, const dmp::Timer & timer)
{
  static size_t fps = 0;
  static float timeElapsed = 0.0f;
  float runTime = timer.time();

  ++fps;

  if ((runTime - timeElapsed) >= 1.0f)
    {
      window.updateFPS(fps, 1000/fps);

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

      updateFPS(mWindow, mTimer);

      // do actual work

      if (mTimer.isPaused())
        {
          sleep(100);
        }
      else
        {
          mScene.update(mTimer.deltaTime());
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
