#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(LFUSION3D_LIB)
#  define LFUSION3D_EXPORT Q_DECL_EXPORT
# else
#  define LFUSION3D_EXPORT Q_DECL_IMPORT
# endif
#else
# define LFUSION3D_EXPORT
#endif
