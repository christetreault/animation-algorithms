#include "Program.hpp"

#include <iostream>
#include <unistd.h>
#include "config.hpp"


static void errorCb(int error, const char * description)
{
  std::cerr << "GLFW Error "
            << error
            << ": "
            << description
            << std::endl;
}

dmp::Program::Program(int width, int height, const char * title)
  : mWindow(width, height, title),
    mRenderer((GLsizei) mWindow.getFramebufferWidth(),
              (GLsizei) mWindow.getFramebufferHeight(),
              basicShader),
    mTimer()
{
  mWindow.keyFn = [](GLFWwindow * w,
                     int key,
                     int scancode,
                     int action,
                     int mods)
    {
      if (action == GLFW_PRESS)
        {
          switch (key)
            {
            case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(w, true);
            break;
            }
        }
    };

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
  buildScene(mScene);
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
          updateScene(mScene, mTimer.deltaTime());
          mRenderer.render(mScene, mTimer);
          mWindow.swapBuffer();
        }

      // poll window system events

      mWindow.pollEvents();
    }
  return EXIT_SUCCESS;
}
