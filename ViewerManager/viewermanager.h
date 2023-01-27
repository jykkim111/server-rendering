#pragma once

//#include <QtCore/qglobal.h>
//
//#ifndef BUILD_STATIC
//# if defined(VIEWERMANAGER_LIB)
//#  define VIEWERMANAGER_EXPORT Q_DECL_EXPORT
//# else
//#  define VIEWERMANAGER_EXPORT Q_DECL_IMPORT
//# endif
//#else
//# define VIEWERMANAGER_EXPORT
//#endif


#ifdef VIEWERMANAGER_EXPORTS
#define VIEWERMANAGER_DLL __declspec(dllexport)
#else
#define VIEWERMANAGER_DLL __declspec(dllimport)
#endif
