#include "OTFPresetViewer.h"
#include <iostream>

OTFPresetViewer::OTFPresetViewer(QStringList listPath, QPointF pos, QWidget* parent)
	: m_pModel(nullptr), m_listNames(listPath), m_nLastIndex(-1), QDialog(parent)
{

	ui.setupUi(this);
	this->setWindowTitle(QString("OTF Preset"));
	this->setWindowFlags(Qt::WindowStaysOnTopHint);
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
	this->move(pos.x(), pos.y());

	m_pModel = new QStandardItemModel();
	ui.m_listPreview->setModel(m_pModel);

	for (const auto& pathAt : m_listNames) {
		QString strName = pathAt.split('/').last().split('.').first();
		std::cout << "str name: " << strName.toStdString() << std::endl;
		QStandardItem* pItem = new QStandardItem(strName);
		m_pModel->appendRow(pItem);
	}
	

	this->setConnections();
}

void OTFPresetViewer::setConnections(void)
{
	connect(ui.m_listPreview, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotImgSelected(QModelIndex)));
	connect(ui.m_btnSelect, SIGNAL(pressed(void)), this, SLOT(slotItemSelected(void)));
	connect(ui.m_btnAdd, SIGNAL(pressed(void)), this, SLOT(slotPresetAdd(void)));
	connect(ui.m_btnDelete, SIGNAL(pressed(void)), this, SLOT(slotPresetDelete(void)));
	connect(ui.m_btnRename, SIGNAL(pressed(void)), this, SLOT(slotPresetRename(void)));
}

OTFPresetViewer::~OTFPresetViewer()
{
	//SAFE_DELETE_OBJECT(m_pModel);
}

void OTFPresetViewer::closeEvent(QCloseEvent* pEvent)
{
	QDialog::closeEvent(pEvent);
	
}
void OTFPresetViewer::keyPressEvent(QKeyEvent* pEvent)
{
	if (pEvent->key() == Qt::Key_Escape) {

	}
	QDialog::keyPressEvent(pEvent);
}


void OTFPresetViewer::slotImgSelected(QModelIndex index)
{
	// row is an item index.
	int idx = index.row();
	emit sigSelected(idx);
	close();
}

void OTFPresetViewer::slotItemSelected(void)
{
	//m_pModel->
}
void OTFPresetViewer::slotPresetAdd(void)
{
	
	std::cout << "slotPresetAdd\n";
}
void OTFPresetViewer::slotPresetDelete(void)
{
	std::cout << "slotPresetDelete\n";
}
void OTFPresetViewer::slotPresetRename(void)
{
	std::cout << "slotPresetRename\n";
}
