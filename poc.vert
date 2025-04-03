#version 450
#extension GL_GOOGLE_include_directive : require
#include "../glslinc/3d.glsl"

layout(push_constant) uniform upc {
  float time;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 txt;
layout(location = 2) in vec3 nrm;

layout(location = 0) out vec2 f_txt;
layout(location = 1) out vec3 f_nrm;

const float fov_rad = radians(90);
const float aspect = 1;
const float far = 100.0;
const float near = 0.01;

const vec3 up = normalize(vec3(0, 1, 0));

mat4 model_matrix(float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return mat4(
    c, 0, -s, 0,
    0, 1, 0, 0,
    s, 0, c, 0,
    0, 0, 0, 1
  );
}

void main() {
  vec4 cam = vec4(0, -2, -5, 0);
  mat4 proj = projection_matrix(fov_rad, aspect, near, far);
  mat4 view = view_matrix(cam.xyz, radians(cam.w), up);
  mat4 modl = model_matrix(time * 3);
  vec4 pvec = vec4(pos.x, -pos.y, pos.z, 1);
  gl_Position = pvec * modl * view * proj;
  f_txt = txt;
  f_nrm = nrm;
}
