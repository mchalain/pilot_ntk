#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "content.h"
extern char *mimetype(char *filepath);

/**
 * Content
 **/
static int
content_read(Content *this, char *buff, int len);

Content *
content_new()
{
	Content *this = malloc(sizeof(*this));
	memset(this, 0, sizeof(*this));
	this->read = content_read;
	return this;
}

void
content_destroy(Content *this)
{
	if (this->destroy)
		this->destroy(this);
	free(this);
	this = NULL;
}

int
content_length(Content *this)
{
	return this->m_length;
}

char *
content_type(Content *this)
{
	return this->m_type;
}

static int
content_read(Content *this, char *buff, int len)
{
	return 0;
}

/**
 * Content file
 **/
static int
contentfile_read(Content *this, char *buff, int len);
static void
contentfile_destroy(Content *this);

typedef struct 
{
	char *filepath;
	int fd;
}ContentfileData;

Content *
contentfile_new(char *base, char *filepath)
{
	ContentfileData *data;
	Content *this = content_new();
	this->read = contentfile_read;
	this->destroy = contentfile_destroy;
	data = malloc(sizeof(ContentfileData));
	this->m_private = (void *) data;
	data->filepath = malloc(strlen(base) + strlen(filepath) + 2);
	sprintf(data->filepath, "%s/%s", base, filepath);
	data->fd = open(data->filepath, O_RDONLY);
	if (data->fd > 0)
	{
		struct stat filestat;
		fstat(data->fd, &filestat);
		this->m_length = filestat.st_size;
		this->m_type = malloc(256);
		strncpy(this->m_type, mimetype(filepath), 256);
	}
	else
	{
		fprintf(stderr, "file %s : %s\n", data->filepath, strerror(errno));
		this->m_status = 1;
	}
	return this;
}

static void
contentfile_destroy(Content *this)
{
	ContentfileData *data = this->m_private;
	if (data->fd > 0)
		close(data->fd);
	if (data->filepath)
		free(data->filepath);
	free(data);
	if (this->m_type)
		free(this->m_type);
}

static int
contentfile_read(Content *this, char *buff, int len)
{
	ContentfileData *data = this->m_private;
	int ret = 0;
	if (data->fd > 0)
		ret = read(data->fd, buff, len);

	return ret;
}

#define MIMETYPE_UNDEF_STR "text/txt"
#define MIMETYPE_UNDEF_ID 0
#define MIMETYPE_HTML_STR "text/html"
#define MIMETYPE_HTML_ID 1

char *g_mimetype[] = {
	MIMETYPE_UNDEF_STR,
	MIMETYPE_HTML_STR
};
char *
mimetype(char *filepath)
{
	if (strstr(filepath, ".html"))
		return g_mimetype[MIMETYPE_HTML_ID];
	return g_mimetype[MIMETYPE_UNDEF_ID];
}
