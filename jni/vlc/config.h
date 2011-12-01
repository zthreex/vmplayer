/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 to allow running VLC as root (uid 0). */
/* #undef ALLOW_RUN_AS_ROOT */

/* Define to 1 if the operating system uses UTF-8 internally */
#define ASSUME_UTF8 1

/* Maximum supported data alignment */
#define ATTRIBUTE_ALIGNED_MAX 64

/* Define to 1 if 3D Now! inline assembly is available. */
/* #undef CAN_COMPILE_3DNOW */

/* Define to 1 if AltiVec inline assembly is available. */
/* #undef CAN_COMPILE_ALTIVEC */

/* Define to 1 if C AltiVec extensions are available. */
/* #undef CAN_COMPILE_C_ALTIVEC */

/* Define to 1 inline MMX assembly is available. */
/* #undef CAN_COMPILE_MMX */

/* Define to 1 if MMX EXT inline assembly is available. */
/* #undef CAN_COMPILE_MMXEXT */

/* Define to 1 if NEON (and ARMv6) assembly is available with NEON_CFLAGS. */
/* #undef CAN_COMPILE_NEON */

/* Define to 1 if SSE inline assembly is available. */
/* #undef CAN_COMPILE_SSE */

/* Define to 1 if SSE2 inline assembly is available. */
/* #undef CAN_COMPILE_SSE2 */

/* Define to 1 if SSE3 inline assembly is available. */
/* #undef CAN_COMPILE_SSE3 */

/* Define to 1 if SSE4A inline assembly is available. */
/* #undef CAN_COMPILE_SSE4A */

/* Define to 1 if SSE4_1 inline assembly is available. */
/* #undef CAN_COMPILE_SSE4_1 */

/* Define to 1 if SSE4_2 inline assembly is available. */
/* #undef CAN_COMPILE_SSE4_2 */

/* Define to 1 if SSSE3 inline assembly is available. */
/* #undef CAN_COMPILE_SSSE3 */

/* The ./configure command line */
#define CONFIGURE_LINE "./../../../configure  '--host=arm-eabi-linux' '--build=x86_64-unknown-linux' '--enable-static-modules' '--disable-a52' '--disable-vlc' '--disable-vlm' '--disable-sout' '--disable-httpd' '--disable-dbus' '--disable-lua' '--disable-libgcrypt' '--enable-live555' '--enable-realrtsp' '--disable-vcd' '--disable-v4l2' '--disable-gnomevfs' '--disable-dvdread' '--disable-dvdnav' '--disable-bluray' '--disable-linsys' '--disable-decklink' '--enable-avformat' '--enable-swscale' '--enable-avcodec' '--disable-postproc' '--disable-libva' '--enable-mkv' '--disable-dv' '--disable-mod' '--disable-sid' '--disable-x264' '--disable-mad' '--disable-schroedinger' '--disable-dirac' '--disable-sdl-image' '--disable-zvbi' '--disable-fluidsynth' '--enable-opensles' '--disable-jack' '--disable-pulse' '--disable-alsa' '--disable-portaudio' '--disable-sdl' '--disable-xcb' '--disable-atmo' '--disable-qt4' '--disable-skins2' '--disable-mtp' '--disable-taglib' '--disable-notify' '--enable-freetype' '--enable-libass' '--disable-svg' '--disable-sqlite' '--disable-udev' '--disable-libxml2' '--enable-android-surface' '--disable-caca' '--disable-glx' '--disable-egl' '--disable-gl' '--disable-gles1' '--disable-gles2' '--disable-goom' '--disable-projectm' 'build_alias=x86_64-unknown-linux' 'host_alias=arm-eabi-linux' 'CC=/opt/android/android-ndk-r5c/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86/bin//arm-linux-androideabi-gcc' 'CFLAGS=-nostdlib -mlong-calls -fstrict-aliasing -fprefetch-loop-arrays -ffast-math -mfpu=neon -mtune=cortex-a8 -ftree-vectorize -mvectorize-with-neon-quad -O2' 'LDFLAGS=-Wl,-rpath-link=/opt/android/android-ndk-r5c/platforms/android-9/arch-arm/usr/lib,-Bdynamic,-dynamic-linker=/system/bin/linker -Wl,--no-undefined -Wl,-shared -L/opt/android/android-ndk-r5c/platforms/android-9/arch-arm/usr/lib -Wl,--fix-cortex-a8' 'LIBS=-lc -ldl -lgcc' 'CPPFLAGS=-I/opt/android/android-ndk-r5c/platforms/android-9/arch-arm/usr/include -I./../../../extras/contrib/hosts/arm-eabi/include         -I/opt/android/android-ndk-r5c/sources/cxx-stl/gnu-libstdc++/include         -I/opt/android/android-ndk-r5c/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/include/' 'CXX=/opt/android/android-ndk-r5c/toolchains/arm-linux-androideabi-4.4.3/prebuilt/*-x86/bin//arm-linux-androideabi-g++' 'CXXFLAGS=-nostdlib -mlong-calls -fstrict-aliasing -fprefetch-loop-arrays -ffast-math -mfpu=neon -mtune=cortex-a8 -ftree-vectorize -mvectorize-with-neon-quad -O2 -D__STDC_VERSION__=199901L -D__STDC_CONSTANT_MACROS' 'PKG_CONFIG_LIBDIR=./../../../extras/contrib/hosts/arm-eabi/lib/pkgconfig'"

