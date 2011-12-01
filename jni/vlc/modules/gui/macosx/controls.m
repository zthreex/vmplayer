/*****************************************************************************
 * controls.m: MacOS X interface module
 *****************************************************************************
 * Copyright (C) 2002-2011 the VideoLAN team
 * $Id: d331bb19795320cda05278f9a4618c53c4f9a5de $
 *
 * Authors: Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Christophe Massiot <massiot@via.ecp.fr>
 *          Derk-Jan Hartman <hartman at videolan dot org>
 *          Benjamin Pracht <bigben at videolan doit org>
 *          Felix Paul Kühne <fkuehne at videolan dot org>
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
#include <stdlib.h>                                      /* malloc(), free() */
#include <sys/param.h>                                    /* for MAXPATHLEN */
#include <string.h>

#import "intf.h"
#import "vout.h"
#import "open.h"
#import "controls.h"
#import "playlist.h"
#import "CoreInteraction.h"
#import <vlc_keys.h>

#pragma mark -
/*****************************************************************************
 * VLCControls implementation
 *****************************************************************************/
@implementation VLCControls

- (id)init
{
    [super init];
    o_fs_panel = [[VLCFSPanel alloc] init];
    return self;
}

- (void)awakeFromNib
{
    [o_specificTime_mi setTitle: _NS("Jump To Time")];
    [o_specificTime_cancel_btn setTitle: _NS("Cancel")];
    [o_specificTime_ok_btn setTitle: _NS("OK")];
    [o_specificTime_sec_lbl setStringValue: _NS("sec.")];
    [o_specificTime_goTo_lbl setStringValue: _NS("Jump to time")];
}


- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];

    [super dealloc];
}

- (IBAction)play:(id)sender
{
    [[VLCCoreInteraction sharedInstance] play];
}

- (IBAction)stop:(id)sender
{
    [[VLCCoreInteraction sharedInstance] stop];
}

- (IBAction)faster:(id)sender
{
    [[VLCCoreInteraction sharedInstance] faster];
}

- (IBAction)slower:(id)sender
{
    [[VLCCoreInteraction sharedInstance] slower];
}

- (IBAction)normalSpeed:(id)sender
{
    [[VLCCoreInteraction sharedInstance] normalSpeed];
}

- (IBAction)prev:(id)sender
{
    [[VLCCoreInteraction sharedInstance] previous];
}

- (IBAction)next:(id)sender
{
    [[VLCCoreInteraction sharedInstance] next];
}

- (IBAction)random:(id)sender
{
    [[VLCCoreInteraction sharedInstance] shuffle];

    vlc_value_t val;
    playlist_t *p_playlist = pl_Get( VLCIntf );
    var_Get( p_playlist, "random", &val );
	if(val.b_bool) {
        [o_btn_shuffle setImage: [NSImage imageNamed:@"shuffle-on"]];
        [o_btn_shuffle setAlternateImage: [NSImage imageNamed:@"shuffle-blue-pressed"]];
    }
    else
    {
        [o_btn_shuffle setImage: [NSImage imageNamed:@"shuffle"]];
        [o_btn_shuffle setAlternateImage: [NSImage imageNamed:@"shuffle-pressed"]];
    }
}

- (IBAction)repeatButtonAction:(id)sender
{
    vlc_value_t looping,repeating;
    intf_thread_t * p_intf = VLCIntf;
    playlist_t * p_playlist = pl_Get( p_intf );

    var_Get( p_playlist, "repeat", &repeating );
    var_Get( p_playlist, "loop", &looping );

    if( !repeating.b_bool && !looping.b_bool )
    {
        /* was: no repeating at all, switching to Repeat One */
        [[VLCCoreInteraction sharedInstance] repeatOne];

        [o_btn_repeat setImage: [NSImage imageNamed:@"repeat-one"]];
        [o_btn_repeat setAlternateImage: [NSImage imageNamed:@"repeat-one-pressed"]];
    }
    else if( repeating.b_bool && !looping.b_bool )
    {
        /* was: Repeat One, switching to Repeat All */
        [[VLCCoreInteraction sharedInstance] repeatAll];

        [o_btn_repeat setImage: [NSImage imageNamed:@"repeat-all"]];
        [o_btn_repeat setAlternateImage: [NSImage imageNamed:@"repeat-all-pressed"]];
    }
    else
    {
        /* was: Repeat All or bug in VLC, switching to Repeat Off */
        [[VLCCoreInteraction sharedInstance] repeatOff];

        [o_btn_repeat setImage: [NSImage imageNamed:@"repeat"]];
        [o_btn_repeat setAlternateImage: [NSImage imageNamed:@"repeat-pressed"]];
    }
}


