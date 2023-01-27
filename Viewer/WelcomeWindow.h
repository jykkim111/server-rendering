#pragma once

// Qt includes
#include <QWidget>
#include <QLayout>
#include <QTreeView>
#include <QStackedWidget>
#include <QPushButton>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QstandardItemModel>
#include <QIcon>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

#include <string>
#include <iostream>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

// Global includes
#include "../ViewerManager/global.h"

// Local includes
#include "build_global_opt.h"
#include "SliceWidget.h"
#include "LocalFileSystemFilterProxyModel.h"
#include "RemoteFileTree.h"
#include "RemoteTreeModel.h"


class WelcomeWindow : public QWidget {
	Q_OBJECT
signals:
	void sigLoadLocalMPR(QString, bool);
	void sigLoadLocalPreviewVolume(QString);
	void sigReqLocalDeleteNode(std::vector<std::string> files);

	void sigLoadPreviewPlane(QString file_path, int slice_idx);
	void sigReturn();

public slots:
	void load_local_preview(QModelIndex idx);
	void load_local_mpr();
	void load_remote_preview(QModelIndex idx);
	void load_remote_mpr();

public:
	WelcomeWindow(QWidget *parent);
	void setPreview(SliceWidget*);
	void buildRemoteFileTree(QJsonObject);
	inline void setLocalModality(std::string modality) { m_local_modality = modality; }

private:
	QHBoxLayout *m_main_layout, *m_buttons_layout_1, *m_buttons_layout_2, *m_buttons_layout_3;
	QWidget *buttons_widget, *m_preview_widget;
	QVBoxLayout *m_table_layout;
	QStackedWidget *m_stacked_trees;

	QPushButton *m_modality_CT, *m_modality_MR, *m_modality_US;
	QPushButton *m_dataset_liver, *m_dataset_pancreas;
	QPushButton *m_refresh_btn;
	QPushButton *m_return_btn, *m_load_btn;

	QTreeView *local_tree_widget;
	// QT File System Model for finding local system hierarchy
	QFileSystemModel *m_local_model;
	// For appling sort filter
	LocalFileSystemFilterProxyModel* m_local_proxy_model;

	int m_cur_modality_idx, m_cur_ds_idx;
	QString m_preview_path;
	std::vector<std::string> m_files;
	int m_preview_plane_idx;
	std::string m_local_modality;
	QModelIndex m_local_idx;

	QTreeView* m_remote_tree_widget;
	RemoteTreeModel* m_remote_model;

	QMetaObject::Connection m_cnnct_local_load_signal;
	QMetaObject::Connection m_cnnct_remote_load_signal;

	cgip::RemoteFileTree* buildRemoteFileTree();
};
