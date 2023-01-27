#include "ArcBall.h"
#include <iostream>

const float PI = std::acos(-1.0f);

ArcBall::ArcBall(void) {
	width = 0;
	height = 0;
}

ArcBall::~ArcBall(void) {

}

void ArcBall::setWidthHeight(int w, int h) {
	width = w;
	height = h;
}




glm::vec3 ArcBall::getArcballVector(int x, int y) {
	
	glm::vec3 coord = glm::vec3(0, 0, 0);

	int diameter = std::min(width, height);

	coord.x = float((2.0 * x - diameter) / diameter);
	coord.z = float((2.0 * y - diameter) / diameter);



	if ( coord.x * coord.x + coord.z * coord.z <= 1.0) {
		coord.y = float(std::sqrt(1.0 - coord.x * coord.x - coord.z * coord.z));
	}
	else
	{
		coord = glm::normalize(coord);
		coord.y = 0.0f;
	}



	return coord;
}

glm::vec3 ArcBall::getRotationAxis(glm::vec3 startPos, glm::vec3 endPos) {
	float rotation_angle = std::acos(std::min(1.0f, glm::dot(startPos, endPos)));
	glm::vec3 rotation_axis = glm::cross(startPos, endPos);

	return rotation_axis;
}

glm::mat4 ArcBall::getCurrentRotation(glm::mat4 current_rotation, glm::mat4 last_rotation) {
	return current_rotation * last_rotation;
}