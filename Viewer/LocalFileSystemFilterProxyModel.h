#pragma once

#include <QSortFilterProxyModel>

class LocalFileSystemFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	LocalFileSystemFilterProxyModel(QStringList exts, QObject* parent = 0);

	/**
 *	Name:		filterAcceptsRow
 *	Author:		Sanguk Park
 *	Params:
 *		- source_row:	    int
 *			row index of QFileSystemModel
 *		- source_parent:	const QModelIndex&
 *			QT Model Index of parent node
 *	Desc:		Return the boolean for acceptance
 *				'True' for allowing the showing row, 'False' for hiding the row.
 */
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

	/**
	 *	Name:		lessThan
	 *	Author:		Sanguk Park
	 *	Params:
	 *		- left:		const QModelIndex&
	 *			QT Model Index for comparing
	 *		- right:	const QModelIndex&
	 *			QT Model Index for comparing
	 *	Desc:		return the boolean for sorting
	 *				directory will be bigger than files, and applied alphabetic order.
	 */
	bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

	/**
	 *	Name:		filePath
	 *	Author:		Sanguk Park
	 *	Params:
	 *		- index:	const QModelIndex&
	 *			QT Model Index to get file path.
	 *	Desc:		return the QT string of absolute file path in sytstem.
	 */
	QString filePath(const QModelIndex& index) const;

private:
	// Allowed extensions list
	QStringList m_include_exts;
};