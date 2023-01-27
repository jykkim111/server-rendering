#pragma once

#include <string>
#include <map>
#include <mutex>
#include <json.hpp>


using json = nlohmann::json;

#include "../CgipCommon/CgipPlane.h"
#include "../CgipCommon/Memory.h"
#include "../CgipException/CgipException.h"

#define TF_HEIGHT	180
#define TF_MARGIN	22

static std::mutex sMutex;
static int g_nProgramWidth = 0;
static int g_nProgramHeight = 0;

const std::vector<std::string> default_mask_color_table = {
	"BackGround",
	"#F44336",
	"#FF5722",
	"#FFC107",
	"#CDDC39",
	"#4CAF50",
	"#3F51B5",
	"#673AB7",
	"#795548"
};

enum SliceType
{
	AXIAL_SLICE,
	CORONAL_SLICE,
	SAGITTAL_SLICE,
};

enum ClickType
{
	LEFT_CLICK,
	RIGHT_CLICK,
	MIDDLE_CLICK
};

enum FunctionType
{
	NONE,
	MPR_INVERSE,
	MPR_THICKNESS,
	MPR_THICKNESS_AVG,
	MPR_THICKNESS_MIN,
	MPR_THICKNESS_MAX,
	MPR_CURVE,
	DVR_SCULPT,
	DVR_CROP,
	DVR_VOI,
	DVR_RESET,
	DVR_A,
	DVR_P,
	DVR_L,
	DVR_R,
	DVR_H,
	DVR_F,
	DVR_MAXIP,
	DVR_MINIP,
	DVR_RAYSUM,
	DVR_OTF,
	MPR_256,
	MPR_512,
	MPR_1024,
	DVR_256,
	DVR_512,
	DVR_1024,
	DS_128,
	DS_192,
	DS_256,
	DS_384,
	DS_512,
	DS_OFF,
	JPEG_ON,
	BYTE_ON,
	C_25,
	C_50,
	C_75,
	C_100,
	SHADER_ON,
	SHADER_OFF
};

enum ActionType {
	CONNECT,
	DISCONNECT,
	LOAD_DATA,
	ZOOM,
	PAN,
	WINDOW,
	INVERSE,
	CENTER,
	RESET,
	MOVE_SLICE,
	GUIDELINE_MOVED,
	OBLIQUE,
	ROTATE,
	THICKNESS,
	THICKNESS_MODE,
	SCULPT,
	RESET_SCULPT,
	VOI,
	ORIENTATION,
	DATA,
	PROJECTION,
	MASK_VISIBLE,
	MASK_OPACITY,
	MASK_COLOR,
	LOAD_MASK,
	SAVE_MASK,
	CURVE,
	PATH,
	INIT,
	SETTINGS,
	RAWP
};

enum RenderingType {
	MPR,
	VR,
	CPR,
	HARDRESET
};

enum VROrientationType {
	A,
	P,
	L,
	R,
	H,
	F
};

enum VRProjectionType {
	OTF,
	MAXIP,
	MINIP,
	RAYSUM
};

const int REMOTE_TREE_CNT = 10;

typedef struct CPR_COORD {
	float x;
	float y;
	float z;
};


typedef struct TF_BGRA
{
	float b;
	float g;
	float r;
	float a;

	TF_BGRA() {
		b = 0.0f;
		g = 0.0f;
		r = 0.0f;
		a = 0.0f;
	}

	void mulRGB(float x) {
		this->b *= x;
		this->g *= x;
		this->r *= x;
	}

	TF_BGRA operator+(TF_BGRA& rhs) {
		TF_BGRA tmp;
		tmp.b = this->b + rhs.b;
		tmp.g = this->g + rhs.g;
		tmp.r = this->r + rhs.r;
		tmp.a = this->a + rhs.a;
		return tmp;
	}

	TF_BGRA operator-(TF_BGRA& rhs) {
		TF_BGRA tmp;
		tmp.b = this->b - rhs.b;
		tmp.g = this->g - rhs.g;
		tmp.r = this->r - rhs.r;
		tmp.a = this->a - rhs.a;
		return tmp;
	}

	TF_BGRA operator*(float mul) {
		TF_BGRA tmp;
		tmp.b = this->b * mul;
		tmp.g = this->g * mul;
		tmp.r = this->r * mul;
		tmp.a = this->a * mul;
		return tmp;
	}

	TF_BGRA operator/(float div) {
		TF_BGRA tmp;
		tmp.b = this->b / div;
		tmp.g = this->g / div;
		tmp.r = this->r / div;
		tmp.a = this->a / div;
		return tmp;
	}
}TF_BGRA;

typedef struct VolumePacket {
	float ImageRescaleIntercept;
	float ImageRescaleSlope;
	float PixelRepresentation;
	std::string PhotometricInterpretation;
	int ImageWindowWidth;
	int ImageWindowCenter;
	int ImageCols;
	int ImageRows;
	float PixelSpacingRow;
	float PixelSpacingCol;
	int BitsStored;
	int BitsAllocated;
	float SliceThickness;
	int MaxVal;
	int MinVal;
};

// packet for sending slice data.
typedef struct SlicePacket {
	// Slice line info
	std::tuple<float, float, float> slice_line;
	std::tuple<float, float> thickness;
	std::tuple<float, float, float> center_point;
	std::tuple<float, float, float> lefttop_point;
	std::tuple<float, float, float> normal_vector;
	std::tuple<float, float, float> width_direction_vetor;
	std::tuple<float, float, float> height_direction_vector;
	std::tuple<float, float> pixel_space;
	std::tuple<int, int, int> image_num;
	float m_axis_size;
	std::tuple<int, int> panning;
	std::tuple<int, int> image_size;
} SlicePacket;




// packet for sending slice data.
typedef struct CPRCurvePacket {
	// Slice line info
	std::tuple<float, float, float> center_point;
	std::tuple<float, float, float> lefttop_point;
	std::tuple<float, float, float> normal_vector;
	std::tuple<float, float, float> width_direction_vetor;
	std::tuple<float, float, float> height_direction_vector;
	std::tuple<float, float> pixel_space;
	float m_zoom_factor;
	std::tuple<int, int> panning;
} CPRCurvePacket;




// structure for saving info of label.
typedef struct MaskLabel {
	bool is_empty = true;
	bool is_visible = true;
	std::string name = "noname";
	std::string color;
} MaskLabel;

typedef struct DVRPacket {
	// VR info
	std::tuple<float, float, float> lefttop_point;
	std::tuple<float, float, float> normal_vector;
	std::tuple<float, float, float> width_direction_vetor;
	std::tuple<float, float, float> height_direction_vector;
	std::tuple<float, float, float> pixel_space;
	float m_axis_size;
	std::tuple<int, int> image_size;
	int* histogram;
} DVRPacket;

const int G_MAX_MASK_NUM = 8;

typedef struct PacketUpdateInfo {
	bool axial_image = false;;
	bool coronal_image = false;
	bool sagittal_image = false;
	bool cpr_image = false;
	bool cpr_curve_image = false;
	bool cpr_path_image = false;
	bool axial_json = false;
	bool coronal_json = false;
	bool sagittal_json = false;
	bool cpr_curve_json = false;
	bool dvr = false;
	bool mask_json = false;
	bool volume_json = false;
	bool error = false;
}PacketUpdateInfo;

// Sampling counts for MPR Slices
const int SAMPLING_CNT_WIDTH = 1024;
const int SAMPLING_CNT_HEIGHT = 1024;
