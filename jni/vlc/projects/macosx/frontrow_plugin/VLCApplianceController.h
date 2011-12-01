/*****************************************************************************
 * VLCApplianceController.h: Front Row plugin
 *****************************************************************************
 * Copyright (C) 2007 - 2008 the VideoLAN Team
 * $Id: ab6ae7f5fcbd5b05086fef39647a8f982f4b8370 $
 *
 * Authors: hyei
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

#import <Cocoa/Cocoa.h>
#import <BackRow/BRMenuController.h>

@interface VLCApplianceController : BRMenuController
{
    NSString * _path;
    NSMutableArray * _contents;
}

- initWithPath:(NSString*)path;

@end
