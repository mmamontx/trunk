#ifndef _LIST_H
#define _LIST_H 1

struct list_item {
	char *msg;
	unsigned char mode;
	unsigned char size;
	struct list_item *item[6];
	struct list_item *parent;
	struct list_item *selected;
};

/*extern struct list_item *level, *root;*/

extern char *list_frame(const struct list_item *, const unsigned char, const unsigned char, const char);
extern int list_print(const struct list_item *, const unsigned char, const char);
extern struct list_item *list_init();
extern void *list(void *);
extern void list_free();

#endif /* !_LIST_H */