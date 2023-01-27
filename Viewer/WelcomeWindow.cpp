#include <QSignalMapper>
#include <QHeaderView>

#include "WelcomeWindow.h"

WelcomeWindow::WelcomeWindow(QWidget *parent) : QWidget(parent) {

	m_main_layout = new QHBoxLayout;
	m_buttons_layout_1 = new QHBoxLayout;
	m_buttons_layout_2 = new QHBoxLayout;
	m_buttons_layout_3 = new QHBoxLayout;
	m_table_layout = new QVBoxLayout;

	m_stacked_trees = new QStackedWidget;

	m_return_btn = new QPushButton("return");
	m_load_btn = new QPushButton("load");
	m_return_btn->setEnabled(false);
	m_load_btn->setEnabled(false);

#ifdef LOCAL_VIEW
	// Set local file system model
	QString root_path = "";
	m_local_model = new QFileSystemModel();
	m_local_model->setRootPath(root_path);

	// Set allowed extensions
	QStringList filters;
	filters.append(".dcm");
	filters.append(".vol");

	m_local_proxy_model = new LocalFileSystemFilterProxyModel(filters, this);
	m_local_proxy_model->setDynamicSortFilter(true);
	m_local_proxy_model->setSourceModel(m_local_model);
	m_local_proxy_model->setRecursiveFilteringEnabled(true);
	m_local_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);

	local_tree_widget = new QTreeView(m_stacked_trees);

	// Set proxy model to view widget
	local_tree_widget->setModel(m_local_proxy_model);
	local_tree_widget->setRootIndex(m_local_proxy_model->mapFromSource(m_local_model->index(root_path)));
	local_tree_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	local_tree_widget->setExpandsOnDoubleClick(false);
	local_tree_widget->setAlternatingRowColors(true);
	local_tree_widget->setColumnWidth(0, 300);
	m_stacked_trees->addWidget(local_tree_widget);
#endif

	m_table_layout->addWidget(m_stacked_trees);

	m_buttons_layout_3->addWidget(m_load_btn);
	m_buttons_layout_3->addWidget(m_return_btn);

	m_main_layout->addLayout(m_table_layout);
	m_preview_widget = new QWidget;
	m_main_layout->addWidget(m_preview_widget);

	this->setLayout(m_main_layout);

	QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	setSizePolicy(size_policy);
	this->setMouseTracking(true);

	m_cur_modality_idx = 0;
	m_cur_ds_idx = 0;

	QSignalMapper *modality_mapper = new QSignalMapper(this);
	QSignalMapper *dataset_mapper = new QSignalMapper(this);

#ifdef LOCAL_VIEW
	connect(local_tree_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(load_local_preview(QModelIndex)));
#endif

	connect(m_return_btn, SIGNAL(clicked()), this, SIGNAL(sigReturn()));

	m_stacked_trees->setCurrentIndex(0);


#ifdef REMOTE_VIEW
	//buildRemoteFileTree();
#endif
}

void WelcomeWindow::setPreview(SliceWidget *preview_axial) {
	QVBoxLayout *preview_layout = new QVBoxLayout;
	preview_layout->addWidget(preview_axial);
	preview_layout->addLayout(m_buttons_layout_3);
	m_preview_widget->setLayout(preview_layout);
}

/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */
/**
	 *	Name:		load_local_preview
	 *	Created:	
	 *	Modified:	2021-01-21
	 *  Author:		CHAEN LEE
	 *	Params:
	 *		- idx:		QModelIndex
	 *			index of current local model tree
	 *	Desc:		Load local preview when clicked
	 */
void WelcomeWindow::load_local_preview(QModelIndex idx) {
	QString selected_file_path = m_local_proxy_model->filePath(idx);

	// Return when selected directory
	if (m_local_proxy_model->hasChildren(idx) 
		|| QFileInfo(selected_file_path).isDir())
		return;

	emit sigLoadPreviewPlane(selected_file_path, 0);
	//emit sigLoadLocalPreviewVolume(m_local_proxy_model->filePath(idx));

	m_local_idx = idx;
	m_load_btn->setEnabled(true);
	disconnect(m_cnnct_local_load_signal);
	m_cnnct_local_load_signal
		= connect(m_load_btn, SIGNAL(clicked()), this, SLOT(load_local_mpr()));
}

