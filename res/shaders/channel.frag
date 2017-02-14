#version 410

in vec4 colorToFrag;

out vec4 outColor;

void main()
{
  outColor = colorToFrag;
}
