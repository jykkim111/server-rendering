#include "Viewer.h"

#include <QAction>
#include <QFileDialog>
#include <QShortcut>
#include <QToolBar>
#include <QPushButton>
#include <QFileInfo>
#include <qjsondocument.h>
#include <QtWebSockets/qwebsocket.h>
#include <qdebug.h>
#include <jpeg8/jpeglib8.h>
#include <QTimer>



#include "../CgipException/CgipException.h"
#include "SelectTableDialog.h"
#include "InputVolumeDimensionDialog.h"
#include "jpgd.h"
#include "iniparser.hpp"

#include <fstream>
#include <sstream>
#include <codecvt>


Viewer::Viewer(QWidget* parent)
	: QMainWindow(parent)
{
	this->installEventFilter(this);
	// Logging
	//freopen("log.txt", "a", stdout);
	m_logger_dev = spdlog::basic_logger_mt("logger_dev", "logs/dev.txt");
	m_logger_daily = spdlog::daily_logger_mt("logger_daily", "logs/daily.txt");
	m_logger_dev->set_level(spdlog::level::debug);
	m_logger_daily->flush_on(spdlog::level::info);
	m_logger_dev->flush_on(spdlog::level::debug);

	registerLogger(m_logger_dev);
	registerLogger(m_logger_daily);

	// Setting manager
	//m_setting_mgr = new SettingMgr();

	this->resize(m_width, m_height - 70);
	this->move(0, 0);
	g_nProgramHeight = m_height - 70;
	g_nProgramWidth = m_width;
	stacked_window = new QStackedWidget;
	setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);

	// main window for MPR
	m_view_slots = new ViewLayout(stacked_window);
	m_cpr_slots = new ViewLayout(stacked_window);

	axial_slice = new SliceWidget(m_view_slots, "Axial", AXIAL_SLICE);
	coronal_slice = new SliceWidget(m_view_slots, "Coronal", CORONAL_SLICE);
	sagittal_slice = new SliceWidget(m_view_slots, "Sagittal", SAGITTAL_SLICE);
	dvr = new DVRWidget(m_view_slots, "DVR");
	//otf_control = new OTFWidget(stacked_window);

	cpr_view = new CPRView(m_cpr_slots);
	//cpr_view_coronal = new CPRView(m_cpr_slots);
	//cpr_view_sagittal = new CPRView(m_cpr_slots);
	path_slice = new CPRPathWidget(m_cpr_slots, "CPR_path", AXIAL_SLICE);
	cpr_slice = new CPRWidget(m_cpr_slots, "CPR_slice", AXIAL_SLICE);
	m_view_slots->setMPRDVRLayout(axial_slice, coronal_slice, sagittal_slice, dvr);
	//m_view_slots->setViewFullScreen(axial_slice, coronal_slice, sagittal_slice, AXIAL_SLICE);

	// Toolbar to help your action
	m_tb_main = new MainToolBar(this);
	m_tb_main->setOrientation(Qt::Vertical);
	m_tb_main->setMovable(false);
	addToolBar(Qt::LeftToolBarArea, m_tb_main);
	m_tb_main->hide();

	// welcome widget
	m_welcome_window = new WelcomeWindow(stacked_window);
	preview_axial_slice = new SliceWidget(m_welcome_window, "Preview", AXIAL_SLICE);
	preview_axial_slice->toggleDrawGuideLine();
	m_welcome_window->setPreview(preview_axial_slice);

	// final settings
	stacked_window->addWidget(m_welcome_window);
	stacked_window->addWidget(m_view_slots);
	stacked_window->addWidget(m_cpr_slots);
	stacked_window->setCurrentIndex(0);
	setCentralWidget(stacked_window);

	// Create Volume manager
	//m_volume_mgr = new VolumeMgr;
	//m_preview_volume_mgr = new VolumeMgr;

	// Create Mask manager
	//m_mask_mgr = new MaskMgr;

	// Create MPR manager
	//m_mpr_mgr = new MPRMgr;
	//m_preview_mpr_mgr = new MPRMgr;


	//fps counter
	m_pTimer = std::make_shared<QTimer>();
	


#ifdef LOCAL_VIEW
	m_rendering_function_mgr = new RenderingFunctionMgr;
	m_rendering_function_mgr->initSamplingCnt(m_mpr_resolution, m_mpr_resolution, m_vr_resolution);
#endif

#ifdef REMOTE_VIEW

	//Create INI file if it does not exist in directory 
	/*
	std::string fullpath = "./config.ini";
	
	INI::File ft;

	if (!ft.Load(fullpath)) {
		std::cout << "Config.ini file does not exist in this directory" << std::endl;
	}

	std::string server_ip = ft.GetSection("WEBSOCKET")->GetValue("ip", -1).AsString();
	std::string server_port = ft.GetSection("WEBSOCKET")->GetValue("port", -1).AsString();
	QString full_URL = QString::fromStdString("ws://" + server_ip + ":" + server_port);

	//make connection with server 
	connect(&socket, &QWebSocket::connected, this, &Viewer::slotConnected);
	connect(&socket, &QWebSocket::disconnected, this, &Viewer::slotDisconnected);
	connect(&socket, &QWebSocket::binaryMessageReceived, this, &Viewer::onBinaryPacketReceived);
	connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
		[=](QAbstractSocket::SocketError error) {
			QMessageBox msgBox;
			msgBox.setText("Socket error :" + socket.errorString());
			msgBox.exec();


		});


	socket.open(QUrl(full_URL));
	std::cout << "attempting to connect socket..." << std::endl;
	*/
	connectSocket();
	m_popup = new QLabel;
	m_popup->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	m_popup->setFixedSize(QSize(200, 200));
	m_popup->setGeometry(((this->width() - m_popup->sizeHint().width()) / 2), (this->height() - m_popup->sizeHint().height()) / 2, 
							m_popup->sizeHint().width(), m_popup->sizeHint().height());
	m_loading = new QMovie("Images/loading.gif");
	m_popup->setMovie(m_loading);
	m_popup->hide();
