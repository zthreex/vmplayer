/*****************************************************************************
 * cache.c: Plugins cache
 *****************************************************************************
 * Copyright (C) 2001-2007 the VideoLAN team
 * $Id: a2e96c3cc23999a74604448f227a677cdcdf1586 $
 *
 * Authors: Sam Hocevar <sam@zoy.org>
 *          Ethan C. Baldridge <BaldridgeE@cadmus.com>
 *          Hans-Peter Jansen <hpj@urpla.net>
 *          Gildas Bazin <gbazin@videolan.org>
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
#include "libvlc.h"

#include <stdlib.h>                                      /* free(), strtol() */
#include <stdio.h>                                              /* sprintf() */
#include <string.h>                                              /* strdup() */
#include <vlc_plugin.h>
#include <errno.h>

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include <assert.h>

#include "config/configuration.h"

#include <vlc_fs.h>

#include "modules/modules.h"


/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
#ifdef HAVE_DYNAMIC_PLUGINS
static int    CacheLoadConfig  ( module_t *, FILE * );

/* Sub-version number
 * (only used to avoid breakage in dev version when cache structure changes) */
#define CACHE_SUBVERSION_NUM 15

/* Cache filename */
#define CACHE_NAME "plugins.dat"
/* Magic for the cache filename */
#define CACHE_STRING "cache "PACKAGE_NAME" "PACKAGE_VERSION


void CacheDelete( vlc_object_t *obj, const char *dir )
{
    char *path;

    assert( dir != NULL );

    if( asprintf( &path, "%s"DIR_SEP CACHE_NAME, dir ) == -1 )
        return;
    msg_Dbg( obj, "removing plugins cache file %s", path );
    vlc_unlink( path );
    free( path );
}

/**
 * Loads a plugins cache file.
 *
 * This function will load the plugin cache if present and valid. This cache
 * will in turn be queried by AllocateAllPlugins() to see if it needs to
 * actually load the dynamically loadable module.
 * This allows us to only fully load plugins when they are actually used.
 */
size_t CacheLoad( vlc_object_t *p_this, const char *dir, module_cache_t ***r )
{
    char *psz_filename;
    FILE *file;
    int i_size, i_read;
    char p_cachestring[sizeof(CACHE_STRING)];
    size_t i_cache;
    int32_t i_file_size, i_marker;

    assert( dir != NULL );

    *r = NULL;
    if( asprintf( &psz_filename, "%s"DIR_SEP CACHE_NAME, dir ) == -1 )
        return 0;

    msg_Dbg( p_this, "loading plugins cache file %s", psz_filename );

    file = vlc_fopen( psz_filename, "rb" );
    if( !file )
    {
        msg_Warn( p_this, "cannot read %s (%m)",
                  psz_filename );
        free( psz_filename );
        return 0;
    }
    free( psz_filename );

    /* Check the file size */
    i_read = fread( &i_file_size, 1, sizeof(i_file_size), file );
    if( i_read != sizeof(i_file_size) )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache "
                  "(too short)" );
        fclose( file );
        return 0;
    }

    fseek( file, 0, SEEK_END );
    if( ftell( file ) != i_file_size )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache "
                  "(corrupted size)" );
        fclose( file );
        return 0;
    }
    fseek( file, sizeof(i_file_size), SEEK_SET );

    /* Check the file is a plugins cache */
    i_size = sizeof(CACHE_STRING) - 1;
    i_read = fread( p_cachestring, 1, i_size, file );
    if( i_read != i_size ||
        memcmp( p_cachestring, CACHE_STRING, i_size ) )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache" );
        fclose( file );
        return 0;
    }

#ifdef DISTRO_VERSION
    /* Check for distribution specific version */
    char p_distrostring[sizeof( DISTRO_VERSION )];
    i_size = sizeof( DISTRO_VERSION ) - 1;
    i_read = fread( p_distrostring, 1, i_size, file );
    if( i_read != i_size ||
        memcmp( p_distrostring, DISTRO_VERSION, i_size ) )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache" );
        fclose( file );
        return 0;
    }
#endif

    /* Check Sub-version number */
    i_read = fread( &i_marker, 1, sizeof(i_marker), file );
    if( i_read != sizeof(i_marker) || i_marker != CACHE_SUBVERSION_NUM )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache "
                  "(corrupted header)" );
        fclose( file );
        return 0;
    }

    /* Check header marker */
    i_read = fread( &i_marker, 1, sizeof(i_marker), file );
    if( i_read != sizeof(i_marker) ||
        i_marker != ftell( file ) - (int)sizeof(i_marker) )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache "
                  "(corrupted header)" );
        fclose( file );
        return 0;
    }

    if (fread( &i_cache, 1, sizeof(i_cache), file ) != sizeof(i_cache) )
    {
        msg_Warn( p_this, "This doesn't look like a valid plugins cache "
                  "(file too short)" );
        fclose( file );
        return 0;
    }

    module_cache_t **pp_cache = malloc( i_cache * sizeof(*pp_cache) );
    if( pp_cache == NULL )
        i_cache = 0; /* don't load anything */

