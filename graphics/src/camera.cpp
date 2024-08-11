#include "camera.h"
#include <cstring>
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"
#include "misc.h"

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

	return mat3(basis[0],basis[1],basis[2]);
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
	mat4 world = mat4(1.0f);
	world[3] =	 vec4(-1.0f *(position - vec3(coords[2]) * near),1);
	return glm::transpose(coords)*world;;
}

mat4 Camera::getProjMatrix()
{
	return mat4(
		vec4((1 / tan(fov / 2))*(aspect),0,0,0),
		vec4(0,1 / tan(fov / 2),0,0),
		vec4(0,0,far/(far - near),1),
		vec4(0,0,-far*near/(far - near),0)
	);
}

void Camera::updateUbo()
{	
	mat4 proj = getProjMatrix();
	mat4 view = getViewMatrix();
	vec4 tempPos = vec4(position,1);

	// Copy camera data into buffers
	uint16_t dsize = 2*SIZE_F_MAT4 + 4 + 4 + 2;
	GLfloat data[dsize];
	memcpy(data, glm::value_ptr(view), SIZE_F_MAT4*sizeof(float));
	memcpy(data + SIZE_F_MAT4, glm::value_ptr(proj), SIZE_F_MAT4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4, glm::value_ptr(tempPos), 4*sizeof(float));
	memcpy(data + 2*SIZE_F_MAT4 + 3, glm::value_ptr(view[2]), 4*sizeof(float));
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
	coords = mat4(rotation(vec3(coords[0]), -pitch)) *  mat4(rotation(vec3(0,0,1),-yaw)) * coords ;
}

void Camera::translate(vec3 delta)
{
    position = position + mat3(coords[0],vec3(0,0,1),vec3(coords[2].x,coords[2].y,0))*delta;
}

void Camera::resize(int width, int height)
{
	this->aspect = (float)height/(float)width;
}