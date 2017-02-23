#include "Renderer.hpp"

#include <set>
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "util.hpp"
#include "config.hpp"
#include "Renderer/Pass.hpp"

#include <iostream>
#include <glm/gtx/string_cast.hpp>

dmp::Renderer::Renderer(GLsizei width,
                        GLsizei height,
                        const std::string shaderName)
{
  initRenderer();
  ifDebug(std::cerr
          << "Supported GLSL version: "
          << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION)
          << std::endl);
  ifDebug(std::cerr
          << "Supported GLSL version: "
          << (char *)glGetString(GL_VERSION)
          << std::endl);
  loadShaders(shaderName);
  initPassConstants();
  resize(width, height);
}

void dmp::Renderer::loadShaders(const std::string shaderName)
{
  auto vertName = shaderName + std::string(".vert");
  auto fragName = shaderName + std::string(".frag");

  mShaderProg.initShader(vertName.c_str(),
                         nullptr, nullptr, nullptr,
                         fragName.c_str());
}

void dmp::Renderer::initRenderer()
{
  // This whole section is quite icky. Maybe I shouldn't be using glew...
  glewExperimental = GL_TRUE;
  expectNoErrors("begin initrenderer");
  expect("Init Glew",
         glewInit() == GLEW_OK);

  auto err = glGetError();

  expect("glewInit will report GL_INVALID_ENUM",
         err == GL_INVALID_ENUM || err == GL_NO_ERROR);

  expectNoErrors("init glew");
  // end yuckiness

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_PROGRAM_POINT_SIZE);

  expectNoErrors("init renderer");
}

void dmp::Renderer::resize(GLsizei width, GLsizei height)
{
  float fWidth = (float) width;
  float fHeight = (float) height;
  glViewport(0, 0, width, height);

  mP = glm::perspective(fieldOfView,
                        fWidth / fHeight,
                        nearZ,
                        farZ);
}

void dmp::Renderer::initPassConstants()
{
  mPassConstants = std::make_unique<UniformBuffer>(1,
                                                   PassConstants::std140Size());
}

void dmp::Renderer::render(const Scene & scene,
                           const Timer & timer,
                           const RenderOptions & ro)
{
  expect("Scene Object Constants not null",
         scene.objectConstants);

  glClear(GL_DEPTH_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT);

  if (ro.drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  expectNoErrors("Clear prior to render");

  expect("there should be objects to draw", !scene.objects.empty());
  size_t materialIndex = scene.objects[0]->materialIndex();

  // Pass constants

  PassConstants pc = {};

  pc.lightColor[0] = scene.lights[0].color;
  pc.lightColor[1] = scene.lights[1].color;
  pc.lightColor[2] = scene.lights[2].color;
  pc.lightColor[3] = scene.lights[3].color;
  pc.lightDir[0] = scene.lights[0].M * scene.lights[0].dir;
  pc.lightDir[1] = scene.lights[1].M * scene.lights[1].dir;
  pc.lightDir[2] = scene.lights[2].M * scene.lights[2].dir;
  pc.lightDir[3] = scene.lights[3].M * scene.lights[3].dir;
  pc.numLights = 4;
  pc.drawMode = ro.drawNormals ? drawNormals : drawShaded;

  pc.P = mP;
  pc.invP = glm::inverse(pc.P);
  pc.V = scene.cameras[0].getV();
  pc.invP = glm::inverse(pc.V);
  pc.PV = pc.P * pc.V;
  pc.invPV = glm::inverse(pc.PV);
  pc.E = scene.cameras[0].getE(pc.PV);
  pc.nearZ = nearZ;
  pc.farZ = farZ;
  pc.deltaT = timer.deltaTime();
  pc.totalT = timer.time();

  mPassConstants->update(0, pc);

  GLuint pcIdx = glGetUniformBlockIndex(mShaderProg, "PassConstants");
  glUniformBlockBinding(mShaderProg, pcIdx, 1);
  mPassConstants->bind(1, 0);

  // Material Constants

  GLuint mcIdx = glGetUniformBlockIndex(mShaderProg, "MaterialConstants");
  glUniformBlockBinding(mShaderProg, mcIdx, 2);
  scene.materialConstants->bind(2, materialIndex);

  // Object Constants

  GLuint ocIdx = glGetUniformBlockIndex(mShaderProg, "ObjectConstants");
  glUniformBlockBinding(mShaderProg, ocIdx, 3);

  // TODO: this should be last
  glDepthMask(GL_FALSE);
  expect("skybox not null", scene.skybox);
  scene.skybox->bind(GL_TEXTURE0);
  scene.skybox->draw();
  expectNoErrors("Draw skybox");
  glDepthMask(GL_TRUE);


  glUseProgram(mShaderProg);

  expectNoErrors("Bind shader program");

  for (size_t i = 0; i < scene.objects.size(); ++i)
    {
      if (scene.objects[i]->materialIndex() != materialIndex)
        {
          materialIndex = scene.objects[i]->materialIndex();
          scene.materialConstants->bind(2, materialIndex);
        }

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,
                    scene.textures[scene.objects[i]->textureIndex()]);
      glUniform1i(glGetUniformLocation(mShaderProg, "tex"),
                  texUnitAsInt(GL_TEXTURE0));


      scene.objectConstants->bind(3, i);

      expectNoErrors("Set uniforms");

      scene.objects[i]->bind();
      scene.objects[i]->draw();
    }

  if (ro.drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
