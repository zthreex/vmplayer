/*****************************************************************************
 * vlc_model.hpp : base for playlist and ml model
 ****************************************************************************
 * Copyright (C) 2010 the VideoLAN team and AUTHORS
 * $Id: 450616927801746bbaf96fe1c3af8c6c87dfe40b $
 *
 * Authors: Srikanth Raju <srikiraju#gmail#com>
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

#ifndef _VLC_MODEL_H_
#define _VLC_MODEL_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "qt4.hpp"
#include "sorting.h"

#include <vlc_input.h>

#include <QModelIndex>
#include <QPixmapCache>
#include <QSize>
#include <QAbstractItemModel>


class VLCModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum {
      IsCurrentRole = Qt::UserRole,
      IsLeafNodeRole,
      IsCurrentsParentNodeRole
    };

    VLCModel( intf_thread_t *_p_intf, QObject *parent = 0 );
    virtual int getId( QModelIndex index ) const = 0;
    virtual QModelIndex currentIndex() const = 0;
    virtual bool popup( const QModelIndex & index,
            const QPoint &point, const QModelIndexList &list ) = 0;
    virtual void doDelete( QModelIndexList ) = 0;
    virtual ~VLCModel();
    static QString getMeta( const QModelIndex & index, int meta );
    static QPixmap getArtPixmap( const QModelIndex & index, const QSize & size );

    static int columnToMeta( int _column )
    {
        int meta = 1;
        int column = 0;

        while( column != _column && meta != COLUMN_END )
        {
            meta <<= 1;
            column++;
        }

        return meta;
    }

    static int columnFromMeta( int meta_col )
    {
        int meta = 1;
        int column = 0;

        while( meta != meta_col && meta != COLUMN_END )
        {
            meta <<= 1;
            column++;
        }

        return column;
    }

public slots:
    virtual void activateItem( const QModelIndex &index ) = 0;

protected:
    intf_thread_t *p_intf;

};


#endif
