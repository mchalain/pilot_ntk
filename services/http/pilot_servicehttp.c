#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <pilot_ntk.h>

#include "httprequest.h"
#include "httpresponse.h"
#include "content.h"

typedef struct
{
	HttpRequest *req;
	HttpResponse *response;
	enum {EBegin, EParse, EContent, EResponse, EEnd} state;
} ServerData;
static void
_pilot_servicehttp_destroy(struct pilot_service *thiz);
static int
_pilot_servicehttp_receive_server(struct pilot_service *thiz);
static int
_pilot_servicehttp_runparser(struct pilot_service *thiz, ServerData *data);

struct pilot_service *
pilot_servicehttp_create(struct pilot_socket *socket)
{
	struct pilot_service *thiz = pilot_service_create(socket);
	thiz->action.receive_server=_pilot_servicehttp_receive_server;
	thiz->action.destroy=_pilot_servicehttp_destroy;
	ServerData *data = malloc(sizeof(ServerData));
	thiz->privatedata = (void *)data;
	data->state = EBegin;
}

static void
_pilot_servicehttp_destroy(struct pilot_service *thiz)
{
	ServerData *data = thiz->privatedata;
	free(data);
}

static int
_pilot_servicehttp_receive_server(struct pilot_service *thiz)
{
	ServerData *data = thiz->privatedata;
	while (data->state != EEnd)
	{
		_pilot_servicehttp_runparser(thiz, data);
	}
	return -1;
}

static int
_pilot_servicehttp_runparser(struct pilot_service *thiz, ServerData *data)
{
	int len;
	switch (data->state)
	{
		case EBegin:
			data->req = httprequest_new();
			data->state = EParse;
		break;
		case EParse:
		{
			char buff[256];
			len = sizeof(buff);

			len = thiz->socket->action.read(thiz->socket, buff, len - 1);
			if (len < 0)
				data->state = EEnd;
			len = httprequest_parse(data->req, buff, len);
			if (len == 0)
				data->state = EContent;
		}
		break;
		case EContent:
		{
			Content *content = contentfile_new("/home/http", data->req->m_filepath);
			data->response = httpresponse_new(content);
			data->state = EResponse;
		}
		break;
		case EResponse:
		{
			char buff[256];
			len = sizeof(buff);
			len = httpresponse_format(data->response, buff, len - 1);
			if (len > 0)
				len = thiz->socket->action.write(thiz->socket, buff, len);
			else if (len == 0)
				data->state = EEnd;
		}
		break;
		case EEnd:
			if (data->response->m_Content)
				content_destroy(data->response->m_Content);
			data->response->m_Content = NULL;
			if (data->response)
				httpresponse_destroy(data->response);
			data->response = NULL;
			if (data->req)
				httprequest_destroy(data->req);
			data->req = NULL;
			data->state = EBegin;
		break;
	}
			
	return 0;
}
