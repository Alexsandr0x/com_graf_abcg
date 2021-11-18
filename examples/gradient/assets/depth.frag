#version 410

in vec4 fragColor;
in vec4 position;
out vec4 outColor;

void main() {

  vec2 st = vec2(position.x + 20, position.y + 20)/vec2(40, 40);

  vec3 color1 = vec3(1.0,0.55,0);
  vec3 color2 = vec3(0.526,0.000,0.615);

  float mixValue = distance(st, vec2(0,1));
  vec3 color = mix(color1, color2, mixValue);

  outColor = vec4(color, 1);
}