#pragma once
#include <qdialog.h>
#include <qgraphicsscene.h>
#include <qgraphicsitem.h>
#include <qstandarditemmodel.h>
#include <qevent.h>
#include "UI_OTFPresetViewer.h"


class OTFPresetViewer : public QDialog
{

	Q_OBJECT

public:
	OTFPresetViewer(QStringList listPath, QPointF pos, QWidget* parent = 0);
	~OTFPresetViewer();

signals:
	void sigSelected(int);
	void sigClosed(void);

public slots:
	void slotImgSelected(QModelIndex);
	void slotItemSelected(void);
	void slotPresetAdd(void);
	void slotPresetDelete(void);
	void slotPresetRename(void);

protected:
	void closeEvent(QCloseEvent* pEvent);
	void keyPressEvent(QKeyEvent* pEvent);

private:
	void setConnections(void);

private:
	Ui::OTFPresetViewer	ui;
	QStringList				m_listNames;
	QStandardItemModel*		m_pModel;
	int					m_nLastIndex;

	QGraphicsScene* m_pScene;
	QGraphicsPixmapItem* m_pRenderPixmap;

};


