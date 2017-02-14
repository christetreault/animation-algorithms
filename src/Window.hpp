#ifndef DMP_WINDOW_HPP
#define DMP_WINDOW_HPP

#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace dmp
{
  class Window
  {
  public:
    Window(int width,
           int height,
           const char * title);
    ~Window();
    bool shouldClose() {return glfwWindowShouldClose(mWindow);}
    operator GLFWwindow *() {return mWindow;}

    void swapBuffer() {glfwSwapBuffers(mWindow);}
    void pollEvents() {glfwPollEvents();}

    void updateFPS(size_t fps, size_t mspf, float scale);

    int getFramebufferWidth() const
    {
      int w, h;
      glfwGetFramebufferSize(mWindow, &w, &h);
      return w;
    }

    int getFramebufferHeight() const
    {
      int w, h;
      glfwGetFramebufferSize(mWindow, &w, &h);
      return h;
    }

    // Input
    std::function<void(GLFWwindow *, int, int, int)> mouseButtonFn;
    std::function<void(GLFWwindow *, double, double)> cursorPosFn;
    std::function<void(GLFWwindow *, int)> cursorEnterFn;
    std::function<void(GLFWwindow *, double, double)> scrollFn;
    std::function<void(GLFWwindow *, int, int, int, int)> keyFn;

    // Windows
    std::function<void(GLFWwindow *, int, int)> windowPosFn;
    std::function<void(GLFWwindow *, int, int)> windowSizeFn;
    std::function<void(GLFWwindow *)> windowCloseFn;
    std::function<void(GLFWwindow *)> windowRefreshFn;
    std::function<void(GLFWwindow *, int)> windowFocusFn;
    std::function<void(GLFWwindow *, int)> windowIconifyFn;
    std::function<void(GLFWwindow *, int, int)> windowFrameBufferResizeFn;

  private:
    GLFWwindow * mWindow = nullptr;
    const char * mTitle = "";
    size_t mFPS = 0;
    size_t mMSPF = 0;

    void initCallbacks();

    // callbacks

    inline static void mouseButton(GLFWwindow * w, int a, int b, int c)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->mouseButtonFn(w, a, b, c);
    }

    inline static void cursorPos(GLFWwindow * w, double a, double b)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->cursorPosFn(w, a, b);
    }

    inline static void cursorEnter(GLFWwindow * w, int a)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->cursorEnterFn(w, a);
    }

    inline static void scroll(GLFWwindow * w, double a, double b)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->scrollFn(w, a, b);
    }

    inline static void key(GLFWwindow * w, int a, int b, int c, int d)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->keyFn(w, a, b, c, d);
    }

    inline static void windowPos(GLFWwindow * w, int a, int b)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowPosFn(w, a, b);
    }

    inline static void windowSize(GLFWwindow * w, int a, int b)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowSizeFn(w, a, b);
    }

    inline static void windowClose(GLFWwindow * w)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowCloseFn(w);
    }

    inline static void windowRefresh(GLFWwindow * w)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowRefreshFn(w);
    }

    inline static void windowFocus(GLFWwindow * w, int a)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowFocusFn(w, a);
    }

    inline static void windowIconify(GLFWwindow * w, int a)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowIconifyFn(w, a);
    }

    inline static void windowFrameBufferResize(GLFWwindow * w, int a, int b)
    {
      Window * window = static_cast<Window*>(glfwGetWindowUserPointer(w));
      window->windowFrameBufferResizeFn(w, a, b);
    }
  };
}

#endif
