#version 410

layout (location = 0) in vec3 posToVert;
layout (location = 1) in vec3 normalToVert;
layout (location = 2) in vec2 texCoordToVert;
layout (location = 3) in vec4 weightsToVert;
layout (location = 4) in ivec4 idxsToVert; // TODO: why isn't this working?!?!

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

layout (std140) uniform ObjectConstants
{
  mat4 M;
  mat4 normalM;

  mat4 WB[128];
};

out vec3 normalToFrag;
out vec3 posToFrag;
out vec2 texCoordToFrag;

void main()
{
  vec3 vPrime = vec3(0.0f);
  vec3 normalPrime = vec3(0.0f);

  /* int idxs[4]; */
  /* idxs[0] = idxsToVert.x; */
  /* idxs[1] = idxsToVert.y; */
  /* idxs[2] = idxsToVert.z; */
  /* idxs[3] = idxsToVert.w; */
  /* float wght[4]; */

  /* wght[0] = weightsToVert.x; */
  /* wght[1] = weightsToVert.y; */
  /* wght[2] = weightsToVert.x; */
  /* wght[3] = weightsToVert.x; */

  /* if (idxs[0] != -1) */
  /*   { */
  /*     for (uint i = 0; i < 4; ++i) */
  /*       { */
  /*         if (idxs[i] == -1) break; */

  /*         vPrime += vec3(weightsToVert[i] * (WB[idxs[i]] * vec4(posToVert, 1.0f))); */

  /*         // Assuming no non-uniform scaling or shearing */

  /*         normalPrime += vec3(weightsToVert[i] * (WB[idxs[i]] * vec4(normalToVert, 0.0f))); */
  /*       } */
  /*   } */
  /* else */
  /*   { */
  vPrime = posToVert;
  normalPrime = normalToVert;
      //}

  gl_Position = PV * M * vec4(vPrime, 1.0f);
  normalToFrag = vec3(normalize(normalM * vec4(normalPrime, 0.0f)));
  posToFrag = vec3(M * vec4(vPrime, 1.0f));
  texCoordToFrag = texCoordToVert;
}