- (IBAction)repeat:(id)sender
{
    vlc_value_t val;
    intf_thread_t * p_intf = VLCIntf;
    playlist_t * p_playlist = pl_Get( p_intf );

    var_Get( p_playlist, "repeat", &val );
    if(! val.b_bool )
        [[VLCCoreInteraction sharedInstance] repeatOne];
    else
        [[VLCCoreInteraction sharedInstance] repeatOff];
}

- (IBAction)loop:(id)sender
{
    vlc_value_t val;
    intf_thread_t * p_intf = VLCIntf;
    playlist_t * p_playlist = pl_Get( p_intf );

    var_Get( p_playlist, "loop", &val );
    if(! val.b_bool )
        [[VLCCoreInteraction sharedInstance] repeatAll];
    else
        [[VLCCoreInteraction sharedInstance] repeatOff];
}

- (IBAction)quitAfterPlayback:(id)sender
{
    vlc_value_t val;
    playlist_t * p_playlist = pl_Get( VLCIntf );
    var_ToggleBool( p_playlist, "play-and-exit" );
}

- (IBAction)forward:(id)sender
{
    [[VLCCoreInteraction sharedInstance] forward];
}

- (IBAction)backward:(id)sender
{
    [[VLCCoreInteraction sharedInstance] backward];
}


- (IBAction)volumeUp:(id)sender
{
    [[VLCCoreInteraction sharedInstance] volumeUp];
}

- (IBAction)volumeDown:(id)sender
{
    [[VLCCoreInteraction sharedInstance] volumeDown];
}

- (IBAction)mute:(id)sender
{
    [[VLCCoreInteraction sharedInstance] mute];
}

- (IBAction)volumeSliderUpdated:(id)sender
{
    [[VLCCoreInteraction sharedInstance] setVolume: [sender intValue]];
}

- (IBAction)showPosition: (id)sender
{
    input_thread_t * p_input = pl_CurrentInput( VLCIntf );
    if( p_input != NULL )
    {
        vout_thread_t *p_vout = input_GetVout( p_input );
        if( p_vout != NULL )
        {
            var_SetInteger( VLCIntf->p_libvlc, "key-action", ACTIONID_POSITION );
            vlc_object_release( (vlc_object_t *)p_vout );
        }
        vlc_object_release( p_input );
    }
}

- (IBAction)toogleFullscreen:(id)sender {
    [[VLCCoreInteraction sharedInstance] toggleFullscreen];
}

- (BOOL) isFullscreen {
    id o_vout_view = [[VLCCoreInteraction sharedInstance] voutView];
    if( o_vout_view )
    {
        return [o_vout_view isFullscreen];
    }
    return NO;
}

- (IBAction)windowAction:(id)sender
{
    NSString *o_title = [sender title];
    input_thread_t * p_input = pl_CurrentInput( VLCIntf );

    if( p_input != NULL )
    {
        vout_thread_t *p_vout = input_GetVout( p_input );
        if( p_vout != NULL )
        {
            id o_vout_view = [[VLCCoreInteraction sharedInstance] voutView];
            if( o_vout_view )
            {
                if( [o_title isEqualToString: _NS("Half Size") ] )
                    [o_vout_view scaleWindowWithFactor: 0.5 animate: YES];
                else if( [o_title isEqualToString: _NS("Normal Size") ] )
                    [o_vout_view scaleWindowWithFactor: 1.0 animate: YES];
                else if( [o_title isEqualToString: _NS("Double Size") ] )
                    [o_vout_view scaleWindowWithFactor: 2.0 animate: YES];
                else if( [o_title isEqualToString: _NS("Float on Top") ] )
                    [o_vout_view toggleFloatOnTop];
                else if( [o_title isEqualToString: _NS("Fit to Screen") ] )
                {
                    id o_window = [o_vout_view voutWindow];
                    if( ![o_window isZoomed] )
                        [o_window performZoom:self];
                }
                else if( [o_title isEqualToString: _NS("Snapshot") ] )
                {
                    [o_vout_view snapshot];
                }
                else
                {
                    [[VLCCoreInteraction sharedInstance] toggleFullscreen];
                }
            }
            vlc_object_release( (vlc_object_t *)p_vout );
        }
        else
        {
            [[VLCCoreInteraction sharedInstance] toggleFullscreen];
        }
        vlc_object_release( p_input );
    }
}

