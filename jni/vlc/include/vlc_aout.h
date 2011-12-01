/*****************************************************************************
 * vlc_aout.h : audio output interface
 *****************************************************************************
 * Copyright (C) 2002-2011 the VideoLAN team
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef VLC_AOUT_H
#define VLC_AOUT_H 1

/**
 * \file
 * This file defines functions, structures and macros for audio output object
 */

/* Max number of pre-filters per input, and max number of post-filters */
#define AOUT_MAX_FILTERS                10

/* Buffers which arrive in advance of more than AOUT_MAX_ADVANCE_TIME
 * will be considered as bogus and be trashed */
#define AOUT_MAX_ADVANCE_TIME           (AOUT_MAX_PREPARE_TIME + CLOCK_FREQ)

/* Buffers which arrive in advance of more than AOUT_MAX_PREPARE_TIME
 * will cause the calling thread to sleep */
#define AOUT_MAX_PREPARE_TIME           (2 * CLOCK_FREQ)

/* Buffers which arrive after pts - AOUT_MIN_PREPARE_TIME will be trashed
 * to avoid too heavy resampling */
#define AOUT_MIN_PREPARE_TIME           AOUT_MAX_PTS_ADVANCE

/* Tolerance values from EBU Recommendation 37 */
/** Maximum advance of actual audio playback time to coded PTS,
 * above which downsampling will be performed */
#define AOUT_MAX_PTS_ADVANCE            (CLOCK_FREQ / 25)

/** Maximum delay of actual audio playback time from coded PTS,
 * above which upsampling will be performed */
#define AOUT_MAX_PTS_DELAY              (3 * CLOCK_FREQ / 50)

/* Max acceptable resampling (in %) */
#define AOUT_MAX_RESAMPLING             10

#include "vlc_es.h"

#define AOUT_FMTS_IDENTICAL( p_first, p_second ) (                          \
    ((p_first)->i_format == (p_second)->i_format)                           \
      && AOUT_FMTS_SIMILAR(p_first, p_second) )

/* Check if i_rate == i_rate and i_channels == i_channels */
#define AOUT_FMTS_SIMILAR( p_first, p_second ) (                            \
    ((p_first)->i_rate == (p_second)->i_rate)                               \
      && ((p_first)->i_physical_channels == (p_second)->i_physical_channels)\
      && ((p_first)->i_original_channels == (p_second)->i_original_channels) )

#define VLC_CODEC_SPDIFL VLC_FOURCC('s','p','d','i')
#define VLC_CODEC_SPDIFB VLC_FOURCC('s','p','d','b')

#define AOUT_FMT_NON_LINEAR( p_format )                 \
    ( ((p_format)->i_format == VLC_CODEC_SPDIFL)       \
       || ((p_format)->i_format == VLC_CODEC_SPDIFB)   \
       || ((p_format)->i_format == VLC_CODEC_A52)       \
       || ((p_format)->i_format == VLC_CODEC_DTS) )

/* This is heavily borrowed from libmad, by Robert Leslie <rob@mars.org> */
/*
 * Fixed-point format: 0xABBBBBBB
 * A == whole part      (sign + 3 bits)
 * B == fractional part (28 bits)
 *
 * Values are signed two's complement, so the effective range is:
 * 0x80000000 to 0x7fffffff
 *       -8.0 to +7.9999999962747097015380859375
 *
 * The smallest representable value is:
 * 0x00000001 == 0.0000000037252902984619140625 (i.e. about 3.725e-9)
 *
 * 28 bits of fractional accuracy represent about
 * 8.6 digits of decimal accuracy.
 *
 * Fixed-point numbers can be added or subtracted as normal
 * integers, but multiplication requires shifting the 64-bit result
 * from 56 fractional bits back to 28 (and rounding.)
 */
