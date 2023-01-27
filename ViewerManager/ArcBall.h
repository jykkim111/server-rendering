#pragma once
#include <qvector3d.h>
#include <qpoint.h>
#include <math.h>
#include <qmatrix4x4.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"

using namespace std;

class ArcBall
{
public:
	ArcBall(void);
	~ArcBall(void);

public:

	void setWidthHeight(int w, int h);
	void setPrincipalAxis(int p_axis) { m_pAxis = p_axis; }
	glm::vec3 getArcballVector(int x, int y);
	glm::vec3 getRotationAxis(glm::vec3 startPos, glm::vec3 endPos);
	glm::mat4 getCurrentRotation(glm::mat4, glm::mat4);

private:
	// private fields.
	int		width;
	int		height;
	int		m_pAxis;

};

class Arcball
{
};


