/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#ifndef LITE3D_LIST_H
#define	LITE3D_LIST_H

#include <lite3d/lite3d_common.h>

typedef struct lite3d_list_node
{
    struct lite3d_list_node *next, *prev;
} lite3d_list_node;

typedef struct lite3d_list
{
    struct lite3d_list_node l;
} lite3d_list;

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
LITE3D_CEXPORT struct lite3d_list *
lite3d_list_init(struct lite3d_list *lite3d_list);

/**
 * @param link
 *   Pointer to the link.
 * @return
 *   The argument.
 */
LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_link_init(struct lite3d_list_node *link);

LITE3D_CEXPORT int
lite3d_list_is_empty(struct lite3d_list *lite3d_list);

LITE3D_CEXPORT int
lite3d_list_alone_link(struct lite3d_list_node *link);

/* Unlinking an element from its lite3d_list. */
LITE3D_CEXPORT void
lite3d_list_unlink_link(struct lite3d_list_node *link);

/* Retrieving the first/last elements. */
LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_first_link(struct lite3d_list *lite3d_list);

LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_last_link(struct lite3d_list *lite3d_list);

/* Adding an element at the lite3d_list ends. */
LITE3D_CEXPORT void
lite3d_list_add_first_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list);

LITE3D_CEXPORT void
lite3d_list_add_last_link(struct lite3d_list_node *new_link, struct lite3d_list *lite3d_list);

/* Insertion of an element near a given one. */
LITE3D_CEXPORT void
lite3d_list_insert_before_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link);

LITE3D_CEXPORT void
lite3d_list_insert_after_link(struct lite3d_list_node *new_link, struct lite3d_list_node *link);

/* Popping an element from the lite3d_list ends. */
LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_remove_first_link(struct lite3d_list *lite3d_list);

LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_remove_last_link(struct lite3d_list *lite3d_list);

LITE3D_CEXPORT void
lite3d_list_bulk_add_first(struct lite3d_list *from_list, struct lite3d_list *to_list);

LITE3D_CEXPORT void
lite3d_list_bulk_add_last(struct lite3d_list *from_list, struct lite3d_list *to_list);

LITE3D_CEXPORT void
lite3d_list_bulk_insert_before_link(struct lite3d_list *from_list, struct lite3d_list_node *link);

LITE3D_CEXPORT void
lite3d_list_bulk_insert_after_link(struct lite3d_list *from_list, struct lite3d_list_node *link);

LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_next(struct lite3d_list_node *link);

LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_prev(struct lite3d_list_node *link);

LITE3D_CEXPORT struct lite3d_list_node *
lite3d_list_find_arg(struct lite3d_list *in_list, lite3d_list_node_arg_predicate_t predicate, void *arg);

LITE3D_CEXPORT void
lite3d_list_iterate(struct lite3d_list *in_list, lite3d_list_iterate_t func);

#endif	/* LITE3D_LIST_H */