- (IBAction)telxTransparent:(id)sender
{
    vlc_object_t *p_vbi;
    p_vbi = (vlc_object_t *) vlc_object_find_name( pl_Get( VLCIntf ), "zvbi" );
    if( p_vbi )
    {
        var_SetBool( p_vbi, "vbi-opaque", [sender state] );
        [sender setState: ![sender state]];
        vlc_object_release( p_vbi );
    }
}

- (IBAction)telxNavLink:(id)sender
{
    intf_thread_t * p_intf = VLCIntf;
    vlc_object_t *p_vbi;
    int i_page = 0;

    if( [[sender title] isEqualToString: _NS("Index")] )
        i_page = 'i' << 16;
    else if( [[sender title] isEqualToString: _NS("Red")] )
        i_page = 'r' << 16;
    else if( [[sender title] isEqualToString: _NS("Green")] )
        i_page = 'g' << 16;
    else if( [[sender title] isEqualToString: _NS("Yellow")] )
        i_page = 'y' << 16;
    else if( [[sender title] isEqualToString: _NS("Blue")] )
        i_page = 'b' << 16;
    if( i_page == 0 ) return;

    p_vbi = (vlc_object_t *) vlc_object_find_name( pl_Get( VLCIntf ), "zvbi" );
    if( p_vbi )
    {
        var_SetInteger( p_vbi, "vbi-page", i_page );
        vlc_object_release( p_vbi );
    }
}

- (IBAction)lockVideosAspectRatio:(id)sender
{
    [[VLCCoreInteraction sharedInstance] setAspectRatioLocked: [sender state]];
    [sender setState: [[VLCCoreInteraction sharedInstance] aspectRatioIsLocked]];
}

- (IBAction)addSubtitleFile:(id)sender
{
    NSInteger i_returnValue = 0;
    input_thread_t * p_input = pl_CurrentInput( VLCIntf );
    if( !p_input ) return;

    input_item_t *p_item = input_GetItem( p_input );
    if( !p_item ) return;

    char *path = input_item_GetURI( p_item );
    if( !path ) path = strdup( "" );

    NSOpenPanel * openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles: YES];
    [openPanel setCanChooseDirectories: NO];
    [openPanel setAllowsMultipleSelection: YES];
    [openPanel setAllowedFileTypes: [NSArray arrayWithObjects: @"cdg",@"@idx",@"srt",@"sub",@"utf",@"ass",@"ssa",@"aqt",@"jss",@"psb",@"rt",@"smi",@"txt",@"smil", nil]];
    [openPanel setDirectoryURL:[NSURL fileURLWithPath:[[NSString stringWithUTF8String:path] stringByExpandingTildeInPath]]];
    i_returnValue = [openPanel runModal];
    free( path );

    if( i_returnValue == NSOKButton )
    {
        NSUInteger c = 0;
        if( !p_input ) return;

        c = [[openPanel URLs] count];

        for (int i = 0; i < c ; i++)
        {
            msg_Dbg( VLCIntf, "loading subs from %s", [[[[openPanel URLs] objectAtIndex: i] path] UTF8String] );
            if( input_AddSubtitle( p_input, [[[[openPanel URLs] objectAtIndex: i] path] UTF8String], TRUE ) )
                msg_Warn( VLCIntf, "unable to load subtitles from '%s'",
                         [[[[openPanel URLs] objectAtIndex: i] path] UTF8String] );
        }
    }
}

- (id)fspanel
{
    if( o_fs_panel )
        return o_fs_panel;
    else
    {
        msg_Err( VLCIntf, "FSPanel is nil" );
        return NULL;
    }
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    intf_thread_t * p_intf = VLCIntf;
    BOOL b_invertedEventFromDevice = NO;
    if ([theEvent respondsToSelector:@selector(isDirectionInvertedFromDevice)])
    {
        if ([theEvent isDirectionInvertedFromDevice])
            b_invertedEventFromDevice = YES;
    }

    float f_yabsvalue = [theEvent deltaY] > 0.0f ? [theEvent deltaY] : -[theEvent deltaY];
    float f_xabsvalue = [theEvent deltaX] > 0.0f ? [theEvent deltaX] : -[theEvent deltaX];
    int i, i_yvlckey, i_xvlckey;

    if (b_invertedEventFromDevice)
    {
        if ([theEvent deltaY] > 0.0f)
            i_yvlckey = KEY_MOUSEWHEELDOWN;
        else
            i_yvlckey = KEY_MOUSEWHEELUP;

        if ([theEvent deltaX] > 0.0f)
            i_xvlckey = KEY_MOUSEWHEELRIGHT;
        else
            i_xvlckey = KEY_MOUSEWHEELLEFT;
    }
    else
    {
        if ([theEvent deltaY] < 0.0f)
            i_yvlckey = KEY_MOUSEWHEELDOWN;
        else
            i_yvlckey = KEY_MOUSEWHEELUP;

        if ([theEvent deltaX] < 0.0f)
            i_xvlckey = KEY_MOUSEWHEELRIGHT;
        else
            i_xvlckey = KEY_MOUSEWHEELLEFT;
    }

    /* Send multiple key event, depending on the intensity of the event */
    for (i = 0; i < (int)(f_yabsvalue/4.+1.) && f_yabsvalue > 0.05 ; i++)
        var_SetInteger( p_intf->p_libvlc, "key-pressed", i_yvlckey );

    /* Prioritize Y event (sound volume) over X event */
    if (f_yabsvalue < 0.05)
    {
        for (i = 0; i < (int)(f_xabsvalue/6.+1.) && f_xabsvalue > 0.05; i++)
         var_SetInteger( p_intf->p_libvlc, "key-pressed", i_xvlckey );
    }
}

