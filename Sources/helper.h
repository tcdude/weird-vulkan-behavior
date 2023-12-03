#pragma once

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

#include <math.h>

static kinc_matrix4x4_t perspective_projection(float fov_y, float aspect,
                                               float zn, float zf) {
  float uh = 1.0 / tanf(fov_y / 2.0f);
  float uw = uh / aspect;
  return (kinc_matrix4x4_t){.m = {uw, 0.0f, 0.0f, 0.0f, 0.0f, uh, 0.0f, 0.0f,
                                  0.0f, 0.0f, (zf + zn) / (zn - zf), -1.0f,
                                  0.0f, 0.0f, 2.0f * zf * zn / (zn - zf),
                                  0.0f}};
}

static kinc_vector3_t vec3_normalized(kinc_vector3_t v) {
  float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  return (kinc_vector3_t){v.x / l, v.y / l, v.z / l};
}

static kinc_vector3_t vec3_cross(kinc_vector3_t vlh, kinc_vector3_t vrh) {
  float x = vlh.y * vrh.z - vlh.z * vrh.y;
  float y = vlh.z * vrh.x - vlh.x * vrh.z;
  float z = vlh.x * vrh.y - vlh.y * vrh.x;
  return (kinc_vector3_t){x, y, z};
}

static kinc_vector3_t vec3_subv(kinc_vector3_t a, kinc_vector3_t b) {
  return (kinc_vector3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

static float vec3_dot(kinc_vector3_t vlh, kinc_vector3_t vrh) {
  return vlh.x * vrh.x + vlh.y * vrh.y + vlh.z * vrh.z;
}

static kinc_matrix4x4_t look_at(kinc_vector3_t eye, kinc_vector3_t at,
                                kinc_vector3_t up) {
  kinc_vector3_t zaxis = vec3_normalized(vec3_subv(at, eye));
  kinc_vector3_t xaxis = vec3_normalized(vec3_cross(zaxis, up));
  kinc_vector3_t yaxis = vec3_cross(xaxis, zaxis);

  return (kinc_matrix4x4_t){xaxis.x,
                            yaxis.x,
                            -zaxis.x,
                            0.0f,
                            xaxis.y,
                            yaxis.y,
                            -zaxis.y,
                            0.0f,
                            xaxis.z,
                            yaxis.z,
                            -zaxis.z,
                            0.0f,
                            -vec3_dot(xaxis, eye),
                            -vec3_dot(yaxis, eye),
                            vec3_dot(zaxis, eye),
                            1.0f};
}
