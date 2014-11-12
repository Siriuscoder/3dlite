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
#ifndef LITE3D_LIST_H
#define	LITE3D_LIST_H

#include <3dlite/common.h>

struct lite3d_list_node
{
    unsigned int poison;
    struct lite3d_list *lite3d_list;
    struct lite3d_list_node *next, *prev;
};

struct lite3d_list
{
    struct lite3d_list_node l;
};

/**
 * Type of functions, returning true or false for given tree_link with some argument.
 */
typedef int
(*lite3d_list_node_arg_predicate_t)(struct lite3d_list_node *link, void *arg);
typedef int
(*lite3d_list_iterate_t)(struct lite3d_list_node *link);

/**
 * @param lite3d_list
 *   Pointer to the lite3d_list being initialized.
 * @return
 *   The argument.
 */
struct lite3d_list *
lite3d_list_init(struct lite3d_list *lite3d_list);

/**
 * @param link
 *   Pointer to the link.
 * @return
 *   The argument.
 */
struct lite3d_list_node *
lite3d_list_link_init(struct lite3d_list_node *link);

int
lite3d_list_is_empty(struct lite3d_list *lite3d_list);

int
lite3d_list_alone_link(struct lite3d_list_node *link);

/* Unlinking an element from its lite3d_list. */
void
lite3d_list_unlink_link(struct lite3d_list_node *link);

/* Retrieving the first/last elements. */
struct lite3d_list_node *
lite3d_list_first_link(struct lite3d_list *lite3d_list);

struct lite3d_list_node *
lite3d_list_last_link(struct lite3d_list *lite3d_list);

/* Adding an element at the lite3d_list ends. */
void
lite3d_list_add_first_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list);

void
lite3d_list_add_last_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list);

/* Insertion of an element near a given one. */
void
lite3d_list_insert_before_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link);

void
lite3d_list_insert_after_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link);

/* Popping an element from the lite3d_list ends. */
struct lite3d_list_node *
lite3d_list_remove_first_link(struct lite3d_list *lite3d_list);

struct lite3d_list_node *
lite3d_list_remove_last_link(struct lite3d_list *lite3d_list);

void
lite3d_list_bulk_add_first(struct lite3d_list *from_list, struct lite3d_list *to_list);

void
lite3d_list_bulk_add_last(struct lite3d_list *from_list, struct lite3d_list *to_list);

void
lite3d_list_bulk_insert_before_link(struct lite3d_list *from_list, struct lite3d_list_node *link);

void
lite3d_list_bulk_insert_after_link(struct lite3d_list *from_list, struct lite3d_list_node *link);

struct lite3d_list_node *
lite3d_list_next(struct lite3d_list_node *link);

struct lite3d_list_node *
lite3d_list_prev(struct lite3d_list_node *link);

struct lite3d_list_node *
lite3d_list_find_arg(struct lite3d_list *in_list, lite3d_list_node_arg_predicate_t predicate, void *arg);

void
lite3d_list_iterate(struct lite3d_list *in_list, lite3d_list_iterate_t func);

#endif	/* LITE3D_LIST_H */

