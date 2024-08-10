#include "camera.h"
#include <cstring>

#define SIZE_F_MAT4 16

static mat3 viewCoordsLeftHanded(vec3 normal)
{
	vec3 basis[3];

	normal = normalize(normal);
	basis[2] = normal;
	basis[0] = cross(normal, vec3({ 0,0,1 }));
	basis[1] = cross(basis[0], basis[2]);
	basis[0] = normalize(basis[0]);
	basis[1] = normalize(basis[1]);

	return basis[0]|basis[1]|basis[2];
}

Camera::Camera(vec3 normal, vec3 pos, int w, int h, GLfloat FOV, GLfloat near, GLfloat far)
	:
	fov(FOV),
	near(near),
	far(far),
	position(pos)
{
	glGenBuffers(1, &ubo);
	coords = mat4(viewCoordsLeftHanded(normal));
	aspect = (float)h / (float)w;

	updateUbo();
}

Camera::~Camera()
{
	glDeleteBuffers(1,&ubo);
}

mat4 Camera::getViewMatrix()
{
	mat4 world = mat4(mat3::id() | -1 * (position - coords.col(2) * near));
	return coords.transpose()* world;;
}

mat4 Camera::getProjMatrix()
{
	return mat4({
		(1 / tan(fov / 2))*(aspect),0,0,0,
		0,1 / tan(fov / 2),0,0,
		0,0,far/(far - near),-far*near/(far - near),
		0,0,1,0
	});
}

void Camera::updateUbo()
{	
	mat4 proj = getProjMatrix().transpose();
	mat4 view = getViewMatrix().transpose();

	// Copy camera data into buffers
	uint16_t dsize = 2*SIZE_F_MAT4 + 4 + 4 + 2;
	GLfloat data[dsize];
	memcpy(data, view.data(), SIZE_F_MAT4*sizeof(float));
	memcpy(data + SIZE_F_MAT4, proj.data(), SIZE_F_MAT4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4, vec4(position).data(), 4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4 + 3, vec4(view).col(2).data(), 4*sizeof(float));
	data[2*SIZE_F_MAT4 + 8] = near;
	data[2*SIZE_F_MAT4 + 8 + 1] = far;

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, dsize*sizeof(float), data, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	return;
}
void Camera::bindUbo(GLuint binding) 
{
	glBindBufferBase(GL_UNIFORM_BUFFER,binding,ubo);
}

void Camera::rotate(float pitch, float yaw)
{
	if (abs(pitch) > PI) return;
	coords = mat4(rot_axis(vec3(coords.col(0)), pitch)) *  mat4(rot_axis(vec3{0,0,1},yaw)) * coords ;
}

void Camera::translate(vec3 delta)
{
	vec3 offsetxy = vec3(vec2(coords.col(0)*delta[0][0] + coords.col(2)*delta[0][2]));
	vec3 offsetz = vec3{0,0,1}*delta[0][1];
    position = position + offsetxy + offsetz;
}

void Camera::resize(int width, int height)
{
	this->aspect = (float)height/(float)width;
}