#define LOAD_IMMEDIATE(a) \
    if( fread( (void *)&a, sizeof(char), sizeof(a), file ) != sizeof(a) ) goto error
#define LOAD_STRING(a) \
{ \
    a = NULL; \
    if( ( fread( &i_size, sizeof(i_size), 1, file ) != 1 ) \
     || ( i_size > 16384 ) ) \
        goto error; \
    if( i_size ) { \
        char *psz = xmalloc( i_size ); \
        if( fread( psz, i_size, 1, file ) != 1 ) { \
            free( psz ); \
            goto error; \
        } \
        if( psz[i_size-1] ) { \
            free( psz ); \
            goto error; \
        } \
        a = psz; \
    } \
}

    for( size_t i = 0; i < i_cache; i++ )
    {
        uint16_t i_size;
        int i_submodules;

        pp_cache[i] = xmalloc( sizeof(module_cache_t) );

        /* Load common info */
        LOAD_STRING( pp_cache[i]->path );
        LOAD_IMMEDIATE( pp_cache[i]->mtime );
        LOAD_IMMEDIATE( pp_cache[i]->size );

        pp_cache[i]->p_module = vlc_module_create();

        /* Load additional infos */
        free( pp_cache[i]->p_module->psz_object_name );
        LOAD_STRING( pp_cache[i]->p_module->psz_object_name );
        LOAD_STRING( pp_cache[i]->p_module->psz_shortname );
        LOAD_STRING( pp_cache[i]->p_module->psz_longname );
        LOAD_STRING( pp_cache[i]->p_module->psz_help );

        LOAD_IMMEDIATE( pp_cache[i]->p_module->i_shortcuts );
        if( pp_cache[i]->p_module->i_shortcuts > MODULE_SHORTCUT_MAX )
            goto error;
        else if( pp_cache[i]->p_module->i_shortcuts == 0 )
            pp_cache[i]->p_module->pp_shortcuts = NULL;
        else
        {
            pp_cache[i]->p_module->pp_shortcuts =
                    xmalloc( sizeof( char ** ) * pp_cache[i]->p_module->i_shortcuts );
            for( unsigned j = 0; j < pp_cache[i]->p_module->i_shortcuts; j++ )
                LOAD_STRING( pp_cache[i]->p_module->pp_shortcuts[j] );
        }

        LOAD_STRING( pp_cache[i]->p_module->psz_capability );
        LOAD_IMMEDIATE( pp_cache[i]->p_module->i_score );
        LOAD_IMMEDIATE( pp_cache[i]->p_module->b_unloadable );

        /* Config stuff */
        if( CacheLoadConfig( pp_cache[i]->p_module, file ) != VLC_SUCCESS )
            goto error;

        LOAD_STRING( pp_cache[i]->p_module->psz_filename );
        LOAD_STRING( pp_cache[i]->p_module->domain );
        if( pp_cache[i]->p_module->domain != NULL )
            vlc_bindtextdomain( pp_cache[i]->p_module->domain );

        LOAD_IMMEDIATE( i_submodules );

        while( i_submodules-- )
        {
            module_t *p_module = vlc_submodule_create( pp_cache[i]->p_module );
            free( p_module->psz_object_name );
            free( p_module->pp_shortcuts );
            LOAD_STRING( p_module->psz_object_name );
            LOAD_STRING( p_module->psz_shortname );
            LOAD_STRING( p_module->psz_longname );
            LOAD_STRING( p_module->psz_help );

            LOAD_IMMEDIATE( p_module->i_shortcuts );
            if( p_module->i_shortcuts > MODULE_SHORTCUT_MAX )
                goto error;
            else if( p_module->i_shortcuts == 0 )
                p_module->pp_shortcuts = NULL;
            else
            {
                p_module->pp_shortcuts = xmalloc( sizeof( char ** ) * p_module->i_shortcuts );
                for( unsigned j = 0; j < p_module->i_shortcuts; j++ )
                    LOAD_STRING( p_module->pp_shortcuts[j] );
            }

            LOAD_STRING( p_module->psz_capability );
            LOAD_IMMEDIATE( p_module->i_score );
            LOAD_IMMEDIATE( p_module->b_unloadable );
            LOAD_STRING( p_module->domain );
        }
    }
    fclose( file );

    *r = pp_cache;
    return i_cache;

 error:

    msg_Warn( p_this, "plugins cache not loaded (corrupted)" );

    /* TODO: cleanup */
    fclose( file );
    return 0;
}


