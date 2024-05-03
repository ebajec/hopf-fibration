#version 330 core

uniform float ANIM_SPEED;
uniform float TIME_S;

layout (location = 0) in vec4 data_in;
out vec4 data_out;

float PI = 3.141592654;

vec3 sphere_path(float t) {
    float time = TIME_S*ANIM_SPEED;
    t = 2*PI*t;
    float s = t + PI*cos(t) - time;
    return vec3(sin(t)*cos(s),sin(t)*sin(s),cos(t));
}

vec4 compute_hopf_inverse(vec4 s) {
    float t = 2*PI*s.w;
    return vec4(
        -sin(t)*(1+s.x),
        cos(t)*(1+s.x),
        cos(t)*s.y - sin(t)*s.z,
        cos(t)*s.z + sin(t)*s.y
    )/sqrt(2*(1+s.x));
}

vec4 stereo_proj(vec4 v) {
    return vec4(v.x/(1-v.w),v.y/(1-v.w),v.z/(1-v.w),data_in.x);
}

void main() {
    vec4 s2_point = vec4(sphere_path(data_in.x),data_in.w);
    vec4 hopf_map = compute_hopf_inverse(s2_point);
    data_out = stereo_proj(hopf_map);
}
