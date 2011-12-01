/*****************************************************************************
 * httplive.c: HTTP Live Streaming stream filter
 *****************************************************************************
 * Copyright (C) 2010-2011 M2X BV
 * $Id: cc3fe48c755b392037599427fd59332efe132d1a $
 *
 * Author: Jean-Paul Saman <jpsaman _AT_ videolan _DOT_ org>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <limits.h>
#include <errno.h>

#include <vlc_common.h>
#include <vlc_plugin.h>

#include <assert.h>

#include <vlc_threads.h>
#include <vlc_arrays.h>
#include <vlc_stream.h>
#include <vlc_url.h>
#include <vlc_memory.h>

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int  Open (vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_category(CAT_INPUT)
    set_subcategory(SUBCAT_INPUT_STREAM_FILTER)
    set_description(N_("Http Live Streaming stream filter"))
    set_capability("stream_filter", 20)
    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 *
 *****************************************************************************/
typedef struct segment_s
{
    int         sequence;   /* unique sequence number */
    int         duration;   /* segment duration (seconds) */
    uint64_t    size;       /* segment size in bytes */
    uint64_t    bandwidth;  /* bandwidth usage of segments (bits per second)*/

    vlc_url_t   url;
    vlc_mutex_t lock;
    block_t     *data;      /* data */
} segment_t;

typedef struct hls_stream_s
{
    int         id;         /* program id */
    int         version;    /* protocol version should be 1 */
    int         sequence;   /* media sequence number */
    int         duration;   /* maximum duration per segment (ms) */
    uint64_t    bandwidth;  /* bandwidth usage of segments (bits per second)*/
    uint64_t    size;       /* stream length (segment->duration * hls->bandwidth/8) */

    vlc_array_t *segments;  /* list of segments */
    vlc_url_t   url;        /* uri to m3u8 */
    vlc_mutex_t lock;
    bool        b_cache;    /* allow caching */
} hls_stream_t;

struct stream_sys_t
{
    vlc_url_t     m3u8;         /* M3U8 url */
    vlc_thread_t  reload;       /* HLS m3u8 reload thread */
    vlc_thread_t  thread;       /* HLS segment download thread */

    /* */
    vlc_array_t  *hls_stream;   /* bandwidth adaptation */
    uint64_t      bandwidth;    /* measured bandwidth (bits per second) */

    /* Download */
    struct hls_download_s
    {
        int         stream;     /* current hls_stream  */
        int         segment;    /* current segment for downloading */
        int         seek;       /* segment requested by seek (default -1) */
        vlc_mutex_t lock_wait;  /* protect segment download counter */
        vlc_cond_t  wait;       /* some condition to wait on */
    } download;

    /* Playback */
    struct hls_playback_s
    {
        uint64_t    offset;     /* current offset in media */
        int         stream;     /* current hls_stream  */
        int         segment;    /* current segment for playback */
    } playback;

    /* Playlist */
    struct hls_playlist_s
    {
        mtime_t     last;       /* playlist last loaded */
        mtime_t     wakeup;     /* next reload time */
        int         tries;      /* times it was not changed */
    } playlist;

    /* state */
    bool        b_cache;    /* can cache files */
    bool        b_meta;     /* meta playlist */
    bool        b_live;     /* live stream? or vod? */
    bool        b_error;    /* parsing error */
};

/****************************************************************************
 * Local prototypes
 ****************************************************************************/
static int  Read   (stream_t *, void *p_read, unsigned int i_read);
static int  Peek   (stream_t *, const uint8_t **pp_peek, unsigned int i_peek);
static int  Control(stream_t *, int i_query, va_list);

static ssize_t read_M3U8_from_stream(stream_t *s, uint8_t **buffer);
static ssize_t read_M3U8_from_url(stream_t *s, vlc_url_t *url, uint8_t **buffer);
static char *ReadLine(uint8_t *buffer, uint8_t **pos, size_t len);

static int hls_Download(stream_t *s, segment_t *segment);

static void* hls_Thread(void *);
static void* hls_Reload(void *);

static segment_t *segment_GetSegment(hls_stream_t *hls, int wanted);
static void segment_Free(segment_t *segment);

/****************************************************************************
 *
 ****************************************************************************/
static const char *const ext[] = {
    "#EXT-X-TARGETDURATION",
    "#EXT-X-MEDIA-SEQUENCE",
    "#EXT-X-KEY",
    "#EXT-X-ALLOW-CACHE",
    "#EXT-X-ENDLIST",
    "#EXT-X-STREAM-INF",
    "#EXT-X-DISCONTINUITY",
    "#EXT-X-VERSION"
};

static bool isHTTPLiveStreaming(stream_t *s)
{
    const uint8_t *peek, *peek_end;

    int64_t i_size = stream_Peek(s->p_source, &peek, 46);
    if (i_size < 1)
        return false;

    if (strncasecmp((const char*)peek, "#EXTM3U", 7) != 0)
        return false;

    /* Parse stream and search for
     * EXT-X-TARGETDURATION or EXT-X-STREAM-INF tag, see
     * http://tools.ietf.org/html/draft-pantos-http-live-streaming-04#page-8 */
    peek_end = peek + i_size;
    while(peek <= peek_end)
    {
        if (*peek == '#')
        {
            for (unsigned int i = 0; i < ARRAY_SIZE(ext); i++)
            {
                char *p = strstr((const char*)peek, ext[i]);
                if (p != NULL)
                    return true;
            }
        }
        peek++;
    };

    return false;
}

/* HTTP Live Streaming */
static hls_stream_t *hls_New(vlc_array_t *hls_stream, const int id, const uint64_t bw, const char *uri)
{
    hls_stream_t *hls = (hls_stream_t *)malloc(sizeof(hls_stream_t));
    if (hls == NULL) return NULL;

    hls->id = id;
    hls->bandwidth = bw;
    hls->duration = -1;/* unknown */
    hls->size = 0;
    hls->sequence = 0; /* default is 0 */
    hls->version = 1;  /* default protocol version */
    hls->b_cache = true;
    vlc_UrlParse(&hls->url, uri, 0);
    hls->segments = vlc_array_new();
    vlc_array_append(hls_stream, hls);
    vlc_mutex_init(&hls->lock);
    return hls;
}

static void hls_Free(hls_stream_t *hls)
{
    vlc_mutex_destroy(&hls->lock);

    if (hls->segments)
    {
        for (int n = 0; n < vlc_array_count(hls->segments); n++)
        {
            segment_t *segment = (segment_t *)vlc_array_item_at_index(hls->segments, n);
            if (segment) segment_Free(segment);
        }
        vlc_array_destroy(hls->segments);
    }

    vlc_UrlClean(&hls->url);
    free(hls);
    hls = NULL;
}

static hls_stream_t *hls_Get(vlc_array_t *hls_stream, const int wanted)
{
    int count = vlc_array_count(hls_stream);
    if (count <= 0)
        return NULL;
    if ((wanted < 0) || (wanted >= count))
        return NULL;
    return (hls_stream_t *) vlc_array_item_at_index(hls_stream, wanted);
}

static inline hls_stream_t *hls_GetFirst(vlc_array_t *hls_stream)
{
    return (hls_stream_t*) hls_Get(hls_stream, 0);
}

static hls_stream_t *hls_GetLast(vlc_array_t *hls_stream)
{
    int count = vlc_array_count(hls_stream);
    if (count <= 0)
        return NULL;
    count--;
    return (hls_stream_t *) hls_Get(hls_stream, count);
}