static int CacheLoadConfig( module_t *p_module, FILE *file )
{
    uint32_t i_lines;
    uint16_t i_size;

    /* Calculate the structure length */
    LOAD_IMMEDIATE( p_module->i_config_items );
    LOAD_IMMEDIATE( p_module->i_bool_items );

    LOAD_IMMEDIATE( i_lines );

    /* Allocate memory */
    if (i_lines)
    {
        p_module->p_config =
            (module_config_t *)calloc( i_lines, sizeof(module_config_t) );
        if( p_module->p_config == NULL )
        {
            p_module->confsize = 0;
            return VLC_ENOMEM;
        }
    }
    p_module->confsize = i_lines;

    /* Do the duplication job */
    for (size_t i = 0; i < i_lines; i++ )
    {
        LOAD_IMMEDIATE( p_module->p_config[i] );

        LOAD_STRING( p_module->p_config[i].psz_type );
        LOAD_STRING( p_module->p_config[i].psz_name );
        LOAD_STRING( p_module->p_config[i].psz_text );
        LOAD_STRING( p_module->p_config[i].psz_longtext );
        LOAD_STRING( p_module->p_config[i].psz_oldname );

        if (IsConfigStringType (p_module->p_config[i].i_type))
        {
            LOAD_STRING (p_module->p_config[i].orig.psz);
            p_module->p_config[i].value.psz =
                    (p_module->p_config[i].orig.psz != NULL)
                        ? strdup (p_module->p_config[i].orig.psz) : NULL;
        }
        else
            memcpy (&p_module->p_config[i].value, &p_module->p_config[i].orig,
                    sizeof (p_module->p_config[i].value));

        p_module->p_config[i].b_dirty = false;

        if( p_module->p_config[i].i_list )
        {
            if( p_module->p_config[i].ppsz_list )
            {
                int j;
                p_module->p_config[i].ppsz_list =
                    xmalloc( (p_module->p_config[i].i_list+1) * sizeof(char *));
                if( p_module->p_config[i].ppsz_list )
                {
                    for( j = 0; j < p_module->p_config[i].i_list; j++ )
                        LOAD_STRING( p_module->p_config[i].ppsz_list[j] );
                    p_module->p_config[i].ppsz_list[j] = NULL;
                }
            }
            if( p_module->p_config[i].ppsz_list_text )
            {
                int j;
                p_module->p_config[i].ppsz_list_text =
                    xmalloc( (p_module->p_config[i].i_list+1) * sizeof(char *));
                if( p_module->p_config[i].ppsz_list_text )
                {
                  for( j = 0; j < p_module->p_config[i].i_list; j++ )
                      LOAD_STRING( p_module->p_config[i].ppsz_list_text[j] );
                  p_module->p_config[i].ppsz_list_text[j] = NULL;
                }
            }
            if( p_module->p_config[i].pi_list )
            {
                p_module->p_config[i].pi_list =
                    xmalloc( (p_module->p_config[i].i_list + 1) * sizeof(int) );
                if( p_module->p_config[i].pi_list )
                {
                    for (int j = 0; j < p_module->p_config[i].i_list; j++)
                        LOAD_IMMEDIATE( p_module->p_config[i].pi_list[j] );
                }
            }
        }

        if( p_module->p_config[i].i_action )
        {
            p_module->p_config[i].ppf_action =
                xmalloc( p_module->p_config[i].i_action * sizeof(void *) );
            p_module->p_config[i].ppsz_action_text =
                xmalloc( p_module->p_config[i].i_action * sizeof(char *) );

            for (int j = 0; j < p_module->p_config[i].i_action; j++)
            {
                p_module->p_config[i].ppf_action[j] = NULL;
                LOAD_STRING( p_module->p_config[i].ppsz_action_text[j] );
            }
        }
    }

    return VLC_SUCCESS;

 error:

    return VLC_EGENERIC;
}

static int CacheSaveBank( FILE *file, module_cache_t *const *, size_t );

/*****************************************************************************
 * SavePluginsCache: saves the plugins cache to a file
 *****************************************************************************/
