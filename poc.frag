#version 450

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_txt;
layout(location = 1) in vec3 f_nrm;

void main() {
  colour = vec4(f_nrm * 0.5 + 0.5, 1.0);
}
