/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <3dlite/list.h>
#include <3dlite/alloc.h>

#define __list_check(expr) (expr)
#define __LIST_POISON       0x0f00
#define __LIST_LINK_POISON  0x0b0b

static inline void
__list_bind(struct lite3d_list_node *prev, struct lite3d_list_node *next)
{
    __list_check(next)->prev = prev;
    __list_check(prev)->next = next;
}

static inline int
__list_link_alone(struct lite3d_list_node *link)
{
    return link == __list_check(link)->next;
}

static inline void
__list_link_init(struct lite3d_list_node *link)
{
    __list_bind(link, link);
}

static inline void
__list_insert_chain(struct lite3d_list_node *first, struct lite3d_list_node *last,
    struct lite3d_list_node *prev, struct lite3d_list_node *next)
{
    __list_bind(prev, first);
    __list_bind(last, next);
}

static inline void
__list_insert_link(struct lite3d_list_node *link, struct lite3d_list_node *prev,
    struct lite3d_list_node *next)
{
    __list_insert_chain(link, link, prev, next);
}

struct lite3d_list_node *
lite3d_list_link_init(struct lite3d_list_node *link)
{
    link->poison = __LIST_LINK_POISON;
    link->lite3d_list = NULL;
    __list_link_init(link);

    return link;
}

struct lite3d_list *
lite3d_list_init(struct lite3d_list *lite3d_list)
{
    __list_link_init(&lite3d_list->l);

    return lite3d_list;
}

int
lite3d_list_alone_link(struct lite3d_list_node *link)
{
    return __list_link_alone(link);
}

int
lite3d_list_is_empty(struct lite3d_list *lite3d_list)
{
    return __list_link_alone(&lite3d_list->l);
}

struct lite3d_list_node *
lite3d_list_first_link(struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *l, *first;

    l = &lite3d_list->l;
    first = l->next;
    return first != l ? first : NULL;
}

struct lite3d_list_node *
lite3d_list_last_link(struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *l, *last;

    l = &lite3d_list->l;
    last = l->prev;
    return last != l ? last : NULL;
}

void
lite3d_list_add_first_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *l;

    l = &lite3d_list->l;
    __list_insert_link(new_link, l, l->next);
}

void
lite3d_list_add_last_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *l;

    l = &lite3d_list->l;
    __list_insert_link(new_link, l->prev, l);
}

void
lite3d_list_insert_before_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link)
{
    __list_insert_link(new_link, link->prev, link);
}

void
lite3d_list_insert_after_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link)
{
    __list_insert_link(new_link, link, link->next);
}

void
lite3d_list_bulk_add_first(struct lite3d_list *from_list, struct lite3d_list *to_list)
{
    struct lite3d_list_node *from, *to;

    if (!lite3d_list_is_empty(from_list))
    {
        from = &from_list->l;
        to = &to_list->l;

        __list_insert_chain(from->next, from->prev, to, to->next);
        __list_link_init(from);
    }
}

void
lite3d_list_bulk_add_last(struct lite3d_list *from_list, struct lite3d_list *to_list)
{
    struct lite3d_list_node *from, *to;

    if (!lite3d_list_is_empty(from_list))
    {
        from = &from_list->l;
        to = &to_list->l;

        __list_insert_chain(from->next, from->prev, to->prev, to);
        __list_link_init(from);
    }
}

void
lite3d_list_bulk_insert_before_link(struct lite3d_list *from_list, struct lite3d_list_node *link)
{
    struct lite3d_list_node *from;

    if (!lite3d_list_is_empty(from_list))
    {
        from = &from_list->l;

        __list_insert_chain(from->next, from->prev, link->prev, link);
        __list_link_init(from);
    }
}

void
lite3d_list_bulk_insert_after_link(struct lite3d_list *from_list, struct lite3d_list_node *link)
{
    struct lite3d_list_node *from;

    if (!lite3d_list_is_empty(from_list))
    {
        from = &from_list->l;

        __list_insert_chain(from->next, from->prev, link, link->next);
        __list_link_init(from);
    }
}

void
lite3d_list_unlink_link(struct lite3d_list_node *link)
{
    __list_bind(link->prev, link->next);
    __list_link_init(link);
}

struct lite3d_list_node *
lite3d_list_remove_first_link(struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *ret;

    if ((ret = lite3d_list_first_link(lite3d_list)))
    {
        lite3d_list_unlink_link(ret);
    }

    return ret;
}

struct lite3d_list_node *
lite3d_list_remove_last_link(struct lite3d_list *lite3d_list)
{
    struct lite3d_list_node *ret;

    if ((ret = lite3d_list_last_link(lite3d_list)))
    {
        lite3d_list_unlink_link(ret);
    }

    return ret;
}

struct lite3d_list_node *
lite3d_list_next(struct lite3d_list_node *link)
{
    return link->next;
}

struct lite3d_list_node *
lite3d_list_prev(struct lite3d_list_node *link)
{
    return link->prev;
}

struct lite3d_list_node *
lite3d_list_find_arg(struct lite3d_list *in_list, lite3d_list_node_arg_predicate_t predicate, void *arg)
{
    struct lite3d_list_node *it;


    for (it = in_list->l.next; it != &(in_list->l); it = lite3d_list_next(it))
    {
        if (predicate(it, arg))
            return it;
    }

    return NULL;
}

void
lite3d_list_iterate(struct lite3d_list *in_list, lite3d_list_iterate_t func)
{
    struct lite3d_list_node *it;


    for (it = in_list->l.next; it != &(in_list->l); it = lite3d_list_next(it))
    {
        if (func(it))
            return;
    }
}
