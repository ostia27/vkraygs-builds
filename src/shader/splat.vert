//All contributions from Meta and affiliates:
//Copyright (c) 2025- Meta Platforms, Inc. and affiliates            (Samuel Rota Bulo')

//All contributions by jaesung-cs:
//Copyright (c) 2024- jaesung-cs

#version 460

layout(std430, set = 1, binding = 1) readonly buffer Instances {
  vec4 instances[];  // (N, 12). 3 for ndc position, 1 padding, 4 for rot scale, 4 for color.
};

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_position;

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
  vec3 ndc_position = instances[index * 3 + 0].xyz;
  mat2 rot_scale = mat2(instances[index * 3 + 1].xy, instances[index * 3 + 1].zw);
  vec4 color = instances[index * 3 + 2];

  // quad positions (-1, -1), (-1, 1), (1, -1), (1, 1), ccw in screen space.
  int vert_index = gl_VertexIndex % 4;
  vec2 position = vec2(vert_index / 2, vert_index % 2) * 2.f - 1.f;

  float confidence_radius = sqrt(2.0*(approxLog(color.a)-log_p_min));

  out_position = position * confidence_radius;
  gl_Position = vec4(ndc_position + vec3(rot_scale * out_position, 0.f), 1.f);
  out_color = color;

}
