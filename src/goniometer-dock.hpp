#pragma once
#include <QFrame>
#include <obs.hpp>

class GoniometerDock : public QFrame {
	Q_OBJECT

public:
	GoniometerDock(QWidget *parent = nullptr);
	~GoniometerDock();

private:
	class NorisQTDisplay *display = nullptr;
	OBSSourceAutoRelease goniometer_src;

private:
	void RegisterCallbackToDisplay();
	static void draw_cb(void *param, uint32_t cx, uint32_t cy);
	void draw_cb(uint32_t cx, uint32_t cy);
};
