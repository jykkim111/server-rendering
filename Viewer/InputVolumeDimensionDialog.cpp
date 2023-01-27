#include "InputVolumeDimensionDialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

InputVolumeDimensionDialog::InputVolumeDimensionDialog(QWidget* parent) 
	: QDialog(parent) 
{
	QFormLayout *layout_dialog = new QFormLayout(this);

	QLabel* width_label = new QLabel(QString("Width"), this);
	QLabel* height_label = new QLabel(QString("Height"), this);
	QLabel* depth_label = new QLabel(QString("Depth"), this);

	QLineEdit* width_edit = new QLineEdit(this);
	QLineEdit* height_edit = new QLineEdit(this);
	QLineEdit* depth_edit = new QLineEdit(this);

	layout_dialog->addRow(width_label, width_edit);
	layout_dialog->addRow(height_label, height_edit);
	layout_dialog->addRow(depth_label, depth_edit);

	m_fields.append(width_edit);
	m_fields.append(height_edit);
	m_fields.append(depth_edit);

	QDialogButtonBox* btn_box = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, this);
	layout_dialog->addWidget(btn_box);

	bool conn = connect(btn_box, &QDialogButtonBox::accepted, this, &InputVolumeDimensionDialog::accept);
	Q_ASSERT(conn);
	conn = connect(btn_box, &QDialogButtonBox::rejected, this, &InputVolumeDimensionDialog::reject);
	Q_ASSERT(conn);

	setLayout(layout_dialog);
}

QStringList InputVolumeDimensionDialog::getStrings(QWidget *parent) {
	InputVolumeDimensionDialog *dialog = new InputVolumeDimensionDialog();
	
	QStringList list_val;

	const int ret = dialog->exec();

	if (ret) {
		foreach(auto field, dialog->m_fields) {
			list_val.append(field->text());
		}
	}

	dialog->deleteLater();

	return list_val;
}