/* Copyright string */
#define COPYRIGHT_MESSAGE "Copyright © 1996-2011 the VideoLAN team"

/* The copyright years */
#define COPYRIGHT_YEARS "1996-2011"

/* Disables runtime check for buggy glibc. */
/* #undef DISABLE_BUGGY_GLIBC_CHECK */

/* Binary specific version */
/* #undef DISTRO_VERSION */

/* Define if you want the HTTP dameon support */
/* #undef ENABLE_HTTPD */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
/* #undef ENABLE_NLS */

/* Define to 1 for stream output support. */
/* #undef ENABLE_SOUT */

/* Define if you want the VideoLAN manager support */
/* #undef ENABLE_VLM */

/* Define to 1 if you have the <a52dec/a52.h> header file. */
/* #undef HAVE_A52DEC_A52_H */

/* Define to 1 if you have the `accept4' function. */
/* #undef HAVE_ACCEPT4 */

/* Define to 1 if you have the <altivec.h> header file. */
/* #undef HAVE_ALTIVEC_H */

/* Define to 1 if you have the <ApplicationServices/ApplicationServices.h>
   header file. */
/* #undef HAVE_APPLICATIONSERVICES_APPLICATIONSERVICES_H */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the `asprintf' function. */
#define HAVE_ASPRINTF 1

/* Define to 1 if you have the `atof' function. */
#define HAVE_ATOF 1

/* Define to 1 if you have the `atoll' function. */
#define HAVE_ATOLL 1

/* Support for __attribute__((packed)) for structs */
#define HAVE_ATTRIBUTE_PACKED 1

/* Define if avcodec has to be built with DxVA2 support. */
/* #undef HAVE_AVCODEC_DXVA2 */

/* Define if avcodec has to be built with VAAPI support. */
/* #undef HAVE_AVCODEC_VAAPI */

/* Define to 1 if you have the `backtrace' function. */
/* #undef HAVE_BACKTRACE */

/* Define to 1 if you have the <Carbon/Carbon.h> header file. */
/* #undef HAVE_CARBON_CARBON_H */

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define to 1 if you have the `connect' function. */
#define HAVE_CONNECT 1

/* Define to 1 if you have the <CoreAudio/CoreAudio.h> header file. */
/* #undef HAVE_COREAUDIO_COREAUDIO_H */

/* Define to 1 if you have the <d2d1.h> header file. */
/* #undef HAVE_D2D1_H */

/* Define to 1 if you have the <d3d9.h> header file. */
/* #undef HAVE_D3D9_H */

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define if you have the D-BUS library */
/* #undef HAVE_DBUS */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the <ddraw.h> header file. */
/* #undef HAVE_DDRAW_H */

/* Define to 1 if you have the <DeckLinkAPIDispatch.cpp> header file. */
/* #undef HAVE_DECKLINKAPIDISPATCH_CPP */

/* Define to 1 if you have the declaration of `nanosleep', and to 0 if you
   don't. */
