/*****************************************************************************
 * threads.c : threads implementation for the VideoLAN client
 *****************************************************************************
 * Copyright (C) 1999-2008 the VideoLAN team
 * $Id: 7f4a29992cdf826a01bfd810f6f58ab65c3bb67c $
 *
 * Authors: Jean-Marc Dressler <polux@via.ecp.fr>
 *          Samuel Hocevar <sam@zoy.org>
 *          Gildas Bazin <gbazin@netcourrier.com>
 *          Clément Sténac
 *          Rémi Denis-Courmont
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <assert.h>

/*** Global locks ***/

void vlc_global_mutex (unsigned n, bool acquire)
{
    static vlc_mutex_t locks[] = {
        VLC_STATIC_MUTEX,
        VLC_STATIC_MUTEX,
        VLC_STATIC_MUTEX,
        VLC_STATIC_MUTEX,
        VLC_STATIC_MUTEX,
    };
    assert (n < (sizeof (locks) / sizeof (locks[0])));
    vlc_mutex_t *lock = locks + n;

    if (acquire)
        vlc_mutex_lock (lock);
    else
        vlc_mutex_unlock (lock);

    /* Compile-time assertion ;-) */
    char enough_locks[(sizeof (locks) / sizeof (locks[0])) - VLC_MAX_MUTEX];
    (void) enough_locks;
}
