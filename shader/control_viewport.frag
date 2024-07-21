#version 330 core

in vec4 fcolor;
in vec3 fpos;
in vec3 fnormal;
out vec4 FragColor;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
    vec4 cam_pos;
    vec4 cam_dir;
	float near;
	float far;
};

float PI = 3.141592654;


vec4 calc_light(vec3 light, vec4 color) {
    vec3 light_ray = normalize(light - fpos);
    vec3 view_dir = normalize(vec3(cam_pos)-fpos);
    float diffuse = max(dot(fnormal,light_ray),0.0f);
    vec3 reflect_dir = reflect(-light_ray,fnormal);
    float spec = pow(max(dot(view_dir,reflect_dir),0.0f),32);

    return (diffuse + 0.2) * color + spec*vec4(1);
}

vec3 hsvtorgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{ 

    vec4 color =    calc_light(vec3(5,5,5),fcolor)
                +   calc_light(vec3(5,8,-8),fcolor);
    FragColor = color;
} 