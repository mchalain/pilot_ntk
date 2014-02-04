#ifndef __CONTENT_H__
#define __CONTENT_H__

struct _content
{
	void *m_private;
	int m_length;
	char *m_type;
	int m_status;
	int (*read)(struct _content *content, char *buf, int len);
	void (*destroy)(struct _content *content);
};

typedef struct _content Content;
Content *
content_new();
void
content_destroy(Content *this);
int
content_length(Content *this);
char *
content_type(Content *this);

Content *
contentfile_new(char *base, char *filepath);


#endif
