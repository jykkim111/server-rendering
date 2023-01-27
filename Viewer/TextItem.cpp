#include "TextItem.h"
#include <qstring.h>
#include <qfont.h>

TextItem::TextItem(const QPointF pt, float val)
{
	this->m_val = val;
	QString str("[" + QString().setNum(val, 'f', 0) + "]");
	this->setPlainText(str);

	QFont font("Times", 10, QFont::Bold);
	this->setFont(font);
	this->setDefaultTextColor(Qt::yellow);
	this->setFlag(QGraphicsTextItem::ItemIgnoresTransformations, true);
	this->setPos(pt.x(), pt.y());
}

TextItem::~TextItem(void)
{
}


