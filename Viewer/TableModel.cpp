#include "TableModel.h"


TableModel::TableModel(int r, int c, std::vector<std::string> data, std::vector<std::string> header_data, QObject *parent) : QAbstractItemModel(parent) {
	m_n_rows = r;
	m_n_cols = c;
	m_table_data = data;
	m_header_data = header_data;
}

TableModel::~TableModel() {

}

QVariant TableModel::data(const QModelIndex &index, int role) const {
	if (index.isValid() && role == Qt::DisplayRole) {
		int row = index.row();
		int col = index.column();
		std::string item = m_table_data[row * m_n_cols + col];
		return QVariant(QString::fromStdString(item));
	}

	return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation,	int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QVariant(QString::fromStdString(m_header_data[section]));

	return QVariant();
}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const {
	if (parent.isValid())
		return QModelIndex();

	return createIndex(row, column);
}

QModelIndex TableModel::parent(const QModelIndex &index) const {
	return QModelIndex();
}

int TableModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;

	return m_n_rows;
}

int TableModel::columnCount(const QModelIndex &parent) const {
	return m_n_cols;
}
