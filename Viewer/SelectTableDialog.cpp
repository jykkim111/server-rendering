#include "SelectTableDialog.h"

SelectTableDialog::SelectTableDialog(RemoteTreeModel* table_model, const char* title)
	: QDialog(0, Qt::Popup)
{
	setWindowTitle(QString(title));

	QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	setSizePolicy(size_policy);
	this->resize(600, 200);

	m_table_view = new QTreeView(this);
	m_table_view->setAlternatingRowColors(true);
	m_table_view->setExpandsOnDoubleClick(false);
	m_table_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_table_view->setStyleSheet("QHeaderView::section { font: bold; background-color: #3291C9 }");

	m_table_view->setModel(table_model);

	for(int i=0;i< table_model->columnCount(); i++)
		m_table_view->resizeColumnToContents(i);

	QHBoxLayout* dialog_layout = new QHBoxLayout(this);
	dialog_layout->addWidget(m_table_view);

	QPushButton* load_button = new QPushButton("load");
	dialog_layout->addWidget(load_button);

	setLayout(dialog_layout);

	connect(load_button, &QPushButton::clicked, [this]() {
		m_indexes = m_table_view->selectionModel()->selectedIndexes();
		done(0);
	});

	connect(this, &SelectTableDialog::rejected, []() {});

	exec();
}

std::vector<std::string> getSelectMaskInfoWithDialog() {
	std::ifstream i("./path/mask_path.json");
	json json_path;
	i >> json_path;

	std::map<std::string, cgip::RemoteFileTree*> root_map;

	cgip::RemoteFileTree* tree_root = new cgip::RemoteFileTree("0", json_path["0"]["-1"][1], json_path["0"]["-1"][0], false);
	root_map.insert(std::pair<std::string, cgip::RemoteFileTree*>("0", tree_root));

	for (auto& el : json_path.items()) {
		//std::cout << el.key() << std::endl;

		cgip::RemoteFileTree* subroot = new cgip::RemoteFileTree(el.key(), json_path[el.key()]["-1"][1], json_path[el.key()]["-1"][0], false);
		root_map.insert(std::pair<std::string, cgip::RemoteFileTree*>(el.key(), subroot));
	}

	for (auto& el : json_path.items()) {
		//std::cout << el.key() << std::endl;

		for (auto& child_el : el.value().items()) {
			//std::cout << ">>" << child_el.key() << child_el.value() << std::endl;

			if (root_map.find(child_el.key()) == root_map.end()) {
				if (child_el.key() == "-1") continue;
				if (child_el.value()[2] == "false") continue;

				cgip::RemoteFileTree* child = new cgip::RemoteFileTree(child_el.key(), child_el.value()[1], child_el.value()[0], true);
				root_map[el.key()]->addChild(child);
			}
			else {
				root_map[el.key()]->addChild(root_map[child_el.key()]);
			}

		}
	}

	RemoteTreeModel* table_model = new RemoteTreeModel(tree_root);
	SelectTableDialog* select_table = new SelectTableDialog(table_model, "Select Mask");

	std::vector<std::string> directories;
	QModelIndexList indexes = select_table->getDirectories();
	for (int i = 0; i < indexes.size(); i++) {
		cgip::RemoteFileTree* item = (cgip::RemoteFileTree*) table_model->getItem(indexes[i]);
		directories.push_back(item->getPath());
	}

	return directories;
}

std::vector<std::string> getSelectMaskInfoWithDialog(QJsonObject json_path) {
	std::map<std::string, cgip::RemoteFileTree*> root_map;

	QJsonObject json_root_path = json_path.value("0").toObject();
	QJsonArray json_root_array = json_root_path.value("-1").toArray();

	cgip::RemoteFileTree* tree_root = new cgip::RemoteFileTree("0", json_root_array[1].toString().toStdString(),
		json_root_array[0].toString().toStdString(), false);
	root_map.insert(std::pair<std::string, cgip::RemoteFileTree*>("0", tree_root));

	for (auto& el : json_path.keys()) {
		//std::cout << el.toStdString() << std::endl;

		QJsonObject root_children = json_path.value(el).toObject();
		QJsonArray root_children_array = root_children.value("-1").toArray();

		cgip::RemoteFileTree* subroot = new cgip::RemoteFileTree(el.toStdString(), root_children_array[1].toString().toStdString(),
			root_children_array[0].toString().toStdString(), false);
		root_map.insert(std::pair<std::string, cgip::RemoteFileTree*>(el.toStdString(), subroot));
	}

	for (auto& el : json_path.keys()) {
		//std::cout << el.toStdString() << std::endl;

		QJsonObject root_children = json_path.value(el).toObject();
		for (auto& child_el : root_children.keys()) {
			//std::cout << ">>" << child_el.toStdString() << std::endl;

			if (root_map.find(child_el.toStdString()) == root_map.end()) {
				if (child_el.toStdString() == "-1") continue;
				QJsonArray child_el_array = root_children.value(child_el).toArray();
				if (child_el_array[2] == "false") continue;

				cgip::RemoteFileTree* child = new cgip::RemoteFileTree(child_el.toStdString(), child_el_array[1].toString().toStdString(),
					child_el_array[0].toString().toStdString(), true);
				root_map[el.toStdString()]->addChild(child);
			}
			else {
				root_map[el.toStdString()]->addChild(root_map[child_el.toStdString()]);
			}

		}
	}

	RemoteTreeModel* table_model = new RemoteTreeModel(tree_root);
	SelectTableDialog* select_table = new SelectTableDialog(table_model, "Select Mask");

	std::vector<std::string> directories;
	QModelIndexList indexes = select_table->getDirectories();
	for (int i = 0; i < indexes.size(); i++) {
		cgip::RemoteFileTree* item = (cgip::RemoteFileTree*) table_model->getItem(indexes[i]);
		directories.push_back(item->getPath());
	}

	return directories;
}