//Copyright (c) Meta Platforms, Inc. and affiliates

#version 460

layout(std430, set = 1, binding = 1) readonly buffer Instances {
  float instances[];  // (N, 14).
};

layout(location = 0) out vec4 rgba;
layout(location = 1) out vec2 out_position;
layout(location = 2) out flat float ccRecip;

layout (set = 0, binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
  vec3 camera_position;
  float pad0;
  uvec2 screen_size;  // (width, height)
};

layout(push_constant, std430) uniform PushConstants {
  mat4 model;
  float mip_bias;
  float log_p_min;
  bool mip_modulation;
};

float approxLog(const float x){
    // Approximation of log(opacity) inspired by the Padé approximant,
    // constrained to f(0) = -4, f(1) = 0, so that radius can never go below 0
    // or above 2
    return (4.0f * x * x - 4.0f) / (-0.964f * x * x + 7.169f * x + 1.0f);
}

void main() {
  // index [0,1,2,2,1,3], 4 vertices for a splat.
  int index = gl_VertexIndex / 4;

  mat3 T=mat3(
    instances[index * 14 + 0],
    instances[index * 14 + 1],
    instances[index * 14 + 2],
    instances[index * 14 + 3],
    instances[index * 14 + 4],
    instances[index * 14 + 5],
    instances[index * 14 + 6],
    instances[index * 14 + 7],
    instances[index * 14 + 8]
  );

  float cc = instances[index * 14 + 9];
  rgba = vec4(instances[index * 14 + 10], instances[index * 14 + 11], instances[index * 14 + 12], instances[index * 14 + 13]);

  ccRecip = 1.0f / cc;

  // quad positions (-1, -1), (-1, 1), (1, -1), (1, 1), ccw in screen space.
  int vert_index = gl_VertexIndex % 4;
  vec2 position = vec2(vert_index / 2, vert_index % 2) * 2.f - 1.f;

  const float k = 2.0f*(approxLog(rgba.a)-log_p_min);


  float radius;
  if (k>0.0f && cc > k) {
    radius = sqrt(k / (1.0f - ccRecip*k));
  } else {
    radius = rgba.a = 0.f;
  }

  out_position = radius * position;
  gl_Position= projection * vec4(T*vec3(out_position, 1.f),1.f);
}
