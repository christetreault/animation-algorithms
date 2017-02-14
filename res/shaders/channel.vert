#version 410

layout (location = 0) in vec2 posToVert;
layout (location = 1) in int type;

layout (std140) uniform PassConstants
{
  vec4 lightColor[8]; // maxLights = 8
  vec4 lightDir[8];
  uint numLights;
  uint drawMode;

  mat4 P;
  mat4 invP;
  mat4 V;
  mat4 invV;
  mat4 PV;
  mat4 invPV;

  vec4 E;

  float nearZ;
  float farZ;
  float deltaT;
  float totalT;
};

out vec4 colorToFrag;

void main()
{
  gl_Position = vec4(posToVert / 5.0f, -1.0f, 1.0f);
  if (type == 0) colorToFrag = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  else if (type == 1) colorToFrag = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  else if (type == 2) colorToFrag = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  else colorToFrag = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
