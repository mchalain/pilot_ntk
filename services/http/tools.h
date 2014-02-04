#ifndef __TOOLS_H__
#define __TOOLS_H__
char *strstore(char **p_output, char *p_input, char *p_tags);
#if _XOPEN_SOURCE < 700 && _POSIX_C_SOURCE < 200809L
char *stpcpy(char *p_output, char *p_input);
char *stpncpy(char *p_output, char *p_input, int p_len);
#endif
char *utf8parsing(char *p_buffer);

#endif
