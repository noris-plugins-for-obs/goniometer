/*
Goniometer Plugin for OBS Studio
Copyright (C) 2026 Norihiro Kamae

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <util/threading.h>

#include "plugin-macros.generated.h"
#include "util.h"

#define TEX_SIZE 256

struct goniometer_source
{
	uint8_t *buf;
	pthread_mutex_t buf_mutex;

	gs_effect_t *effect;
	gs_texture_t *tex;

	// properties
	int track;
};

static void goniometer_update(void *data, obs_data_t *settings);
void audio_cb(void *param, size_t mix_idx, struct audio_data *data);

static const char *goniometer_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("Goniometer.Name");
}

static void *goniometer_create(obs_data_t *settings, obs_source_t *source)
{
	struct goniometer_source *src = bzalloc(sizeof(struct goniometer_source));

	src->buf = bzalloc(TEX_SIZE * TEX_SIZE);
	src->track = -1;
	pthread_mutex_init(&src->buf_mutex, NULL);

	obs_enter_graphics();
	src->effect = create_effect_from_module_file("goniometer.effect");
	obs_leave_graphics();

	goniometer_update(src, settings);

	return src;
}

static void goniometer_destroy(void *data)
{
	struct goniometer_source *src = data;

	if (valid_track(src->track))
		obs_remove_raw_audio_callback(src->track, audio_cb, src);

	obs_enter_graphics();
	gs_texture_destroy(src->tex);
	obs_leave_graphics();

	pthread_mutex_destroy(&src->buf_mutex);
	bfree(src->buf);
	bfree(src);
}

static void goniometer_update(void *data, obs_data_t *settings)
{
	struct goniometer_source *src = data;

	int track = (int)obs_data_get_int(settings, "track") - 1;
	if (src->track != track && valid_track(track)) {
		if (valid_track(src->track))
			obs_remove_raw_audio_callback(src->track, audio_cb, src);
		obs_add_raw_audio_callback(track, NULL, audio_cb, src);
		src->track = track;
	}
}

static obs_properties_t *goniometer_get_properties(void *data)
{
	struct goniometer_source *src = data;
	UNUSED_PARAMETER(src);

	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int(props, "track", obs_module_text("Props.Track"), 1, MAX_AUDIO_MIXES, 1);

	return props;
}

static void goniometer_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "track", 1);
}

static inline int float_to_int(float x)
{
	if (-1.0 <= x && x <= 1.0f)
		return (int)((x + 1.0f) * (TEX_SIZE - 1) / 2);
	else if (x >= 1.0f)
		return TEX_SIZE - 1;
	else if (x <= -1.0f)
		return 0;
	return (TEX_SIZE - 1) / 2;
}

void audio_cb(void *param, size_t mix_idx, struct audio_data *data)
{
	struct goniometer_source *src = param;

	const float **data_in = (const float **)data->data;
	if (!data_in || !data_in[0] || !data_in[1])
		return;

	pthread_mutex_lock(&src->buf_mutex);
	for (uint32_t iframe = 0; iframe < data->frames; iframe++) {
		float l_flt = data_in[0][iframe];
		float r_flt = data_in[1][iframe];
		int x_int = float_to_int((r_flt - l_flt) * 0.5f);
		int y_int = float_to_int((l_flt + r_flt) * 0.5f);
		src->buf[x_int + y_int * TEX_SIZE] = 255;
	}
	pthread_mutex_unlock(&src->buf_mutex);
}

static uint32_t goniometer_get_width(void *data)
{
	UNUSED_PARAMETER(data);
	return 256;
}

static uint32_t goniometer_get_height(void *data)
{
	UNUSED_PARAMETER(data);
	return 256;
}

static void set_image(struct goniometer_source *src, const uint8_t *buf)
{
	if (!src->tex)
		src->tex = gs_texture_create(TEX_SIZE, TEX_SIZE, GS_R8, 1, &buf, GS_DYNAMIC);
	else
		gs_texture_set_image(src->tex, buf, TEX_SIZE, false);

	if (!src->tex)
		blog(LOG_ERROR, "Failed to create texture buf=%p", buf);
}

static void goniometer_tick(void *data, float second)
{
	struct goniometer_source *src = data;
	UNUSED_PARAMETER(second);

	pthread_mutex_lock(&src->buf_mutex);
	obs_enter_graphics();
	set_image(src, src->buf);
	memset(src->buf, 0, TEX_SIZE * TEX_SIZE);
	obs_leave_graphics();
	pthread_mutex_unlock(&src->buf_mutex);
}

static void goniometer_render(void *data, gs_effect_t *unused_effect)
{
	struct goniometer_source *src = data;
	UNUSED_PARAMETER(unused_effect);

	if (!src->effect) {
		blog(LOG_WARNING, "Effect was not loaded");
		return;
	}
	if (!src->tex) {
		blog(LOG_WARNING, "Texture was not created");
		return;
	}
	if (!src->effect || !src->tex)
		return;

	// TODO: Use decay

	gs_effect_set_texture(gs_effect_get_param_by_name(src->effect, "image"), src->tex);
	while (gs_effect_loop(src->effect, "Draw")) {
		gs_draw_sprite(src->tex, 0, TEX_SIZE, TEX_SIZE);
	}
}

const struct obs_source_info goniometer_source_info = {
	.id = ID_PREFIX "source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
	.get_name = goniometer_get_name,
	.create = goniometer_create,
	.destroy = goniometer_destroy,
	.update = goniometer_update,
	.get_properties = goniometer_get_properties,
	.get_defaults = goniometer_get_defaults,
	.get_width = goniometer_get_width,
	.get_height = goniometer_get_height,
	.video_tick = goniometer_tick,
	.video_render = goniometer_render,
};
