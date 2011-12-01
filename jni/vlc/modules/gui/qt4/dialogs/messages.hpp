/*****************************************************************************
 * Messages.hpp : Information about a stream
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id: 3b9f66cd754df8e2d5bc315b3f40a17195ed3d57 $
 *
 * Authors: Jean-Baptiste Kempf <jb (at) videolan.org>
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

#ifndef QVLC_MESSAGES_DIALOG_H_
#define QVLC_MESSAGES_DIALOG_H_ 1

#include "util/qvlcframe.hpp"
#include "util/singleton.hpp"
#include "ui/messages_panel.h"

class QTabWidget;
class QPushButton;
class QSpinBox;
class QGridLayout;
class QLabel;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;
class MsgEvent;

class MessagesDialog : public QVLCFrame, public Singleton<MessagesDialog>
{
    Q_OBJECT
private:
    MessagesDialog( intf_thread_t * );
    virtual ~MessagesDialog();

    Ui::messagesPanelWidget ui;
    msg_subscription_t *sub;
    msg_cb_data_t *cbData;
    static void sinkMessage( msg_cb_data_t *, msg_item_t *, unsigned );
    void customEvent( QEvent * );
    void sinkMessage( MsgEvent * );

private slots:
    bool save();
    void updateConfig();
    void changeVerbosity( int );
    void clear();
    void updateTree();
    void tabChanged( int );

private:
    void buildTree( QTreeWidgetItem *, vlc_object_t * );

    friend class    Singleton<MessagesDialog>;
    QPushButton *updateButton;
};

#endif
