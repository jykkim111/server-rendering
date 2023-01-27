#include "RemoteTreeModel.h"

#include <QColor>
#include <QBrush>

#include <iostream>

RemoteTreeModel::RemoteTreeModel(
	cgip::RemoteFileTree* tree,
	QObject* parent)
	: QAbstractItemModel(parent)
{
	m_root_item = tree;
}

RemoteTreeModel::~RemoteTreeModel() {

}


QVariant RemoteTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		cgip::RemoteFileTree* item = getItem(index);

		int col = index.column();
		std::string item_data = "";
		if (col == 0) {
			item_data = item->getName();
			return QVariant(QString::fromStdString(item_data));
		}
		else
			return QVariant();
	}
	else if (role == Qt::TextAlignmentRole) {
		int col = index.column();

		//if(col == 1) return Qt::AlignCenter;
	}
	return QVariant();
}

QVariant RemoteTreeModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		if (m_root_item) {
			if (section == 0) {
				return QVariant(QString("Name"));
			}

		}
	}
	return QVariant();
}

QModelIndex RemoteTreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	cgip::RemoteFileTree* parentItem = getItem(parent);

	cgip::RemoteFileTree* childItem = parentItem->getChild(row);
	if (childItem) {
		return createIndex(row, column, childItem);
	}
	else
		return QModelIndex();
}

QModelIndex RemoteTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	cgip::RemoteFileTree* childItem = getItem(index);
	cgip::RemoteFileTree* parentItem = childItem->getParent();

	if (parentItem == m_root_item)
		return QModelIndex();
	return createIndex(parentItem->getChildIndex(), 0, parentItem);
}

bool RemoteTreeModel::removeRow(int row, const QModelIndex& parent) {
	cgip::RemoteFileTree* parentItem = getItem(parent);
	if (!parentItem)
		return false;

	beginRemoveRows(parent, row, row);
	bool success = parentItem->removeChild(row);
	endRemoveRows();

	return success;
}

int RemoteTreeModel::rowCount(const QModelIndex& parent) const {
	cgip::RemoteFileTree* parent_item = getItem(parent);

	return parent_item->getCntChildren();
}

int RemoteTreeModel::columnCount(const QModelIndex& parent) const {
	// TODO:: fixed column 
	return 1;
}

void RemoteTreeModel::sort(int column, Qt::SortOrder order) {
	int order_val = 0;
	if (order == Qt::DescendingOrder) order_val = 1;
}


cgip::RemoteFileTree* RemoteTreeModel::getItem(const QModelIndex& index) const {
	if (index.isValid()) {
		cgip::RemoteFileTree* item = static_cast<cgip::RemoteFileTree*>(index.internalPointer());
		if (item) return item;
	}

	return m_root_item;
}