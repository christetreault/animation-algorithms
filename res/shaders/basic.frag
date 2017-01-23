#version 410

in vec4 normalToFrag;
in vec4 posToFrag;
in vec2 texCoordToFrag;

out vec4 outColor;

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

layout (std140) uniform MaterialConstants
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

layout (std140) uniform ObjectConstants
{
  mat4 M;
  mat4 normalM;
};

uniform sampler2D tex;

void main()
{
  if (drawMode == 1)
    {
      outColor = normalToFrag;
      return;
    }

  outColor = vec4(0.0, 0.0, 0.0, 0.0);

  for (uint i = 0; i < numLights; ++i)
    {
      vec4 am = lightColor[i] * ambient;

      vec4 dir = normalize(-lightDir[i]);
      float intensity = max(dot(normalToFrag, dir), 0.0f);
      vec4 diff = intensity * lightColor[i] * diffuse;

      vec4 spec = vec4(0.0);
      if (intensity > 0.0f)
        {
          vec4 posInv = normalize(-posToFrag);
          vec4 hlf = normalize(dir + posInv);

          spec = lightColor[i]
            * specular
            * pow(max(dot(normalToFrag, hlf), 0.0), shininess);
        }
      outColor += (am + diff + spec);
    }

  outColor = texture(tex, texCoordToFrag) * outColor;
}