static hls_stream_t *hls_Find(vlc_array_t *hls_stream, hls_stream_t *hls_new)
{
    int count = vlc_array_count(hls_stream);
    for (int n = 0; n < count; n++)
    {
        hls_stream_t *hls = vlc_array_item_at_index(hls_stream, n);
        if (hls)
        {
            /* compare */
            if ((hls->id == hls_new->id) &&
                (hls->bandwidth == hls_new->bandwidth))
                return hls;
        }
    }
    return NULL;
}

static uint64_t hls_GetStreamSize(hls_stream_t *hls)
{
    /* NOTE: Stream size is calculated based on segment duration and
     * HLS stream bandwidth from the .m3u8 file. If these are not correct
     * then the deviation from exact byte size will be big and the seek/
     * progressbar will not behave entirely as one expects. */
    uint64_t size = 0UL;
    int count = vlc_array_count(hls->segments);
    for (int n = 0; n < count; n++)
    {
        segment_t *segment = segment_GetSegment(hls, n);
        if (segment)
        {
            size += (segment->duration * (hls->bandwidth / 8));
        }
    }
    return size;
}

/* Segment */
static segment_t *segment_New(hls_stream_t* hls, const int duration, const char *uri)
{
    segment_t *segment = (segment_t *)malloc(sizeof(segment_t));
    if (segment == NULL)
        return NULL;

    segment->duration = duration; /* seconds */
    segment->size = 0; /* bytes */
    segment->sequence = 0;
    segment->bandwidth = 0;
    vlc_UrlParse(&segment->url, uri, 0);
    segment->data = NULL;
    vlc_array_append(hls->segments, segment);
    vlc_mutex_init(&segment->lock);
    return segment;
}

static void segment_Free(segment_t *segment)
{
    vlc_mutex_destroy(&segment->lock);

    vlc_UrlClean(&segment->url);
    if (segment->data)
        block_Release(segment->data);
    free(segment);
    segment = NULL;
}

static segment_t *segment_GetSegment(hls_stream_t *hls, const int wanted)
{
    assert(hls);

    int count = vlc_array_count(hls->segments);
    if (count <= 0)
        return NULL;
    if ((wanted < 0) || (wanted >= count))
        return NULL;
    return (segment_t *) vlc_array_item_at_index(hls->segments, wanted);
}

static segment_t *segment_Find(hls_stream_t *hls, const int sequence)
{
    assert(hls);

    int count = vlc_array_count(hls->segments);
    if (count <= 0) return NULL;
    for (int n = 0; n < count; n++)
    {
        segment_t *segment = vlc_array_item_at_index(hls->segments, n);
        if (segment == NULL) break;
        if (segment->sequence == sequence)
            return segment;
    }
    return NULL;
}

static int ChooseSegment(stream_t *s, const int current)
{
    stream_sys_t *p_sys = (stream_sys_t *)s->p_sys;
    hls_stream_t *hls = hls_Get(p_sys->hls_stream, current);
    if (hls == NULL) return 0;

    /* Choose a segment to start which is no closer then
     * 3 times the target duration from the end of the playlist.
     */
    int wanted = 0;
    int duration = 0;
    int sequence = 0;
    int count = vlc_array_count(hls->segments);
    int i = p_sys->b_live ? count - 1 : 0;

    while((i >= 0) && (i < count) && vlc_object_alive(s))
    {
        segment_t *segment = segment_GetSegment(hls, i);
        assert(segment);

        if (segment->duration > hls->duration)
        {
            msg_Err(s, "EXTINF:%d duration is larger then EXT-X-TARGETDURATION:%d",
                    segment->duration, hls->duration);
        }

        duration += segment->duration;
        if (duration >= 3 * hls->duration)
        {
            /* Start point found */
            wanted = p_sys->b_live ? i : 0;
            sequence = segment->sequence;
            break;
        }

        if (p_sys->b_live)
            i-- ;
        else
          i++;
    }

    msg_Info(s, "Choose segment %d/%d (sequence=%d)", wanted, count, sequence);
    return wanted;
}

/* Parsing */
static char *parse_Attributes(const char *line, const char *attr)
{
    char *p;
    char *begin = (char *) line;
    char *end = begin + strlen(line);

    /* Find start of attributes */
    if ((p = strchr(begin, ':' )) == NULL)
        return NULL;

    begin = p;
    do
    {
        if (strncasecmp(begin, attr, strlen(attr)) == 0)
        {
            /* <attr>=<value>[,]* */
            p = strchr(begin, ',');
            begin += strlen(attr) + 1;
            if (begin >= end)
                return NULL;
            if (p == NULL) /* last attribute */
                return strndup(begin, end - begin);
            /* copy till ',' */
            return strndup(begin, p - begin);
        }
        begin++;
    } while(begin < end);

    return NULL;
}

static char *relative_URI(stream_t *s, const char *uri, const char *path)
{
    stream_sys_t *p_sys = s->p_sys;

    char *p = strchr(uri, ':');
    if (p != NULL)
        return NULL;

    if (p_sys->m3u8.psz_path == NULL)
        return NULL;

    char *psz_path = strdup(p_sys->m3u8.psz_path);
    if (psz_path == NULL) return NULL;
    p = strrchr(psz_path, '/');
    if (p) *p = '\0';

    char *psz_uri = NULL;
    if (p_sys->m3u8.psz_password || p_sys->m3u8.psz_username)
    {
        if (asprintf(&psz_uri, "%s://%s:%s@%s:%d%s/%s", p_sys->m3u8.psz_protocol,
                     p_sys->m3u8.psz_username, p_sys->m3u8.psz_password,
                     p_sys->m3u8.psz_host, p_sys->m3u8.i_port,
                     path ? path : psz_path, uri) < 0)
            goto fail;
    }
    else
    {
        if (asprintf(&psz_uri, "%s://%s:%d%s/%s", p_sys->m3u8.psz_protocol,
                     p_sys->m3u8.psz_host, p_sys->m3u8.i_port,
                     path ? path : psz_path, uri) < 0)
           goto fail;
    }
    free(psz_path);
    return psz_uri;

fail:
    free(psz_path);
    return NULL;
}

static char *ConstructUrl(vlc_url_t *url)
{
    if ((url->psz_protocol == NULL) ||
        (url->psz_path == NULL))
        return NULL;

    if (url->i_port <= 0)
    {
        if (strncmp(url->psz_protocol, "https", 5) == 0)
            url->i_port = 443;
        else
            url->i_port = 80;
    }

    char *psz_url = NULL;
    if (url->psz_password || url->psz_username)
    {
        if (asprintf(&psz_url, "%s://%s:%s@%s:%d%s",
                     url->psz_protocol,
                     url->psz_username, url->psz_password,
                     url->psz_host, url->i_port, url->psz_path) < 0)
            return NULL;
    }
    else
    {
        if (asprintf(&psz_url, "%s://%s:%d%s",
                     url->psz_protocol,
                     url->psz_host, url->i_port, url->psz_path) < 0)
            return NULL;
    }

    return psz_url;
}

static int parse_SegmentInformation(hls_stream_t *hls, char *p_read, int *duration)
{
    assert(hls);
    assert(p_read);

    /* strip of #EXTINF: */
    char *p_next = NULL;
    char *token = strtok_r(p_read, ":", &p_next);
    if (token == NULL)
        return VLC_EGENERIC;

    /* read duration */
    token = strtok_r(NULL, ",", &p_next);
    if (token == NULL)
        return VLC_EGENERIC;

    int value;
    if (hls->version < 3)
    {
       value = strtol(token, NULL, 10);
       if (errno == ERANGE)
       {
           *duration = -1;
           return VLC_EGENERIC;
       }
       *duration = value;
    }
    else
    {
        double d = strtod(token, (char **) NULL);
        if (errno == ERANGE)
        {
            *duration = -1;
            return VLC_EGENERIC;
        }
        if ((d) - ((int)d) >= 0.5)
            value = ((int)d) + 1;
        else
            value = ((int)d);
    }

    /* Ignore the rest of the line */

    return VLC_SUCCESS;
}