#define HAVE_DECL_NANOSLEEP 1

/* Define to 1 if you have the `dirfd' function. */
#define HAVE_DIRFD 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if you have the dlopen API */
#define HAVE_DL_DLOPEN 1

/* Define if you have Windows' LoadLibrary */
/* #undef HAVE_DL_WINDOWS */

/* Define to 1 if you have the <dshow.h> header file. */
/* #undef HAVE_DSHOW_H */

/* Define if you have dvdnav_describe_title_chapters. */
/* #undef HAVE_DVDNAV_DESCRIBE_TITLE_CHAPTERS */

/* Define if you have dvdnav_get_video_resolution. */
/* #undef HAVE_DVDNAV_GET_VIDEO_RESOLUTION */

/* Define to 1 if you have the <dxva2api.h> header file. */
/* #undef HAVE_DXVA2API_H */

/* Define if dynamic plugins are supported */
#define HAVE_DYNAMIC_PLUGINS 1

/* Define to 1 if you have the <ebml/EbmlVersion.h> header file. */
#define HAVE_EBML_EBMLVERSION_H 1

/* Define to 1 if you have the `eventfd' function. */
#define HAVE_EVENTFD 1

/* Define to 1 if you have the <execinfo.h> header file. */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <faad.h> header file. */
/* #undef HAVE_FAAD_H */

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the `fdopendir' function. */
#define HAVE_FDOPENDIR 1

/* Define to 1 if you have the `flockfile' function. */
#define HAVE_FLOCKFILE 1

/* Define to 1 if you have the <fontconfig/fontconfig.h> header file. */
/* #undef HAVE_FONTCONFIG_FONTCONFIG_H */

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the `fstatvfs' function. */
/* #undef HAVE_FSTATVFS */

/* Define to 1 if you have the `fsync' function. */
#define HAVE_FSYNC 1

/* Define to 1 if you have the `getdelim' function. */
/* #undef HAVE_GETDELIM */

/* Define to 1 if you have the `getenv' function. */
#define HAVE_GETENV 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getpid' function. */
#define HAVE_GETPID 1

/* Define to 1 if you have the `getpwuid_r' function. */
/* #undef HAVE_GETPWUID_R */

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define to 1 if you have the <GL/gl.h> header file. */
/* #undef HAVE_GL_GL_H */

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the `if_nameindex' function. */
/* #undef HAVE_IF_NAMEINDEX */

/* Define to 1 if you have the `if_nametoindex' function. */
#define HAVE_IF_NAMETOINDEX 1

/* Define to 1 if you have the `inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define to 1 if you have inet_ntop(). */
#define HAVE_INET_NTOP 1

/* Define to 1 if you have inet_pton(). */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* For FreeBSD VCD support */
/* #undef HAVE_IOC_TOC_HEADER_IN_SYS_CDIO_H */

/* Define to 1 if you have the `isatty' function. */
#define HAVE_ISATTY 1

/* Define to 1 if you have the <kate/kate.h> header file. */
/* #undef HAVE_KATE_KATE_H */

/* Define to 1 if you have the <lauxlib.h> header file. */
/* #undef HAVE_LAUXLIB_H */

/* Define to 1 if you have the <libavcodec/avcodec.h> header file. */
#define HAVE_LIBAVCODEC_AVCODEC_H 1

/* Define to 1 if you have the <libavcodec/dxva2.h> header file. */
/* #undef HAVE_LIBAVCODEC_DXVA2_H */

/* Define to 1 if you have the <libavcodec/vaapi.h> header file. */
/* #undef HAVE_LIBAVCODEC_VAAPI_H */

/* Define to 1 if you have the <libavformat/avformat.h> header file. */
#define HAVE_LIBAVFORMAT_AVFORMAT_H 1