#endif


	/* ----------------------------   Connect Signals   ------------------------- */

	//connect(m_volume_mgr, &VolumeMgr::sigReadVolume, [this](RemoteTreeType type) { slotLoadVolume(type); });
	//connect(m_preview_volume_mgr, &VolumeMgr::sigReadVolume, [this](RemoteTreeType type) { });
	//connect(m_preview_volume_mgr, SIGNAL(sigReadPlane()), this, SLOT(slotLoadPreviewPlane()));
	connect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(OnTimerCallbackFunction()));
	connect(m_view_slots, SIGNAL(sigChangeViewLayout()), this, SLOT(slotChangeViewLayout()));
	connect(m_view_slots, &ViewLayout::sigChangeAxialLayout, [this]() {this->slotFullScreen(AXIAL_SLICE);});
	connect(m_view_slots, &ViewLayout::sigChangeSagittalLayout, [this]() {this->slotFullScreen(SAGITTAL_SLICE);});
	connect(m_view_slots, &ViewLayout::sigChangeCoronalLayout, [this]() {this->slotFullScreen(CORONAL_SLICE);});
	connect(m_view_slots, &ViewLayout::sigChangeTF, this, &Viewer::slotTFChanged);
	connect(m_view_slots, &ViewLayout::sigChangeColor, this, &Viewer::slotColorChanged);
	connect(m_view_slots, &ViewLayout::sigOTFClicked, this, &Viewer::slotOTFClicked);
	connect(m_view_slots, &ViewLayout::sigOTFReleased, this, &Viewer::slotOTFReleased);
	connect(m_view_slots, &ViewLayout::sigChangeAll, this, &Viewer::slotChangeAll);

	connect(m_cpr_slots, SIGNAL(sigChangeViewLayout()), this, SLOT(slotChangeViewLayout()));
	connect(m_cpr_slots, &ViewLayout::sigChangeAxialLayout, [this]() {this->slotCPRChangeAxis(AXIAL_SLICE); });
	connect(m_cpr_slots, &ViewLayout::sigChangeSagittalLayout, [this]() {this->slotCPRChangeAxis(SAGITTAL_SLICE); });
	connect(m_cpr_slots, &ViewLayout::sigChangeCoronalLayout, [this]() {this->slotCPRChangeAxis(CORONAL_SLICE); });


	connect(m_tb_main, &MainToolBar::sigThicknessChanged, this, &Viewer::slotThicknessChanged);
	connect(m_tb_main, &MainToolBar::sigFunctionChanged, this, &Viewer::slotFunctionChanged);
	connect(m_tb_main, &MainToolBar::sigReqBackToWelcome, this, &Viewer::slotBackToWelcome);

	connect(m_tb_main, &MainToolBar::sigReqLoadMask, this, &Viewer::slotReqOpenLocalMask);
	connect(m_tb_main, &MainToolBar::sigReqExportMask, this, &Viewer::slotReqExportMask);
	connect(m_tb_main, &MainToolBar::sigReqAddMask, this, &Viewer::slotAddMask);
	connect(m_tb_main, &MainToolBar::sigReqChangeMaskVisible, this, &Viewer::slotSetMaskVisible);
	connect(m_tb_main, &MainToolBar::sigReqChangeMaskOpacity, this, &Viewer::slotChangeMaskOpacity);
	connect(m_tb_main, &MainToolBar::sigReqChangeMaskColor, this, &Viewer::slotChangeMaskColor);
	//connect(m_tb_main, &MainToolBar::sigOTFClicked, this, &Viewer::slotOTFClicked);
	//connect(m_tb_main, &MainToolBar::sigOTFReleased, this, &Viewer::slotOTFReleased);
	connect(m_tb_main, &MainToolBar::sigImageTypeChanged, this, &Viewer::slotImageTypeChanged);
	//connect(m_tb_main, &MainToolBar::sigChangeTF, this, &Viewer::slotTFChanged);
	//connect(m_tb_main, &MainToolBar::sigChangeColor, this, &Viewer::slotColorChanged);
	connect(m_tb_main, &MainToolBar::sigShaderStatus, this, &Viewer::slotShaderStatusChanged);
	connect(m_tb_main, &MainToolBar::sigSliderDragged, this, &Viewer::slotSliderDragged);
	connect(m_tb_main, &MainToolBar::sigSliderClicked, this, &Viewer::slotSliderClicked);
	connect(m_tb_main, &MainToolBar::sigSliderReleased, this, &Viewer::slotSliderReleased);

	connect(axial_slice, &SliceWidget::sigReqFullScreen, [this]() {this->slotCPRScreen(AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigReqFullScreen, [this]() {this->slotCPRScreen(CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigReqFullScreen, [this]() {this->slotCPRScreen(SAGITTAL_SLICE);});
	connect(axial_slice, &SliceWidget::sigReqCPRScreen, [this]() {this->slotCPRScreen(AXIAL_SLICE); });
	connect(coronal_slice, &SliceWidget::sigReqCPRScreen, [this]() {this->slotCPRScreen(CORONAL_SLICE); });
	connect(sagittal_slice, &SliceWidget::sigReqCPRScreen, [this]() {this->slotCPRScreen(SAGITTAL_SLICE); });


	//IMAGE ACTIONS
	connect(axial_slice, &SliceWidget::sigGuideLineMoved, [this](bool h, bool v, float dh, float dv) {this->slotGuideLineMoved(h, v, dh, dv, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigGuideLineMoved, [this](bool h, bool v, float dh, float dv) {this->slotGuideLineMoved(h, v, dh, dv, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigGuideLineMoved, [this](bool h, bool v, float dh, float dv) {this->slotGuideLineMoved(h, v, dh, dv, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigGuideLineRotated, [this](float a) {this->slotGuideLineRotated(a, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigGuideLineRotated, [this](float a) {this->slotGuideLineRotated(a, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigGuideLineRotated, [this](float a) {this->slotGuideLineRotated(a, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigGuideLineThickness, [this](float a, bool h, bool v) {this->slotGuideLineThickness(a, h, v, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigGuideLineThickness, [this](float a, bool h, bool v) {this->slotGuideLineThickness(a, h, v, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigGuideLineThickness, [this](float a, bool h, bool v) {this->slotGuideLineThickness(a, h, v, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigScreenDragged, [this](float dx, float dy, ClickType c) {this->slotScreenDragged(dx, dy, c, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigScreenDragged, [this](float dx, float dy, ClickType c) {this->slotScreenDragged(dx, dy, c, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigScreenDragged, [this](float dx, float dy, ClickType c) {this->slotScreenDragged(dx, dy, c, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigDoubleClicked, [this](float x, float y, ClickType c) {this->slotDoubleClicked(x, y, c, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigDoubleClicked, [this](float x, float y, ClickType c) {this->slotDoubleClicked(x, y, c, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigDoubleClicked, [this](float x, float y, ClickType c) {this->slotDoubleClicked(x, y, c, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigMouseReleased, [this](float x, float y, ClickType c) {this->slotMouseReleased(x, y, c, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigMouseReleased, [this](float x, float y, ClickType c) {this->slotMouseReleased(x, y, c, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigMouseReleased, [this](float x, float y, ClickType c) {this->slotMouseReleased(x, y, c, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigWheelChanged, [this](float d) {this->slotWheelChanged(d, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigWheelChanged, [this](float d) {this->slotWheelChanged(d, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigWheelChanged, [this](float d) {this->slotWheelChanged(d, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigClicked, [this](float x, float y, ClickType c) {this->slotClicked(x, y, c, AXIAL_SLICE);});
	connect(coronal_slice, &SliceWidget::sigClicked, [this](float x, float y, ClickType c) {this->slotClicked(x, y, c, CORONAL_SLICE);});
	connect(sagittal_slice, &SliceWidget::sigClicked, [this](float x, float y, ClickType c) {this->slotClicked(x, y, c, SAGITTAL_SLICE);});

	connect(axial_slice, &SliceWidget::sigZoomRatioChanged, [this](float ratio) {this->slotZoomByRatio(ratio, AXIAL_SLICE); });
	connect(coronal_slice, &SliceWidget::sigZoomRatioChanged, [this](float ratio) {this->slotZoomByRatio(ratio, CORONAL_SLICE); });
	connect(sagittal_slice, &SliceWidget::sigZoomRatioChanged, [this](float ratio) {this->slotZoomByRatio(ratio, SAGITTAL_SLICE); });

	connect(axial_slice, &SliceWidget::sigCPRInit, [this] () {this->slotCPRInit(AXIAL_SLICE); });
	connect(coronal_slice, &SliceWidget::sigCPRInit, [this]() { this->slotCPRInit(CORONAL_SLICE); });
	connect(sagittal_slice, &SliceWidget::sigCPRInit, [this]() { this->slotCPRInit(SAGITTAL_SLICE); });

	connect(cpr_view, &CPRView::sigCPRInit, [this](SliceType slice_type) {this->slotCPRInit(slice_type); });
	connect(cpr_view, &CPRView::sigUpdateCPRMod, [this](std::vector<QPointF> pathPoints, int numOfPoints, SliceType slice_type) {this->slotUpdateCPRCurve(pathPoints, numOfPoints, slice_type); });
	connect(cpr_view, &CPRView::sigUpdateCPRPath, [this](std::vector<QPointF> pathPoints, float pathLength, SliceType slice_type) {this->slotUpdateCPRPath(pathPoints, pathLength, slice_type); });
	connect(cpr_view, &CPRView::sigClicked, [this](SliceType slice_type, int mode) {this->slotCPRClicked(slice_type, mode); });
	connect(cpr_view, &CPRView::sigMouseReleased, [this](SliceType slice_type, int mode) {this->slotCPRReleased(slice_type, mode); });


	connect(axial_slice, &SliceWidget::sigUpdateCPRPath, [this](std::vector<QPointF> pathPoints, float pathLength) {this->slotUpdateCPRPath(pathPoints, pathLength, AXIAL_SLICE); });
	connect(coronal_slice, &SliceWidget::sigUpdateCPRPath, [this](std::vector<QPointF> pathPoints, float pathLength) {this->slotUpdateCPRPath(pathPoints, pathLength, CORONAL_SLICE); });
	connect(sagittal_slice, &SliceWidget::sigUpdateCPRPath, [this](std::vector<QPointF> pathPoints, float pathLength) {this->slotUpdateCPRPath(pathPoints, pathLength, SAGITTAL_SLICE); });

	connect(preview_axial_slice, &SliceWidget::sigScreenDragged, [this](float dx, float dy, ClickType c) {this->slotScreenDragged(dx, dy, c, AXIAL_SLICE, true); });
	connect(preview_axial_slice, &SliceWidget::sigDoubleClicked, [this](float x, float y, ClickType c) {this->slotDoubleClicked(x, y, c, AXIAL_SLICE, true); });
	connect(preview_axial_slice, &SliceWidget::sigWheelChanged, [this](float d) {this->slotWheelChanged(d, AXIAL_SLICE, true); });

	connect(dvr, &DVRWidget::sigMouseClicked, [this](float dx, float dy, int width, int height, ClickType c) {this->slotClicked(dx, dy, width, height, c);});
	//connect(dvr, &DVRWidget::sigScreenDragged, [this](float dx, float dy, ClickType c) {this->slotScreenDragged(dx, dy, c);});
	connect(dvr, &DVRWidget::sigVR_rotate, [this](float cur_x, float cur_y, int width, int height) {this->slotVR_rotate(cur_x, cur_y, width, height); });
	connect(dvr, &DVRWidget::sigVR_pan, [this](float dx, float dy) {this->slotVR_pan(dx, dy); });
	connect(dvr, &DVRWidget::sigDoubleClicked, [this](ClickType c) {this->slotDoubleClicked(c);});
	connect(dvr, &DVRWidget::sigWheelChanged, [this](float d) {this->slotWheelChanged(d);});
	connect(dvr, &DVRWidget::sigSculpt, this, &Viewer::slotSculpt);
	connect(dvr, &DVRWidget::sigCrop, this, &Viewer::slotCrop);
	connect(dvr, &DVRWidget::sigVOI, this, &Viewer::slotVOI);
	connect(dvr, &DVRWidget::sigRunOTF, this, &Viewer::slotRunOTF);
	connect(dvr, &DVRWidget::sigMouseReleased, [this]() { this->slotMouseReleased(); });
	connect(dvr, &DVRWidget::sigZoomRatioChanged, [this](float ratio) {this->slotZoomByRatio(ratio); });

	//connect(m_mask_mgr, &MaskMgr::sigChangeMaskColorTable, axial_slice, &SliceWidget::slotChangeMaskColorTable);
	//connect(m_mask_mgr, &MaskMgr::sigChangeMaskColorTable, coronal_slice, &SliceWidget::slotChangeMaskColorTable);
	//connect(m_mask_mgr, &MaskMgr::sigChangeMaskColorTable, sagittal_slice, &SliceWidget::slotChangeMaskColorTable);

	// Connection between a data load page and main viewer.

	connect(m_welcome_window, SIGNAL(sigLoadLocalMPR(QString, bool)), this, SLOT(slotReqOpenVolume(QString, bool)));
	//connect(m_welcome_window, SIGNAL(sigLoadLocalPreviewVolume(QString)), this, SLOT(slotReqOpenPreviewVolume(QString)));

	connect(m_welcome_window, &WelcomeWindow::sigLoadPreviewPlane,
		[this](
			QString file_path, int idx) {
				//slotReqOpenPreviewPlane(file_path, idx);
		});
	connect(m_welcome_window, SIGNAL(sigReturn()), this, SLOT(slotReturnToMPR()));

	// Log starting info
	//m_logger_daily->info("Viewer started");
}

void Viewer::connectSocket() {
	//Create INI file if it does not exist in directory 
	std::string fullpath = "./config.ini";

	INI::File ft;

	if (!ft.Load(fullpath)) {
		std::cout << "Config.ini file does not exist in this directory" << std::endl;
	}

	std::string server_ip = ft.GetSection("WEBSOCKET")->GetValue("ip", -1).AsString();
	std::string server_port = ft.GetSection("WEBSOCKET")->GetValue("port", -1).AsString();
	QString full_URL = QString::fromStdString("ws://" + server_ip + ":" + server_port);

	//make connection with server 
	connect(&socket, &QWebSocket::connected, this, &Viewer::slotConnected);
	connect(&socket, &QWebSocket::disconnected, this, &Viewer::slotDisconnected);
	connect(&socket, &QWebSocket::binaryMessageReceived, this, &Viewer::onBinaryPacketReceived);
	connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
		[=](QAbstractSocket::SocketError error) {
			QMessageBox msgBox;
			msgBox.setText("Socket error :" + socket.errorString());
			msgBox.exec();
			socket.close();
			connectSocket();
		});

	socket.open(QUrl(full_URL));
	std::cout << "attempting to connect socket..." << std::endl;
}



bool Viewer::eventFilter(QObject* obj, QEvent* event) {
	QEvent::Type event_type = event->type();
	if (event_type == QEvent::NonClientAreaMouseButtonPress) {
		user_moved_window = false;
	}
	else if (event_type == QEvent::Move && isVisible()) {
		user_moved_window = true;
	}
	else if (event_type == QEvent::NonClientAreaMouseButtonRelease) {
		if (user_moved_window) {
			cur_screen = QGuiApplication::screenAt(QCursor::pos());
			QRect screen_dimensions = cur_screen->geometry();
			m_width = screen_dimensions.width();
			m_height = screen_dimensions.height();
			this->resize(m_width, m_height - 90);
			
			user_moved_window = false; 
		}
	}
	return QObject::eventFilter(obj, event);
}


void Viewer::OnTimerCallbackFunction() {
	if (axial_fps_counter != 0){
		int axial_fps = axial_fps_counter;
		axial_slice->setFPSInfo(axial_fps);
	}
	else {
		axial_slice->setFPSInfo(0);
	}

	if (sagittal_fps_counter != 0) {
		int sagittal_fps = sagittal_fps_counter;
		sagittal_slice->setFPSInfo(sagittal_fps);
	}
	else {
		sagittal_slice->setFPSInfo(0);
	}

	if (coronal_fps_counter != 0) {
		int coronal_fps = coronal_fps_counter;
		coronal_slice->setFPSInfo(coronal_fps);
	}
	else {
		coronal_slice->setFPSInfo(0);
	}

	if (dvr_fps_counter != 0) {
		int dvr_fps = dvr_fps_counter;
		dvr->setFPSInfo(dvr_fps);
	}
	else {
		dvr->setFPSInfo(0);
	}
	
	axial_fps_counter = 0;
	sagittal_fps_counter = 0;
	coronal_fps_counter = 0;
	dvr_fps_counter = 0;
		
	
	m_pTimer->start(1000);
}

/* ========================================================================== */
/*									QT Slots								  */
/* ========================================================================== */


std::string Viewer::strSliceType(SliceType slice_type) {
	switch (slice_type) {
	case AXIAL_SLICE:
		return "Axial";
	case CORONAL_SLICE:
		return "Coronal";
	case SAGITTAL_SLICE:
		return "Sagittal";
	}
}


void Viewer::slotReqOpenLocalMask() {

#ifdef LOCAL_VIEW
	QFileDialog file_dialog(this);
	file_dialog.setFileMode(QFileDialog::AnyFile);

	QStringList paths = file_dialog.getOpenFileNames(this, "Open Mask file", QDir::currentPath(),
		tr("mask file (*.dat)"));

	if (paths.isEmpty()) return; // Prevent cancelation

	std::vector<std::string> dir_list;

	// Save last opened paths
	for (const auto& path : paths) {
		dir_list.push_back(path.toStdString());
	}

	json msg;
	msg["Params"]["value1"] = dir_list;

	m_rendering_function_mgr->action_manager(MPR, LOAD_MASK, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	json param;
	getPacket("Tree", "mask", param);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotOpenRemoteMask(QJsonObject json_path) {

#ifdef REMOTE_VIEW
	std::vector<std::string> dir_list = getSelectMaskInfoWithDialog(json_path);

	if (!dir_list.empty()) {
		json msg;
		msg["Params"]["value1"] = dir_list;

		getPacket("MPR", "load_mask", msg);
	}
#endif

}

void Viewer::slotReqExportMask() {

	QString selected_dir = QFileDialog::getExistingDirectory(this,
		"Select Mask Folder",
		QDir::currentPath(),
		QFileDialog::ShowDirsOnly);

	if (selected_dir == "")
		return;

	std::string dir = selected_dir.toStdString();

	json msg;
	msg["Params"]["value1"] = dir;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, SAVE_MASK, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "save_mask", msg);
#endif

}

void Viewer::slotReqOpenVolume(QString dir, bool is_leaf) {

	//m_logger_dev->debug("Requested to open volume. {}", dir.toStdString());
	current_img_dir = dir.toUtf8().toStdString();
	current_img_is_leaf = is_leaf;
	// Change the window
	stacked_window->setCurrentIndex(1);

	// Remove mask layers in main toolbar
	m_tb_main->resetMaskLayer();
	m_tb_main->show();

	/////////////////////////////////////////////////////
	//Send dicom directory to server for loading
	json load_data_msg;
	load_data_msg["Params"]["value1"] = dir.toUtf8().toStdString();
	load_data_msg["Params"]["value2"] = is_leaf;
	load_data_msg["Params"]["value3"] = dvr->getWidth();
	load_data_msg["Params"]["value4"] = dvr->getHeight();
	

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, LOAD_DATA, load_data_msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "load_data", load_data_msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif

	m_pTimer->start(1000);
}


void Viewer::slotBackToWelcome() {
	m_tb_main->hide();
	stacked_window->setCurrentIndex(0);
}

void Viewer::slotReturnToMPR() {
	m_tb_main->show();
	stacked_window->setCurrentIndex(1);
	m_pTimer->start(1000);
}

void Viewer::slotChangeViewLayout() {
	cpr_view->resetSlice();
	cpr_slice->resetSlice();
	path_slice->resetSlice();
	m_view_slots->setMPRDVRLayout(axial_slice, coronal_slice, sagittal_slice, dvr);
	stacked_window->setCurrentIndex(1);
}


void Viewer::slotFullScreen(SliceType slice_type) {
	m_view_slots->setViewFullScreen(axial_slice, coronal_slice, sagittal_slice, dvr, slice_type);
	stacked_window->setCurrentIndex(1);
}

void Viewer::slotCPRScreen(SliceType slice_type) {
	cpr_view->resetSlice();
	switch (slice_type) {
	case AXIAL_SLICE:
		cpr_view->setMode(MPR_CURVE, AXIAL_SLICE);
		m_cpr_slots->setCPRLayout(cpr_view, cpr_slice, path_slice, AXIAL_SLICE);
		break;
	case SAGITTAL_SLICE:
		cpr_view->setMode(MPR_CURVE, SAGITTAL_SLICE);
		m_cpr_slots->setCPRLayout(cpr_view, cpr_slice, path_slice, SAGITTAL_SLICE);
		break;
	case CORONAL_SLICE:
		cpr_view->setMode(MPR_CURVE, CORONAL_SLICE);
		m_cpr_slots->setCPRLayout(cpr_view, cpr_slice, path_slice, CORONAL_SLICE);
		break;
	}
	printf("coming through CPR screen\n");
	stacked_window->setCurrentIndex(2);
}

void Viewer::slotCPRChangeAxis(SliceType slice_type) {
	switch (slice_type) {
	case AXIAL_SLICE:
		cpr_view->setSlice(m_axial_packet, m_rendering_function_mgr->getMPRImage(AXIAL_SLICE));
		m_cpr_slots->setCPRLayout(axial_slice, cpr_slice, path_slice, AXIAL_SLICE);
		//axial_slice->toCPRMod();
		break;
	case SAGITTAL_SLICE:
		cpr_view->setSlice(m_sagittal_packet, m_rendering_function_mgr->getMPRImage(SAGITTAL_SLICE));
		m_cpr_slots->setCPRLayout(sagittal_slice, cpr_slice, path_slice, SAGITTAL_SLICE);
		//sagittal_slice->toCPRMod();
		break;
	case CORONAL_SLICE:
		cpr_view->setSlice(m_coronal_packet, m_rendering_function_mgr->getMPRImage(CORONAL_SLICE));
		m_cpr_slots->setCPRLayout(coronal_slice, cpr_slice, path_slice, CORONAL_SLICE);
		//coronal_slice->toCPRMod();
		break;
	}
	printf("coming through CPR change axis\n");
	stacked_window->setCurrentIndex(2);

}

void Viewer::slotRunOTF() {
	m_view_slots->setOTFView(axial_slice, coronal_slice, sagittal_slice, dvr);
	stacked_window->setCurrentIndex(3);
}

/* ========================================================================== */
/*								        	MPR			 					  */
/* ========================================================================== */

void Viewer::slotScreenDragged(float dx, float dy, ClickType c, SliceType slice_type, bool is_preview) {
	QString actionType;
	if (is_preview) {
		return;
	}

	if (c == LEFT_CLICK) {

		json zoom_msg;
		zoom_msg["Params"]["value1"] = strSliceType(slice_type);
		zoom_msg["Params"]["value2"] = 0;
		zoom_msg["Params"]["value3"] = dy;

		// zoom slice by dragging up & down
#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(MPR, ZOOM, zoom_msg["Params"]);
#endif

#ifdef REMOTE_VIEW
		actionType = "zoom";
		getPacket("MPR", actionType, zoom_msg);
#endif
	}
	else if (c == MIDDLE_CLICK) {
		// slice panning
		json pan_msg;
		pan_msg["Params"]["value1"] = strSliceType(slice_type);
		pan_msg["Params"]["value2"] = dx;
		pan_msg["Params"]["value3"] = dy;

#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(MPR, PAN, pan_msg["Params"]);


#endif
#ifdef REMOTE_VIEW
		actionType = "pan";
		getPacket("MPR", actionType, pan_msg);
#endif
	}

	else if (c == RIGHT_CLICK) {
		json window_msg;
		window_msg["Params"]["value1"] = strSliceType(slice_type);
		window_msg["Params"]["value2"] = dx;
		window_msg["Params"]["value3"] = dy;

		/*
		axial_slice->setWL(dy);
		axial_slice->setWW(dx);
		coronal_slice->setWL(dy);
		coronal_slice->setWW(dx);
		sagittal_slice->setWL(dy);
		sagittal_slice->setWW(dx);
		*/

#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(MPR, WINDOW, window_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
		actionType = "window";
		getPacket("MPR", actionType, window_msg);
#endif
	}

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotZoomByRatio(float ratio, SliceType slice_type) {
	QString actionType;

	json zoom_msg;
	zoom_msg["Params"]["value1"] = strSliceType(slice_type);
	zoom_msg["Params"]["value2"] = 1;
	zoom_msg["Params"]["value3"] = ratio;
	
	// zoom slice by dragging up & down
#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(MPR, ZOOM, zoom_msg["Params"]);
#endif

#ifdef REMOTE_VIEW
		actionType = "zoom";
		getPacket("MPR", actionType, zoom_msg);
#endif

#ifdef LOCAL_VIEW
		localUpdate();
#endif
}



void Viewer::slotDoubleClicked(float x, float y, ClickType c, SliceType slice_type, bool is_preview) {
	if (is_preview) {
		return;
	}
	else {
		if (c == LEFT_CLICK) {
			// set axis center to double clicked location
			json center_msg;
			center_msg["Params"]["value1"] = strSliceType(slice_type);
			center_msg["Params"]["value2"] = x;
			center_msg["Params"]["value3"] = y;

#ifdef LOCAL_VIEW
			m_rendering_function_mgr->action_manager(MPR, CENTER, center_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
			getPacket("MPR", "center", center_msg);
#endif
		}
		else if (c == RIGHT_CLICK) {
			// reset planes to initial position
			json reset_msg; //empty json file as there are no parameters
			reset_msg["Params"]["value1"] = strSliceType(slice_type);
			axial_slice->setWL(m_volume_packet.ImageWindowCenter);
			axial_slice->setWW(m_volume_packet.ImageWindowWidth);
			coronal_slice->setWL(m_volume_packet.ImageWindowCenter);
			coronal_slice->setWW(m_volume_packet.ImageWindowWidth);
			sagittal_slice->setWL(m_volume_packet.ImageWindowCenter);
			sagittal_slice->setWW(m_volume_packet.ImageWindowWidth);
#ifdef LOCAL_VIEW
			m_rendering_function_mgr->action_manager(MPR, RESET, reset_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
			getPacket("MPR", "reset", reset_msg);
#endif
		}


#ifdef LOCAL_VIEW
		localUpdate();
#endif
	}
}

void Viewer::slotMouseReleased(float x, float y, ClickType c, SliceType slice_type) {
	if (c == LEFT_CLICK) {
		if (m_downsample_resolution > 0) {
			m_mpr_is_downsampling = false;
			slotResolutionChanged(MPR, m_mpr_resolution_temp, m_mpr_is_downsampling);
		}

#ifdef LOCAL_VIEW
		localUpdate();
#endif
	}
	else if (c == RIGHT_CLICK) {
		// update windowing of other slices
	}
}

void Viewer::slotWheelChanged(float d, SliceType slice_type, bool is_preview) {
	// move slice when wheel scrolled
	if (is_preview) {

		return;
	}

	json move_msg;
	move_msg["Params"]["value1"] = strSliceType(slice_type);
	move_msg["Params"]["value2"] = d;
	std::cout << "d from viewer: " << d << std::endl;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, MOVE_SLICE, move_msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "move_slice", move_msg);
#endif



#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotGuideLineMoved(bool h, bool v, float x, float y, SliceType slice_type) {
	json move_msg;
	move_msg["Params"]["value1"] = strSliceType(slice_type);
	move_msg["Params"]["value2"] = x;
	move_msg["Params"]["value3"] = y;
	move_msg["Params"]["value4"] = h;
	move_msg["Params"]["value5"] = v;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, GUIDELINE_MOVED, move_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "guideline_move", move_msg);
#endif


#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotGuideLineRotated(float a, SliceType slice_type) {
	json oblique_msg;
	oblique_msg["Params"]["value1"] = strSliceType(slice_type);
	oblique_msg["Params"]["value2"] = a;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, OBLIQUE, oblique_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "oblique", oblique_msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotThicknessChanged(SliceType slice_type, int v) {
	json msg;
	msg["Params"]["value1"] = strSliceType(slice_type);
	msg["Params"]["value2"] = v;
#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, THICKNESS, msg["Params"]);
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "thickness", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotThicknessModeChanged(int v) {
	json msg;
	msg["Params"]["value1"] = v;
#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, THICKNESS_MODE, msg["Params"]);
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "thickness_mode", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotGuideLineThickness(float d, bool h, bool v, SliceType slice_type) {
	json move_msg;
	move_msg["Params"]["value1"] = strSliceType(slice_type);
	move_msg["Params"]["value2"] = d;
	move_msg["Params"]["value3"] = h;
	move_msg["Params"]["value4"] = v;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, THICKNESS, move_msg["Params"]);
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "thickness", move_msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotClicked(float x, float y, ClickType c, SliceType slice_type) {
	if (c == LEFT_CLICK) {
		m_mpr_resolution_temp = m_mpr_resolution;
		if (m_downsample_resolution > 0){ 
			m_mpr_is_downsampling = true;
			slotResolutionChanged(MPR, m_downsample_resolution, m_mpr_is_downsampling);
		}
	}
	//else if (c == RIGHT_CLICK) {
	//}
}

void Viewer::slotInverse(bool v) {
	json msg;
	msg["Params"]["value1"] = strSliceType(AXIAL_SLICE);
	msg["Params"]["value2"] = v;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, INVERSE, msg["Params"]);
#endif
#ifdef LOCAL_VIEW
	localUpdate();
#endif
#ifdef REMOTE_VIEW
	getPacket("MPR", "inverse", msg);
#endif
}


/* ========================================================================== */
/*								        	MPR			 					  */
/* ========================================================================== */

void Viewer::slotCPRInit(SliceType slice_type) {

	json msg;

	msg["Params"]["value1"] = strSliceType(slice_type);

#ifdef LOCAL_VIEW
	currentCPRView = slice_type;
	m_rendering_function_mgr->action_manager(CPR, INIT, msg["Params"]);
#endif
#ifdef LOCAL_VIEW
	localUpdate();
#endif
#ifdef REMOTE_VIEW
	getPacket("CPR", "init", msg);
#endif
}



void Viewer::slotUpdateCPRCurve(std::vector<QPointF> pathPoints, int numOfPoints, SliceType slice_type) {

	json msg;

	std::vector<float> p_x;
	std::vector<float> p_y;

	for (int i = 0; i < pathPoints.size(); i++) {



		float x, y;
		x = pathPoints[i].x() / cpr_view->getWidth(); 
		//y = (pathPoints[i].y() - (cpr_view->height() / 2 - cpr_view->getImageHeight() / 2)) / cpr_view->getWidth(); 
		y = pathPoints[i].y() / cpr_view->getWidth();

		p_x.push_back(x);
		p_y.push_back(y);

	}

	//std::cout << "cpr_view size :" << cpr_view->getWidth() << ", " << cpr_view->getHeight() << std::endl;
	//std::cout << "cpr_view image size :" << cpr_view->getImageWidth() << ", " << cpr_view->getImageHeight() << std::endl;
	//std::cout << "cpr y adding value: " << (cpr_view->height() / 2.0 - cpr_view->getImageHeight() / 2.0) / (cpr_view->getHeight() / 2.0) << std::endl;
	//
	//std::cout << "Viewer CPR update Mod parameter check : p_x[0] == " << p_x[0] << std::endl;
	//std::cout << "Viewer CPR update Mod parameter check : p_y[0] == " << p_y[0] << std::endl;
	//
	//std::cout << "Viewer CPRupdate Mod parameter check : p_x[last] == " << p_x[p_x.size() - 1] << std::endl;
	//std::cout << "Viewer CPRupdate Mod parameter check : p_y[last] == " << p_y[p_y.size() - 1] << std::endl;


	msg["Params"]["value1"] = strSliceType(slice_type);
	msg["Params"]["value2"] = p_x;
	msg["Params"]["value3"] = p_y;
	msg["Params"]["value4"] = numOfPoints;



#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(CPR, CURVE, msg["Params"]);
#endif
#ifdef LOCAL_VIEW
	localUpdate();
#endif
#ifdef REMOTE_VIEW
	getPacket("CPR", "curve", msg);
#endif


}

void Viewer::slotUpdateCPRPath(std::vector<QPointF> pathPoints, float pathLength, SliceType slice_type) {

	json msg;

	std::vector<float> p_x;
	std::vector<float> p_y;

	for (int i = 0; i < pathPoints.size(); i++) {

		float x, y;
		x = pathPoints[i].x() / cpr_view->getWidth();
		y = pathPoints[i].y() / cpr_view->getWidth();

		p_x.push_back(x);
		p_y.push_back(y);
	}


	msg["Params"]["value1"] = strSliceType(slice_type);
	msg["Params"]["value2"] = p_x;
	msg["Params"]["value3"] = p_y;
	msg["Params"]["value4"] = pathLength;
	


#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(CPR, PATH, msg["Params"]);
#endif
#ifdef LOCAL_VIEW
	localUpdate();
#endif
#ifdef REMOTE_VIEW
	getPacket("CPR", "path", msg);
#endif


}

void Viewer::slotCPRClicked(SliceType slice_type, int mode) {
	m_cpr_resolution_temp = m_cpr_resolution;
	if (m_downsample_resolution > 0) {
		slotCPRResolutionChanged(m_downsample_resolution, mode, slice_type);
	}
}

void Viewer::slotCPRReleased(SliceType slice_type, int mode) {
	if (m_downsample_resolution > 0) {
		slotCPRResolutionChanged(m_cpr_resolution_temp, mode, slice_type);
	}
}


void Viewer::slotCPRResolutionChanged(int res, int mode, SliceType slice_type) {
	m_cpr_resolution = res;


#ifdef REMOTE_VIEW
	json msg;

	getPacket("CPR", "data", msg);
	
#endif

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->setCPRSamplingCnt(res, res, mode, slice_type);
	localUpdate();
#endif
}


/* ========================================================================== */
/*								        	DVR			 					  */
/* ========================================================================== */

void Viewer::slotClicked(float dx, float dy, int width, int height, ClickType c) {
	QString actionType;

	if (c == LEFT_CLICK) {
		// rotate
		json msg;
		msg["Params"]["value1"] = dx;
		msg["Params"]["value2"] = dy;
		msg["Params"]["value3"] = width;
		msg["Params"]["value4"] = height;
		msg["Params"]["value5"] = true;

		m_vr_resolution_temp = m_vr_resolution;
		if (m_downsample_resolution > 0) {
			m_dvr_is_downsampling = true;
			slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
		}

#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(VR, ROTATE, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
		actionType = "rotate";
		getPacket("VR", actionType, msg);
#endif
	}
	else if (c == MIDDLE_CLICK) {
		m_vr_resolution_temp = m_vr_resolution;
		if (m_downsample_resolution > 0) {
			m_dvr_is_downsampling = true;
			slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
		}
	}

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotOTFClicked() {
	m_vr_resolution_temp = m_vr_resolution;
	if (m_downsample_resolution > 0) {
		m_dvr_is_downsampling = true;
		slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
	}
}

void Viewer::slotOTFReleased() {
	if (m_downsample_resolution > 0) {
		m_dvr_is_downsampling = false;
		slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
	}

}

void Viewer::slotSliderDragged(float specular_val, float brightness_val) {

	json msg;
	msg["Params"]["value1"] = 1;
	msg["Params"]["value2"] = specular_val / 10.0;
	msg["Params"]["value3"] = brightness_val / 10.0;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, SETTINGS, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "settings", msg);
#endif
}

void Viewer::slotSliderClicked() {
	m_vr_resolution_temp = m_vr_resolution;
	if (m_downsample_resolution > 0) {
		m_dvr_is_downsampling = true;
		slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
	}
}

void Viewer::slotSliderReleased() {
	if (m_downsample_resolution > 0) {
		m_dvr_is_downsampling = false;
		slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
	}
}


void Viewer::slotVR_rotate(float cur_x, float cur_y, int width, int height) {
	QString actionType;
	json msg;
	msg["Params"]["value1"] = cur_x;
	msg["Params"]["value2"] = cur_y;
	msg["Params"]["value3"] = width;
	msg["Params"]["value4"] = height;


#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, ROTATE, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	actionType = "rotate";
	getPacket("VR", actionType, msg);
#endif
#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotVR_pan(float dx, float dy) {

	QString actionType;
	// panning
	json msg;
	msg["Params"]["value1"] = dx;
	msg["Params"]["value2"] = dy;

#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(VR, PAN, msg["Params"]);
#endif
#ifdef REMOTE_VIEW
		actionType = "pan";
		getPacket("VR", actionType, msg);
#endif
#ifdef LOCAL_VIEW
		localUpdate();
#endif
}


void Viewer::slotMouseReleased(void) {
	QString actionType;
	if (m_downsample_resolution > 0) {
		m_dvr_is_downsampling = false;
		slotResolutionChanged(VR, m_vr_resolution_temp, m_dvr_is_downsampling);
	}

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotDoubleClicked(ClickType c) {
	if (c == RIGHT_CLICK) {
		// reset planes to initial position
		json msg; //empty json file as there are no parameters

#ifdef LOCAL_VIEW
		m_rendering_function_mgr->action_manager(VR, RESET, msg["Params"]);
#endif
#ifdef REMOTE_VIEW
		getPacket("VR", "reset", msg);
#endif
	}

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotWheelChanged(float d) {
	// move slice when wheel scrolled
	json msg;
	
	msg["Params"]["value1"] = 0;
	msg["Params"]["value2"] = d;
#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, ZOOM, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "zoom", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotTFChanged(QList<OTFPolygon*> polyList, int offset) {
	
	m_otf_params.clear();

	for (int i = 0; i < polyList.size(); i++) {
		std::vector<std::vector<int>> pointList;
		QList<EllipseItem_otf*> points = polyList[i]->getPointList();
		for (int j = 0; j < points.size(); j++) {
			std::vector<int> coords;
			int ptX = points[j]->pos().x();
			int ptY = points[j]->pos().y();
			coords.push_back(ptX);
			coords.push_back(ptY);
			pointList.push_back(coords);
		}

		m_otf_params.push_back(pointList);
	}


	json msg;
	msg["Params"]["value1"] = OTF;
	msg["Params"]["value2"] = 0;
	msg["Params"]["value3"] = m_otf_params;



#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, PROJECTION, msg["Params"]);
	localUpdate();
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "projection", msg);
#endif

}

void Viewer::slotColorChanged(QList<OTFColor> colorList) {
	
	std::vector<OTFColor> colorScale = colorList.toVector().toStdVector();
	std::map<int, std::vector<int>> m_otf_color;

	//json m_otf_color;

	for (int i = 0; i < colorScale.size(); i++) {
		std::vector<int> rgb;
		int intensity = colorScale[i]._intensity;
		rgb.push_back(colorScale[i]._color.red());
		rgb.push_back(colorScale[i]._color.blue());
		rgb.push_back(colorScale[i]._color.green());
		
		m_otf_color[intensity] = rgb;
	}

	json msg;
	msg["Params"]["value1"] = OTF;
	msg["Params"]["value2"] = 1;
	msg["Params"]["value3"] = m_otf_color;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, PROJECTION, msg["Params"]);
	localUpdate();
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "projection", msg);
#endif

}

void Viewer::slotChangeAll(QList<OTFColor> colorList, QList<OTFPolygon*> polyList, int offset) {

	std::vector<OTFColor> colorScale = colorList.toVector().toStdVector();
	std::map<int, std::vector<int>> m_otf_color;

	//json m_otf_color;

	for (int i = 0; i < colorScale.size(); i++) {
		std::vector<int> rgb;
		int intensity = colorScale[i]._intensity;
		rgb.push_back(colorScale[i]._color.red());
		rgb.push_back(colorScale[i]._color.blue());
		rgb.push_back(colorScale[i]._color.green());

		m_otf_color[intensity] = rgb;
	}

	m_otf_params.clear();

	for (int i = 0; i < polyList.size(); i++) {
		std::vector<std::vector<int>> pointList;
		QList<EllipseItem_otf*> points = polyList[i]->getPointList();
		for (int j = 0; j < points.size(); j++) {
			std::vector<int> coords;
			int ptX = points[j]->pos().x();
			int ptY = points[j]->pos().y();
			coords.push_back(ptX);
			coords.push_back(ptY);
			pointList.push_back(coords);
		}

		m_otf_params.push_back(pointList);
	}

	json msg;
	msg["Params"]["value1"] = OTF;
	msg["Params"]["value2"] = 2;
	msg["Params"]["value3"] = m_otf_color;
	msg["Params"]["value4"] = m_otf_params;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, PROJECTION, msg["Params"]);
	localUpdate();
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "projection", msg);
#endif

}


void Viewer::slotZoomByRatio(float ratio) {
	QString actionType;

	printf("ratio: %f", ratio);

	json zoom_msg;
	zoom_msg["Params"]["value1"] = 1;
	zoom_msg["Params"]["value2"] = ratio;

	// zoom slice by dragging up & down
#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, ZOOM, zoom_msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	actionType = "zoom";
	getPacket("VR", actionType, zoom_msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}



void Viewer::slotShaderStatusChanged(int status) {
	json msg;
	msg["Params"]["value1"] = 0;
	msg["Params"]["value2"] = status;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, SETTINGS, msg["Params"]);
	localUpdate();
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "settings", msg);
#endif
}



/* ========================================================================== */
/*								        MASK			 					  */
/* ========================================================================== */

void Viewer::slotAddMask(int cur_idx) {
	MaskLabel mask_label;
	//mask_label.color = mask_color_table[cur_idx + 1];

	m_tb_main->slotAddedMask(cur_idx + 1, mask_label);
}

void Viewer::slotSetMaskVisible(int idx, bool b) {
	json msg;
	msg["Params"]["value1"] = idx;
	msg["Params"]["value2"] = b;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, MASK_VISIBLE, msg["Params"]);
	//m_rendering_function_mgr->action_manager(VR, MASK_VISIBLE, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "mask_visible", msg);
	//getPacket("VR", "mask_visible", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif

	m_tb_main->slotChangeMaskVisible(idx, b);
}

void Viewer::slotChangeMaskOpacity(int val) {
	if (val == -1) { // start changing
		m_mpr_resolution_temp = m_mpr_resolution;
		m_vr_resolution_temp = m_vr_resolution;
		if (m_downsample_resolution > 0) {
			m_dvr_is_downsampling = true;
			m_mpr_is_downsampling = true;
			slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
			slotResolutionChanged(MPR, m_downsample_resolution, m_mpr_is_downsampling);
		}
		return;
	}
	else if (val == -2) { // end changing
		if (m_downsample_resolution > 0) {
			m_dvr_is_downsampling = false;
			m_mpr_is_downsampling = false;
			slotResolutionChanged(VR, m_downsample_resolution, m_dvr_is_downsampling);
			slotResolutionChanged(MPR, m_downsample_resolution, m_mpr_is_downsampling);
		}
		return;
	}

	json msg;
	msg["Params"]["value1"] = val;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, MASK_OPACITY, msg["Params"]);
	//m_rendering_function_mgr->action_manager(VR, MASK_OPACITY, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "mask_opacity", msg);
	//getPacket("VR", "mask_opacity", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotChangeMaskColor(int idx, std::string v) {
	json msg;
	msg["Params"]["value1"] = idx;
	msg["Params"]["value2"] = v;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(MPR, MASK_COLOR, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("MPR", "mask_color", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}



void Viewer::slotHardReset(){

	stacked_window->setCurrentIndex(1);
	
	json msg;
	//클라이언트가 초기 로딩을 할때 directory 이름을 save
	//후에 reset 등 작업을 할때에 사용하여 welcome window 가 아닌 기존에 계속 보고 있던 image 를 초기 rendering 상태로 reset 함
	// Remove mask layers in main toolbar
	m_tb_main->resetMaskLayer();
	m_tb_main->show();

	/////////////////////////////////////////////////////
	//Send dicom directory to server for loading
	msg["Params"]["value1"] = current_img_dir;
	msg["Params"]["value2"] = current_img_is_leaf;
	msg["Params"]["value3"] = dvr->getWidth();
	msg["Params"]["value4"] = dvr->getHeight();

#ifdef REMOTE_VIEW
	getPacket("HARDRESET", "load_data", msg);
#endif


}


void Viewer::closeEvent(QCloseEvent* event) {

#ifdef REMOTE_VIEW
	json param;
	getPacket("Connection", "disconnect", param);
#endif

	event->accept();
}


/* ========================================================================== */
/*								       Functions			 				  */
/* ========================================================================== */

void Viewer::slotFunctionChanged(int n, FunctionType function_type) {
	switch (function_type) {
	case MPR_INVERSE:
		slotInverse(n);
		break;
	case DVR_SCULPT:
		if (n > 0)
			dvr->setMode(DVR_SCULPT);
		else {
			dvr->setMode(NONE);
		}
		break;

	case DVR_CROP:
		if (n > 0)
			dvr->setMode(DVR_CROP);
		else {
			dvr->setMode(NONE);
		}
		break;

	case DVR_VOI:
		if (n > 0)
			dvr->setMode(DVR_VOI);
		else {
			dvr->setMode(NONE);
		}
		break;

	case DVR_RESET:
		if (n > 0) {
			dvr->setMode(NONE);
			slotDVRReset();
		}
		break;

	case DVR_A:
	case DVR_P:
	case DVR_L:
	case DVR_R:
	case DVR_H:
	case DVR_F:
		if (n > 0) {
			VROrientationType ori_type;
			switch (function_type) {
			case DVR_A:
				ori_type = A;
				break;
			case DVR_P:
				ori_type = P;
				break;
			case DVR_L:
				ori_type = L;
				break;
			case DVR_R:
				ori_type = R;
				break;
			case DVR_H:
				ori_type = H;
				break;
			case DVR_F:
				ori_type = F;
				break;

			}
			slotDVROrientation(ori_type);
		}
		break;

	case MPR_256:
	case MPR_512:
	case MPR_1024:
	case DVR_256:
	case DVR_512:
	case DVR_1024:
		if (n > 0) {
			RenderingType rt;
			int res;
			switch (function_type) {
			case MPR_256:
				rt = MPR;
				res = 256;
				break;
			case MPR_512:
				rt = MPR;
				res = 512;
				break;
			case MPR_1024:
				rt = MPR;
				res = 1024;
				break;
			case DVR_256:
				rt = VR;
				res = 256;
				break;
			case DVR_512:
				rt = VR;
				res = 512;
				break;
			case DVR_1024:
				rt = VR;
				res = 1024;
				break;
			}
			slotResolutionChanged(rt, res, false);
		}
		break;
	case DS_128:
		m_downsample_resolution = 128;
		break;
	case DS_192:
		m_downsample_resolution = 192;
		break;
	case DS_256:
		m_downsample_resolution = 256;
		break;
	case DS_384:
		m_downsample_resolution = 384;
		break;
	case DS_512:
		m_downsample_resolution = 512;
		break;
	case DS_OFF:
		m_downsample_resolution = -1;
		break;
	case C_25:
		m_compression_rate = 25;
		break;
	case C_50:
		m_compression_rate = 50;
		break;
	case C_75:
		m_compression_rate = 75;
		break;
	case C_100:
		m_compression_rate = 100;
		break;
	case MPR_THICKNESS_AVG:
		slotThicknessModeChanged(0);
		break;
	case MPR_THICKNESS_MIN:
		slotThicknessModeChanged(1);
		break;
	case MPR_THICKNESS_MAX:
		slotThicknessModeChanged(2);
		break;
	case DVR_MAXIP:
	case DVR_MINIP:
	case DVR_RAYSUM:
	case DVR_OTF:
		if (n > 0) {
			VRProjectionType proj_type;
			switch (function_type) {
			case DVR_OTF:
				proj_type = OTF;
				break;
			case DVR_MAXIP:
				proj_type = MAXIP;
				break;
			case DVR_MINIP:
				proj_type = MINIP;
				break;
			case DVR_RAYSUM:
				proj_type = RAYSUM;
				break;
			}
			slotProjectionChanged(proj_type);
		}
		break;
	}
}

void Viewer::slotProjectionChanged(VRProjectionType type) {
	json msg;
	msg["Params"]["value1"] = (int)type;
	msg["Params"]["value2"] = 0;
	msg["Params"]["value3"] = { {{0, 0}, {0, 0}, {0, 0}, {0, 0}} };
	if (type == OTF) {
		msg["Params"]["value3"] = m_otf_params;
	}

#ifdef REMOTE_VIEW
	getPacket("VR", "projection", msg);
#endif

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, PROJECTION, msg["Params"]);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}


void Viewer::slotSculpt(std::vector<int> p_x, std::vector<int> p_y) {
	json msg;
	msg["Params"]["value1"] = p_x;
	msg["Params"]["value2"] = p_y;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, SCULPT, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "sculpt", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotCrop(std::vector<int> p_x, std::vector<int> p_y) {
	json msg;
	msg["Params"]["value1"] = p_x;
	msg["Params"]["value2"] = p_y;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, SCULPT, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "sculpt", msg);
#endif

	m_tb_main->toggleButton(DVR_CROP);

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotVOI(std::vector<int> p_x, std::vector<int> p_y) {
	json msg;
	msg["Params"]["value1"] = p_x;
	msg["Params"]["value2"] = p_y;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, VOI, msg["Params"]);
#endif

	m_tb_main->toggleButton(DVR_VOI);

#ifdef REMOTE_VIEW
	getPacket("VR", "voi", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotDVRReset() {
	json msg;
#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, RESET_SCULPT, msg);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "reset_sculpt", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotDVROrientation(VROrientationType type) {
	json msg;
	msg["Params"]["value1"] = (int)type;

#ifdef LOCAL_VIEW
	m_rendering_function_mgr->action_manager(VR, ORIENTATION, msg["Params"]);
#endif

#ifdef REMOTE_VIEW
	getPacket("VR", "orientation", msg);
#endif

#ifdef LOCAL_VIEW
	localUpdate();
#endif
}

void Viewer::slotResolutionChanged(RenderingType rendering_type, int res, bool is_downsampling) {
	if (rendering_type == MPR) {
		m_mpr_resolution = res;
	}
	else if (rendering_type == VR) {
		m_vr_resolution = res;
	}


#ifdef REMOTE_VIEW
	json msg;
	if (rendering_type == MPR) {
		getPacket("MPR", "data", msg);
	}
	else if (rendering_type == VR) {
		getPacket("VR", "data", msg);
	}
#endif

#ifdef LOCAL_VIEW
	if (rendering_type == MPR) {
		m_rendering_function_mgr->setMPRSamplingCnt(res, res, is_downsampling);
	}
	else if (rendering_type == VR) {
		m_rendering_function_mgr->setVRSamplingCnt(res, is_downsampling);
	}
	localUpdate();
#endif
}

/* ========================================================================== */
/* 							   Private Functions							  */
/* ========================================================================== */


void Viewer::slotImageTypeChanged(int image_type) {

	if (image_type == 1) {
		m_jpeg_on = true;
	}
	else {
		m_jpeg_on = false;
	}
}

/**
* Decompresses JPEG image data that came through socket.
**/
QByteArray decomp_jpeg(QByteArray payload, int width, int height) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	const std::size_t count = payload.size();
	int int_img_size = count;
	unsigned long img_size = count;
	unsigned char* img_data = (unsigned char*)malloc(img_size);
	std::memcpy(img_data, payload.constData(), img_size);
	int uncomp_size = width * height * 3;
	JSAMPARRAY buffer;    /* Output row buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, img_data, img_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	unsigned char* decomp_buffer = (unsigned char*)malloc(uncomp_size);
	int row_stride = cinfo.output_width * cinfo.output_components;


	while (cinfo.output_scanline < cinfo.output_height) {
		//printf("output_scanline: %d\n", cinfo.output_scanline);
		unsigned char* buffer_array[1];
		buffer_array[0] = decomp_buffer + (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(img_data);

	const char* img = reinterpret_cast<const char*>(decomp_buffer);
	QByteArray uncomp_img(img, uncomp_size);

	free(decomp_buffer);
	return uncomp_img;

}

QByteArray decomp_jpeg(QByteArray payload, int sampling_rate) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	const std::size_t count = payload.size();
	int int_img_size = count;
	unsigned long img_size = count;
	unsigned char* img_data = (unsigned char*)malloc(img_size);
	std::memcpy(img_data, payload.constData(), img_size);
	int uncomp_size = sampling_rate * sampling_rate * 3;
	JSAMPARRAY buffer;    /* Output row buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, img_data, img_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	unsigned char* decomp_buffer = (unsigned char*)malloc(uncomp_size);
	int row_stride = cinfo.output_width * cinfo.output_components;


	while (cinfo.output_scanline < cinfo.output_height) {
		//printf("output_scanline: %d\n", cinfo.output_scanline);
		unsigned char* buffer_array[1];
		buffer_array[0] = decomp_buffer + (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(img_data);

	const char* img = reinterpret_cast<const char*>(decomp_buffer);
	QByteArray uncomp_img(img, uncomp_size);

	free(decomp_buffer);
	return uncomp_img;

}


void Viewer::onBinaryPacketReceived(QByteArray message) {

	std::string msg_size = std::to_string(message.size());

	QByteArray payload = message;

	char payload_type = payload.back();
	
	PacketUpdateInfo packet_update_info;
	struct jpeg_decompress_struct cinfo;


	if (payload_type == 's') {
		payload.chop(1);

		QString str_payload(payload);
		QJsonDocument render_info = QJsonDocument::fromJson(str_payload.toUtf8());
		QJsonObject summary = render_info.object();
		int state = summary.value(QString("state")).toInt();
		QString state_message = summary.value(QString("message")).toString();

		if (state >= 0) {
			m_popup->show();
			m_loading->start();
		}
		else if (state == -1) {
			printf("Server Exception :: %s\n", state, state_message.toStdString().c_str());
		}

		return;
	}
	else if(m_popup->isVisible()){
		m_popup->hide();
		m_loading->stop();
	}

	if (payload_type == 't') {
		payload.chop(1);

		QString str_payload(payload);
		QJsonDocument render_info = QJsonDocument::fromJson(str_payload.toUtf8());
		QJsonObject summary = render_info.object();
		QJsonObject tree_info = summary["tree"].toObject();

		m_welcome_window->buildRemoteFileTree(tree_info);

		return;
	}
	else if (payload_type == 'm') {
		payload.chop(1);

		QString str_payload(payload);
		QJsonDocument render_info = QJsonDocument::fromJson(str_payload.toUtf8());
		QJsonObject summary = render_info.object();
		QJsonObject tree_info = summary["tree"].toObject();

		slotOpenRemoteMask(tree_info);

		return;
	}
	else if (payload_type == 'e') {
		payload.chop(1);
		QString str_payload(payload);
		QMessageBox msgBox;
		msgBox.setText("error :" + str_payload);
		msgBox.exec();
		slotBackToWelcome();

	}
	else if (payload_type == 'h') {
		payload.chop(1);

		QString str_payload(payload);
		QJsonDocument render_info = QJsonDocument::fromJson(str_payload.toUtf8());
		QJsonObject summary = render_info.object();
		QJsonObject update_info_obj = summary.value(QString("update_info")).toObject();
		
		packet_update_info.volume_json = update_info_obj.value(QString("volume_json")).toBool();
		packet_update_info.axial_image = update_info_obj.value(QString("axial_image")).toBool();
		packet_update_info.coronal_image = update_info_obj.value(QString("coronal_image")).toBool();
		packet_update_info.sagittal_image = update_info_obj.value(QString("sagittal_image")).toBool();
		packet_update_info.axial_json = update_info_obj.value(QString("axial_json")).toBool();
		packet_update_info.coronal_json = update_info_obj.value(QString("coronal_json")).toBool();
		packet_update_info.sagittal_json = update_info_obj.value(QString("sagittal_json")).toBool();
		packet_update_info.dvr = update_info_obj.value(QString("dvr")).toBool();
		packet_update_info.mask_json = update_info_obj.value(QString("mask_json")).toBool();


		QJsonObject axial_obj = summary.value(QString("axial")).toObject();
		QJsonObject sagittal_obj = summary.value(QString("sagittal")).toObject();
		QJsonObject coronal_obj = summary.value(QString("coronal")).toObject();
		QJsonObject dvr_obj = summary.value(QString("dvr")).toObject();
		QJsonObject mask_obj = summary.value(QString("mask")).toObject();
		QJsonObject volume_obj = summary.value(QString("volume")).toObject();

		if (packet_update_info.volume_json) {
			QJsonObject volume_feature = volume_obj.value(QString("volume")).toObject();
			m_volume_packet.ImageRescaleIntercept = volume_feature.value(QString("ImageRescaleIntercept")).toDouble();
			m_volume_packet.ImageRescaleSlope = volume_feature.value(QString("ImageRescaleSlope")).toDouble();
			m_volume_packet.PixelRepresentation = volume_feature.value(QString("PixelRepresentation")).toDouble();
			m_volume_packet.PhotometricInterpretation = volume_feature.value(QString("PhotometricInterpretation")).toString().toStdString();
			m_volume_packet.ImageWindowWidth = volume_feature.value(QString("ImageWindowWidth")).toInt();
			m_volume_packet.ImageWindowCenter = volume_feature.value(QString("ImageWindowCenter")).toInt();
			m_volume_packet.ImageCols = volume_feature.value(QString("ImageCols")).toInt();
			m_volume_packet.ImageRows = volume_feature.value(QString("ImageRows")).toInt();
			m_volume_packet.PixelSpacingRow = volume_feature.value(QString("PixelSpacingRow")).toDouble();
			m_volume_packet.PixelSpacingCol = volume_feature.value(QString("PixelSpacingCol")).toDouble();
			m_volume_packet.BitsStored = volume_feature.value(QString("BitsStored")).toInt();
			m_volume_packet.BitsAllocated = volume_feature.value(QString("BitsAllocated")).toInt();
			m_volume_packet.SliceThickness = volume_feature.value(QString("SliceThickness")).toDouble();
			m_volume_packet.MaxVal = volume_feature.value(QString("MaxVal")).toInt();
			m_volume_packet.MinVal = volume_feature.value(QString("MinVal")).toInt();

			axial_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
			axial_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });
			axial_slice->setWL(m_volume_packet.ImageWindowCenter);
			axial_slice->setWW(m_volume_packet.ImageWindowWidth);
			coronal_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
			coronal_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });
			coronal_slice->setWL(m_volume_packet.ImageWindowCenter);
			coronal_slice->setWW(m_volume_packet.ImageWindowWidth);
			sagittal_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
			sagittal_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });
			sagittal_slice->setWL(m_volume_packet.ImageWindowCenter);
			sagittal_slice->setWW(m_volume_packet.ImageWindowWidth);

			m_view_slots->resetOTF(m_volume_packet);
		}
		if (packet_update_info.axial_json) {

	
			QJsonObject axial_feature = axial_obj.value(QString("features")).toObject();
			QJsonObject axial_mpr = axial_feature.value(QString("mpr")).toObject();
			QJsonArray temp_slice_line = axial_mpr.value(QString("slice_line")).toArray();
			m_axial_packet.slice_line = std::make_tuple(temp_slice_line[0].toDouble(), temp_slice_line[1].toDouble(), temp_slice_line[2].toDouble());
			QJsonArray temp_thickness = axial_mpr.value(QString("thickness")).toArray();
			m_axial_packet.thickness = std::make_tuple(temp_thickness[0].toDouble(), temp_thickness[1].toDouble());
			QJsonArray temp_center_point = axial_mpr.value(QString("center_point")).toArray();
			m_axial_packet.center_point = std::make_tuple(temp_center_point[0].toDouble(), temp_center_point[1].toDouble(), temp_center_point[2].toDouble());
			QJsonArray temp_lefttop_point = axial_mpr.value(QString("lefttop_point")).toArray();
			m_axial_packet.lefttop_point = std::make_tuple(temp_lefttop_point[0].toDouble(), temp_lefttop_point[1].toDouble(), temp_lefttop_point[2].toDouble());
			QJsonArray temp_normal_vector = axial_mpr.value(QString("normal_vector")).toArray();
			m_axial_packet.normal_vector = std::make_tuple(temp_normal_vector[0].toDouble(), temp_normal_vector[1].toDouble(), temp_normal_vector[2].toDouble());
			QJsonArray temp_width_direction_vector = axial_mpr.value(QString("width_direction_vetor")).toArray();
			m_axial_packet.width_direction_vetor = std::make_tuple(temp_width_direction_vector[0].toDouble(), temp_width_direction_vector[1].toDouble(), temp_width_direction_vector[2].toDouble());
			QJsonArray temp_height_direction_vector = axial_mpr.value(QString("height_direction_vector")).toArray();
			m_axial_packet.height_direction_vector = std::make_tuple(temp_height_direction_vector[0].toDouble(), temp_height_direction_vector[1].toDouble(), temp_height_direction_vector[2].toDouble());
			QJsonArray temp_pixel_space = axial_mpr.value(QString("pixel_space")).toArray();
			m_axial_packet.pixel_space = std::make_tuple(temp_pixel_space[0].toDouble(), temp_pixel_space[1].toDouble());
			QJsonValue m_axial_size = axial_mpr.value(QString("m_axis_size"));
			m_axial_packet.m_axis_size = m_axial_size.toDouble();
			QJsonValue m_axial_image_size = axial_mpr.value(QString("image_size")).toArray();
			m_axial_packet.image_size = std::make_tuple(m_axial_image_size[0].toDouble(), m_axial_image_size[1].toDouble());
			
			axial_slice->setZoomRatioInfo(m_axial_packet.m_axis_size);

			if (!packet_update_info.axial_image) {
				axial_slice->setGuideLine(m_axial_packet);
			}
		}
		if (packet_update_info.sagittal_json) {
			QJsonObject sagittal_feature = sagittal_obj.value(QString("features")).toObject();
			QJsonObject sagittal_mpr = sagittal_feature.value(QString("mpr")).toObject();
			QJsonArray temp_slice_line = sagittal_mpr.value(QString("slice_line")).toArray();
			m_sagittal_packet.slice_line = std::make_tuple(temp_slice_line[0].toDouble(), temp_slice_line[1].toDouble(), temp_slice_line[2].toDouble());
			QJsonArray temp_thickness = sagittal_mpr.value(QString("thickness")).toArray();
			m_sagittal_packet.thickness = std::make_tuple(temp_thickness[0].toDouble(), temp_thickness[1].toDouble());
			QJsonArray temp_center_point = sagittal_mpr.value(QString("center_point")).toArray();
			m_sagittal_packet.center_point = std::make_tuple(temp_center_point[0].toDouble(), temp_center_point[1].toDouble(), temp_center_point[2].toDouble());
			QJsonArray temp_lefttop_point = sagittal_mpr.value(QString("lefttop_point")).toArray();
			m_sagittal_packet.lefttop_point = std::make_tuple(temp_lefttop_point[0].toDouble(), temp_lefttop_point[1].toDouble(), temp_lefttop_point[2].toDouble());
			QJsonArray temp_normal_vector = sagittal_mpr.value(QString("normal_vector")).toArray();
			m_sagittal_packet.normal_vector = std::make_tuple(temp_normal_vector[0].toDouble(), temp_normal_vector[1].toDouble(), temp_normal_vector[2].toDouble());
			QJsonArray temp_width_direction_vector = sagittal_mpr.value(QString("width_direction_vetor")).toArray();
			m_sagittal_packet.width_direction_vetor = std::make_tuple(temp_width_direction_vector[0].toDouble(), temp_width_direction_vector[1].toDouble(), temp_width_direction_vector[2].toDouble());
			QJsonArray temp_height_direction_vector = sagittal_mpr.value(QString("height_direction_vector")).toArray();
			m_sagittal_packet.height_direction_vector = std::make_tuple(temp_height_direction_vector[0].toDouble(), temp_height_direction_vector[1].toDouble(), temp_height_direction_vector[2].toDouble());
			QJsonArray temp_pixel_space = sagittal_mpr.value(QString("pixel_space")).toArray();
			m_sagittal_packet.pixel_space = std::make_tuple(temp_pixel_space[0].toDouble(), temp_pixel_space[1].toDouble());
			QJsonValue m_sagittal_size = sagittal_mpr.value(QString("m_axis_size"));
			m_sagittal_packet.m_axis_size= m_sagittal_size.toDouble();
			QJsonValue m_sagittal_image_size = sagittal_mpr.value(QString("image_size")).toArray();
			m_sagittal_packet.image_size = std::make_tuple(m_sagittal_image_size[0].toDouble(), m_sagittal_image_size[1].toDouble());
		
			sagittal_slice->setZoomRatioInfo(m_sagittal_packet.m_axis_size);


			if (!packet_update_info.sagittal_image) {
				sagittal_slice->setGuideLine(m_sagittal_packet);
			}
		}
		if (packet_update_info.coronal_json) {
			QJsonObject coronal_feature = coronal_obj.value(QString("features")).toObject();
			QJsonObject coronal_mpr = coronal_feature.value(QString("mpr")).toObject();
			QJsonArray temp_slice_line = coronal_mpr.value(QString("slice_line")).toArray();
			m_coronal_packet.slice_line = std::make_tuple(temp_slice_line[0].toDouble(), temp_slice_line[1].toDouble(), temp_slice_line[2].toDouble());
			QJsonArray temp_thickness = coronal_mpr.value(QString("thickness")).toArray();
			m_coronal_packet.thickness = std::make_tuple(temp_thickness[0].toDouble(), temp_thickness[1].toDouble());
			QJsonArray temp_center_point = coronal_mpr.value(QString("center_point")).toArray();
			m_coronal_packet.center_point = std::make_tuple(temp_center_point[0].toDouble(), temp_center_point[1].toDouble(), temp_center_point[2].toDouble());
			QJsonArray temp_lefttop_point = coronal_mpr.value(QString("lefttop_point")).toArray();
			m_coronal_packet.lefttop_point = std::make_tuple(temp_lefttop_point[0].toDouble(), temp_lefttop_point[1].toDouble(), temp_lefttop_point[2].toDouble());
			QJsonArray temp_normal_vector = coronal_mpr.value(QString("normal_vector")).toArray();
			m_coronal_packet.normal_vector = std::make_tuple(temp_normal_vector[0].toDouble(), temp_normal_vector[1].toDouble(), temp_normal_vector[2].toDouble());
			QJsonArray temp_width_direction_vector = coronal_mpr.value(QString("width_direction_vetor")).toArray();
			m_coronal_packet.width_direction_vetor = std::make_tuple(temp_width_direction_vector[0].toDouble(), temp_width_direction_vector[1].toDouble(), temp_width_direction_vector[2].toDouble());
			QJsonArray temp_height_direction_vector = coronal_mpr.value(QString("height_direction_vector")).toArray();
			m_coronal_packet.height_direction_vector = std::make_tuple(temp_height_direction_vector[0].toDouble(), temp_height_direction_vector[1].toDouble(), temp_height_direction_vector[2].toDouble());
			QJsonArray temp_pixel_space = coronal_mpr.value(QString("pixel_space")).toArray();
			m_coronal_packet.pixel_space = std::make_tuple(temp_pixel_space[0].toDouble(), temp_pixel_space[1].toDouble());
			QJsonValue m_coronal_size = coronal_mpr.value(QString("m_axis_size"));
			m_coronal_packet.m_axis_size = m_coronal_size.toDouble();
			QJsonValue m_coronal_image_size = coronal_mpr.value(QString("image_size")).toArray();
			m_coronal_packet.image_size = std::make_tuple(m_coronal_image_size[0].toDouble(), m_coronal_image_size[1].toDouble());

			coronal_slice->setZoomRatioInfo(m_coronal_packet.m_axis_size);


			if (!packet_update_info.coronal_image) {
				coronal_slice->setGuideLine(m_coronal_packet);
			}
		
		}
		if (packet_update_info.dvr) {
			QJsonObject dvr_feature = dvr_obj.value(QString("features")).toObject();
			QJsonObject dvr_dvr = dvr_feature.value(QString("dvr")).toObject();
			QJsonArray temp_lefttop_point = dvr_dvr.value(QString("lefttop_point")).toArray();
			m_dvr_packet.lefttop_point = std::make_tuple(temp_lefttop_point[0].toDouble(), temp_lefttop_point[1].toDouble(), temp_lefttop_point[2].toDouble());
			QJsonArray temp_normal_vector = dvr_dvr.value(QString("normal_vector")).toArray();
			m_dvr_packet.normal_vector = std::make_tuple(temp_normal_vector[0].toDouble(), temp_normal_vector[1].toDouble(), temp_normal_vector[2].toDouble());
			QJsonArray temp_width_direction_vector = dvr_dvr.value(QString("width_direction_vetor")).toArray();
			m_dvr_packet.width_direction_vetor = std::make_tuple(temp_width_direction_vector[0].toDouble(), temp_width_direction_vector[1].toDouble(), temp_width_direction_vector[2].toDouble());
			QJsonArray temp_height_direction_vector = dvr_dvr.value(QString("height_direction_vector")).toArray();
			m_dvr_packet.height_direction_vector = std::make_tuple(temp_height_direction_vector[0].toDouble(), temp_height_direction_vector[1].toDouble(), temp_height_direction_vector[2].toDouble());
			QJsonArray temp_pixel_space = dvr_dvr.value(QString("pixel_space")).toArray();
			m_dvr_packet.pixel_space = std::make_tuple(temp_pixel_space[0].toDouble(), temp_pixel_space[1].toDouble(), temp_pixel_space[2].toDouble());
			QJsonValue m_dvr_size = dvr_dvr.value(QString("m_axis_size"));
			m_dvr_packet.m_axis_size = m_dvr_size.toDouble();
			QJsonValue m_dvr_image_size = dvr_dvr.value(QString("image_size")).toArray();
			m_dvr_packet.image_size = std::make_tuple(m_dvr_image_size[0].toDouble(), m_dvr_image_size[1].toDouble());

			dvr->setZoomRatioInfo(m_dvr_packet.m_axis_size);


			//printf("[DVR] pixel_space: %.1f, %.1f, %.1f\n", std::get<0>(m_dvr_packet.pixel_space), std::get<1>(m_dvr_packet.pixel_space), std::get<2>(m_dvr_packet.pixel_space));
		}
		if (packet_update_info.mask_json) {
			m_tb_main->resetMaskLayer();

			QJsonObject mask_mask = mask_obj.value(QString("mask")).toObject();

			QJsonObject mask[G_MAX_MASK_NUM+1];
			for (int i = 1;i <= G_MAX_MASK_NUM; i++) {
				mask[i] = mask_mask.value(QString(std::to_string(i).c_str())).toObject();
			}

			int max_idx = 0;
			for (int i = 1; i <= G_MAX_MASK_NUM; i++) {
				if (!mask[i].value(QString("is_empty")).toBool()) {
					if (i > max_idx) max_idx = i;
				}
			}

			for (int i = 1; i <= max_idx; i++) {
				MaskLabel mask_label;
				if (!mask[i].value(QString("is_empty")).toBool()) {
					mask_label.is_empty = mask[i].value(QString("is_empty")).toBool();
					mask_label.name = mask[i].value(QString("name")).toString().toStdString();
					mask_label.is_visible = mask[i].value(QString("is_visible")).toBool();
					mask_label.color = mask[i].value(QString("color")).toString().toStdString();
				}
				m_tb_main->slotAddedMask(i, mask_label);
			}
		}
		char secondary = payload.back();

		if (secondary == 'a') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			int image_width = std::get<0>(m_axial_packet.image_size);
			int image_height = std::get<1>(m_axial_packet.image_size);


			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, image_width, image_height);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				axial_slice->setSlice(m_axial_packet, new_plane);
				axial_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				axial_slice->setSlice(m_axial_packet, new_plane);
				axial_fps_counter++;
			}
			
			
		}
		if (secondary == 's') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);
			int image_width = std::get<0>(m_sagittal_packet.image_size);
			int image_height = std::get<1>(m_sagittal_packet.image_size);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, image_width, image_height);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				sagittal_slice->setSlice(m_sagittal_packet, new_plane);

				sagittal_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				sagittal_slice->setSlice(m_sagittal_packet, new_plane);

				sagittal_fps_counter++;
			}
			
		}
		if (secondary == 'c') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			int image_width = std::get<0>(m_coronal_packet.image_size);
			int image_height = std::get<1>(m_coronal_packet.image_size);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, image_width, image_height);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				coronal_slice->setSlice(m_coronal_packet, new_plane);

				coronal_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				coronal_slice->setSlice(m_coronal_packet, new_plane);

				coronal_fps_counter++;
			}
			
		}
		if (secondary == 'p') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, cur_resolution);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				cpr_view->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				cpr_view->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}

		}
		if (secondary == 'v') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, cur_resolution);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				cpr_slice->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				cpr_slice->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}

		}

		if (secondary == 't') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, cur_resolution);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				path_slice->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, cur_resolution);

				path_slice->setSlice(m_coronal_packet, new_plane);

				//coronal_fps_counter++;
			}

		}


		if (secondary == 'd') {
			payload.chop(1);
			int cur_resolution = std::stoi(QString(payload.right(4)).toStdString());
			payload.chop(4);

			int image_width = std::get<0>(m_dvr_packet.image_size);
			int image_height = std::get<1>(m_dvr_packet.image_size);

			if (m_jpeg_on) {
				QByteArray uncomp_img = decomp_jpeg(payload, image_width, image_height);
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);
				dvr->setSlice(m_dvr_packet, new_plane);
				dvr_fps_counter++;
			}
			else {
				QByteArray uncomp_img = payload;
				cgip::CgipPlane new_plane = _base64_to_plane(uncomp_img, image_width, image_height);

				dvr->setSlice(m_dvr_packet, new_plane);

				dvr_fps_counter++;
			}
			
		}

	}

	if (axial_slice->getMode() < 0) {
		axial_slice->setMode(0);
		coronal_slice->setMode(0);
		sagittal_slice->setMode(0);
		dvr->setMode(0);
	}
}


void Viewer::slotConnected() {
	std::cout << "connected....";

#ifdef REMOTE_VIEW
	m_tb_main->hide();
	stacked_window->setCurrentIndex(0);

	json param;
	getPacket("Connection", "connect", param);

	getPacket("Tree", "data", param);
	server_status = 200;
#endif

}

void Viewer::slotDisconnected() {
	socket.abort();
	std::cout << "disconnected....";

}




void Viewer::getPacket(QString renderingType, QString actionType, json params) {

	/* --- for local test purposes ---*/

	//if (slice_type == AXIAL_SLICE)
	//	dir = "F:/temp/axial";
	//else if (slice_type == CORONAL_SLICE)
	//	dir = "F:/temp/coronal";
	//else if (slice_type == SAGITTAL_SLICE)
	//	dir = "F:/temp/sagittal";


	json message;
	message["RenderingType"] = renderingType.toStdString();
	message["Action"] = actionType.toStdString();
	message["UserID"] = "rlawodyd";
	message["ImageType"] = m_jpeg_on;
	message["MPRSamplingRateWidth"] = m_mpr_resolution;
	message["MPRSamplingRateHeight"] = m_mpr_resolution;
	message["isDVRDownsampling"] = m_dvr_is_downsampling;
	message["isMPRDownsampling"] = m_mpr_is_downsampling;
	message["VRSamplingRate"] = m_vr_resolution;
	message["CompressionRate"] = m_compression_rate;
	message["Params"]["value1"] = NULL;
	message["Params"]["value2"] = NULL;
	message["Params"]["value3"] = NULL;
	message["Params"]["value4"] = NULL;
	message["Params"]["value5"] = NULL;
	message["Params"]["value6"] = NULL;

	if (!params.empty())
		message.merge_patch(params);

	//std::cout << message.dump() << endl;

	QString action_msg = QString::fromUtf8(message.dump().c_str());

	socket.sendTextMessage(action_msg);
}

void Viewer::localUpdate() {

	PacketUpdateInfo update_info = m_rendering_function_mgr->getPacketUpdateInfo();


	if (update_info.error) {
		QString errorMsg(m_rendering_function_mgr->getErrorMessage().c_str());
		QMessageBox msgBox;
		msgBox.setText("Dicom error :" + errorMsg);
		msgBox.exec();
		slotBackToWelcome();
		return;
	}


	if (update_info.volume_json) {
		json j = m_rendering_function_mgr->getVolumeInfobyJson();
		m_volume_packet.ImageRescaleIntercept = j["volume"]["ImageRescaleIntercept"];
		m_volume_packet.MaxVal = j["volume"]["MaxVal"];
		m_volume_packet.MinVal = j["volume"]["MinVal"];
		

		axial_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
		axial_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });
		coronal_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
		coronal_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });
		sagittal_slice->setWLRange({ m_volume_packet.MinVal, m_volume_packet.MaxVal });
		sagittal_slice->setWWRange({ 10, m_volume_packet.MaxVal - m_volume_packet.MinVal });

		m_view_slots->resetOTF(m_volume_packet);

	}
	if (update_info.axial_json) {
		json j = m_rendering_function_mgr->getMPRInfobyJson(AXIAL_SLICE);
		m_axial_packet.slice_line = j["mpr"]["slice_line"];
		m_axial_packet.thickness = j["mpr"]["thickness"];
		
		init_axial_size = j["mpr"]["m_axis_size"];
		float ratio = j["mpr"]["m_axis_size"];
		axial_slice->setZoomRatioInfo(ratio);

		
		if (!update_info.axial_image) {
			axial_slice->setGuideLine(m_axial_packet);
		}
	}
	if (update_info.sagittal_json) {
		json j = m_rendering_function_mgr->getMPRInfobyJson(SAGITTAL_SLICE);
		m_sagittal_packet.slice_line = j["mpr"]["slice_line"];
		m_sagittal_packet.thickness = j["mpr"]["thickness"];
		
		float ratio = j["mpr"]["m_axis_size"];
		sagittal_slice->setZoomRatioInfo(ratio);

		if (!update_info.sagittal_image) {
			sagittal_slice->setGuideLine(m_sagittal_packet);
		}
	}
	if (update_info.coronal_json) {
		json j = m_rendering_function_mgr->getMPRInfobyJson(CORONAL_SLICE);
		m_coronal_packet.slice_line = j["mpr"]["slice_line"];
		m_coronal_packet.thickness = j["mpr"]["thickness"];

		float ratio = j["mpr"]["m_axis_size"];
		coronal_slice->setZoomRatioInfo(ratio);


		if (!update_info.coronal_image) {
			coronal_slice->setGuideLine(m_coronal_packet);
		}

	}

	if (update_info.cpr_curve_json) {
		//json j = m_rendering_function_mgr->getCPRCurveInfobyJson(currentCPRView);
		//float ratio = j["cpr"]["m_zoom_factor"];
		//cpr_slice ->setZoomRatioInfo(ratio);
	}



	if (update_info.mask_json) {
		m_tb_main->resetMaskLayer();

		json j = m_rendering_function_mgr->getMaskInfobyJson();

		int max_idx = 0;
		for (int i = 1; i <= G_MAX_MASK_NUM; i++) {
			if (!j["mask"][std::to_string(i)]["is_empty"]) {
				if (i > max_idx) max_idx = i;
			}
		}

		for (int i = 1; i <= max_idx; i++) {
			MaskLabel mask_label;
			if (!j["mask"][std::to_string(i)]["is_empty"]) {
				mask_label.is_empty = j["mask"][std::to_string(i)]["is_empty"];
				mask_label.name = j["mask"][std::to_string(i)]["name"].dump();
				mask_label.is_visible = j["mask"][std::to_string(i)]["is_visible"];
				mask_label.color = j["mask"][std::to_string(i)]["color"].dump();
			}
			m_tb_main->slotAddedMask(i, mask_label);
		}
	}

	if (update_info.axial_image) {
		axial_slice->setSlice(m_axial_packet, m_rendering_function_mgr->getMPRImage(AXIAL_SLICE));
		m_rendering_function_mgr->write_jpeg_file(&m_rendering_function_mgr->getMPRImage(AXIAL_SLICE), "axial_test.jpeg", 100);

	}
	if (update_info.coronal_image) {
		coronal_slice->setSlice(m_coronal_packet, m_rendering_function_mgr->getMPRImage(CORONAL_SLICE));
		//m_rendering_function_mgr->write_jpeg_file(&m_rendering_function_mgr->getMPRImage(CORONAL_SLICE), "coronal_test.jpeg", 100);
	}
	if (update_info.sagittal_image) {
		sagittal_slice->setSlice(m_sagittal_packet, m_rendering_function_mgr->getMPRImage(SAGITTAL_SLICE));
		
	}
	if (update_info.dvr) {
		json j = m_rendering_function_mgr->getVRInfobyJson();
		//m_dvr_packet.window_width = j["dvr"]["window_width"];
		//m_dvr_packet.window_level = j["dvr"]["window_level"];
		float ratio = j["dvr"]["m_axis_size"];
		dvr->setZoomRatioInfo(ratio);
		m_rendering_function_mgr->write_jpeg_file(&m_rendering_function_mgr->getVRImage(), "dvr_test.jpeg", 100);
		dvr->setSlice(m_dvr_packet, m_rendering_function_mgr->getVRImage());

		//int ratio = j["dvr"]["m_axis_size"];
	}
	if (update_info.cpr_image) {
		if (currentCPRView == AXIAL_SLICE) {
			cpr_view->setSlice(m_axial_packet, m_rendering_function_mgr->getMPRImage(AXIAL_SLICE));
		}
		else if (currentCPRView == CORONAL_SLICE) {
			cpr_view->setSlice(m_coronal_packet, m_rendering_function_mgr->getMPRImage(CORONAL_SLICE));
		}
		else {
			cpr_view->setSlice(m_sagittal_packet, m_rendering_function_mgr->getMPRImage(SAGITTAL_SLICE));
		}
	}

	if (update_info.cpr_curve_image) {
		cpr_slice->setSlice(m_cpr_packet, m_rendering_function_mgr->getCPRImage());
	}
	if (update_info.cpr_path_image) {
		path_slice->setSlice(m_cpr_packet, m_rendering_function_mgr->getCPRPathImage());
	}


	if (axial_slice->getMode() < 0) {
		axial_slice->setMode(0);
		coronal_slice->setMode(0);
		sagittal_slice->setMode(0);
		dvr->setMode(0);
	}
}

cgip::CgipPlane Viewer::_base64_to_plane(QByteArray barray, int width, int height) {
	cgip::CgipPlane new_plane(width, height, 3);

//#pragma omp parallel for
	for (int xy = 0; xy < new_plane.getWidth() * new_plane.getHeight(); xy++) {
		int y = xy / new_plane.getWidth();
		int x = xy % new_plane.getWidth();

		new_plane.setPixelValue(x, y, 0, barray[3 * (y * width + x) + 0]);
		new_plane.setPixelValue(x, y, 1, barray[3 * (y * width + x) + 1]);
		new_plane.setPixelValue(x, y, 2, barray[3 * (y * width + x) + 2]);
	}

	return new_plane;
};


cgip::CgipPlane Viewer::_base64_to_plane(QByteArray barray, int sampling_rate) {
	cgip::CgipPlane new_plane(sampling_rate, sampling_rate, 3);

	//#pragma omp parallel for
	for (int xy = 0; xy < new_plane.getWidth() * new_plane.getHeight(); xy++) {
		int y = xy / new_plane.getWidth();
		int x = xy % new_plane.getWidth();

		new_plane.setPixelValue(x, y, 0, barray[3 * (y * sampling_rate + x) + 0]);
		new_plane.setPixelValue(x, y, 1, barray[3 * (y * sampling_rate + x) + 1]);
		new_plane.setPixelValue(x, y, 2, barray[3 * (y * sampling_rate + x) + 2]);
	}

	return new_plane;
};