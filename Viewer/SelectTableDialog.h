#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTreeView>
#include <QLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <algorithm>
#include <map>
#include <json.hpp>

#include "RemoteTreeModel.h"

#include <fstream>

using json = nlohmann::json;

class SelectTableDialog : public QDialog
{
	Q_OBJECT

public:
	SelectTableDialog(RemoteTreeModel* table_model, const char* title = "Select");

	inline QModelIndexList getDirectories() { return m_indexes; }

private:
	QTreeView* m_table_view;
	QModelIndexList m_indexes;
};

// TODO:: replace with lambda or rename
inline bool cmp(std::pair<int, long>& a, std::pair<int, long>& b) {
	return a.second >= b.second;
}
std::vector<std::string> getSelectMaskInfoWithDialog();
std::vector<std::string> getSelectMaskInfoWithDialog(QJsonObject json_path);