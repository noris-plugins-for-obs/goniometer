#pragma once

#ifdef __cplusplus
extern "C" {
#endif

gs_effect_t *create_effect_from_module_file(const char *basename);

static inline bool valid_track(int track)
{
	return 0 <= track && track < MAX_AUDIO_MIXES;
}

#ifdef __cplusplus
}
#endif
