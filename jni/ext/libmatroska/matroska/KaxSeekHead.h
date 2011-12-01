/****************************************************************************
** libmatroska : parse Matroska files, see http://www.matroska.org/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libmatroska.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** See http://www.matroska.org/license/lgpl/ for LGPL licensing information.**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
	\file
	\version \$Id: KaxSeekHead.h 725 2011-03-27 17:09:02Z robux4 $
	\author Steve Lhomme     <robux4 @ users.sf.net>
*/
#ifndef LIBMATROSKA_SEEK_HEAD_H
#define LIBMATROSKA_SEEK_HEAD_H

#include "matroska/KaxTypes.h"
#include "ebml/EbmlMaster.h"
#include "ebml/EbmlBinary.h"
#include "ebml/EbmlUInteger.h"
#include "matroska/KaxDefines.h"

using namespace LIBEBML_NAMESPACE;

START_LIBMATROSKA_NAMESPACE

class KaxSegment;

DECLARE_MKX_MASTER(KaxSeek)
	public:
		int64 Location() const;
		bool IsEbmlId(const EbmlId & aId) const;
		bool IsEbmlId(const KaxSeek & aPoint) const;
};

DECLARE_MKX_MASTER(KaxSeekHead)
	public:
		/*!
			\brief add an element to index in the Meta Seek data
			\note the element should already be written in the file
		*/
		void IndexThis(const EbmlElement & aElt, const KaxSegment & ParentSegment);

		KaxSeek * FindFirstOf(const EbmlCallbacks & Callbacks) const;
		KaxSeek * FindNextOf(const KaxSeek &aPrev) const;
};

END_LIBMATROSKA_NAMESPACE

#endif // LIBMATROSKA_SEEK_HEAD_H
