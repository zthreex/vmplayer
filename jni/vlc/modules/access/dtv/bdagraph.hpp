/*****************************************************************************
 * bdagraph.h : DirectShow BDA graph builder header for vlc
 *****************************************************************************
 * Copyright ( C ) 2007 the VideoLAN team
 *
 * Author: Ken Self <kenself(at)optusnet(dot)com(dot)au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
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

using namespace std;
#ifndef _MSC_VER
#   include <wtypes.h>
#   include <unknwn.h>
#   include <ole2.h>
#   include <limits.h>
#   ifdef _WINGDI_
#      undef _WINGDI_
#   endif
#   define _WINGDI_ 1
#   define AM_NOVTABLE
#   define _OBJBASE_H_
#   undef _X86_
#   ifndef _I64_MAX
#      define _I64_MAX 0x7FFFFFFFFFFFFFFFLL
#   endif
#   define LONGLONG long long
/* Work-around a bug in w32api-2.5 */
/* #   define QACONTAINERFLAGS QACONTAINERFLAGS_SOMETHINGELSE */
#endif

/* Needed to call CoInitializeEx */
#define _WIN32_DCOM

#include <dshow.h>
#include <comcat.h>
#include "dtv/bdadefs.h"

class BDAOutput
{
public:
    BDAOutput( vlc_object_t * );
    ~BDAOutput();

    void    Push( block_t * );
    ssize_t Pop(void *, size_t);
    void    Empty();

private:
    vlc_object_t *p_access;
    vlc_mutex_t   lock;
    vlc_cond_t    wait;
    block_t      *p_first;
    block_t     **pp_next;
};

/* The main class for building the filter graph */
class BDAGraph : public ISampleGrabberCB
{
public:
    BDAGraph( vlc_object_t * );
    virtual ~BDAGraph();

    /* */
    int SubmitTuneRequest(void);

    int SetCQAM(long);
    int SetATSC(long);
    int SetDVBT(long, uint32_t, uint32_t, long, int, uint32_t, int);
    int SetDVBC(long, const char *, long);
    int SetDVBS(long, long, uint32_t, int, char, long, long, long);

    /* */
    ssize_t Pop(void *, size_t);

private:
    /* ISampleGrabberCB methods */
    ULONG ul_cbrc;
    STDMETHODIMP_( ULONG ) AddRef( ) { return ++ul_cbrc; }
    STDMETHODIMP_( ULONG ) Release( ) { return --ul_cbrc; }
    STDMETHODIMP QueryInterface( REFIID /*riid*/, void** /*p_p_object*/ )
        { return E_NOTIMPL; }
    STDMETHODIMP SampleCB( double d_time, IMediaSample* p_sample );
    STDMETHODIMP BufferCB( double d_time, BYTE* p_buffer, long l_buffer_len );

    vlc_object_t *p_access;
    CLSID     guid_network_type;
    long      l_tuner_used;        /* Index of the Tuning Device */
    /* registration number for the RunningObjectTable */
    DWORD     d_graph_register;

    BDAOutput       output;

    IMediaControl*  p_media_control;
    IGraphBuilder*  p_filter_graph;
    ITuningSpace*   p_tuning_space;
    ITuneRequest*   p_tune_request;

    ICreateDevEnum* p_system_dev_enum;
    IBaseFilter*    p_network_provider;
    IScanningTuner* p_scanning_tuner;
    IBaseFilter*    p_tuner_device;
    IBaseFilter*    p_capture_device;
    IBaseFilter*    p_sample_grabber;
    IBaseFilter*    p_mpeg_demux;
    IBaseFilter*    p_transport_info;
    ISampleGrabber* p_grabber;

    HRESULT CreateTuneRequest( );
    HRESULT Build( );
    HRESULT FindFilter( REFCLSID clsid, long* i_moniker_used,
        IBaseFilter* p_upstream, IBaseFilter** p_p_downstream );
    HRESULT Connect( IBaseFilter* p_filter_upstream,
        IBaseFilter* p_filter_downstream );
    HRESULT Start( );
    HRESULT Destroy( );
    HRESULT Register( );
    void Deregister( );
};
