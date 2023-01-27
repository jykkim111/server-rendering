#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "RemoteFileTree.h"

class cgip::RemoteFileTree;

class RemoteTreeModel : public QAbstractItemModel {

public:
	RemoteTreeModel(cgip::RemoteFileTree* tree, QObject* parent = 0);
	~RemoteTreeModel();

	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	bool removeRow(int row, const QModelIndex& parent);

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	cgip::RemoteFileTree* getItem(const QModelIndex& index) const;
private:

	cgip::RemoteFileTree* m_root_item;
	int m_sorting_counter = 0;
};