static int parse_AddSegment(stream_t *s, hls_stream_t *hls, const int duration, const char *uri)
{
    assert(hls);
    assert(uri);

    /* Store segment information */
    char *psz_path = NULL;
    if (hls->url.psz_path != NULL)
    {
        psz_path = strdup(hls->url.psz_path);
        if (psz_path == NULL)
            return VLC_ENOMEM;
        char *p = strrchr(psz_path, '/');
        if (p) *p = '\0';
    }
    char *psz_uri = relative_URI(s, uri, psz_path);
    free(psz_path);

    vlc_mutex_lock(&hls->lock);
    segment_t *segment = segment_New(hls, duration, psz_uri ? psz_uri : uri);
    if (segment)
        segment->sequence = hls->sequence + vlc_array_count(hls->segments) - 1;
    vlc_mutex_unlock(&hls->lock);

    free(psz_uri);
    return segment ? VLC_SUCCESS : VLC_ENOMEM;
}

static int parse_TargetDuration(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    int duration = -1;
    int ret = sscanf(p_read, "#EXT-X-TARGETDURATION:%d", &duration);
    if (ret != 1)
    {
        msg_Err(s, "expected #EXT-X-TARGETDURATION:<s>");
        return VLC_EGENERIC;
    }

    hls->duration = duration; /* seconds */
    return VLC_SUCCESS;
}

static int parse_StreamInformation(stream_t *s, vlc_array_t **hls_stream,
                                   hls_stream_t **hls, char *p_read, const char *uri)
{
    int id;
    uint64_t bw;
    char *attr;

    assert(*hls == NULL);

    attr = parse_Attributes(p_read, "PROGRAM-ID");
    if (attr == NULL)
    {
        msg_Err(s, "#EXT-X-STREAM-INF: expected PROGRAM-ID=<value>");
        return VLC_EGENERIC;
    }
    id = atol(attr);
    free(attr);

    attr = parse_Attributes(p_read, "BANDWIDTH");
    if (attr == NULL)
    {
        msg_Err(s, "#EXT-X-STREAM-INF: expected BANDWIDTH=<value>");
        return VLC_EGENERIC;
    }
    bw = atoll(attr);
    free(attr);

    if (bw == 0)
    {
        msg_Err(s, "#EXT-X-STREAM-INF: bandwidth cannot be 0");
        return VLC_EGENERIC;
    }

    msg_Info(s, "bandwidth adaption detected (program-id=%d, bandwidth=%"PRIu64").", id, bw);

    char *psz_uri = relative_URI(s, uri, NULL);

    *hls = hls_New(*hls_stream, id, bw, psz_uri ? psz_uri : uri);

    free(psz_uri);

    return (*hls == NULL) ? VLC_ENOMEM : VLC_SUCCESS;
}

static int parse_MediaSequence(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    int sequence;
    int ret = sscanf(p_read, "#EXT-X-MEDIA-SEQUENCE:%d", &sequence);
    if (ret != 1)
    {
        msg_Err(s, "expected #EXT-X-MEDIA-SEQUENCE:<s>");
        return VLC_EGENERIC;
    }

    if (hls->sequence > 0)
        msg_Err(s, "EXT-X-MEDIA-SEQUENCE already present in playlist (new=%d, old=%d)",
                    sequence, hls->sequence);

    hls->sequence = sequence;
    return VLC_SUCCESS;
}

static int parse_Key(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    /* #EXT-X-KEY:METHOD=<method>[,URI="<URI>"][,IV=<IV>] */
    int err = VLC_SUCCESS;
    char *attr = parse_Attributes(p_read, "METHOD");
    if (attr == NULL)
    {
        msg_Err(s, "#EXT-X-KEY: expected METHOD=<value>");
        return err;
    }

    if (strncasecmp(attr, "NONE", 4) == 0)
    {

        char *uri = parse_Attributes(p_read, "URI");
        if (uri != NULL)
        {
            msg_Err(s, "#EXT-X-KEY: URI not expected");
            err = VLC_EGENERIC;
        }
        free(uri);
        /* IV is only supported in version 2 and above */
        if (hls->version >= 2)
        {
            char *iv = parse_Attributes(p_read, "IV");
            if (iv != NULL)
            {
                msg_Err(s, "#EXT-X-KEY: IV not expected");
                err = VLC_EGENERIC;
            }
            free(iv);
        }
    }
    else
    {
        msg_Warn(s, "playback of encrypted HTTP Live media is not supported.");
        err = VLC_EGENERIC;
    }
    free(attr);
    return err;
}

static int parse_ProgramDateTime(stream_t *s, hls_stream_t *hls, char *p_read)
{
    VLC_UNUSED(hls);
    msg_Dbg(s, "tag not supported: #EXT-X-PROGRAM-DATE-TIME %s", p_read);
    return VLC_SUCCESS;
}

static int parse_AllowCache(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    char answer[4] = "\0";
    int ret = sscanf(p_read, "#EXT-X-ALLOW-CACHE:%3s", answer);
    if (ret != 1)
    {
        msg_Err(s, "#EXT-X-ALLOW-CACHE, ignoring ...");
        return VLC_EGENERIC;
    }

    hls->b_cache = (strncmp(answer, "NO", 2) != 0);
    return VLC_SUCCESS;
}

static int parse_Version(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    int version;
    int ret = sscanf(p_read, "#EXT-X-VERSION:%d", &version);
    if (ret != 1)
    {
        msg_Err(s, "#EXT-X-VERSION: no protocol version found, should be version 1.");
        return VLC_EGENERIC;
    }

    /* Check version */
    hls->version = version;
    if (hls->version != 1)
    {
        msg_Err(s, "#EXT-X-VERSION should be version 1 iso %d", version);
        return VLC_EGENERIC;
    }
    return VLC_SUCCESS;
}

static int parse_EndList(stream_t *s, hls_stream_t *hls)
{
    assert(hls);

    s->p_sys->b_live = false;
    msg_Info(s, "video on demand (vod) mode");
    return VLC_SUCCESS;
}

static int parse_Discontinuity(stream_t *s, hls_stream_t *hls, char *p_read)
{
    assert(hls);

    /* FIXME: Do we need to act on discontinuity ?? */
    msg_Dbg(s, "#EXT-X-DISCONTINUITY %s", p_read);
    return VLC_SUCCESS;
}

/* The http://tools.ietf.org/html/draft-pantos-http-live-streaming-04#page-8
 * document defines the following new tags: EXT-X-TARGETDURATION,
 * EXT-X-MEDIA-SEQUENCE, EXT-X-KEY, EXT-X-PROGRAM-DATE-TIME, EXT-X-
 * ALLOW-CACHE, EXT-X-STREAM-INF, EXT-X-ENDLIST, EXT-X-DISCONTINUITY,
 * and EXT-X-VERSION.
 */
