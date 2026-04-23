#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WITH_ASSERT_THREAD
#define ASSERT_THREAD(type)                                                                     \
	do {                                                                                    \
		if (!obs_in_task_thread(type))                                                  \
			blog(LOG_ERROR, "%s: ASSERT_THREAD failed: Expected " #type, __func__); \
	} while (false)
#else
#define ASSERT_THREAD(type)
#endif

gs_effect_t *create_effect_from_module_file(const char *basename);

static inline bool valid_track(int track)
{
	return 0 <= track && track < MAX_AUDIO_MIXES;
}

#ifdef __cplusplus
}
#endif
