#ifndef DMP_RENDERER_HPP
#define DMP_RENDERER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Scene.hpp"
#include "Renderer/Shader.hpp"
#include "Timer.hpp"

namespace dmp
{
  struct RenderOptions
  {
    bool drawWireframe = false;
    bool drawNormals = false;
  };

  class Renderer
  {
  public:
    Renderer() = delete;
    Renderer(const Renderer &) = delete;
    Renderer & operator=(const Renderer &) = delete;
    Renderer(Renderer && other) = default;
    Renderer & operator=(Renderer && other) = default;
    ~Renderer() = default;

    Renderer(GLsizei width, GLsizei height, const std::string shaderFile);
    void resize(GLsizei width, GLsizei height);

    void render(const Scene & scene,
                const Timer & timer,
                const RenderOptions & ro);
  private:
    void initRenderer();
    void loadShaders(const std::string shaderFile);
    void initPassConstants();

    glm::mat4 mP;
    Shader mShaderProg;

    std::unique_ptr<UniformBuffer> mPassConstants;
  };

  // Opengl constants

  inline int uniformBufferOffsetAlignment()
  {
    int alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    return alignment;
  }
};

#endif
