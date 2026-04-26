#pragma once
#include <QFrame>
#include <obs.hpp>
#include <obs-frontend-api.h>

class GoniometerDock : public QFrame {
	Q_OBJECT

public:
	GoniometerDock(QWidget *parent = nullptr);
	~GoniometerDock();

private:
	class NorisQTDisplay *display = nullptr;
	OBSSourceAutoRelease goniometer_src;
	OBSDataAutoRelease goniometer_src_data;
	bool frontend_exited = false;

private:
	void RegisterCallbackToDisplay();
	static void draw_cb(void *param, uint32_t cx, uint32_t cy);
	void draw_cb(uint32_t cx, uint32_t cy);

	static void on_frontend_event(enum obs_frontend_event event, void *data);
	void on_frontend_event_exit();

	static void scenes_save_cb(obs_data_t *save_data, bool saving, void *private_data);
	void scenes_load_cb(obs_data_t *data);
	void scenes_save_cb(obs_data_t *data);

	void contextMenuEvent(class QContextMenuEvent *event);
	void OnPropsClicked();
};