- (BOOL)keyEvent:(NSEvent *)o_event
{
    BOOL eventHandled = NO;
    unichar key = [[o_event charactersIgnoringModifiers] characterAtIndex: 0];

    if( key )
    {
        input_thread_t * p_input = pl_CurrentInput( VLCIntf );
        if( p_input != NULL )
        {
            vout_thread_t *p_vout = input_GetVout( p_input );

            if( p_vout != NULL )
            {
                /* Escape */
                if( key == (unichar) 0x1b )
                {
                    id o_vout_view = [[VLCCoreInteraction sharedInstance] voutView];
                    if( o_vout_view && [o_vout_view isFullscreen] )
                    {
                        [o_vout_view toggleFullscreen];
                        eventHandled = YES;
                    }
                }
                else if( key == ' ' )
                {
                    [self play:self];
                    eventHandled = YES;
                }
                vlc_object_release( (vlc_object_t *)p_vout );
            }
            vlc_object_release( p_input );
        }
    }
    return eventHandled;
}

- (IBAction)goToSpecificTime:(id)sender
{
    if( sender == o_specificTime_cancel_btn )
    {
        [NSApp endSheet: o_specificTime_win];
        [o_specificTime_win close];
    }
    else if( sender == o_specificTime_ok_btn )
    {
        input_thread_t * p_input = pl_CurrentInput( VLCIntf );
        if( p_input )
        {
            int64_t timeInSec = 0;
            NSString * fieldContent = [o_specificTime_enter_fld stringValue];
            if( [[fieldContent componentsSeparatedByString: @":"] count] > 1 &&
                [[fieldContent componentsSeparatedByString: @":"] count] <= 3 )
            {
                NSArray * ourTempArray = \
                    [fieldContent componentsSeparatedByString: @":"];

                if( [[fieldContent componentsSeparatedByString: @":"] count] == 3 )
                {
                    timeInSec += ([[ourTempArray objectAtIndex: 0] intValue] * 3600); //h
                    timeInSec += ([[ourTempArray objectAtIndex: 1] intValue] * 60); //m
                    timeInSec += [[ourTempArray objectAtIndex: 2] intValue];        //s
                }
                else
                {
                    timeInSec += ([[ourTempArray objectAtIndex: 0] intValue] * 60); //m
                    timeInSec += [[ourTempArray objectAtIndex: 1] intValue]; //s
                }
            }
            else
                timeInSec = [fieldContent intValue];

            input_Control( p_input, INPUT_SET_TIME, (int64_t)(timeInSec * 1000000));
            vlc_object_release( p_input );
        }

        [NSApp endSheet: o_specificTime_win];
        [o_specificTime_win close];
    }
    else
    {
        input_thread_t * p_input = pl_CurrentInput( VLCIntf );
        if( p_input )
        {
            /* we can obviously only do that if an input is available */
            vlc_value_t pos, length;
            var_Get( p_input, "time", &pos );
            [o_specificTime_enter_fld setIntValue: (pos.i_time / 1000000)];
            var_Get( p_input, "length", &length );
            [o_specificTime_stepper setMaxValue: (length.i_time / 1000000)];

            [NSApp beginSheet: o_specificTime_win modalForWindow: \
                [NSApp mainWindow] modalDelegate: self didEndSelector: nil \
                contextInfo: nil];
            [o_specificTime_win makeKeyWindow];
            vlc_object_release( p_input );
        }
    }
}

@end
