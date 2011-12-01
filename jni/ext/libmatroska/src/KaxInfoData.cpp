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
	\version \$Id: KaxInfoData.cpp 725 2011-03-27 17:09:02Z robux4 $
	\author Steve Lhomme     <robux4 @ users.sf.net>
	\author John Cannon      <spyder2555 @ users.sf.net>
*/
#include "matroska/KaxInfoData.h"
#include "matroska/KaxContexts.h"
#include "matroska/KaxDefines.h"
#include "matroska/KaxSemantic.h"

START_LIBMATROSKA_NAMESPACE

KaxPrevUID::KaxPrevUID(EBML_EXTRA_DEF)
:KaxSegmentUID(EBML_DEF_BINARY_CTX(KaxPrevUID) EBML_DEF_SEP EBML_EXTRA_CALL)
{
}

KaxNextUID::KaxNextUID(EBML_EXTRA_DEF)
:KaxSegmentUID(EBML_DEF_BINARY_CTX(KaxNextUID) EBML_DEF_SEP EBML_EXTRA_CALL)
{
}

#if defined(HAVE_EBML2) || defined(HAS_EBML2)
KaxSegmentUID::KaxSegmentUID(EBML_DEF_CONS EBML_DEF_SEP EBML_EXTRA_DEF)
:EbmlBinary(EBML_DEF_PARAM EBML_DEF_SEP EBML_EXTRA_CALL)
{
}
#endif

END_LIBMATROSKA_NAMESPACE