void WelcomeWindow::load_local_mpr() {
	m_return_btn->setEnabled(true);
	m_load_btn->setEnabled(false);

	// Disable connection on the load btn
	disconnect(m_cnnct_local_load_signal);

	// if ultrasound, load only leaf node
	bool is_leaf;
	QString selected_path = m_local_proxy_model->filePath(m_local_idx);
	QString parent_path = m_local_proxy_model->filePath(m_local_idx.parent());

	if (selected_path.toLower().endsWith(".dcm")) {	// Dicom files
		if (m_local_modality == "US" || m_local_modality == "ES") {
			m_preview_path = selected_path;
			is_leaf = true;
		}
		else {
			m_preview_path = parent_path;
			is_leaf = false;
		}
	}
	else if (selected_path.toLower().endsWith(".vol")) {
		m_preview_path = selected_path;
		is_leaf = true;
	} else{
		// TODO: Handle error
		// TODO: Add image(png) slices
	}
	
	emit sigLoadLocalMPR(m_preview_path, is_leaf);
}


void WelcomeWindow::load_remote_mpr() {
	m_return_btn->setEnabled(true);
	m_load_btn->setEnabled(false);

	// Disable connection on the load btn
	disconnect(m_cnnct_local_load_signal);

	QModelIndex parent_idx = m_local_idx.parent();
	cgip::RemoteFileTree* parent_item = (cgip::RemoteFileTree*)m_remote_model->getItem(parent_idx);

	if (m_preview_path.toLower().endsWith(".dcm")) {
		m_preview_path = QString(parent_item->getPath().c_str());
		emit sigLoadLocalMPR(m_preview_path, false);
	}
	else if (m_preview_path.toLower().endsWith(".vol")) {
		emit sigLoadLocalMPR(m_preview_path, true);
	}
}

void WelcomeWindow::load_remote_preview(QModelIndex idx) {
	cgip::RemoteFileTree* item = (cgip::RemoteFileTree*) m_remote_model->getItem(idx);

	if (!item->getIsFile()) return;

	//if (!(QString(item->getPath().c_str()).toLower().endsWith(".dcm") || QString(item->getPath().c_str()).toLower().endsWith(".vol"))) return;

	m_load_btn->setEnabled(true);
	disconnect(m_load_btn, &QPushButton::clicked, 0, 0);

	m_local_idx = idx;
	m_preview_path = QString(item->getPath().c_str());

	m_cnnct_local_load_signal
		= connect(m_load_btn, SIGNAL(clicked()), this, SLOT(load_remote_mpr()));
}

cgip::RemoteFileTree* WelcomeWindow::buildRemoteFileTree() {
	std::ifstream i("./path/dataset_path.json");
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

	m_remote_tree_widget = new QTreeView(m_stacked_trees);
	m_remote_tree_widget->setAlternatingRowColors(true);
	m_remote_tree_widget->setExpandsOnDoubleClick(false);
	//m_remote_tree_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// Enable multi-select
	m_remote_tree_widget->setStyleSheet("QHeaderView::section { font: bold; background-color: #3291C9 }");

	m_remote_model = new RemoteTreeModel(tree_root);
	m_remote_tree_widget->setModel(m_remote_model);

	m_stacked_trees->addWidget(m_remote_tree_widget);

	connect(m_remote_tree_widget, &QTreeView::doubleClicked, [=](QModelIndex idx) { load_remote_preview(idx); });

	return tree_root;
}

void WelcomeWindow::buildRemoteFileTree(QJsonObject json_path) {

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

	m_remote_tree_widget = new QTreeView(m_stacked_trees);
	m_remote_tree_widget->setAlternatingRowColors(true);
	m_remote_tree_widget->setExpandsOnDoubleClick(false);
	//m_remote_tree_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// Enable multi-select
	m_remote_tree_widget->setStyleSheet("QHeaderView::section { font: bold; background-color: #3291C9 }");

	m_remote_model = new RemoteTreeModel(tree_root);
	m_remote_tree_widget->setModel(m_remote_model);

	m_stacked_trees->addWidget(m_remote_tree_widget);

	connect(m_remote_tree_widget, &QTreeView::doubleClicked, [=](QModelIndex idx) { load_remote_preview(idx); });
}
