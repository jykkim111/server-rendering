#pragma once


#include <qgraphicsitem.h>
#include <qpoint.h>

/*
	class TextItem_otf
	 : Re-implementation of QGraphicsTextItem
*/
class TextItem : public QGraphicsTextItem
{	
	float m_val;
public:
	TextItem(const QPointF pt, float val);
	~TextItem(void);

	void setVal(float val) { this->m_val = val; }
	float getVal() { return m_val; }

};

