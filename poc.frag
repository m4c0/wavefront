#version 450

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_txt;

void main() {
  colour = vec4(f_txt, 1.0, 1.0);
}
