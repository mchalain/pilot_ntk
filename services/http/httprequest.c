#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tools.h"
#include "httprequest.h"

#define STRNCMP strncmp
/**
 * HttpSession
 **/
HttpRequest *
httprequest_new()
{
	HttpRequest *http;
 	http = malloc(sizeof(*http));
	memset(http, 0, sizeof(*http));
	http->m_state = E_ParseAction;
	return http;
}

void httprequest_destroy(HttpRequest *p_http)
{
	if (p_http->m_filepath)
	{
		free(p_http->m_filepath);
		p_http->m_filepath = NULL;
	}
	if (p_http->m_host)
	{
		free(p_http->m_host);
	}
/*
 * 	if (p_http->m_get)
	{
		free(p_http->m_get);
		p_http->m_get = NULL;
	}
 */
	if (p_http->m_useragent)
	{
		free(p_http->m_useragent);
		p_http->m_useragent = NULL;
	}
	if (p_http->m_content.data)
	{
		free(p_http->m_content.data);
		p_http->m_content.data = NULL;
		p_http->m_content.length = 0;
	}
	free(p_http);
	p_http = NULL;
}

char *
_endline(char *ptr)
{
	while (*ptr != '\r')
		if (*ptr == 0) return ptr; else ++ptr;
	while (*ptr != '\n')
		if (*ptr == 0) return ptr; else ++ptr;
	return ++ptr;
}

int
httprequest_parse(HttpRequest *p_http, char *p_buffer, int p_length)
{
	char *ptr = p_buffer;
	char *tmp;
	int ret = -1;

	if (p_length == 0)
		return 0;
	do
	{
		switch (p_http->m_state)
		{
			case E_ParseAction:
			{
				if (!strncmp(ptr, "GET ", 4))
				{
					p_http->m_state = E_ParseURL;
					p_http->m_cmd = E_Get;
					ptr += 4;
				}
				else if (!strncmp(ptr, "HEAD ", 5))
				{
					p_http->m_state = E_ParseHeader;
					p_http->m_cmd = E_Head;
					ptr += 5;
				}
				else if (!strncmp(ptr, "POST ", 5))
				{
					p_http->m_state = E_ParseURL;
					p_http->m_cmd = E_Post;
					ptr += 5;
				}
				else
					ret = ptr - p_buffer;
			break;
			case E_ParseURL:
				/**
				 * filepath format:
				 * //dirname/index.html
				 */
				tmp = _endline(ptr);
				if (*tmp != 0)
				{
					/// delete the first '/'
					while (*ptr++ != '/');
					
					ptr = strstore(&p_http->m_filepath, ptr, " ?\n\r");

					if (*ptr == '?')
					{
						ptr++;
						p_http->m_state = E_ParseGet;
					}
					else if (*ptr == ' ')
					{
						/// the following must be the protocol
						ptr = tmp;
						p_http->m_state = E_ParseHeader;
					}
					else if (*ptr == '\r'|| *ptr == '\n')
					{
						ptr = tmp;
						p_http->m_state = E_ParseHeader;
					}
				}
				else
					ret = ptr - p_buffer;
			}
			break;
			case E_ParseGet:
			{
				tmp = _endline(ptr);
				if (*tmp != 0)
				{
					char *equal;
					p_http->m_get[p_http->m_get_nb].name = NULL;
					ptr = strstore(&(p_http->m_get[p_http->m_get_nb].name), ptr, " &\n\r");
					equal = strchrnul(p_http->m_get[p_http->m_get_nb].name, '=');
					if (*equal != '\0')
					{
						p_http->m_get[p_http->m_get_nb].value = equal + 1;
						*equal = '\0';
					}
					p_http->m_get_nb++;
					ptr = tmp;
					p_http->m_state = E_ParseHeader;
				}
				else
					ret = ptr - p_buffer;
			}
			break;
			case E_ParseHeader:
			{
				tmp = _endline(ptr);
				if (*tmp != 0)
				{
					if (!STRNCMP(ptr, "Host: ", 6))
					{
						ptr += 6;
						ptr = strstore(&p_http->m_host, ptr, " :\n\r");
						char *ddot = strchrnul(p_http->m_host, ':');
						if (*ddot == ':')
						{
							p_http->m_port = atoi(ddot + 1);
							*ddot = 0;
						}
						else
						{
							p_http->m_port = 80;
						}
					}
					else if (!STRNCMP(ptr, "User-Agent: ", 12))
					{
						ptr += 12;
						ptr = strstore(&p_http->m_useragent, ptr, "\n\r");
					}
					else if (!STRNCMP(ptr, "Content-Type: ", 14))
					{
						ptr += 14;
						if (!strncmp(ptr, "application/x-www-form-urlencoded", 32))
						{
							ptr += 32;
							p_http->m_content.type = E_Form;
						}
					}
					else if (!STRNCMP(ptr, "Content-Length: ", 16))
					{
						ptr += 16;
						p_http->m_content.length = atoi(ptr);
					}
					else if (*ptr == '\n' || *ptr == '\r')
					{
						p_http->m_state = E_ParseBody;
					}
					ptr = tmp;
				}
				else
					ret = ptr - p_buffer;
			}
			break;
			case E_ParseBody:
			{
				if (p_http->m_content.data)
				{
					p_http->m_content.data = malloc(p_http->m_content.length + 1);
					p_http->m_content.ptr = p_http->m_content.data;
				}
				memcpy(p_http->m_content.ptr, ptr, p_length);
				p_http->m_content.ptr += p_length;
				if (p_http->m_content.ptr - p_http->m_content.data == p_http->m_content.length)
					p_http->m_state = E_ParseEnd;
			}
			break;
			case E_ParseEnd:
				ret = 0;
			break;
		}
	} while (ret < 0);
	ptr = stpncpy(p_buffer, ptr, ret);
	return ret;
}


