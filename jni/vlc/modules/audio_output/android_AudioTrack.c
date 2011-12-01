
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>

#include <dlfcn.h>

// _ZN7android11AudioSystem19getOutputFrameCountEPii
typedef int (*AudioSystem_getOutputFrameCount)(int *, int);
// _ZN7android11AudioSystem16getOutputLatencyEPji
typedef int (*AudioSystem_getOutputLatency)(unsigned int *, int);
// _ZN7android11AudioSystem21getOutputSamplingRateEPii
typedef int (*AudioSystem_getOutputSamplingRate)(int *, int);

// _ZN7android10AudioTrack16getMinFrameCountEPiij
typedef int (*AudioTrack_getMinFrameCount)(int *, int, unsigned int);

// _ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_ii
typedef void (*AudioTrack_ctor)(void *, int, unsigned int, int, int, int, unsigned int, void (*)(int, void *, void *), void *, int, int);
// _ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_i
typedef void (*AudioTrack_ctor_legacy)(void *, int, unsigned int, int, int, int, unsigned int, void (*)(int, void *, void *), void *, int);
// _ZN7android10AudioTrackD1Ev
typedef void (*AudioTrack_dtor)(void *);
// _ZNK7android10AudioTrack9initCheckEv
typedef int (*AudioTrack_initCheck)(void *);
// _ZN7android10AudioTrack5startEv
typedef int (*AudioTrack_start)(void *);
// _ZN7android10AudioTrack4stopEv
typedef int (*AudioTrack_stop)(void *);
// _ZN7android10AudioTrack5writeEPKvj
typedef int (*AudioTrack_write)(void *, void  const*, unsigned int);
// _ZN7android10AudioTrack5flushEv
typedef int (*AudioTrack_flush)(void *);

struct aout_sys_t {
    int type;
    uint32_t rate;
    int channel;
    int format;
    int size;
    void *libmedia;
    void *AudioTrack;
};

static AudioSystem_getOutputFrameCount as_getOutputFrameCount = NULL;
static AudioSystem_getOutputLatency as_getOutputLatency = NULL;
static AudioSystem_getOutputSamplingRate as_getOutputSamplingRate = NULL;
static AudioTrack_getMinFrameCount at_getMinFrameCount = NULL;
static AudioTrack_ctor at_ctor = NULL;
static AudioTrack_ctor_legacy at_ctor_legacy = NULL;
static AudioTrack_dtor at_dtor = NULL;
static AudioTrack_initCheck at_initCheck = NULL;
static AudioTrack_start at_start = NULL;
static AudioTrack_stop at_stop = NULL;
static AudioTrack_write at_write = NULL;
static AudioTrack_flush at_flush = NULL;

static void *InitLibrary();

static int  Open(vlc_object_t *);
static void Close(vlc_object_t *);
static void Play(aout_instance_t *);

vlc_module_begin ()
    set_shortname("AndroidAudioTrack")
    set_description(N_("Android AudioTrack audio output"))
    set_capability("audio output", 25)
    set_category(CAT_AUDIO)
    set_subcategory(SUBCAT_AUDIO_AOUT)
    add_shortcut("android")
    set_callbacks(Open, Close)
vlc_module_end ()

void *InitLibrary() {
    void *p_library;

    p_library = dlopen("libmedia.so", RTLD_NOW);
    if (!p_library)
        return NULL;
    as_getOutputFrameCount = (AudioSystem_getOutputFrameCount)(dlsym(p_library, "_ZN7android11AudioSystem19getOutputFrameCountEPii"));
    as_getOutputLatency = (AudioSystem_getOutputLatency)(dlsym(p_library, "_ZN7android11AudioSystem16getOutputLatencyEPji"));
    as_getOutputSamplingRate = (AudioSystem_getOutputSamplingRate)(dlsym(p_library, "_ZN7android11AudioSystem21getOutputSamplingRateEPii"));
    at_getMinFrameCount = (AudioTrack_getMinFrameCount)(dlsym(p_library, "_ZN7android10AudioTrack16getMinFrameCountEPiij"));
    at_ctor = (AudioTrack_ctor)(dlsym(p_library, "_ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_ii"));
    at_ctor_legacy = (AudioTrack_ctor_legacy)(dlsym(p_library, "_ZN7android10AudioTrackC1EijiiijPFviPvS1_ES1_i"));
    at_dtor = (AudioTrack_dtor)(dlsym(p_library, "_ZN7android10AudioTrackD1Ev"));
    at_initCheck = (AudioTrack_initCheck)(dlsym(p_library, "_ZNK7android10AudioTrack9initCheckEv"));
    at_start = (AudioTrack_start)(dlsym(p_library, "_ZN7android10AudioTrack5startEv"));
    at_stop = (AudioTrack_stop)(dlsym(p_library, "_ZN7android10AudioTrack4stopEv"));
    at_write = (AudioTrack_write)(dlsym(p_library, "_ZN7android10AudioTrack5writeEPKvj"));
    at_flush = (AudioTrack_flush)(dlsym(p_library, "_ZN7android10AudioTrack5flushEv"));
    // need the first 3 or the last 1
    if (!((as_getOutputFrameCount && as_getOutputLatency && as_getOutputSamplingRate) || at_getMinFrameCount)) {
        dlclose(p_library);
        return NULL;
    }
    // need all in the list
    if (!((at_ctor || at_ctor_legacy) && at_dtor && at_initCheck && at_start && at_stop && at_write && at_flush)) {
        dlclose(p_library);
        return NULL;
    }
    return p_library;
}

