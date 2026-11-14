/* SPDX-License-Identifier: MIT */
/**
 * @file audio.h
 * @brief Sound effects, as an optional capability.
 *
 * Two implementations exist and CMake links exactly one of them:
 *  - audio_gst.c plays the bundled sounds through GStreamer;
 *  - audio_null.c does nothing, for builds without GStreamer.
 *
 * Callers never need to know which one they got: bs_audio_new() may return
 * NULL, and every function accepts NULL.
 *
 * The environment variable BATTLESHIP_AUDIO_SINK names the GStreamer element
 * to send the sound to instead of the system default (for instance
 * "pulsesink", or "fakesink" on a machine without a sound card).
 */
#ifndef BATTLESHIP_UI_AUDIO_H
#define BATTLESHIP_UI_AUDIO_H

/** The sound effects of the game. */
typedef enum bs_sound {
    BS_SOUND_HIT = 0,
    BS_SOUND_MISS
} bs_sound;

/** Number of sound effects. */
#define BS_SOUND_COUNT 2

/** A sound device. Opaque. */
typedef struct bs_audio bs_audio;

/** Opens the sound output, or returns NULL when sound is unavailable. */
bs_audio *bs_audio_new(void);

/** Plays a sound effect, cutting short a previous occurrence of the same one. */
void bs_audio_play(bs_audio *audio, bs_sound sound);

/**
 * Number of sounds that could not be played since bs_audio_new(). A sound
 * that fails once is not attempted again.
 */
unsigned bs_audio_error_count(const bs_audio *audio);

/** Closes the sound output. */
void bs_audio_free(bs_audio *audio);

#endif /* BATTLESHIP_UI_AUDIO_H */
