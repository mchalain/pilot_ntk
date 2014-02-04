/*
GET //index.html HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:23.0) Gecko/20100101 Firefox/23.0
Accept: text/html,application/xhtml+xml,application/xml;
Accept-Language: fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3
Accept-Encoding: gzip, deflate
Connection: keep-alive

*/
#include <string.h>
#include <stdio.h>
#include "httprequest.h"
#include "httpresponse.h"
#include "content.h"

int
main(int argc, char **argv)
{
	int ret = 1;
	HttpRequest *req;
	req = httprequest_new();
	
	char buff[128];
	int len = sizeof(buff);
	char *ptr = buff;
	while ((ret != -1))
	{
		len = read(0, ptr, len - 1);
		ptr[len] = 0;
		if (len < 1)
			break;
		ptr = buff;
		ret = 1;
		while ((len > 0) && (ret > 0))
		{
			ret = httprequest_parse(req, ptr, len);
			if (ret > 0)
			{
				ptr += ret;
				len -= ret;
			}
		}
		len = sizeof(buff) - strlen(ptr);
		ptr = stpcpy(buff, ptr);
	}
	Content *content = contentfile_new("/home/http", req->m_filepath);
	HttpResponse *response = httpresponse_new(content);
	do
	{
		len = sizeof(buff);
		len = httpresponse_format(response, buff, len);
		if (len > 0)
			len = write(1, buff, len);
	} while (len > 0);
	content_destroy(content);
	httpresponse_destroy(response);
	httprequest_destroy(req);
	return 0;
}
