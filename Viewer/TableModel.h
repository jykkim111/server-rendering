#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


class TableModel : public QAbstractItemModel {
	Q_OBJECT;

public:
	TableModel(
		int r, 
		int c, 
		std::vector<std::string> data, 
		std::vector<std::string> header_data,
		QObject *parent = 0
	);
	~TableModel();

	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
	std::vector<std::string> m_table_data;
	std::vector<std::string> m_header_data;
	int m_n_rows, m_n_cols;
};