typedef int32_t vlc_fixed_t;
#define FIXED32_FRACBITS 28
#define FIXED32_MIN ((vlc_fixed_t) -0x80000000L)
#define FIXED32_MAX ((vlc_fixed_t) +0x7fffffffL)
#define FIXED32_ONE ((vlc_fixed_t) 0x10000000)

/*
 * Channels descriptions
 */

/* Values available for physical and original channels */
#define AOUT_CHAN_CENTER            0x1
#define AOUT_CHAN_LEFT              0x2
#define AOUT_CHAN_RIGHT             0x4
#define AOUT_CHAN_REARCENTER        0x10
#define AOUT_CHAN_REARLEFT          0x20
#define AOUT_CHAN_REARRIGHT         0x40
#define AOUT_CHAN_MIDDLELEFT        0x100
#define AOUT_CHAN_MIDDLERIGHT       0x200
#define AOUT_CHAN_LFE               0x1000

/* Values available for original channels only */
#define AOUT_CHAN_DOLBYSTEREO       0x10000
#define AOUT_CHAN_DUALMONO          0x20000
#define AOUT_CHAN_REVERSESTEREO     0x40000

#define AOUT_CHAN_PHYSMASK          0xFFFF
#define AOUT_CHAN_MAX               9

/* Values used for the audio-device and audio-channels object variables */
#define AOUT_VAR_MONO               1
#define AOUT_VAR_STEREO             2
#define AOUT_VAR_2F2R               4
#define AOUT_VAR_3F2R               5
#define AOUT_VAR_5_1                6
#define AOUT_VAR_6_1                7
#define AOUT_VAR_7_1                8
#define AOUT_VAR_SPDIF              10

#define AOUT_VAR_CHAN_STEREO        1
#define AOUT_VAR_CHAN_RSTEREO       2
#define AOUT_VAR_CHAN_LEFT          3
#define AOUT_VAR_CHAN_RIGHT         4
#define AOUT_VAR_CHAN_DOLBYS        5

/*****************************************************************************
 * Main audio output structures
 *****************************************************************************/

#define aout_BufferFree( buffer ) block_Release( buffer )

/* Size of a frame for S/PDIF output. */
#define AOUT_SPDIF_SIZE 6144

/* Number of samples in an A/52 frame. */
#define A52_FRAME_NB 1536

/* Max input rate factor (1/4 -> 4) */
#define AOUT_MAX_INPUT_RATE (4)

/** audio output buffer FIFO */
struct aout_fifo_t
{
    aout_buffer_t *         p_first;
    aout_buffer_t **        pp_last;
    date_t                  end_date;
};

/* FIXME to remove once aout.h is cleaned a bit more */
#include <vlc_block.h>

#define AOUT_RESAMPLING_NONE     0
#define AOUT_RESAMPLING_UP       1
#define AOUT_RESAMPLING_DOWN     2

/** an output stream for the audio output */
typedef struct aout_output_t
{
    audio_sample_format_t   output;
    /* Indicates whether the audio output is currently starving, to avoid
     * printing a 1,000 "output is starving" messages. */
    bool              b_starving;

    /* post-filters */
    filter_t *              pp_filters[AOUT_MAX_FILTERS];
    int                     i_nb_filters;

    aout_fifo_t             fifo;

    struct module_t *       p_module;
    struct aout_sys_t *     p_sys;
    void (*pf_play)( aout_instance_t * );
    void (* pf_pause)( aout_instance_t *, bool, mtime_t );
    int (* pf_volume_set )( aout_instance_t *, float, bool );
    int                     i_nb_samples;
} aout_output_t;

struct aout_mixer_t;

/** audio output thread descriptor */
struct aout_instance_t
{
    VLC_COMMON_MEMBERS

    /* Lock for volume variables (FIXME: should be in input manager) */
    vlc_mutex_t             volume_lock;
    vlc_mutex_t             lock;

    /* Input streams & pre-filters */
    aout_input_t *          p_input;

    /* Mixer */
    audio_sample_format_t   mixer_format;
    float                   mixer_multiplier;
    struct aout_mixer_t    *p_mixer;

