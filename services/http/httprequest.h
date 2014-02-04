#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#define MAXGET 50
struct _httprequest
{
	enum {E_ParseEnd, E_ParseAction, E_ParseURL, E_ParseGet, E_ParseHeader, E_ParseBody } m_state;
	enum {E_Get, E_Head, E_Post} m_cmd;
	char *m_filepath;
	char *m_host;
	int m_port;
	char *m_useragent;
	struct
	{
		char *name;
		char *value;
	} m_get[MAXGET];
	int m_get_nb;
	struct
	{
		enum {E_File, E_Form} type;
		int length;
		char *data;
		char *ptr;
	} m_content;
};

typedef struct _httprequest HttpRequest;
HttpRequest *httprequest_new();
void httprequest_destroy(HttpRequest *p_http);
int httprequest_parse(HttpRequest *p_http, char *p_buffer, int p_length);


#endif