static int parse_M3U8(stream_t *s, vlc_array_t *streams, uint8_t *buffer, const ssize_t len)
{
    stream_sys_t *p_sys = s->p_sys;
    uint8_t *p_read, *p_begin, *p_end;

    assert(streams);
    assert(buffer);

    msg_Dbg(s, "parse_M3U8\n%s", buffer);
    p_begin = buffer;
    p_end = p_begin + len;

    char *line = ReadLine(p_begin, &p_read, p_end - p_begin);
    if (line == NULL)
        return VLC_ENOMEM;
    p_begin = p_read;

    if (strncmp(line, "#EXTM3U", 7) != 0)
    {
        msg_Err(s, "missing #EXTM3U tag .. aborting");
        free(line);
        return VLC_EGENERIC;
    }

    free(line);
    line = NULL;

    /* What is the version ? */
    int version = 1;
    uint8_t *p = (uint8_t *)strstr((const char *)buffer, "#EXT-X-VERSION:");
    if (p != NULL)
    {
        uint8_t *tmp = NULL;
        char *psz_version = ReadLine(p, &tmp, p_end - p);
        if (psz_version == NULL)
            return VLC_ENOMEM;
        int ret = sscanf((const char*)psz_version, "#EXT-X-VERSION:%d", &version);
        if (ret != 1)
        {
            msg_Warn(s, "#EXT-X-VERSION: no protocol version found, assuming version 1.");
            version = 1;
        }
        free(psz_version);
        p = NULL;
    }

    /* Is it a live stream ? */
    p_sys->b_live = (strstr((const char *)buffer, "#EXT-X-ENDLIST") == NULL) ? true : false;

    /* Is it a meta index file ? */
    bool b_meta = (strstr((const char *)buffer, "#EXT-X-STREAM-INF") == NULL) ? false : true;

    int err = VLC_SUCCESS;

    if (b_meta)
    {
        msg_Info(s, "Meta playlist");

        /* M3U8 Meta Index file */
        do {
            /* Next line */
            line = ReadLine(p_begin, &p_read, p_end - p_begin);
            if (line == NULL)
                break;
            p_begin = p_read;

            /* */
            if (strncmp(line, "#EXT-X-STREAM-INF", 17) == 0)
            {
                p_sys->b_meta = true;
                char *uri = ReadLine(p_begin, &p_read, p_end - p_begin);
                if (uri == NULL)
                    err = VLC_ENOMEM;
                else
                {
                    hls_stream_t *hls = NULL;
                    err = parse_StreamInformation(s, &streams, &hls, line, uri);
                    free(uri);

                    /* Download playlist file from server */
                    uint8_t *buf = NULL;
                    ssize_t len = read_M3U8_from_url(s, &hls->url, &buf);
                    if (len < 0)
                        err = VLC_EGENERIC;
                    else
                    {
                        /* Parse HLS m3u8 content. */
                        err = parse_M3U8(s, streams, buf, len);
                        free(buf);
                    }

                    if (hls)
                    {
                        hls->version = version;
                        if (!p_sys->b_live)
                            hls->size = hls_GetStreamSize(hls); /* Stream size (approximate) */
                    }
                }
                p_begin = p_read;
            }

            free(line);
            line = NULL;

            if (p_begin >= p_end)
                break;

        } while ((err == VLC_SUCCESS) && vlc_object_alive(s));

    }
    else
    {
        msg_Info(s, "%s Playlist HLS protocol version: %d", p_sys->b_live ? "Live": "VOD", version);

        hls_stream_t *hls = NULL;
        if (p_sys->b_meta)
            hls = hls_GetLast(streams);
        else
        {
            /* No Meta playlist used */
            hls = hls_New(streams, 0, -1, NULL);
            if (hls)
            {
                /* Get TARGET-DURATION first */
                p = (uint8_t *)strstr((const char *)buffer, "#EXT-X-TARGETDURATION:");
                if (p)
                {
                    uint8_t *p_rest = NULL;
                    char *psz_duration = ReadLine(p, &p_rest,  p_end - p);
                    if (psz_duration == NULL)
                        return VLC_EGENERIC;
                    err = parse_TargetDuration(s, hls, psz_duration);
                    free(psz_duration);
                    p = NULL;
                }

                /* Store version */
                hls->version = version;
            }
            else return VLC_ENOMEM;
        }
        assert(hls);

        /* */
        int segment_duration = -1;
        do
        {
            /* Next line */
            line = ReadLine(p_begin, &p_read, p_end - p_begin);
            if (line == NULL)
                break;
            p_begin = p_read;

            if (strncmp(line, "#EXTINF", 7) == 0)
                err = parse_SegmentInformation(hls, line, &segment_duration);
            else if (strncmp(line, "#EXT-X-TARGETDURATION", 21) == 0)
                err = parse_TargetDuration(s, hls, line);
            else if (strncmp(line, "#EXT-X-MEDIA-SEQUENCE", 21) == 0)
                err = parse_MediaSequence(s, hls, line);
            else if (strncmp(line, "#EXT-X-KEY", 10) == 0)
                err = parse_Key(s, hls, line);
            else if (strncmp(line, "#EXT-X-PROGRAM-DATE-TIME", 24) == 0)
                err = parse_ProgramDateTime(s, hls, line);
            else if (strncmp(line, "#EXT-X-ALLOW-CACHE", 18) == 0)
                err = parse_AllowCache(s, hls, line);
            else if (strncmp(line, "#EXT-X-DISCONTINUITY", 20) == 0)
                err = parse_Discontinuity(s, hls, line);
            else if (strncmp(line, "#EXT-X-VERSION", 14) == 0)
                err = parse_Version(s, hls, line);
            else if (strncmp(line, "#EXT-X-ENDLIST", 14) == 0)
                err = parse_EndList(s, hls);
            else if ((strncmp(line, "#", 1) != 0) && (*line != '\0') )
            {
                err = parse_AddSegment(s, hls, segment_duration, line);
                segment_duration = -1; /* reset duration */
            }

            free(line);
            line = NULL;

            if (p_begin >= p_end)
                break;

        } while ((err == VLC_SUCCESS) && vlc_object_alive(s));

        free(line);
    }

    return err;
}

static int get_HTTPLiveMetaPlaylist(stream_t *s, vlc_array_t **streams)
{
    stream_sys_t *p_sys = s->p_sys;
    assert(*streams);

    /* Download new playlist file from server */
    uint8_t *buffer = NULL;
    ssize_t len = read_M3U8_from_url(s, &p_sys->m3u8, &buffer);
    if (len < 0)
        return VLC_EGENERIC;

    /* Parse HLS m3u8 content. */
    int err = parse_M3U8(s, *streams, buffer, len);
    free(buffer);

    return err;
}

/* Reload playlist */
static int hls_UpdatePlaylist(stream_t *s, hls_stream_t *hls_new, hls_stream_t **hls)
{
    int count = vlc_array_count(hls_new->segments);

    msg_Info(s, "updating hls stream (program-id=%d, bandwidth=%"PRIu64") has %d segments",
             hls_new->id, hls_new->bandwidth, count);
    for (int n = 0; n < count; n++)
    {
        segment_t *p = segment_GetSegment(hls_new, n);
        if (p == NULL) return VLC_EGENERIC;

        vlc_mutex_lock(&(*hls)->lock);
        segment_t *segment = segment_Find(*hls, p->sequence);
        if (segment)
        {
            /* they should be the same */
            if ((p->sequence != segment->sequence) ||
                (p->duration != segment->duration) ||
                (strcmp(p->url.psz_path, segment->url.psz_path) != 0))
            {
                msg_Err(s, "existing segment found with different content - resetting");
                msg_Err(s, "- sequence: new=%d, old=%d", p->sequence, segment->sequence);
                msg_Err(s, "- duration: new=%d, old=%d", p->duration, segment->duration);
                msg_Err(s, "- file: new=%s", p->url.psz_path);
                msg_Err(s, "        old=%s", segment->url.psz_path);

                /* Resetting content */
                segment->sequence = p->sequence;
                segment->duration = p->duration;
                vlc_UrlClean(&segment->url);
                vlc_UrlParse(&segment->url, p->url.psz_buffer, 0);
                segment_Free(p);
            }
        }
        else
        {
            int last = vlc_array_count((*hls)->segments) - 1;
            segment_t *l = segment_GetSegment(*hls, last);
            if (l == NULL) goto fail_and_unlock;

            if ((l->sequence + 1) != p->sequence)
            {
                msg_Err(s, "gap in sequence numbers found: new=%d expected %d",
                        p->sequence, l->sequence+1);
            }
            vlc_array_append((*hls)->segments, p);
            msg_Info(s, "- segment %d appended", p->sequence);
        }
        vlc_mutex_unlock(&(*hls)->lock);
    }
    return VLC_SUCCESS;

fail_and_unlock:
    vlc_mutex_unlock(&(*hls)->lock);
    return VLC_EGENERIC;
}

