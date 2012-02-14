#ifndef UNO_GLOBAL_H
#define UNO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UNO_LIBRARY)
#  define UNOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define UNOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // UNO_GLOBAL_H
