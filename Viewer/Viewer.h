#pragma once

#include <Windows.h>
#include <experimental/filesystem>

// Qt includes
#include <QtWidgets/QMainWindow>
#include <QLayout>
#include <QStackedWidget>
#include <QtWebSockets/qwebsocket.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScreen>
#include <QMovie>
#include <QString>


// External library includes
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

// Manager classes includes
//#include "../ViewerManager/MaskMgr.h"
//#include "../ViewerManager/MPRMgr.h"
//#include "../ViewerManager/DVRMgr.h"
//#include "../ViewerManager/VolumeMgr.h"
#include "../ViewerManager/global.h"
#include "../ViewerManager/RenderingFunctionMgr.h"

#include "build_global_opt.h"

// Local includes
#include "ViewLayout.h"
#include "WelcomeWindow.h"
#include "ToolBar.h"
#include "DVRWidget.h"
#include "SliceWidget.h"
#include "RemoteFileTree.h"
#include "OTFWidget.h"
#include "SelectTableDialog.h"
#include "CPRPathWidget.h"
#include "CPRWidget.h"
#include "CPRView.h"


class Viewer : public QMainWindow
{
	Q_OBJECT

public:
	Viewer(QWidget* parent = Q_NULLPTR);

private slots:


	/* ----------------------------   Slots for IO   ------------------------- */
	void slotReqOpenVolume(QString, bool);
	void slotReqOpenLocalMask();
	void slotOpenRemoteMask(QJsonObject json_path);
	void slotReqExportMask();
	//void slotReqOpenPreviewVolume(QString);
	//void slotLoadPreviewVolume(RemoteTreeType);
	//void slotLoadPreviewPlane();
	//void slotReqOpenPreviewPlane(QString file_path, int idx);

	/* ----------------------------   Slots for User Actions   ------------------------- */
	void slotChangeViewLayout();

	// MPR Action
	void slotGuideLineMoved(bool, bool, float, float, SliceType);
	void slotGuideLineRotated(float, SliceType);
	void slotGuideLineThickness(float, bool, bool, SliceType);
	void slotThicknessChanged(SliceType, int);
	void slotThicknessModeChanged(int);
	void slotScreenDragged(float, float, ClickType, SliceType, bool is_preview = false);
	void slotDoubleClicked(float, float, ClickType, SliceType, bool is_preview = false);
	void slotMouseReleased(float, float, ClickType, SliceType);
	void slotWheelChanged(float, SliceType, bool is_preview = false);
	void slotClicked(float, float, ClickType, SliceType);
	void slotInverse(bool);
	void slotZoomByRatio(float, SliceType);


	//CPRAction
	void slotCPRInit(SliceType slice_type);
	void slotUpdateCPRCurve(std::vector <QPointF> pathPoints, int numOfPoints, SliceType slice_type);
	//void slotUpdateCPRPath(QPointF, QPointF, SliceType);
	void slotUpdateCPRPath(std::vector<QPointF> pathPoints, float pathLength, SliceType slice_type);
	void slotCPRClicked(SliceType, int mode);
	void slotCPRReleased(SliceType, int mode);



	// DVR Action
	void slotClicked(float, float, int, int, ClickType);
	void slotDoubleClicked(ClickType);
	//void slotScreenDragged(float, float, ClickType);
	void slotVR_rotate(float, float, int, int);
	void slotVR_pan(float, float);
	void slotWheelChanged(float);
	//void slotMouseReleased(float, float, ClickType);
	void slotMouseReleased();
	void slotTFChanged(QList<OTFPolygon*>, int offset);
	void slotColorChanged(QList<OTFColor>);
	void slotChangeAll(QList<OTFColor>, QList<OTFPolygon*>, int offset);
	void slotOTFClicked();
	void slotOTFReleased();
	void slotShaderStatusChanged(int);
	void slotSliderDragged(float, float);
	void slotSliderClicked();
	void slotSliderReleased();
	void slotRunOTF();
	void slotZoomByRatio(float);

	void slotReturnToMPR();
	void slotBackToWelcome();

	// Mask Action
	void slotSetMaskVisible(int, bool);
	void slotChangeMaskOpacity(int);
	void slotChangeMaskColor(int, std::string);
	void slotAddMask(int);

	void slotFullScreen(SliceType);
	void slotCPRScreen(SliceType);
	void slotCPRChangeAxis(SliceType);