static int hls_ReloadPlaylist(stream_t *s)
{
    stream_sys_t *p_sys = s->p_sys;

    vlc_array_t *hls_streams = vlc_array_new();
    if (hls_streams == NULL)
        return VLC_ENOMEM;

    msg_Info(s, "Reloading HLS live meta playlist");

    if (get_HTTPLiveMetaPlaylist(s, &hls_streams) != VLC_SUCCESS)
    {
        /* Free hls streams */
        for (int i = 0; i < vlc_array_count(hls_streams); i++)
        {
            hls_stream_t *hls;
            hls = (hls_stream_t *)vlc_array_item_at_index(hls_streams, i);
            if (hls) hls_Free(hls);
        }
        vlc_array_destroy(hls_streams);

        msg_Err(s, "reloading playlist failed");
        return VLC_EGENERIC;
    }

    /* merge playlists */
    int count = vlc_array_count(hls_streams);
    for (int n = 0; n < count; n++)
    {
        hls_stream_t *hls_new = hls_Get(hls_streams, n);
        if (hls_new == NULL)
            continue;

        hls_stream_t *hls_old = hls_Find(p_sys->hls_stream, hls_new);
        if (hls_old == NULL)
        {   /* new hls stream - append */
            vlc_array_append(p_sys->hls_stream, hls_new);
            msg_Info(s, "new HLS stream appended (id=%d, bandwidth=%"PRIu64")",
                     hls_new->id, hls_new->bandwidth);
        }
        else if (hls_UpdatePlaylist(s, hls_new, &hls_old) != VLC_SUCCESS)
            msg_Info(s, "failed updating HLS stream (id=%d, bandwidth=%"PRIu64")",
                     hls_new->id, hls_new->bandwidth);
    }

    vlc_array_destroy(hls_streams);
    return VLC_SUCCESS;
}

/****************************************************************************
 * hls_Thread
 ****************************************************************************/
static int BandwidthAdaptation(stream_t *s, int progid, uint64_t *bandwidth)
{
    stream_sys_t *p_sys = s->p_sys;
    int candidate = -1;
    uint64_t bw = *bandwidth;
    uint64_t bw_candidate = 0;

    int count = vlc_array_count(p_sys->hls_stream);
    for (int n = 0; n < count; n++)
    {
        /* Select best bandwidth match */
        hls_stream_t *hls = hls_Get(p_sys->hls_stream, n);
        if (hls == NULL) break;

        /* only consider streams with the same PROGRAM-ID */
        if (hls->id == progid)
        {
            if ((bw >= hls->bandwidth) && (bw_candidate < hls->bandwidth))
            {
                msg_Dbg(s, "candidate %d bandwidth (bits/s) %"PRIu64" >= %"PRIu64,
                         n, bw, hls->bandwidth); /* bits / s */
                bw_candidate = hls->bandwidth;
                candidate = n; /* possible candidate */
            }
        }
    }
    *bandwidth = bw_candidate;
    return candidate;
}

static int Download(stream_t *s, hls_stream_t *hls, segment_t *segment, int *cur_stream)
{
    stream_sys_t *p_sys = s->p_sys;

    assert(hls);
    assert(segment);

    vlc_mutex_lock(&segment->lock);
    if (segment->data != NULL)
    {
        /* Segment already downloaded */
        vlc_mutex_unlock(&segment->lock);
        return VLC_SUCCESS;
    }

    /* sanity check - can we download this segment on time? */
    if ((p_sys->bandwidth > 0) && (hls->bandwidth > 0))
    {
        uint64_t size = (segment->duration * hls->bandwidth); /* bits */
        int estimated = (int)(size / p_sys->bandwidth);
        if (estimated > segment->duration)
        {
            msg_Warn(s,"downloading of segment %d takes %ds, which is longer then its playback (%ds)",
                        segment->sequence, estimated, segment->duration);
        }
    }

    mtime_t start = mdate();
    if (hls_Download(s, segment) != VLC_SUCCESS)
    {
        vlc_mutex_unlock(&segment->lock);
        return VLC_EGENERIC;
    }
    mtime_t duration = mdate() - start;

    vlc_mutex_unlock(&segment->lock);

    msg_Info(s, "downloaded segment %d from stream %d",
                segment->sequence, *cur_stream);

    /* check for division by zero */
    double ms = (double)duration / 1000.0; /* ms */
    if (ms <= 0.0)
        return VLC_SUCCESS;

    uint64_t bw = ((double)(segment->size * 8) / ms) * 1000; /* bits / s */
    p_sys->bandwidth = bw;
    if (p_sys->b_meta && (hls->bandwidth != bw))
    {
        int newstream = BandwidthAdaptation(s, hls->id, &bw);

        /* FIXME: we need an average here */
        if ((newstream >= 0) && (newstream != *cur_stream))
        {
            msg_Info(s, "detected %s bandwidth (%"PRIu64") stream",
                     (bw >= hls->bandwidth) ? "faster" : "lower", bw);
            *cur_stream = newstream;
        }
    }
    return VLC_SUCCESS;
}

static void* hls_Thread(void *p_this)
{
    stream_t *s = (stream_t *)p_this;
    stream_sys_t *p_sys = s->p_sys;

    int canc = vlc_savecancel();

    while (vlc_object_alive(s))
    {
        hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->download.stream);
        assert(hls);

        /* Sliding window (~60 seconds worth of movie) */
        vlc_mutex_lock(&hls->lock);
        int count = vlc_array_count(hls->segments);
        vlc_mutex_unlock(&hls->lock);

        /* Is there a new segment to process? */
        if ((!p_sys->b_live && (p_sys->playback.segment < (count - 6))) ||
            (p_sys->download.segment >= count))
        {
            /* wait */
            vlc_mutex_lock(&p_sys->download.lock_wait);
            while (((p_sys->download.segment - p_sys->playback.segment > 6) ||
                    (p_sys->download.segment >= count)) &&
                   (p_sys->download.seek == -1))
            {
                vlc_cond_wait(&p_sys->download.wait, &p_sys->download.lock_wait);
                if (p_sys->b_live /*&& (mdate() >= p_sys->playlist.wakeup)*/)
                    break;
                if (!vlc_object_alive(s))
                    break;
            }
            /* */
            if (p_sys->download.seek >= 0)
            {
                p_sys->download.segment = p_sys->download.seek;
                p_sys->download.seek = -1;
            }
            vlc_mutex_unlock(&p_sys->download.lock_wait);
        }

        if (!vlc_object_alive(s)) break;

        vlc_mutex_lock(&hls->lock);
        segment_t *segment = segment_GetSegment(hls, p_sys->download.segment);
        vlc_mutex_unlock(&hls->lock);

        if ((segment != NULL) &&
            (Download(s, hls, segment, &p_sys->download.stream) != VLC_SUCCESS))
        {
            if (!vlc_object_alive(s)) break;

            if (!p_sys->b_live)
            {
                p_sys->b_error = true;
                break;
            }
        }

        /* download succeeded */
        /* determine next segment to download */
        vlc_mutex_lock(&p_sys->download.lock_wait);
        if (p_sys->download.seek >= 0)
        {
            p_sys->download.segment = p_sys->download.seek;
            p_sys->download.seek = -1;
        }
        else if (p_sys->download.segment < count)
            p_sys->download.segment++;
        vlc_cond_signal(&p_sys->download.wait);
        vlc_mutex_unlock(&p_sys->download.lock_wait);
    }

    vlc_restorecancel(canc);
    return NULL;
}

