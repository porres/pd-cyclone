/* Copyright (c) 2001-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* reading/writing midifiles, a prototype version */

#ifndef __MIFI_H__
#define __MIFI_H__

/* event types, as returned by mifi_read_event() */
#define MIFI_READ_FATAL  -3  /* unexpected eof, error in last track, or file error */
#define MIFI_READ_EOF    -2  /* regular eof */
#define MIFI_READ_SKIP   -1  /* error and successful skip to the next track */
#define MIFI_META_SEQNUM         0
#define MIFI_META_TEXT           1
#define MIFI_META_COPYRIGHT      2
#define MIFI_META_TRACKNAME      3
#define MIFI_META_INSTRUMENT     4
#define MIFI_META_LYRIC          5
#define MIFI_META_MARKER         6
#define MIFI_META_CUE            7
#define MIFI_META_MAXPRINTABLE  15  /* 1..15 are various text meta-events */
#define MIFI_META_CHANNEL     0x20  /* channel prefix */
#define MIFI_META_EOT         0x2f  /* end of track */
#define MIFI_META_TEMPO       0x51
#define MIFI_META_SMPTE       0x54  /* SMPTE offset */
#define MIFI_META_TIMESIG     0x58  /* time signature */
#define MIFI_META_KEYSIG      0x59  /* key signature */
/* ...channel status codes go here, too obvious to #define... */
#define MIFI_SYSEX_FIRST      0xf0
#define MIFI_SYSEX_NEXT       0xf7
/* this code is not returned as an event type, but in e_status of t_mifi_event */
#define MIFI_EVENT_META       0xff

/* true if one of channel messages */
#define MIFI_IS_CHANNEL(status)    (((status) & 0x80) && (status) < 0xf0)
/* true if one of the two shorter channel messages */
#define MIFI_ONE_DATABYTE(status)  (((status) & 0xe0) == 0xc0)

/* derived from t_squeve */
typedef struct _mifi_event
{
    uint32  e_length;
    uchar  *e_data;
    size_t  e_bufsize;
    uint32  e_delay;
    uchar   e_status;
    uchar   e_channel;
    uchar   e_meta;      /* meta-event type */
} t_mifi_event;

/* This structure holds midi data stream properties, i.e. both the info stored
   in midifile header, and the current state according to position in a stream. */
/* LATER clean up t_sq and derive t_mifi_stream */
typedef struct _sq t_mifi_stream;

/* prototypes of public interface routines */

t_mifi_event *mifi_event_new(void);
void mifi_event_free(t_mifi_event *e);
int mifi_event_settext(t_mifi_event *e, int type, char *text);
void mifi_event_printmeta(t_mifi_event *e);

t_mifi_stream *mifi_stream_new(void);
void mifi_stream_reset(t_mifi_stream *x);
void mifi_stream_free(t_mifi_stream *x);

t_mifi_stream *mifi_read_start(t_mifi_stream *x,
			       const char *filename, const char *dirname,
			       int complain);
int mifi_read_restart(t_mifi_stream *x);
void mifi_read_end(t_mifi_stream *x);
int mifi_read_event(t_mifi_stream *x, t_mifi_event *e);
int mifi_read_analyse(t_mifi_stream *stp);
int mifi_read_doit(t_mifi_stream *stp);

t_mifi_stream *mifi_write_start(t_mifi_stream *x,
				const char *filename, const char *dirname);
void mifi_write_end(t_mifi_stream *x);
int mifi_write_start_track(t_mifi_stream *x);
int mifi_write_adjust_track(t_mifi_stream *x, uint32 eotdelay);
int mifi_write_event(t_mifi_stream *x, t_mifi_event *e);

#endif
