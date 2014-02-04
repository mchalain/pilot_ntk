#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#define MAXGET 50
struct _content;
struct _httpresponse
{
	enum {E_ResponseStatus, E_ResponseHeader, E_ResponseContent, E_ResponseEnd } m_state;
	struct _content *m_Content;
	char *m_header;
	char *m_ptr;
	int m_status;
};

typedef struct _httpresponse HttpResponse;
HttpResponse *
httpresponse_new(struct _content *content);
void
httpresponse_destroy(HttpResponse *p_http);
int
httpresponse_format(HttpResponse *p_http, char *buff, int len);

#endif
