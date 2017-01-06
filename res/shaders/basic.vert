#version 410

layout (location = 0) in vec4 posToVert;
layout (location = 1) in vec4 normalToVert;

layout (std140) uniform PassConstants
{
  vec4 lightColor[8]; // maxLights = 8
  vec4 lightDir[8];
  uint numLights;

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

layout (std140) uniform ObjectConstants
{
  mat4 M;
  mat4 normalM;
};

out vec4 normalToFrag;
out vec4 posToFrag;

void main()
{
  gl_Position = PV * M * posToVert;
  normalToFrag = normalize(normalM * normalToVert);
  posToFrag = normalize(M * posToVert);
}
