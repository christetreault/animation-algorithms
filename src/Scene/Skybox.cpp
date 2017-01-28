#include "Skybox.hpp"
#include "../util.hpp"
#include "../config.hpp"
#include "../Renderer/Texture.hpp"
#include "../ext/stb_image.h"

#include <iostream>

static const GLfloat verts[] =
  {
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f
  };

static const GLuint idxs[] =
  {
    2, 1, 0,
    0, 3, 2,
    6, 5, 1,
    1, 2, 6,
    5, 6, 7,
    7, 4, 5,
    3, 0, 4,
    4, 7, 3,
    1, 5, 4,
    4, 0, 1,
    6, 2, 3,
    3, 7, 6
  };

void dmp::Skybox::initSkybox(std::vector<const char *> tex)
{
  glGenTextures(1, &mTexId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mTexId);

  expectNoErrors("gen and bind cubemap");

  int width, height, channels;
  unsigned char * data;

  for (size_t i = 0; i < tex.size(); ++i)
    {
      data = stbi_load(tex[i],
                       &width,
                       &height,
                       &channels,
                       3); // don't want a transparent skybox
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + ((GLenum)i),
                   0,
                   GL_RGB,
                   width,
                   height,
                   0,
                   GL_RGB,
                   GL_UNSIGNED_BYTE,
                   data);
      stbi_image_free(data);

      auto msg = std::string("Load cubemap image #") + std::to_string(i);
      expectNoErrors(msg);
    }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  expectNoErrors("finish loading cubemap");

  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  glGenBuffers(1, &mEBO);

  expectNoErrors("Gen vao/vbo/ebo");

  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  expectNoErrors("Fill VBO");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  expectNoErrors("fill EBO");

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        3 * sizeof(float),
                        (GLvoid*) 0);

  expectNoErrors("assign vertex attributes");

  auto vertName = skyboxShader + std::string(".vert");
  auto fragName = skyboxShader + std::string(".frag");

  mShaderProg.initShader(vertName.c_str(),
                         nullptr, nullptr, nullptr,
                         fragName.c_str());

  expectNoErrors("load skybox shader");

  mValid = true;
}

void dmp::Skybox::bind(GLenum texUnit)
{
  expect("Skybox valid", mValid);
  glUseProgram(mShaderProg);
  expectNoErrors("Bind shader program");
  glActiveTexture(texUnit);
  expectNoErrors("activate texture unit");
  glBindTexture(GL_TEXTURE_CUBE_MAP, mTexId);
  expectNoErrors("bind texture");

  GLuint pcIdx = glGetUniformBlockIndex(mShaderProg, "PassConstants");
  glUniformBlockBinding(mShaderProg, pcIdx, 1);

  glUniform1i(glGetUniformLocation(mShaderProg, "skybox"),
              texUnitAsInt(texUnit));

  expectNoErrors("set uniform");
  glBindVertexArray(mVAO);
  expectNoErrors("bind VAO");
}

void dmp::Skybox::draw()
{
  expect("skybox valid", mValid);
  glDrawElements(GL_TRIANGLES,
                 36,
                 GL_UNSIGNED_INT,
                 0);
}

void dmp::Skybox::freeSkybox()
{
  expect("skybox valid prior to delete", mValid);

  glDeleteVertexArrays(1, &mVAO);
  glDeleteBuffers(1, &mVBO);
  glDeleteBuffers(1, &mEBO);
  glDeleteTextures(1, &mTexId);
}