void CacheSave (vlc_object_t *p_this, const char *dir,
                module_cache_t *const *pp_cache, size_t n)
{
    char *filename, *tmpname;

    if (asprintf (&filename, "%s"DIR_SEP CACHE_NAME, dir ) == -1)
        return;

    if (asprintf (&tmpname, "%s.%"PRIu32, filename, (uint32_t)getpid ()) == -1)
    {
        free (filename);
        return;
    }
    msg_Dbg (p_this, "saving plugins cache %s", filename);

    FILE *file = vlc_fopen (tmpname, "wb");
    if (file == NULL)
    {
        if (errno != EACCES && errno != ENOENT)
            msg_Warn (p_this, "cannot create %s (%m)", tmpname);
        goto out;
    }

    if (CacheSaveBank (file, pp_cache, n))
    {
        msg_Warn (p_this, "cannot write %s (%m)", tmpname);
        clearerr (file);
        fclose (file);
        vlc_unlink (tmpname);
        goto out;
    }

#if !defined( WIN32 ) && !defined( __OS2__ )
    vlc_rename (tmpname, filename); /* atomically replace old cache */
    fclose (file);
#else
    vlc_unlink (filename);
    fclose (file);
    vlc_rename (tmpname, filename);
#endif
out:
    free (filename);
    free (tmpname);
}

static int CacheSaveConfig (FILE *, const module_t *);
static int CacheSaveSubmodule (FILE *, const module_t *);

static int CacheSaveBank (FILE *file, module_cache_t *const *pp_cache,
                          size_t i_cache)
{
    uint32_t i_file_size = 0;

    /* Empty space for file size */
    if (fwrite (&i_file_size, sizeof (i_file_size), 1, file) != 1)
        goto error;

    /* Contains version number */
    if (fputs (CACHE_STRING, file) == EOF)
        goto error;
#ifdef DISTRO_VERSION
    /* Allow binary maintaner to pass a string to detect new binary version*/
    if (fputs( DISTRO_VERSION, file ) == EOF)
        goto error;
#endif
    /* Sub-version number (to avoid breakage in the dev version when cache
     * structure changes) */
    i_file_size = CACHE_SUBVERSION_NUM;
    if (fwrite (&i_file_size, sizeof (i_file_size), 1, file) != 1 )
        goto error;

    /* Header marker */
    i_file_size = ftell( file );
    if (fwrite (&i_file_size, sizeof (i_file_size), 1, file) != 1)
        goto error;

    if (fwrite( &i_cache, sizeof (i_cache), 1, file) != 1)
        goto error;

#define SAVE_IMMEDIATE( a ) \
    if (fwrite (&a, sizeof(a), 1, file) != 1) \
        goto error
#define SAVE_STRING( a ) \
    { \
        uint16_t i_size = (a != NULL) ? (strlen (a) + 1) : 0; \
        if ((fwrite (&i_size, sizeof (i_size), 1, file) != 1) \
         || (a && (fwrite (a, 1, i_size, file) != i_size))) \
            goto error; \
    } while(0)

    for (unsigned i = 0; i < i_cache; i++)
    {
        uint32_t i_submodule;

        /* Save common info */
        SAVE_STRING( pp_cache[i]->path );
        SAVE_IMMEDIATE( pp_cache[i]->mtime );
        SAVE_IMMEDIATE( pp_cache[i]->size );

        /* Save additional infos */
        SAVE_STRING( pp_cache[i]->p_module->psz_object_name );
        SAVE_STRING( pp_cache[i]->p_module->psz_shortname );
        SAVE_STRING( pp_cache[i]->p_module->psz_longname );
        SAVE_STRING( pp_cache[i]->p_module->psz_help );
        SAVE_IMMEDIATE( pp_cache[i]->p_module->i_shortcuts );
        for (unsigned j = 0; j < pp_cache[i]->p_module->i_shortcuts; j++)
            SAVE_STRING( pp_cache[i]->p_module->pp_shortcuts[j] );

        SAVE_STRING( pp_cache[i]->p_module->psz_capability );
        SAVE_IMMEDIATE( pp_cache[i]->p_module->i_score );
        SAVE_IMMEDIATE( pp_cache[i]->p_module->b_unloadable );

        /* Config stuff */
        if (CacheSaveConfig (file, pp_cache[i]->p_module))
            goto error;

        SAVE_STRING( pp_cache[i]->p_module->psz_filename );
        SAVE_STRING( pp_cache[i]->p_module->domain );

        i_submodule = pp_cache[i]->p_module->submodule_count;
        SAVE_IMMEDIATE( i_submodule );
        if( CacheSaveSubmodule( file, pp_cache[i]->p_module->submodule ) )
            goto error;
    }

    /* Fill-up file size */
    i_file_size = ftell( file );
    fseek( file, 0, SEEK_SET );
    if (fwrite (&i_file_size, sizeof (i_file_size), 1, file) != 1
     || fflush (file)) /* flush libc buffers */
        goto error;
    return 0; /* success! */

error:
    return -1;
}

