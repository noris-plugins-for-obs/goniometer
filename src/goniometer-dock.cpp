#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QMainWindow>
#include <QGridLayout>
#include <NorisQTDisplay.hpp>

#include "plugin-macros.generated.h"
#include "goniometer.h"
#include "util.h"
#include "goniometer-dock.hpp"

GoniometerDock::GoniometerDock(QWidget *parent) : QFrame(parent)
{
	ASSERT_THREAD(OBS_TASK_UI);

	auto *layout = new QGridLayout(this);

	display = new NorisQTDisplay(this);
	connect(display, &NorisQTDisplay::DisplayCreated, this, &GoniometerDock::RegisterCallbackToDisplay);
	layout->addWidget(display, 0, 0);
}

GoniometerDock::~GoniometerDock()
{
	disconnect(display, &NorisQTDisplay::DisplayCreated, this, &GoniometerDock::RegisterCallbackToDisplay);
	obs_display_remove_draw_callback(display->GetDisplay(), GoniometerDock::draw_cb, this);
	delete display;
	display = nullptr;
}

void GoniometerDock::RegisterCallbackToDisplay()
{
	if (!goniometer_src) {
		OBSDataAutoRelease settings = obs_data_create();
		obs_data_set_int(settings, "track", 1);
		goniometer_src = obs_source_create_private(ID_PREFIX "source", "goniometer", settings);
	}

	obs_display_add_draw_callback(display->GetDisplay(), GoniometerDock::draw_cb, this);
}

void GoniometerDock::draw_cb(void *param, uint32_t cx, uint32_t cy)
{
	auto *dock = static_cast<GoniometerDock *>(param);
	dock->draw_cb(cx, cy);
}

void GoniometerDock::draw_cb(uint32_t cx, uint32_t cy)
{
	gs_blend_state_push();
	gs_reset_blend_state();

	if (goniometer_src) {
		int w_src = obs_source_get_width(goniometer_src);
		int h_src = obs_source_get_height(goniometer_src);

		int w = cx;
		int h = cy;

		if (w * h_src > h * w_src)
			w = h * w_src / h_src;
		else
			h = w * h_src / w_src;

		gs_projection_push();
		gs_viewport_push();
		gs_set_viewport((cx - w) / 2, (cy - h) / 2, w, h);
		gs_ortho(0.0f, w_src, -1.0f, h_src, -100.0f, 100.0f);

		obs_source_video_render(goniometer_src);

		gs_viewport_pop();
		gs_projection_pop();
	}

	gs_blend_state_pop();
}

extern "C" QWidget *create_goniometer_dock()
{
	ASSERT_THREAD(OBS_TASK_UI);

	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	return static_cast<QWidget *>(new GoniometerDock(main_window));
}