/* Define to 1 if you have the <libavformat/avio.h> header file. */
#define HAVE_LIBAVFORMAT_AVIO_H 1

/* Define to 1 if you have the <libavutil/avutil.h> header file. */
#define HAVE_LIBAVUTIL_AVUTIL_H 1

/* Define this if you have libcddb installed */
/* #undef HAVE_LIBCDDB */

/* Define to 1 if you have the <libcrystalhd/bc_drv_if.h> header file. */
/* #undef HAVE_LIBCRYSTALHD_BC_DRV_IF_H */

/* Define to 1 if you have the <libcrystalhd/bc_dts_defs.h> header file. */
/* #undef HAVE_LIBCRYSTALHD_BC_DTS_DEFS_H */

/* Define to 1 if you have the `mingw32' library (-lmingw32). */
/* #undef HAVE_LIBMINGW32 */

/* Define to 1 if you have the `ncursesw' library (-lncursesw). */
/* #undef HAVE_LIBNCURSESW */

/* Define if libproxy is available */
/* #undef HAVE_LIBPROXY */

/* Define to 1 if you have the <libsmbclient.h> header file. */
/* #undef HAVE_LIBSMBCLIENT_H */

/* Define to 1 if you have the <libssh2.h> header file. */
/* #undef HAVE_LIBSSH2_H */

/* Define to 1 if you have the <libswscale/swscale.h> header file. */
#define HAVE_LIBSWSCALE_SWSCALE_H 1

/* Define to 1 if you have the <libtar.h> header file. */
/* #undef HAVE_LIBTAR_H */

/* Define if libv4l2 is available */
/* #undef HAVE_LIBV4L2 */

/* Define to 1 if you have the <linux/dccp.h> header file. */
#define HAVE_LINUX_DCCP_H 1

/* Define to 1 if you have the <linux/fb.h> header file. */
/* #undef HAVE_LINUX_FB_H */

/* Define to 1 if you have the <linux/magic.h> header file. */
#define HAVE_LINUX_MAGIC_H 1

/* Define to 1 if you have the <linux/version.h> header file. */
#define HAVE_LINUX_VERSION_H 1

/* Define to 1 if you have the <linux/videodev2.h> header file. */
/* #undef HAVE_LINUX_VIDEODEV2_H */

/* Define to 1 if you have the <liveMedia_version.hh> header file. */
/* #undef HAVE_LIVEMEDIA_VERSION_HH */

/* Define to 1 if you have the `lldiv' function. */
#define HAVE_LLDIV 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the lrintf function */
#define HAVE_LRINTF 1

/* Define to 1 if you have the `lstat' function. */
#define HAVE_LSTAT 1

/* Define to 1 if you have the <lualib.h> header file. */
/* #undef HAVE_LUALIB_H */

/* Define to 1 if you have the <lua.h> header file. */
/* #undef HAVE_LUA_H */

/* Define to 1 if you have the <machine/param.h> header file. */
/* #undef HAVE_MACHINE_PARAM_H */

/* Define to 1 if you have the <mad.h> header file. */
/* #undef HAVE_MAD_H */

/* Define to 1 if building for the Maemo platform. */
/* #undef HAVE_MAEMO */

/* Define to 1 if you have the <matroska/KaxAttachments.h> header file. */
#define HAVE_MATROSKA_KAXATTACHMENTS_H 1

/* Define to 1 if you have the <matroska/KaxVersion.h> header file. */
#define HAVE_MATROSKA_KAXVERSION_H 1

/* Define to 1 if you have the `memalign' function. */
#define HAVE_MEMALIGN 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* Define to 1 if MMX intrinsics are available. */
/* #undef HAVE_MMX_INTRINSICS */

/* Define to 1 if you have the <mpcdec/mpcdec.h> header file. */
/* #undef HAVE_MPCDEC_MPCDEC_H */