static int Open(vlc_object_t *p_this) {
    struct aout_sys_t *p_sys;
    void *p_library;
    aout_instance_t *p_aout = (aout_instance_t*)(p_this);
    int status;
    int afSampleRate, afFrameCount, afLatency, minBufCount, minFrameCount;
    int type, channel, rate, format, size;

    p_library = InitLibrary();
    if (!p_library) {
        msg_Err(VLC_OBJECT(p_this), "Could not initialize libmedia.so!");
        return VLC_EGENERIC;
    }
    p_sys = (struct aout_sys_t*)malloc(sizeof(aout_sys_t));
    if (p_sys == NULL)
        return VLC_ENOMEM;
    p_sys->libmedia = p_library;
    // AudioSystem::MUSIC = 3
    type = 3;
    p_sys->type = type;
    // 4000 <= frequency <= 48000
    if (p_aout->output.output.i_rate < 4000)
        p_aout->output.output.i_rate = 4000;
    if (p_aout->output.output.i_rate > 48000)
        p_aout->output.output.i_rate = 48000;
    rate = p_aout->output.output.i_rate;
    p_sys->rate = rate;
    // U8/S16 only
    if (p_aout->output.output.i_format != VLC_CODEC_U8 && p_aout->output.output.i_format != VLC_CODEC_S16L)
        p_aout->output.output.i_format = VLC_CODEC_S16L;
    // AudioSystem::PCM_16_BIT = 1
    // AudioSystem::PCM_8_BIT = 2
    format = (p_aout->output.output.i_format == VLC_CODEC_S16L) ? 1 : 2;
    p_sys->format = format;
    // TODO: android supports more channels
    channel = aout_FormatNbChannels(&p_aout->output.output);
    if (channel > 2) {
        channel = 2;
        p_aout->output.output.i_physical_channels = AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT;
    }
    // AudioSystem::CHANNEL_OUT_STEREO = 12
    // AudioSystem::CHANNEL_OUT_MONO = 4
    channel = (channel == 2) ? 12 : 4;
    p_sys->channel = channel;
    // use the minium value
    if (!at_getMinFrameCount) {
        status = as_getOutputSamplingRate(&afSampleRate, type);
        status ^= as_getOutputFrameCount(&afFrameCount, type);
        status ^= as_getOutputLatency((uint32_t*)(&afLatency), type);
        if (status != 0) {
            free(p_sys);
            return VLC_EGENERIC;
        }
        minBufCount = afLatency / ((1000 * afFrameCount) / afSampleRate);
        if (minBufCount < 2)
            minBufCount = 2;
        minFrameCount = (afFrameCount * rate * minBufCount) / afSampleRate;
        p_aout->output.i_nb_samples = minFrameCount;
    }
    else {
        status = at_getMinFrameCount(&p_aout->output.i_nb_samples, type, rate);
        if (status != 0) {
            free(p_sys);
            return VLC_EGENERIC;
        }
    }
    p_aout->output.i_nb_samples <<= 1;
    p_sys->size = p_aout->output.i_nb_samples;
    // sizeof(AudioTrack) == 0x58 (not sure) on 2.2.1, this should be enough
    p_sys->AudioTrack = malloc(256);
    if (!p_sys->AudioTrack) {
        free(p_sys);
        return VLC_ENOMEM;
    }
    // higher than android 2.2
    if (at_ctor)
        at_ctor(p_sys->AudioTrack, p_sys->type, p_sys->rate, p_sys->format, p_sys->channel, p_sys->size, 0, NULL, NULL, 0, 0);
    // higher than android 1.6
    else if (at_ctor_legacy)
        at_ctor_legacy(p_sys->AudioTrack, p_sys->type, p_sys->rate, p_sys->format, p_sys->channel, p_sys->size, 0, NULL, NULL, 0);
    status = at_initCheck(p_sys->AudioTrack);
    // android 1.6
    if (status != 0) {
        p_sys->channel = (p_sys->channel == 12) ? 2 : 1;
        at_ctor_legacy(p_sys->AudioTrack, p_sys->type, p_sys->rate, p_sys->format, p_sys->channel, p_sys->size, 0, NULL, NULL, 0);
        status = at_initCheck(p_sys->AudioTrack);
    }
    if (status != 0) {
        msg_Err(p_aout, "Cannot create AudioTrack!");
        free(p_sys->AudioTrack);
        free(p_sys);
        return VLC_EGENERIC;
    }

    p_aout->output.p_sys = p_sys;
    p_aout->output.pf_play = Play;

    at_start(p_sys->AudioTrack);

    return VLC_SUCCESS;
}

static void Close(vlc_object_t *p_this) {
    aout_instance_t *p_aout = (aout_instance_t*)p_this;
    struct aout_sys_t *p_sys = p_aout->output.p_sys;

    at_stop(p_sys->AudioTrack);
    at_flush(p_sys->AudioTrack);
    at_dtor(p_sys->AudioTrack);
    free(p_sys->AudioTrack);
    free(p_sys);
}

static void Play(aout_instance_t *p_aout) {
    struct aout_sys_t *p_sys = p_aout->output.p_sys;
    int length;
    aout_buffer_t *p_buffer;

    while ((p_buffer = aout_FifoPop(&p_aout->output.fifo)) != NULL) {
        length = 0;
        while (length < p_buffer->i_buffer) {
            length += at_write(p_sys->AudioTrack, (char*)(p_buffer->p_buffer) + length, p_buffer->i_buffer - length);
        }
        aout_BufferFree(p_buffer);
    }
}

