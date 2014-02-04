#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tools.h"
#include "httpresponse.h"
#include "content.h"

#define STATUS_200_STR "HTTP/1.0 200 OK\n\r"
#define STATUS_200_ID 0
#define STATUS_404_STR "HTTP/1.0 404 File Not Found\n\rContent-Length: 0\n\r"
#define STATUS_404_ID 1
#define STATUS_501_STR "HTTP/1.1 501 Not Implemented\n\r"
#define STATUS_501_ID 2

char *g_status[] = {
	STATUS_200_STR,
	STATUS_404_STR,
	STATUS_501_STR,
};

HttpResponse *
httpresponse_new(Content *content)
{
	HttpResponse *http;
	http = malloc(sizeof(*http));
	memset(http, 0, sizeof(*http));
	http->m_Content = content;
	http->m_state = E_ResponseStatus;
	return http;
}

void
httpresponse_destroy(HttpResponse *p_http)
{
	free(p_http);
}

int
httpresponse_format(HttpResponse *p_http, char *buff, int len)
{
	int ret = 0;

	switch (p_http->m_state)
	{
		case E_ResponseStatus:

			if (p_http->m_Content->m_status > -1)
			{
				if (p_http->m_ptr == NULL)
					p_http->m_ptr = g_status[p_http->m_Content->m_status];
				p_http->m_ptr = stpncpy(buff, p_http->m_ptr, len);
			}
			/// not enought place inside the buffer to push all data
			if (!p_http->m_ptr)
				return -1;
			else if (*p_http->m_ptr != 0 )
				return len;
			else
			{
				p_http->m_ptr = NULL;
				p_http->m_state = E_ResponseHeader;
				ret = strlen(g_status[p_http->m_Content->m_status]);
			}
		break;
		case E_ResponseHeader:
			if (p_http->m_Content != NULL)
			{
				if (p_http->m_header == NULL)
				{
					int len = 256;
					int ret = 0;
					do
					{
						p_http->m_header = malloc(len);
						ret = snprintf(p_http->m_header, len,
								"Content-Type: %s\n\r"
								"Content-Length: %d\n\r\n\r", 
								(p_http->m_Content->m_type)?p_http->m_Content->m_type:"", 
								p_http->m_Content->m_length);
						if (ret < len)
							break;
						len += 64;
						free(p_http->m_header);
					} while (1);
					p_http->m_ptr = p_http->m_header;
				}
				strncpy(buff, p_http->m_ptr, len);
			}
			if (!p_http->m_ptr)
				return -1;
			else if (*(p_http->m_ptr + len) != 0 )
			{
				p_http->m_ptr += len;
				return len;
			}
			else
			{
				ret = strlen(p_http->m_ptr);
				free(p_http->m_header);
				p_http->m_header = NULL;
				p_http->m_ptr = NULL;
				if (p_http->m_Content->m_status == STATUS_200_ID)
					p_http->m_state = E_ResponseContent;
				else
					p_http->m_state = E_ResponseEnd;
			}
		break;
		case E_ResponseContent:
			if (p_http->m_Content != NULL)
			{
				if (p_http->m_Content->read != NULL)
					ret = p_http->m_Content->read(p_http->m_Content, buff, len);
				if (ret == 0)
					p_http->m_state = E_ResponseEnd;
			}
			else
				p_http->m_state = E_ResponseEnd;
		break;
	}
	return ret;
}
