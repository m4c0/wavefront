#version 450

layout(location = 0) in vec2 txt;

void main() {
  gl_Position = vec4(txt * 2.0 - 1.0, 0, 1);
}
