/*****************************************************************************
 * es_out_timeshift.h: Es Out timeshift.
 *****************************************************************************
 * Copyright (C) 2008 Laurent Aimar
 * $Id: d554fa7a22d5347d6a9a3aec18503c0f623cb589 $
 *
 * Authors: Laurent Aimar < fenrir _AT_ via _DOT_ ecp _DOT_ fr>
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

#ifndef LIBVLC_INPUT_ES_OUT_TIMESHIFT_H
#define LIBVLC_INPUT_ES_OUT_TIMESHIFT_H 1

#include <vlc_common.h>


es_out_t *input_EsOutTimeshiftNew( input_thread_t *, es_out_t *, int i_rate );

#endif
