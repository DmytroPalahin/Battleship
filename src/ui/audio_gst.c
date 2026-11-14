/* SPDX-License-Identifier: MIT */
/* GStreamer implementation of audio.h: one "playbin" pipeline per sound. */
#include "audio.h"

#include "resources.h"

#include <gst/gst.h>
#include <stdbool.h>

/* One sound effect: its pipeline and what became of it. */
typedef struct sound_player {
    GstElement *playbin;
    guint bus_watch;
    bool broken; /* failed once: not attempted again */
} sound_player;

struct bs_audio {
    sound_player players[BS_SOUND_COUNT];
};

/* The sounds are read straight from the GResource bundle. */
static const char *const SOUND_URIS[BS_SOUND_COUNT] = {
    "resource://" BS_RESOURCE_PREFIX "/sounds/hit.wav",
    "resource://" BS_RESOURCE_PREFIX "/sounds/miss.wav",
};

/* Releases the device at the end of a sound; retires a sound that fails. */
static gboolean on_bus_message(GstBus *bus, GstMessage *message, gpointer user_data)
{
    sound_player *player = user_data;
    (void)bus;

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_EOS:
        gst_element_set_state(player->playbin, GST_STATE_READY);
        break;
    case GST_MESSAGE_ERROR: {
        GError *error = NULL;
        gst_message_parse_error(message, &error, NULL);
        g_warning("sound playback failed: %s", error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        gst_element_set_state(player->playbin, GST_STATE_NULL);
        player->broken = true;
        break;
    }
    default:
        break;
    }
    return G_SOURCE_CONTINUE;
}

bs_audio *bs_audio_new(void)
{
    GError *error = NULL;
    if (!gst_init_check(NULL, NULL, &error)) {
        g_warning("sound disabled: %s", error != NULL ? error->message : "GStreamer failed");
        g_clear_error(&error);
        return NULL;
    }

    const char *sink_name = g_getenv("BATTLESHIP_AUDIO_SINK");
    bs_audio *audio = g_new0(bs_audio, 1);

    for (int sound = 0; sound < BS_SOUND_COUNT; sound++) {
        sound_player *player = &audio->players[sound];
        player->playbin = gst_element_factory_make("playbin", NULL);
        if (player->playbin == NULL) {
            g_warning("sound disabled: the GStreamer 'playbin' element is missing");
            bs_audio_free(audio);
            return NULL;
        }
        g_object_set(player->playbin, "uri", SOUND_URIS[sound], NULL);

        if (sink_name != NULL && *sink_name != '\0') {
            GstElement *sink = gst_element_factory_make(sink_name, NULL);
            if (sink != NULL) {
                g_object_set(player->playbin, "audio-sink", sink, NULL);
            } else {
                g_warning("unknown audio sink '%s', using the default one", sink_name);
            }
        }

        GstBus *bus = gst_element_get_bus(player->playbin);
        player->bus_watch = gst_bus_add_watch(bus, on_bus_message, player);
        gst_object_unref(bus);
    }
    return audio;
}

void bs_audio_play(bs_audio *audio, bs_sound sound)
{
    if (audio == NULL || (int)sound < 0 || (int)sound >= BS_SOUND_COUNT) {
        return;
    }
    const sound_player *player = &audio->players[sound];
    if (player->broken) {
        return;
    }

    GstState state = GST_STATE_NULL;
    GstState pending = GST_STATE_VOID_PENDING;
    gst_element_get_state(player->playbin, &state, &pending, 0);

    if (state >= GST_STATE_PAUSED || pending >= GST_STATE_PAUSED) {
        /*
         * Already sounding, or about to: start over from the beginning. The
         * pipeline is never torn down while it streams, which would race with
         * its own start-up when two shots follow each other closely. A
         * pipeline that is still starting refuses the seek, and that is fine:
         * the sound is on its way.
         */
        (void)gst_element_seek_simple(player->playbin, GST_FORMAT_TIME,
                                      GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0);
    } else {
        gst_element_set_state(player->playbin, GST_STATE_PLAYING);
    }
}

unsigned bs_audio_error_count(const bs_audio *audio)
{
    unsigned count = 0;
    if (audio != NULL) {
        for (int sound = 0; sound < BS_SOUND_COUNT; sound++) {
            count += audio->players[sound].broken ? 1U : 0U;
        }
    }
    return count;
}

void bs_audio_free(bs_audio *audio)
{
    if (audio == NULL) {
        return;
    }
    for (int sound = 0; sound < BS_SOUND_COUNT; sound++) {
        sound_player *player = &audio->players[sound];
        if (player->bus_watch != 0) {
            g_source_remove(player->bus_watch);
        }
        if (player->playbin != NULL) {
            gst_element_set_state(player->playbin, GST_STATE_NULL);
            gst_object_unref(player->playbin);
        }
    }
    g_free(audio);
}
