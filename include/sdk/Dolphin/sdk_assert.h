#ifndef _DOLPHIN_SDK_ASSERT_H
#define _DOLPHIN_SDK_ASSERT_H

#ifdef DEBUG
#define ASSERTLINE(line, cond)                      ((cond) || (OSPanic(__FILE__, line, "Failed assertion " #cond), 0))
#define ASSERTMSGLINE(line, cond, msg)              ((cond) || (OSPanic(__FILE__, line, msg), 0))
#define ASSERTMSG1LINE(line, cond, msg, arg1)       ((cond) || (OSPanic(__FILE__, line, msg, arg1), 0))
#define ASSERTMSG2LINE(line, cond, msg, arg1, arg2) ((cond) || (OSPanic(__FILE__, line, msg, arg1, arg2), 0))
#else
#define ASSERTLINE(line, cond)                      (void)0
#define ASSERTMSGLINE(line, cond, msg)              (void)0
#define ASSERTMSG1LINE(line, cond, msg, arg1)       (void)0
#define ASSERTMSG2LINE(line, cond, msg, arg1, arg2) (void)0
#endif

#endif
