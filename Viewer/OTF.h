#pragma once

#include <qgraphicsitem.h>

//#define TF_HEIGHT	240		// QGraphicsScene coordinate. // fixed view height : 260
//#define TF_MARGIN	22		// QGraphicsScene coordinate.
#define TF_HEIGHT	180		// QGraphicsScene coordinate. // fixed view height : 200
#define TF_MARGIN	22		// QGraphicsScene coordinate.

/* Element of TFOBJECT */
enum class COMPONENT {
	POLYGON,
	POINT,
	LINE,
	COLOR,
	NONE
};

/* Current Component id. */
enum class CURR_COMPONENT {
	BACKGROUND,
	POINT,
	LINE,
	POLYGON,
	COLOROBJ,
	COLORTAB
};

/* ELEMENT structure for specify elements */
// used to track Current Activated Component.
struct ELEMENT {
	COMPONENT	_which;
	int		_iIdx;
	int		_iPolyIdx;
};