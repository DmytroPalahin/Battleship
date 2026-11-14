/* SPDX-License-Identifier: MIT */
/*
 * GStreamer implementation of audio.h.
 *
 * Every occurrence of a sound gets a pipeline of its own (a "playbin"), which
 * is started once and then left alone until it reports that it has finished or
 * failed. No command ever reaches a pipeline that is streaming: restarting one
 * in place, by seeking it or by stopping and starting it again, races with its
 * own threads when two shots follow each other within milliseconds.
 */
#include "audio.h"

#include "resources.h"

#include <gst/gst.h>
#include <stdbool.h>

/* Occurrences that may sound at once; a burst beyond that drops the newest. */
#define MAX_VOICES 4

/* One occurrence of a sound, from its first sample to its last. */
typedef struct voice {
    bs_audio *audio;     /* the device it belongs to */
    GstElement *playbin; /* NULL while the slot is free */
    guint bus_watch;
    bs_sound sound;
} voice;

struct bs_audio {
    voice voices[MAX_VOICES];
    char *sink_name;             /* owned; NULL for the system default */
    bool broken[BS_SOUND_COUNT]; /* failed once: not attempted again */
};

/* The sounds are read straight from the GResource bundle. */
static const char *const SOUND_URIS[BS_SOUND_COUNT] = {
    "resource://" BS_RESOURCE_PREFIX "/sounds/hit.wav",
    "resource://" BS_RESOURCE_PREFIX "/sounds/miss.wav",
};

/* Reads BATTLESHIP_AUDIO_SINK; the result is owned by the caller. */
static char *chosen_sink(void)
{
    const char *name = g_getenv("BATTLESHIP_AUDIO_SINK");
    if (name == NULL || *name == '\0') {
        return NULL;
    }

    GstElementFactory *factory = gst_element_factory_find(name);
    if (factory == NULL) {
        g_warning("unknown audio sink '%s', using the default one", name);
        return NULL;
    }
    gst_object_unref(factory);
    return g_strdup(name);
}

/* Builds the pipeline of one occurrence, or returns NULL. */
static GstElement *new_pipeline(const bs_audio *audio, bs_sound sound)
{
    GstElement *playbin = gst_element_factory_make("playbin", NULL);
    if (playbin == NULL) {
        return NULL;
    }
    g_object_set(playbin, "uri", SOUND_URIS[sound], NULL);

    if (audio->sink_name != NULL) {
        GstElement *sink = gst_element_factory_make(audio->sink_name, NULL);
        if (sink != NULL) {
            g_object_set(playbin, "audio-sink", sink, NULL);
        }
    }
    return playbin;
}

/* Dismantles a voice that has finished, failed or outlived the program. */
static void voice_release(voice *slot)
{
    gst_element_set_state(slot->playbin, GST_STATE_NULL);
    gst_object_unref(slot->playbin);
    slot->playbin = NULL;
    slot->bus_watch = 0;
}

/* Names the element that failed and what it reported. */
static void warn_about(GstMessage *message)
{
    GError *error = NULL;
    gchar *details = NULL;
    GstObject *source = GST_MESSAGE_SRC(message);

    gst_message_parse_error(message, &error, &details);
    g_warning(
        "sound playback failed in '%s': %s (%s)", source != NULL ? GST_OBJECT_NAME(source) : "?",
        error != NULL ? error->message : "unknown error", details != NULL ? details : "no details");
    g_clear_error(&error);
    g_free(details);
}

/* The end of a voice: its last sample, or an error that retires its sound. */
static gboolean on_voice_message(GstBus *bus, GstMessage *message, gpointer user_data)
{
    voice *slot = user_data;
    const GstMessageType type = GST_MESSAGE_TYPE(message);
    (void)bus;

    if (type != GST_MESSAGE_EOS && type != GST_MESSAGE_ERROR) {
        return G_SOURCE_CONTINUE;
    }
    if (type == GST_MESSAGE_ERROR) {
        warn_about(message);
        slot->audio->broken[slot->sound] = true;
    }
    voice_release(slot);
    return G_SOURCE_REMOVE; /* the watch goes with its pipeline */
}

bs_audio *bs_audio_new(void)
{
    GError *error = NULL;
    if (!gst_init_check(NULL, NULL, &error)) {
        g_warning("sound disabled: %s", error != NULL ? error->message : "GStreamer failed");
        g_clear_error(&error);
        return NULL;
    }

    GstElementFactory *playbin = gst_element_factory_find("playbin");
    if (playbin == NULL) {
        g_warning("sound disabled: the GStreamer 'playbin' element is missing");
        return NULL;
    }
    gst_object_unref(playbin);

    bs_audio *audio = g_new0(bs_audio, 1);
    audio->sink_name = chosen_sink();
    return audio;
}

void bs_audio_play(bs_audio *audio, bs_sound sound)
{
    if (audio == NULL || (int)sound < 0 || (int)sound >= BS_SOUND_COUNT || audio->broken[sound]) {
        return;
    }

    voice *slot = NULL;
    for (int index = 0; index < MAX_VOICES && slot == NULL; index++) {
        if (audio->voices[index].playbin == NULL) {
            slot = &audio->voices[index];
        }
    }
    if (slot == NULL) {
        return; /* enough is sounding already */
    }

    slot->playbin = new_pipeline(audio, sound);
    if (slot->playbin == NULL) {
        g_warning("sound playback failed: no pipeline could be built");
        audio->broken[sound] = true;
        return;
    }
    slot->audio = audio;
    slot->sound = sound;

    GstBus *bus = gst_element_get_bus(slot->playbin);
    slot->bus_watch = gst_bus_add_watch(bus, on_voice_message, slot);
    gst_object_unref(bus);

    if (gst_element_set_state(slot->playbin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        g_warning("sound playback failed: the pipeline refused to start");
        audio->broken[sound] = true;
        g_source_remove(slot->bus_watch);
        voice_release(slot);
    }
}

unsigned bs_audio_error_count(const bs_audio *audio)
{
    unsigned count = 0;
    if (audio != NULL) {
        for (int sound = 0; sound < BS_SOUND_COUNT; sound++) {
            count += audio->broken[sound] ? 1U : 0U;
        }
    }
    return count;
}

void bs_audio_free(bs_audio *audio)
{
    if (audio == NULL) {
        return;
    }
    for (int index = 0; index < MAX_VOICES; index++) {
        voice *slot = &audio->voices[index];
        if (slot->playbin != NULL) {
            g_source_remove(slot->bus_watch);
            voice_release(slot);
        }
    }
    g_free(audio->sink_name);
    g_free(audio);
}