static void* hls_Reload(void *p_this)
{
    stream_t *s = (stream_t *)p_this;
    stream_sys_t *p_sys = s->p_sys;

    assert(p_sys->b_live);

    int canc = vlc_savecancel();

    while (vlc_object_alive(s))
    {
        double wait = 1;
        mtime_t now = mdate();
        if (now >= p_sys->playlist.wakeup)
        {
            /* reload the m3u8 */
            if (hls_ReloadPlaylist(s) != VLC_SUCCESS)
            {
                /* No change in playlist, then backoff */
                p_sys->playlist.tries++;
                if (p_sys->playlist.tries == 1) wait = 0.5;
                else if (p_sys->playlist.tries == 2) wait = 1;
                else if (p_sys->playlist.tries >= 3) wait = 3;
            }
            else p_sys->playlist.tries = 0;

            hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->download.stream);
            assert(hls);

            /* determine next time to update playlist */
            p_sys->playlist.last = now;
            p_sys->playlist.wakeup = now + ((mtime_t)(hls->duration * wait)
                                                   * (mtime_t)1000000);
        }

        mwait(p_sys->playlist.wakeup);
    }

    vlc_restorecancel(canc);
    return NULL;
}

static int Prefetch(stream_t *s, int *current)
{
    stream_sys_t *p_sys = s->p_sys;
    int stream;

    /* Try to pick best matching stream */;
again:
    stream = *current;

    hls_stream_t *hls = hls_Get(p_sys->hls_stream, *current);
    if (hls == NULL)
        return VLC_EGENERIC;

    segment_t *segment = segment_GetSegment(hls, p_sys->download.segment);
    if (segment == NULL )
        return VLC_EGENERIC;

    if (Download(s, hls, segment, current) != VLC_SUCCESS)
        return VLC_EGENERIC;

    /* Found better bandwidth match, try again */
    if (*current != stream)
        goto again;

    /* Download first 2 segments of this HLS stream */
    stream = *current;
    for (int i = 0; i < 2; i++)
    {
        segment_t *segment = segment_GetSegment(hls, p_sys->download.segment);
        if (segment == NULL )
            return VLC_EGENERIC;

        if (segment->data)
        {
            p_sys->download.segment++;
            continue;
        }

        if (Download(s, hls, segment, current) != VLC_SUCCESS)
            return VLC_EGENERIC;

        p_sys->download.segment++;

        /* adapt bandwidth? */
        if (*current != stream)
        {
            hls_stream_t *hls = hls_Get(p_sys->hls_stream, *current);
            if (hls == NULL)
                return VLC_EGENERIC;

             stream = *current;
        }
    }

    return VLC_SUCCESS;
}

/****************************************************************************
 *
 ****************************************************************************/
static int hls_Download(stream_t *s, segment_t *segment)
{
    assert(segment);

    /* Construct URL */
    char *psz_url = ConstructUrl(&segment->url);
    if (psz_url == NULL)
           return VLC_ENOMEM;

    stream_t *p_ts = stream_UrlNew(s, psz_url);
    free(psz_url);
    if (p_ts == NULL)
        return VLC_EGENERIC;

    segment->size = stream_Size(p_ts);
    assert(segment->size > 0);

    segment->data = block_Alloc(segment->size);
    if (segment->data == NULL)
    {
        stream_Delete(p_ts);
        return VLC_ENOMEM;
    }

    assert(segment->data->i_buffer == segment->size);

    ssize_t length = 0, curlen = 0;
    uint64_t size;
    do
    {
        size = stream_Size(p_ts);
        if (size > segment->size)
        {
            msg_Dbg(s, "size changed %"PRIu64, segment->size);
            segment->data = block_Realloc(segment->data, 0, size);
            if (segment->data == NULL)
            {
                stream_Delete(p_ts);
                return VLC_ENOMEM;
            }
            segment->size = size;
            assert(segment->data->i_buffer == segment->size);
        }
        length = stream_Read(p_ts, segment->data->p_buffer + curlen, segment->size - curlen);
        if (length <= 0)
            break;
        curlen += length;
    } while (vlc_object_alive(s));

    stream_Delete(p_ts);
    return VLC_SUCCESS;
}

/* Read M3U8 file */
static ssize_t read_M3U8_from_stream(stream_t *s, uint8_t **buffer)
{
    int64_t total_bytes = 0;
    int64_t total_allocated = 0;
    uint8_t *p = NULL;

    while (1)
    {
        char buf[4096];
        int64_t bytes;

        bytes = stream_Read(s, buf, sizeof(buf));
        if (bytes == 0)
            break;      /* EOF ? */
        else if (bytes < 0)
            return bytes;

        if ( (total_bytes + bytes + 1) > total_allocated )
        {
            if (total_allocated)
                total_allocated *= 2;
            else
                total_allocated = __MIN(bytes+1, sizeof(buf));

            p = realloc_or_free(p, total_allocated);
            if (p == NULL)
                return VLC_ENOMEM;
        }

        memcpy(p+total_bytes, buf, bytes);
        total_bytes += bytes;
    }

    if (total_allocated == 0)
        return VLC_EGENERIC;

    p[total_bytes] = '\0';
    *buffer = p;

    return total_bytes;
}

static ssize_t read_M3U8_from_url(stream_t *s, vlc_url_t *url, uint8_t **buffer)
{
    assert(*buffer == NULL);

    /* Construct URL */
    char *psz_url = ConstructUrl(url);
    if (psz_url == NULL)
           return VLC_ENOMEM;

    stream_t *p_m3u8 = stream_UrlNew(s, psz_url);
    free(psz_url);
    if (p_m3u8 == NULL)
        return VLC_EGENERIC;

    ssize_t size = read_M3U8_from_stream(p_m3u8, buffer);
    stream_Delete(p_m3u8);

    return size;
}

static char *ReadLine(uint8_t *buffer, uint8_t **pos, const size_t len)
{
    assert(buffer);

    char *line = NULL;
    uint8_t *begin = buffer;
    uint8_t *p = begin;
    uint8_t *end = p + len;

    while (p < end)
    {
        if ((*p == '\n') || (*p == '\0'))
            break;
        p++;
    }

    /* copy line excluding \n or \0 */
    line = strndup((char *)begin, p - begin);

    if (*p == '\0')
        *pos = end;
    else
    {
        /* next pass start after \n */
        p++;
        *pos = p;
    }

    return line;
}

/****************************************************************************
 * Open
 ****************************************************************************/
