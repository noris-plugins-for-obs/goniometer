#pragma once

#ifdef __cplusplus
extern "C" {
class QWidget;
#else
typedef void QWidget;
#endif

QWidget *create_goniometer_dock();

#ifdef __cplusplus
}
#endif
