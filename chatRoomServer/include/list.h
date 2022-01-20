/*
 * =====================================================================================
 *
 *       Filename:  list.h
 *
 *    Description: representation of linked list 
 *
 * =====================================================================================
 */

#ifndef __LIST_H_
#define __LIST_H_

typedef struct _ListElmt {
	void 				*data;
	struct _ListElmt 	*next;
} ListElmt;

typedef struct _List {
	int 				size;

	int 				(*match) (const void *key1, const void *key2);
	void 				(*destroy) (void *data);

	ListElmt 			*head;
	ListElmt			*tail;
} List;

extern void list_init(List *list, void (*destroy)(void *data));

extern void list_destroy(List *list);

extern int list_ins_next(List *list, ListElmt *element, const void *data);

extern int list_rem_next(List *list, ListElmt *element, void **data);

#define list_size(list)						((list)->size)

#define list_head(list)						((list)->head)

#define list_tail(list)						((list)->tail)

#define list_is_head(list, element)			((element) == (list)->head ? 1 : 0)

#define list_is_tail(element)				((element)->next == NULL ? 1 : 0)

#define list_data(element)					((element)->data)

#define list_next(element)					((element)->next)


#endif	/*  __LIST_H_ */
