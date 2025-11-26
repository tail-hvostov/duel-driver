#ifndef _COMMON_OPS_H_
#define _COMMON_OPS_H_

extern char* buf;
extern unsigned int sc_w;
extern unsigned int sc_h;
extern unsigned int buf_size;
extern unsigned int video_size;

bool init_video_params(unsigned int extra_buf);

#endif