static int CacheSaveSubmodule( FILE *file, const module_t *p_module )
{
    if( !p_module )
        return 0;
    if( CacheSaveSubmodule( file, p_module->next ) )
        goto error;

    SAVE_STRING( p_module->psz_object_name );
    SAVE_STRING( p_module->psz_shortname );
    SAVE_STRING( p_module->psz_longname );
    SAVE_STRING( p_module->psz_help );
    SAVE_IMMEDIATE( p_module->i_shortcuts );
    for( unsigned j = 0; j < p_module->i_shortcuts; j++ )
         SAVE_STRING( p_module->pp_shortcuts[j] );

    SAVE_STRING( p_module->psz_capability );
    SAVE_IMMEDIATE( p_module->i_score );
    SAVE_IMMEDIATE( p_module->b_unloadable );
    SAVE_STRING( p_module->domain );
    return 0;

error:
    return -1;
}


static int CacheSaveConfig (FILE *file, const module_t *p_module)
{
    uint32_t i_lines = p_module->confsize;

    SAVE_IMMEDIATE( p_module->i_config_items );
    SAVE_IMMEDIATE( p_module->i_bool_items );
    SAVE_IMMEDIATE( i_lines );

    for (size_t i = 0; i < i_lines ; i++)
    {
        SAVE_IMMEDIATE( p_module->p_config[i] );

        SAVE_STRING( p_module->p_config[i].psz_type );
        SAVE_STRING( p_module->p_config[i].psz_name );
        SAVE_STRING( p_module->p_config[i].psz_text );
        SAVE_STRING( p_module->p_config[i].psz_longtext );
        SAVE_STRING( p_module->p_config[i].psz_oldname );

        if (IsConfigStringType (p_module->p_config[i].i_type))
            SAVE_STRING( p_module->p_config[i].orig.psz );

        if( p_module->p_config[i].i_list )
        {
            if( p_module->p_config[i].ppsz_list )
            {
                for (int j = 0; j < p_module->p_config[i].i_list; j++)
                    SAVE_STRING( p_module->p_config[i].ppsz_list[j] );
            }

            if( p_module->p_config[i].ppsz_list_text )
            {
                for (int j = 0; j < p_module->p_config[i].i_list; j++)
                    SAVE_STRING( p_module->p_config[i].ppsz_list_text[j] );
            }
            if( p_module->p_config[i].pi_list )
            {
                for (int j = 0; j < p_module->p_config[i].i_list; j++)
                    SAVE_IMMEDIATE( p_module->p_config[i].pi_list[j] );
            }
        }

        for (int j = 0; j < p_module->p_config[i].i_action; j++)
            SAVE_STRING( p_module->p_config[i].ppsz_action_text[j] );
    }
    return 0;

error:
    return -1;
}

/*****************************************************************************
 * CacheMerge: Merge a cache module descriptor with a full module descriptor.
 *****************************************************************************/
void CacheMerge( vlc_object_t *p_this, module_t *p_cache, module_t *p_module )
{
    (void)p_this;

    p_cache->pf_activate = p_module->pf_activate;
    p_cache->pf_deactivate = p_module->pf_deactivate;
    p_cache->handle = p_module->handle;

    /* FIXME: This looks too simplistic an algorithm to me. What if the module
     * file was altered such that the number of order of submodules was
     * altered... after VLC started -- Courmisch, 09/2008 */
    module_t *p_child = p_module->submodule,
             *p_cchild = p_cache->submodule;
    while( p_child && p_cchild )
    {
        p_cchild->pf_activate = p_child->pf_activate;
        p_cchild->pf_deactivate = p_child->pf_deactivate;
        p_child = p_child->next;
        p_cchild = p_cchild->next;
    }

    p_cache->b_loaded = true;
    p_module->b_loaded = false;
}

/*****************************************************************************
 * CacheFind: finds the cache entry corresponding to a file
 *****************************************************************************/
module_t *CacheFind( module_bank_t *p_bank,
                     const char *path, time_t mtime, off_t size )
{
    module_cache_t **cache = p_bank->pp_loaded_cache;
    size_t n = p_bank->i_loaded_cache;

    for( size_t i = 0; i < n; i++ )
        if( !strcmp( cache[i]->path, path )
         && cache[i]->mtime == mtime
         && cache[i]->size == size )
       {
            module_t *module = cache[i]->p_module;
            cache[i]->p_module = NULL;
            return module;
       }

    return NULL;
}

#endif /* HAVE_DYNAMIC_PLUGINS */
