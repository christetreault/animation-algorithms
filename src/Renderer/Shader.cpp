#include "Shader.hpp"
#include <fstream>
#include <utility>
#include <GL/glew.h>
#include <iostream>

std::map<const std::string, std::vector<char>> dmp::Shader::memo;

std::vector<char> dmp::Shader::loadGLSL(const std::string & path)
{
  std::ifstream fin(path, std::ios::in);
  expect("file opened successfully", fin);
  fin.seekg(0, std::ios_base::end);
  size_t endPos = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  memo[path].resize(endPos);

  fin.read(memo[path].data(), endPos);

  fin.close();

  auto bytecodeIter = memo.find(path);

  expect("Failed to load GLSL file!",
         bytecodeIter != memo.end());


  return bytecodeIter->second;
}

static GLuint compileShader(std::vector<char> source, GLenum type)
{
  GLuint id = glCreateShader(type);

  auto sourcePtr = source.data();
  auto sourceLen = (GLint) source.size();
  glShaderSource(id, 1, &sourcePtr, &sourceLen);
  glCompileShader(id);

  GLint result = GL_FALSE;
  GLint infoLogLen = 0;

  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);

  ifDebug(if (infoLogLen > 0)
            {
              std::vector<char> errors(infoLogLen + 1);
              glGetShaderInfoLog(id, infoLogLen, nullptr, &errors[0]);
              std::cerr << "Shader compilation errors: "
                        << errors.data()
                        << std::endl;
            });

  expect("GLSL compilation failures",
         result == GL_TRUE);

  return id;
}

dmp::Shader::Shader(const char * vertPath,
                    const char * geomPath,
                    const char * tescPath,
                    const char * tesePath,
                    const char * fragPath)
{
  initShader(vertPath, geomPath,
             tescPath, tesePath,
             fragPath);
}
void dmp::Shader::initShader(const char * vertPath,
                             const char * geomPath,
                             const char * tescPath,
                             const char * tesePath,
                             const char * fragPath)
{
  GLuint vertId = 0;
  GLuint geomId = 0;
  GLuint tescId = 0;
  GLuint teseId = 0;
  GLuint fragId = 0;


  if (vertPath)
    {
      auto vertSrc = loadGLSL(vertPath);
      vertId = compileShader(vertSrc, GL_VERTEX_SHADER);
      expect("Load vertex shader", vertId != 0);
    }

  if (geomPath)
    {
      auto geomSrc = loadGLSL(geomPath);
      geomId = compileShader(geomSrc, GL_GEOMETRY_SHADER);
      expect("Load geometry shader", geomId != 0);
    }

  if (tescPath)
    {
      auto tescSrc = loadGLSL(tescPath);
      tescId = compileShader(tescSrc, GL_TESS_CONTROL_SHADER);
      expect("Load tess control shader", tescId != 0);
    }

  if (tesePath)
    {
      auto teseSrc = loadGLSL(tesePath);
      teseId = compileShader(teseSrc, GL_TESS_EVALUATION_SHADER);
      expect("Load tess evaluation shader", teseId != 0);
    }

  if (fragPath)
    {
      auto fragSrc = loadGLSL(fragPath);
      fragId = compileShader(fragSrc, GL_FRAGMENT_SHADER);
      expect("Load fragment shader", fragId != 0);
    }

  expectNoErrors("Compile shader sources");

  mShaderProg = glCreateProgram();
  if (vertPath) glAttachShader(mShaderProg, vertId);
  if (geomPath) glAttachShader(mShaderProg, geomId);
  if (tesePath) glAttachShader(mShaderProg, teseId);
  if (tescPath) glAttachShader(mShaderProg, tescId);
  if (fragPath) glAttachShader(mShaderProg, fragId);

  glLinkProgram(mShaderProg);

  GLint result = GL_FALSE;
  GLint infoLogLen = 0;

  glGetProgramiv(mShaderProg, GL_LINK_STATUS, &result);
  glGetProgramiv(mShaderProg, GL_INFO_LOG_LENGTH, &infoLogLen);

  ifDebug(if (infoLogLen > 0)
            {
              std::vector<char> errors(infoLogLen + 1);
              glGetProgramInfoLog(mShaderProg, infoLogLen, nullptr, &errors[0]);
              std::cerr << "Shader compilation errors: "
                        << errors.data()
                        << std::endl;
            });

  expect("GLSL linking failures",
         result == GL_TRUE);

  if (vertPath) glDetachShader(mShaderProg, vertId);
  if (geomPath) glDetachShader(mShaderProg, geomId);
  if (tesePath) glDetachShader(mShaderProg, teseId);
  if (tescPath) glDetachShader(mShaderProg, tescId);
  if (fragPath) glDetachShader(mShaderProg, fragId);

  if (vertPath) glDeleteShader(vertId);
  if (geomPath) glDeleteShader(geomId);
  if (tesePath) glDeleteShader(teseId);
  if (tescPath) glDeleteShader(tescId);
  if (fragPath) glDeleteShader(fragId);

  expect("Create shader program",
         mShaderProg != 0);
}
