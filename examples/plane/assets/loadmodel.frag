#version 410

out vec4 outColor;

void main() {
  float i = 1.0 - gl_FragCoord.z;

  outColor = vec4(1, 1, 1, 1);
}