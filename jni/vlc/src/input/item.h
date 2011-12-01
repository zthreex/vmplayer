/*****************************************************************************
 * item.h
 *****************************************************************************
 * Copyright (C) 2008 Laurent Aimar
 * $Id: 9b021366f9b59b73ef7e3300c548e79811e5691c $
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

#ifndef LIBVLC_INPUT_ITEM_H
#define LIBVLC_INPUT_ITEM_H 1

#include "input_interface.h"

void input_item_SetErrorWhenReading( input_item_t *p_i, bool b_error );
void input_item_UpdateTracksInfo( input_item_t *item, const es_format_t *fmt );

#endif
