#pragma once

#include <QDialog>

class QLineEdit;

class InputVolumeDimensionDialog : public QDialog {
	Q_OBJECT;

public:
	InputVolumeDimensionDialog(QWidget* parent = nullptr);
	
	static QStringList getStrings(QWidget *parent);
private:
	QList<QLineEdit*> m_fields;
};