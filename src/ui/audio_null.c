/* SPDX-License-Identifier: MIT */
/* Silent implementation of audio.h, used when GStreamer is not available. */
#include "audio.h"

#include <stddef.h>

bs_audio *bs_audio_new(void)
{
    return NULL;
}

void bs_audio_play(bs_audio *audio, bs_sound sound)
{
    (void)audio;
    (void)sound;
}

unsigned bs_audio_error_count(const bs_audio *audio)
{
    (void)audio;
    return 0;
}

void bs_audio_free(bs_audio *audio)
{
    (void)audio;
}