static int Open(vlc_object_t *p_this)
{
    stream_t *s = (stream_t*)p_this;
    stream_sys_t *p_sys;

    if (!isHTTPLiveStreaming(s))
        return VLC_EGENERIC;

    msg_Info(p_this, "HTTP Live Streaming (%s)", s->psz_path);

    /* */
    s->p_sys = p_sys = calloc(1, sizeof(*p_sys));
    if (p_sys == NULL)
        return VLC_ENOMEM;

    char *psz_uri = NULL;
    if (asprintf(&psz_uri,"%s://%s", s->psz_access, s->psz_path) < 0)
    {
        free(p_sys);
        return VLC_ENOMEM;
    }
    vlc_UrlParse(&p_sys->m3u8, psz_uri, 0);
    free(psz_uri);

    p_sys->bandwidth = -1;
    p_sys->b_live = true;
    p_sys->b_meta = false;
    p_sys->b_error = false;

    p_sys->hls_stream = vlc_array_new();
    if (p_sys->hls_stream == NULL)
    {
        vlc_UrlClean(&p_sys->m3u8);
        free(p_sys);
        return VLC_ENOMEM;
    }

    /* */
    s->pf_read = Read;
    s->pf_peek = Peek;
    s->pf_control = Control;

    /* Parse HLS m3u8 content. */
    uint8_t *buffer = NULL;
    ssize_t len = read_M3U8_from_stream(s->p_source, &buffer);
    if (len < 0)
        goto fail;
    if (parse_M3U8(s, p_sys->hls_stream, buffer, len) != VLC_SUCCESS)
    {
        free(buffer);
        goto fail;
    }
    free(buffer);

    /* Choose first HLS stream to start with */
    int current = p_sys->playback.stream = 0;
    p_sys->playback.segment = p_sys->download.segment = ChooseSegment(s, current);

    if (p_sys->b_live && (p_sys->playback.segment < 0))
    {
        msg_Warn(s, "less data then 3 times 'target duration' available for live playback, playback may stall");
    }

    if (Prefetch(s, &current) != VLC_SUCCESS)
    {
        msg_Err(s, "fetching first segment failed.");
        goto fail;
    }


    p_sys->download.stream = current;
    p_sys->playback.stream = current;
    p_sys->download.seek = -1;

    vlc_mutex_init(&p_sys->download.lock_wait);
    vlc_cond_init(&p_sys->download.wait);

    /* Initialize HLS live stream */
    if (p_sys->b_live)
    {
        hls_stream_t *hls = hls_Get(p_sys->hls_stream, current);
        p_sys->playlist.last = mdate();
        p_sys->playlist.wakeup = p_sys->playlist.last +
                ((mtime_t)hls->duration * UINT64_C(1000000));

        if (vlc_clone(&p_sys->reload, hls_Reload, s, VLC_THREAD_PRIORITY_LOW))
        {
            goto fail_thread;
        }
    }

    if (vlc_clone(&p_sys->thread, hls_Thread, s, VLC_THREAD_PRIORITY_INPUT))
    {
        if (p_sys->b_live)
            vlc_join(p_sys->reload, NULL);
        goto fail_thread;
    }

    return VLC_SUCCESS;

fail_thread:
    vlc_mutex_destroy(&p_sys->download.lock_wait);
    vlc_cond_destroy(&p_sys->download.wait);

fail:
    /* Free hls streams */
    for (int i = 0; i < vlc_array_count(p_sys->hls_stream); i++)
    {
        hls_stream_t *hls;
        hls = (hls_stream_t *)vlc_array_item_at_index(p_sys->hls_stream, i);
        if (hls) hls_Free(hls);
    }
    vlc_array_destroy(p_sys->hls_stream);

    /* */
    vlc_UrlClean(&p_sys->m3u8);
    free(p_sys);
    return VLC_EGENERIC;
}

/****************************************************************************
 * Close
 ****************************************************************************/
static void Close(vlc_object_t *p_this)
{
    stream_t *s = (stream_t*)p_this;
    stream_sys_t *p_sys = s->p_sys;

    assert(p_sys->hls_stream);

    /* */
    vlc_mutex_lock(&p_sys->download.lock_wait);
    vlc_cond_signal(&p_sys->download.wait);
    vlc_mutex_unlock(&p_sys->download.lock_wait);

    /* */
    if (p_sys->b_live)
        vlc_join(p_sys->reload, NULL);
    vlc_join(p_sys->thread, NULL);
    vlc_mutex_destroy(&p_sys->download.lock_wait);
    vlc_cond_destroy(&p_sys->download.wait);

    /* Free hls streams */
    for (int i = 0; i < vlc_array_count(p_sys->hls_stream); i++)
    {
        hls_stream_t *hls;
        hls = (hls_stream_t *)vlc_array_item_at_index(p_sys->hls_stream, i);
        if (hls) hls_Free(hls);
    }
    vlc_array_destroy(p_sys->hls_stream);

    /* */
    vlc_UrlClean(&p_sys->m3u8);
    free(p_sys);
}

/****************************************************************************
 * Stream filters functions
 ****************************************************************************/
static segment_t *GetSegment(stream_t *s)
{
    stream_sys_t *p_sys = s->p_sys;
    segment_t *segment = NULL;

    /* Is this segment of the current HLS stream ready? */
    hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->playback.stream);
    if (hls != NULL)
    {
        vlc_mutex_lock(&hls->lock);
        segment = segment_GetSegment(hls, p_sys->playback.segment);
        if (segment != NULL)
        {
            /* This segment is ready? */
            if (segment->data != NULL)
            {
                p_sys->b_cache = hls->b_cache;
                vlc_mutex_unlock(&hls->lock);
                goto check;
            }
        }
        vlc_mutex_unlock(&hls->lock);
    }

    /* Was the HLS stream changed to another bitrate? */
    int i_stream = 0;
    segment = NULL;
    while(vlc_object_alive(s))
    {
        /* Is the next segment ready */
        hls_stream_t *hls = hls_Get(p_sys->hls_stream, i_stream);
        if (hls == NULL)
            return NULL;

        vlc_mutex_lock(&hls->lock);
        segment = segment_GetSegment(hls, p_sys->playback.segment);
        if (segment == NULL)
        {
            vlc_mutex_unlock(&hls->lock);
            break;
        }

        vlc_mutex_lock(&p_sys->download.lock_wait);
        int i_segment = p_sys->download.segment;
        vlc_mutex_unlock(&p_sys->download.lock_wait);

        /* This segment is ready? */
        if ((segment->data != NULL) &&
            (p_sys->playback.segment < i_segment))
        {
            p_sys->playback.stream = i_stream;
            p_sys->b_cache = hls->b_cache;
            vlc_mutex_unlock(&hls->lock);
            goto check;
        }
        vlc_mutex_unlock(&hls->lock);

        if (!p_sys->b_meta)
            break;

        /* Was the stream changed to another bitrate? */
        i_stream++;
        if (i_stream >= vlc_array_count(p_sys->hls_stream))
            break;
    }
    /* */
    return NULL;

check:
    /* sanity check */
    if (segment->data->i_buffer == 0)
    {
        vlc_mutex_lock(&hls->lock);
        int count = vlc_array_count(hls->segments);
        vlc_mutex_unlock(&hls->lock);

        if ((p_sys->download.segment - p_sys->playback.segment == 0) &&
            ((count != p_sys->download.segment) || p_sys->b_live))
            msg_Err(s, "playback will stall");
        else if ((p_sys->download.segment - p_sys->playback.segment < 3) &&
                 ((count != p_sys->download.segment) || p_sys->b_live))
            msg_Warn(s, "playback in danger of stalling");
    }
    return segment;
}

static ssize_t hls_Read(stream_t *s, uint8_t *p_read, unsigned int i_read)
{
    stream_sys_t *p_sys = s->p_sys;
    ssize_t copied = 0;

    do
    {
        /* Determine next segment to read. If this is a meta playlist and
         * bandwidth conditions changed, then the stream might have switched
         * to another bandwidth. */
        segment_t *segment = GetSegment(s);
        if (segment == NULL)
            break;

        vlc_mutex_lock(&segment->lock);
        if (segment->data->i_buffer == 0)
        {
            if (!p_sys->b_cache || p_sys->b_live)
            {
                block_Release(segment->data);
                segment->data = NULL;
            }
            else
            {   /* reset playback pointer to start of buffer */
                uint64_t size = segment->size - segment->data->i_buffer;
                if (size > 0)
                {
                    segment->data->i_buffer += size;
                    segment->data->p_buffer -= size;
                }
            }
            p_sys->playback.segment++;
            vlc_mutex_unlock(&segment->lock);

            /* signal download thread */
            vlc_mutex_lock(&p_sys->download.lock_wait);
            vlc_cond_signal(&p_sys->download.wait);
            vlc_mutex_unlock(&p_sys->download.lock_wait);
            continue;
        }

        if (segment->size == segment->data->i_buffer)
            msg_Info(s, "playing segment %d from stream %d",
                     segment->sequence, p_sys->playback.stream);

        ssize_t len = -1;
        if (i_read <= segment->data->i_buffer)
            len = i_read;
        else if (i_read > segment->data->i_buffer)
            len = segment->data->i_buffer;

        if (len > 0)
        {
            memcpy(p_read + copied, segment->data->p_buffer, len);
            segment->data->i_buffer -= len;
            segment->data->p_buffer += len;
            copied += len;
            i_read -= len;
        }
        vlc_mutex_unlock(&segment->lock);

    } while ((i_read > 0) && vlc_object_alive(s));

    return copied;
}

