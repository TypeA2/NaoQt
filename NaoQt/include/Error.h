#pragma once

#ifdef QT_DEBUG
#include <QDebug>
#endif

#define __FNAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