    /* Output plug-in */
    aout_output_t           output;
};

/**
 * It describes the audio channel order VLC expect.
 */
static const uint32_t pi_vlc_chan_order_wg4[] =
{
    AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT,
    AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
    AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT, AOUT_CHAN_REARCENTER,
    AOUT_CHAN_CENTER, AOUT_CHAN_LFE, 0
};

/*****************************************************************************
 * Prototypes
 *****************************************************************************/

VLC_API aout_buffer_t * aout_OutputNextBuffer( aout_instance_t *, mtime_t, bool ) VLC_USED;

/**
 * This function computes the reordering needed to go from pi_chan_order_in to
 * pi_chan_order_out.
 * If pi_chan_order_in or pi_chan_order_out is NULL, it will assume that vlc
 * internal (WG4) order is requested.
 */
VLC_API int aout_CheckChannelReorder( const uint32_t *pi_chan_order_in, const uint32_t *pi_chan_order_out, uint32_t i_channel_mask, int i_channels, int *pi_chan_table );
VLC_API void aout_ChannelReorder( uint8_t *, int, int, const int *, int );

/**
 * This fonction will compute the extraction parameter into pi_selection to go
 * from i_channels with their type given by pi_order_src[] into the order
 * describe by pi_order_dst.
 * It will also set :
 * - *pi_channels as the number of channels that will be extracted which is
 * lower (in case of non understood channels type) or equal to i_channels.
 * - the layout of the channels (*pi_layout).
 *
 * It will return true if channel extraction is really needed, in which case
 * aout_ChannelExtract must be used
 *
 * XXX It must be used when the source may have channel type not understood
 * by VLC. In this case the channel type pi_order_src[] must be set to 0.
 * XXX It must also be used if multiple channels have the same type.
 */
VLC_API bool aout_CheckChannelExtraction( int *pi_selection, uint32_t *pi_layout, int *pi_channels, const uint32_t pi_order_dst[AOUT_CHAN_MAX], const uint32_t *pi_order_src, int i_channels );

/**
 * Do the actual channels extraction using the parameters created by
 * aout_CheckChannelExtraction.
 *
 * XXX this function does not work in place (p_dst and p_src must not overlap).
 * XXX Only 8, 16, 24, 32, 64 bits per sample are supported.
 */
VLC_API void aout_ChannelExtract( void *p_dst, int i_dst_channels, const void *p_src, int i_src_channels, int i_sample_count, const int *pi_selection, int i_bits_per_sample );

/* */
static inline unsigned aout_FormatNbChannels(const audio_sample_format_t *fmt)
{
    return popcount(fmt->i_physical_channels & AOUT_CHAN_PHYSMASK);
}

VLC_API unsigned int aout_BitsPerSample( vlc_fourcc_t i_format ) VLC_USED;
VLC_API void aout_FormatPrepare( audio_sample_format_t * p_format );
VLC_API void aout_FormatPrint( aout_instance_t * p_aout, const char * psz_text, const audio_sample_format_t * p_format );
VLC_API const char * aout_FormatPrintChannels( const audio_sample_format_t * ) VLC_USED;

VLC_API mtime_t aout_FifoFirstDate( const aout_fifo_t * ) VLC_USED;
VLC_API aout_buffer_t *aout_FifoPop( aout_fifo_t * p_fifo ) VLC_USED;

/* From intf.c : */
VLC_API void aout_VolumeSoftInit( aout_instance_t * );
VLC_API void aout_VolumeNoneInit( aout_instance_t * );
VLC_API int aout_ChannelsRestart( vlc_object_t *, const char *, vlc_value_t, vlc_value_t, void * );

/* */
VLC_API vout_thread_t * aout_filter_RequestVout( filter_t *, vout_thread_t *p_vout, video_format_t *p_fmt ) VLC_USED;

#endif /* VLC_AOUT_H */