static int Read(stream_t *s, void *buffer, unsigned int i_read)
{
    stream_sys_t *p_sys = s->p_sys;
    ssize_t length = 0;

    assert(p_sys->hls_stream);

    if (p_sys->b_error)
        return 0;

    if (buffer == NULL)
    {
        /* caller skips data, get big enough buffer */
        msg_Warn(s, "buffer is NULL (allocate %d)", i_read);
        buffer = calloc(1, i_read);
        if (buffer == NULL)
            return 0; /* NO MEMORY left*/
    }

    length = hls_Read(s, (uint8_t*) buffer, i_read);
    if (length < 0)
        return 0;

    p_sys->playback.offset += length;
    return length;
}

static int Peek(stream_t *s, const uint8_t **pp_peek, unsigned int i_peek)
{
    stream_sys_t *p_sys = s->p_sys;
    size_t curlen = 0;
    segment_t *segment;

again:
    segment = GetSegment(s);
    if (segment == NULL)
    {
        msg_Err(s, "segment %d should have been available (stream %d)",
                p_sys->playback.segment, p_sys->playback.stream);
        return 0; /* eof? */
    }

    vlc_mutex_lock(&segment->lock);

    /* remember segment to peek */
    int peek_segment = p_sys->playback.segment;
    do
    {
        if (i_peek < segment->data->i_buffer)
        {
            *pp_peek = segment->data->p_buffer;
            curlen += i_peek;
        }
        else
        {
            p_sys->playback.segment++;
            vlc_mutex_unlock(&segment->lock);
            goto again;
        }
    } while ((curlen < i_peek) && vlc_object_alive(s));

    /* restore segment to read */
    p_sys->playback.segment = peek_segment;

    vlc_mutex_unlock(&segment->lock);

    return curlen;
}

static bool hls_MaySeek(stream_t *s)
{
    stream_sys_t *p_sys = s->p_sys;

    if (p_sys->hls_stream == NULL)
        return false;

    hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->playback.stream);
    if (hls == NULL) return false;

    if (p_sys->b_live)
    {
        vlc_mutex_lock(&hls->lock);
        int count = vlc_array_count(hls->segments);
        vlc_mutex_unlock(&hls->lock);

        vlc_mutex_lock(&p_sys->download.lock_wait);
        bool may_seek = (p_sys->download.segment < (count - 2));
        vlc_mutex_unlock(&p_sys->download.lock_wait);
        return may_seek;
    }
    return true;
}

static uint64_t GetStreamSize(stream_t *s)
{
    stream_sys_t *p_sys = s->p_sys;

    if (p_sys->b_live)
        return 0;

    hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->playback.stream);
    if (hls == NULL) return 0;

    vlc_mutex_lock(&hls->lock);
    uint64_t size = hls->size;
    vlc_mutex_unlock(&hls->lock);

    return size;
}

static int segment_Seek(stream_t *s, const uint64_t pos)
{
    stream_sys_t *p_sys = s->p_sys;

    hls_stream_t *hls = hls_Get(p_sys->hls_stream, p_sys->playback.stream);
    if (hls == NULL)
        return VLC_EGENERIC;

    vlc_mutex_lock(&hls->lock);

    bool b_found = false;
    uint64_t length = 0;
    uint64_t size = hls->size;
    int count = vlc_array_count(hls->segments);

    for (int n = 0; n < count; n++)
    {
        segment_t *segment = vlc_array_item_at_index(hls->segments, n);
        if (segment == NULL)
        {
            vlc_mutex_unlock(&hls->lock);
            return VLC_EGENERIC;
        }

        vlc_mutex_lock(&segment->lock);
        length += segment->duration * (hls->bandwidth/8);
        vlc_mutex_unlock(&segment->lock);

        if (!b_found && (pos <= length))
        {
            if (count - n >= 3)
            {
                p_sys->playback.segment = n;
                b_found = true;
                break;
            }
            /* Do not search in last 3 segments */
            vlc_mutex_unlock(&hls->lock);
            return VLC_EGENERIC;
        }
    }

    /* */
    if (!b_found && (pos >= size))
    {
        p_sys->playback.segment = count - 1;
        b_found = true;
    }

    /* */
    if (b_found)
    {
        /* restore segment to start position */
        segment_t *segment = segment_GetSegment(hls, p_sys->playback.segment);
        if (segment == NULL)
        {
            vlc_mutex_unlock(&hls->lock);
            return VLC_EGENERIC;
        }

        vlc_mutex_lock(&segment->lock);
        if (segment->data)
        {
            uint64_t size = segment->size -segment->data->i_buffer;
            if (size > 0)
            {
                segment->data->i_buffer += size;
                segment->data->p_buffer -= size;
            }
        }
        vlc_mutex_unlock(&segment->lock);

        /* start download at current playback segment */
        vlc_mutex_unlock(&hls->lock);

        /* Wake up download thread */
        vlc_mutex_lock(&p_sys->download.lock_wait);
        p_sys->download.seek = p_sys->playback.segment;
        vlc_cond_signal(&p_sys->download.wait);
        vlc_mutex_unlock(&p_sys->download.lock_wait);

        /* Wait for download to be finished */
        vlc_mutex_lock(&p_sys->download.lock_wait);
        msg_Info(s, "seek to segment %d", p_sys->playback.segment);
        while (((p_sys->download.seek != -1) ||
                (p_sys->download.segment - p_sys->playback.segment < 3)) &&
                (p_sys->download.segment < (count - 6)))
        {
            vlc_cond_wait(&p_sys->download.wait, &p_sys->download.lock_wait);
            if (!vlc_object_alive(s) || s->b_error) break;
        }
        vlc_mutex_unlock(&p_sys->download.lock_wait);

        return VLC_SUCCESS;
    }
    vlc_mutex_unlock(&hls->lock);

    return b_found ? VLC_SUCCESS : VLC_EGENERIC;
}

static int Control(stream_t *s, int i_query, va_list args)
{
    stream_sys_t *p_sys = s->p_sys;

    switch (i_query)
    {
        case STREAM_CAN_SEEK:
        case STREAM_CAN_FASTSEEK:
            *(va_arg (args, bool *)) = hls_MaySeek(s);
            break;
        case STREAM_GET_POSITION:
            *(va_arg (args, uint64_t *)) = p_sys->playback.offset;
            break;
        case STREAM_SET_POSITION:
            if (hls_MaySeek(s))
            {
                uint64_t pos = (uint64_t)va_arg(args, uint64_t);
                if (segment_Seek(s, pos) == VLC_SUCCESS)
                {
                    p_sys->playback.offset = pos;
                    break;
                }
            }
            return VLC_EGENERIC;
        case STREAM_GET_SIZE:
            *(va_arg (args, uint64_t *)) = GetStreamSize(s);
            break;
        default:
            return VLC_EGENERIC;
    }
    return VLC_SUCCESS;
}