/* Define to 1 if you have the <mpc/mpcdec.h> header file. */
/* #undef HAVE_MPC_MPCDEC_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/udplite.h> header file. */
/* #undef HAVE_NETINET_UDPLITE_H */

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the `nrand48' function. */
#define HAVE_NRAND48 1

/* Define to 1 if you have the `openat' function. */
#define HAVE_OPENAT 1

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
/* #undef HAVE_OPENGL_GL_H */

/* Define to 1 if you have the `pipe2' function. */
/* #undef HAVE_PIPE2 */

/* Define to 1 if you have the <png.h> header file. */
/* #undef HAVE_PNG_H */

/* Define to 1 if the OS is usabl... err, has poll(). */
#define HAVE_POLL 1

/* Define to 1 if you have the `posix_fadvise' function. */
/* #undef HAVE_POSIX_FADVISE */

/* Define to 1 if you have the `posix_madvise' function. */
/* #undef HAVE_POSIX_MADVISE */

/* Define to 1 if you have the `posix_memalign' function. */
/* #undef HAVE_POSIX_MEMALIGN */

/* Define to 1 if you have the <postproc/postprocess.h> header file. */
/* #undef HAVE_POSTPROC_POSTPROCESS_H */

/* Define to 1 if you have the `pread' function. */
#define HAVE_PREAD 1

/* Define to 1 if using libprojectM 2.x */
/* #undef HAVE_PROJECTM2 */

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the `pthread_mutex_lock' function. */
#define HAVE_PTHREAD_MUTEX_LOCK 1

/* Define to 1 if you have the <QuickTime/QuickTime.h> header file. */
/* #undef HAVE_QUICKTIME_QUICKTIME_H */

/* Define to 1 if you have the `rewind' function. */
#define HAVE_REWIND 1

/* Define to 1 if you have the `sched_getaffinity' function. */
/* #undef HAVE_SCHED_GETAFFINITY */

/* For NetBSD VCD support */
/* #undef HAVE_SCSIREQ_IN_SYS_SCSIIO_H */

/* Define to 1 if you have the <scsi/scsi.h> header file. */
/* #undef HAVE_SCSI_SCSI_H */

/* Define to 1 if you have the <search.h> header file. */
/* #undef HAVE_SEARCH_H */

/* Define to 1 if you have the `send' function. */
#define HAVE_SEND 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the <SLES/OpenSLES.h> header file. */
#define HAVE_SLES_OPENSLES_H 1

/* Define to 1 if you have the <soundcard.h> header file. */
/* #undef HAVE_SOUNDCARD_H */

/* Define to 1 if you have the <sqlite3.h> header file. */
/* #undef HAVE_SQLITE3_H */

/* Define to 1 if SSE2 intrinsics are available. */
/* #undef HAVE_SSE2_INTRINSICS */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcasestr' function. */
#define HAVE_STRCASESTR 1

/* Define to 1 if you have the `strcoll' function and it is properly defined.
   */
/* #undef HAVE_STRCOLL */

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `stricmp' function. */
/* #undef HAVE_STRICMP */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcpy' function. */
#define HAVE_STRLCPY 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if you have the `strnicmp' function. */
/* #undef HAVE_STRNICMP */

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the `strsep' function. */
#define HAVE_STRSEP 1

/* Define to 1 if you have the `strtof' function. */
#define HAVE_STRTOF 1

/* Define to 1 if you have the `strtok_r' function. */
#define HAVE_STRTOK_R 1

/* Define to 1 if you have the `strtoll' function. */
#define HAVE_STRTOLL 1

/* Define to 1 if you have the `swab' function. */
/* #undef HAVE_SWAB */

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/dl.h> header file. */
/* #undef HAVE_SYS_DL_H */

/* Define to 1 if you have the <sys/eventfd.h> header file. */
/* #undef HAVE_SYS_EVENTFD_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/mount.h> header file. */
#define HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/shm.h> header file. */
/* #undef HAVE_SYS_SHM_H */

/* Define to 1 if you have the <sys/soundcard.h> header file. */
/* #undef HAVE_SYS_SOUNDCARD_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/videoio.h> header file. */
/* #undef HAVE_SYS_VIDEOIO_H */

/* Define to 1 if you have the `tdestroy' function. */
/* #undef HAVE_TDESTROY */

