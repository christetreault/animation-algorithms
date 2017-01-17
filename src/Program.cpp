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
                      const char * title, const char * file)
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

  mScene.build(cameraFn, file);

  mWindow.keyFn = [&mCameraState=mCameraState](GLFWwindow * w,
                                               int key,
                                               int scancode,
                                               int action,
                                               int mods)
    {
      static const float minZoom = 1.0f;
      static const float maxZoom = 100.0f;
      static const float zoomInc = 1.0f;
      static const float minElev = -(glm::half_pi<float>() - 0.1f);
      static const float maxElev = glm::half_pi<float>() - 0.1f;
      static const float minHorz = -std::numeric_limits<float>::infinity();
      static const float maxHorz = std::numeric_limits<float>::infinity();
      static const float rotInc = 0.2f;

      if (action == GLFW_PRESS)
        {
          switch (key)
            {
            case GLFW_KEY_ESCAPE:
              glfwSetWindowShouldClose(w, true);
              break;
            case GLFW_KEY_UP:
              mCameraState[VERTICAL]
                = glm::clamp(mCameraState[VERTICAL] - rotInc,
                             minElev,
                             maxElev);
              break;
            case GLFW_KEY_DOWN:
              mCameraState[VERTICAL]
                = glm::clamp(mCameraState[VERTICAL] + rotInc,
                             minElev,
                             maxElev);
              break;
            case GLFW_KEY_RIGHT:
              mCameraState[HORIZONTAL]
                = glm::clamp(mCameraState[HORIZONTAL] + rotInc,
                             minHorz,
                             maxHorz);
              break;
            case GLFW_KEY_LEFT:
              mCameraState[HORIZONTAL]
                = glm::clamp(mCameraState[HORIZONTAL] - rotInc,
                             minHorz,
                             maxHorz);
              break;
            case GLFW_KEY_PAGE_UP:
              mCameraState[DISTANCE]
                = glm::clamp(mCameraState[DISTANCE] - zoomInc,
                             minZoom,
                             maxZoom);
              break;
            case GLFW_KEY_PAGE_DOWN:
              mCameraState[DISTANCE]
                = glm::clamp(mCameraState[DISTANCE] + zoomInc,
                             minZoom,
                             maxZoom);
              break;
            }
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
          mRenderer.render(mScene, mTimer);
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