	// Functions
	void slotFunctionChanged(int, FunctionType);
	void slotSculpt(std::vector<int>, std::vector<int>);
	void slotCrop(std::vector<int>, std::vector<int>);
	void slotVOI(std::vector<int>, std::vector<int>);
	void slotDVRReset();
	void slotDVROrientation(VROrientationType type);
	void slotResolutionChanged(RenderingType, int, bool);
	void slotCPRResolutionChanged(int, int, SliceType);
	void slotProjectionChanged(VRProjectionType type);
	void slotImageTypeChanged(int);
	

	//Hard Reset
	void slotHardReset();


	/*----------------------------	Slots for Network Communication	  -------------------- */

	void slotConnected();
	void slotDisconnected();


#ifdef REMOTE_VIEW
#endif
	void onBinaryPacketReceived(QByteArray message);
	//FPS
	void OnTimerCallbackFunction();



public:
	std::shared_ptr<QTimer> m_pTimer;

	

private:

	void connectSocket();


	QScreen* cur_screen = QGuiApplication::primaryScreen();
	QScreen* prev_screen = cur_screen;
	int m_width = cur_screen->size().width();
	int m_height = cur_screen->size().height();
	int screen_center_x = m_width / 2;
	int screen_center_y = m_height / 2;

	MainToolBar* m_tb_main;
	ViewLayout* m_view_slots;
	ViewLayout* m_cpr_slots;
	WelcomeWindow* m_welcome_window;
	bool eventFilter(QObject* obj, QEvent* event);

	// Manager objects
	//MaskMgr* m_mask_mgr;
	//MPRMgr*			m_preview_mpr_mgr;
	//SettingMgr*		m_setting_mgr;
	//VolumeMgr*		m_preview_volume_mgr;
	RenderingFunctionMgr* m_rendering_function_mgr;

	SliceWidget* axial_slice, * coronal_slice, * sagittal_slice;
	CPRPathWidget* path_slice;
	CPRView* cpr_view;
	SliceType currentCPRView;
	CPRWidget* cpr_slice;
	DVRWidget* dvr;
	OTFWidget* otf;
	OTFWidget* otf_control;
	SliceWidget* preview_axial_slice;
	QStackedWidget* stacked_window;

	VolumePacket m_volume_packet;
	SlicePacket m_axial_packet, m_sagittal_packet, m_coronal_packet, m_cpr_packet;
	DVRPacket m_dvr_packet;

	QLabel* m_popup;
	QMovie* m_loading;

	// image info
	int m_mpr_resolution = SAMPLING_CNT_WIDTH;
	int m_vr_resolution = SAMPLING_CNT_WIDTH;
	int m_cpr_resolution = SAMPLING_CNT_WIDTH;
	int m_mpr_resolution_temp, m_vr_resolution_temp, m_cpr_resolution_temp;
	int m_downsample_resolution = 128;
	int m_compression_rate = 75;
	bool m_jpeg_on = true;
	bool m_otf_on = false;
	bool m_maxip_on = false;
	bool m_minip_on = false;
	bool m_raysum_on = false;
	bool user_moved_window = false;
	bool got_init_axial = true;
	bool got_init_coronal = true;
	bool got_init_sagittal = true;
	bool m_mpr_is_downsampling = false;
	bool m_dvr_is_downsampling = false;
	float init_axial_size = 0;
	float init_coronal_size = 0;
	float init_sagittal_size = 0;
	std::string current_img_dir;
	bool current_img_is_leaf = false;
	std::vector<std::vector<std::vector<int>>> m_otf_params;

	// Logger
	std::shared_ptr<spdlog::logger> m_logger_dev;
	std::shared_ptr<spdlog::logger> m_logger_daily;

	void closeEvent(QCloseEvent* event) override;


	std::string strSliceType(SliceType slice_type);
	void getPacket(QString renderingType, QString actionType, json params);

	void localUpdate();

	// function update slice
	cgip::CgipPlane _base64_to_plane(QByteArray, int width, int height);
	cgip::CgipPlane _base64_to_plane(QByteArray, int sampling_rate);


	//Network Variables 
	QWebSocket socket;
	QUrl url;
	int server_status = 0;

	//fps
	int axial_fps_counter = 0;
	int sagittal_fps_counter = 0;
	int coronal_fps_counter = 0;
	int dvr_fps_counter = 0;
};