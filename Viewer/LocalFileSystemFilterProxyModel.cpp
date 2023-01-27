#include <QFileSystemModel>
#include <QFileInfo>

#include "LocalFileSystemFilterProxyModel.h"


#include <iostream>

LocalFileSystemFilterProxyModel::LocalFileSystemFilterProxyModel(QStringList exts, QObject* parent)
    :QSortFilterProxyModel(parent) {

    m_include_exts = exts;
}

/**
 *	Name:		filterAcceptsRow
 *  Created:	2021-01-09
 *	Modified:	2021-01-09
 *	Author:		Sanguk Park
 *	Params:
 *		- source_row:	    int
 *			row index of QFileSystemModel
 *		- source_parent:	const QModelIndex&
 *			QT Model Index of parent node
 *	Desc:		Return the boolean for acceptance
 *				'True' for allowing the showing row, 'False' for hiding the row.
 */
bool LocalFileSystemFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    QFileSystemModel* fsm = qobject_cast<QFileSystemModel*>(sourceModel());

    QModelIndex idx = fsm->index(source_row, 0, source_parent);
    QString file_path = fsm->filePath(idx);

    // Allow directory row 
    if (QFileInfo(file_path).isDir()) {
        return true;
    }

    // Allow file with selected extensions 
    for (auto ext : m_include_exts) {
        if (file_path.toLower().endsWith(ext))
            return true;
    }
    return false;

}


/**
 *	Name:		lessThan
 *  Created:	2021-01-05
 *	Modified:	2021-01-05
 *	Author:		Sanguk Park
 *	Params:
 *		- left:		const QModelIndex&
 *			QT Model Index for comparing
 *		- right:	const QModelIndex&
 *			QT Model Index for comparing
 *	Desc:		return the boolean for sorting
 *				directory will be bigger than files, and applied alphabetic order.
 */
bool LocalFileSystemFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    // If sorting by file names column
    if (sortColumn() == 0) {
        QFileSystemModel* fsm = qobject_cast<QFileSystemModel*>(sourceModel());
        bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

        QFileInfo leftFileInfo = fsm->fileInfo(left);
        QFileInfo rightFileInfo = fsm->fileInfo(right);


        // If DotAndDot move in the beginning
        if (sourceModel()->data(left).toString() == "..")
            return asc;
        if (sourceModel()->data(right).toString() == "..")
            return !asc;

        // Move dirs upper
        if (!leftFileInfo.isDir() && rightFileInfo.isDir()) {
            return !asc;
        }
        if (leftFileInfo.isDir() && !rightFileInfo.isDir()) {
            return asc;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
};

/**
 *	Name:		filePath
 *  Created:	2021-01-05
 *	Modified:	2021-01-05
 *	Author:		Sanguk Park
 *	Params:
 *		- index:	const QModelIndex&
 *			QT Model Index to get file path.
 *	Desc:		return the QT string of absolute file path in sytstem.
 *              using the source QFileSystemModel.
 */
QString LocalFileSystemFilterProxyModel::filePath(const QModelIndex& index) const {
    QFileSystemModel* fsm = qobject_cast<QFileSystemModel*>(sourceModel());
    auto const source_index = mapToSource(index);
    return fsm->filePath(source_index);
}
