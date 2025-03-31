#version 450
#extension GL_GOOGLE_include_directive : require
#include "../glslinc/3d.glsl"

layout(location = 0) in vec2 pos;

const float fov_rad = radians(90);
const float aspect = 1;
const float far = 100.0;
const float near = 0.01;

const vec3 up = normalize(vec3(0, 1, 0));

void main() {
  vec4 cam = vec4(0, 0, -5, 0);
  mat4 proj = projection_matrix(fov_rad, aspect, near, far);
  mat4 view = view_matrix(cam.xyz, radians(cam.w), up);
  gl_Position = vec4(pos.x, -pos.y, 0, 1) * view * proj;
}