/* Define if libtiger is available. */
/* #undef HAVE_TIGER */

/* Define to 1 if you have the <tremor/ivorbiscodec.h> header file. */
/* #undef HAVE_TREMOR_IVORBISCODEC_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <unzip.h> header file. */
/* #undef HAVE_UNZIP_H */

/* Define to 1 if you have the `uselocale' function. */
/* #undef HAVE_USELOCALE */

/* Define to 1 if you have the <valgrind/valgrind.h> header file. */
/* #undef HAVE_VALGRIND_VALGRIND_H */

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Define if <stdarg.h> defines va_copy. */
#define HAVE_VA_COPY 1

/* Define to 1 if you have the `vmsplice' function. */
/* #undef HAVE_VMSPLICE */

/* Define to 1 if you have the <X11/Xlib.h> header file. */
/* #undef HAVE_X11_XLIB_H */

/* Define to 1 if you have the <xlocale.h> header file. */
/* #undef HAVE_XLOCALE_H */

/* Define to 1 if you have the <xosd.h> header file. */
/* #undef HAVE_XOSD_H */

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define if <stdarg.h> defines __va_copy. */
#define HAVE___VA_COPY 1

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST 

/* Dynamic object extension */
#define LIBEXT ".so"

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define if you want to use the VLC media library */
/* #undef MEDIA_LIBRARY */

/* Define to 1 if debug code should NOT be compiled */
#define NDEBUG 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define if <sys/param.h> defines ntohl. */
/* #undef NTOHL_IN_SYS_PARAM_H */

/* Define if you want to optimize memory usage over performance */
/* #undef OPTIMIZE_MEMORY */

/* Name of package */
#define PACKAGE "vlc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "vlc"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "vlc 1.2.0-git"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "vlc"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.2.0-git"

/* version development string */
#define PACKAGE_VERSION_DEV "git"

/* version extra number */
#define PACKAGE_VERSION_EXTRA 0

/* version major number */
#define PACKAGE_VERSION_MAJOR 1

/* version minor number */
#define PACKAGE_VERSION_MINOR 2

/* version revision number */
#define PACKAGE_VERSION_REVISION 0

/* Define if you want to use SQLite module */
/* #undef SQLITE_MODULE */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if <strings.h> defines strncasecmp. */
#define STRNCASECMP_IN_STRINGS_H 1

/* Define if you want to use the VLC update mechanism */
/* #undef UPDATE_CHECK */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "1.2.0-git"

/* Simple version string */
#define VERSION_MESSAGE "1.2.0-git Twoflower"

/* compiler */
#define VLC_COMPILER "gcc version 4.4.3 (GCC) "

/* user who ran configure */
#define VLC_COMPILE_BY "tewilove"

/* host which ran configure */
#define VLC_COMPILE_HOST "localhost"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Define if the zvbi module is built */
/* #undef ZVBI_COMPILED */

/* Define to 64 for large files support. */
#define _FILE_OFFSET_BITS 64

/* Define to 2 to get glibc warnings. */
#define _FORTIFY_SOURCE 2

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to expose reentrant functions. */
#define _REENTRANT /**/

/* Same as _REENTANT for some other OSes. */
#define _THREAD_SAFE /**/

/* Define to '0x0501' for IE 5.01 (and shell) APIs. */
/* #undef _WIN32_IE */

/* Define to '0x0501' for Windows XP APIs. */
/* #undef _WIN32_WINNT */

/* Define within the LibVLC source code tree. */
#define __LIBVLC__ /**/

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Alias fdatasync() to fsync() if missing. */
/* #undef fdatasync */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#define restrict __restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `sockaddr' if <sys/socket.h> does not define. */
/* #undef sockaddr_storage */

/* Define to `int' if <sys/socket.h> does not define. */
/* #undef socklen_t */

/* Define to `sa_family' if <sys/socket.h> does not define. */
/* #undef ss_family */

/* Define to `int' if <stddef.h> does not define. */
/* #undef ssize_t */

#include <vlc_fixups.h>
