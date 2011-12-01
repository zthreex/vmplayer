/*****************************************************************************
 * postprocessing.h: Post processing related helpers
 *****************************************************************************
 * Copyright (C) 2010 Laurent Aimar
 * $Id: 2915363015af084c6a67e46246a3d3a09f3e5ec9 $
 *
 * Authors: Laurent Aimar <fenrir _AT_ videolan _DOT_ org>
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

#ifndef LIBVLC_VOUT_POSTPROCESSING_H
#define LIBVLC_VOUT_POSTPROCESSING_H

typedef struct {
    int qtype;
} vout_postprocessing_support_t;

void vout_SetPostProcessingState(vout_thread_t *, vout_postprocessing_support_t *, int qtype);

#endif
