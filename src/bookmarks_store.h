/*
 * File: bookmarks_store.h
 * In-process bookmarks store for vibedillo.
 * Ported from dpi/bookmarks.c (original Dillo bookmarks DPI).
 *
 * Copyright 2002-2007 Jorge Arellano Cid <jcid@dillo.org>
 * Copyright 2024 Rodrigo Arias Mallo <rodarima@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __BOOKMARKS_STORE_H__
#define __BOOKMARKS_STORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque bookmark handle */
typedef struct BookmarkStore BookmarkStore;

/* Initialize the bookmarks store (load from disk) */
BookmarkStore *bookmarks_store_init(void);

/* Free all resources and save to disk */
void bookmarks_store_free(BookmarkStore *store);

/* Add a bookmark to section 0 (Unclassified) */
void bookmarks_store_add(BookmarkStore *store, const char *url, const char *title);

/* Get the main bookmarks page as HTML */
char *bookmarks_store_get_page(BookmarkStore *store);

/* Get the modify page as HTML */
char *bookmarks_store_get_modify_page(BookmarkStore *store);

/* Get the "add section" page as HTML */
char *bookmarks_store_get_add_section_page(BookmarkStore *store);

/* Get the "add URL" page as HTML */
char *bookmarks_store_get_add_url_page(BookmarkStore *store);

/* Get the "update" page as HTML for marked items */
char *bookmarks_store_get_update_page(BookmarkStore *store, const char *query_string);

/* Process a modify request (delete, move, modify, add_section, add_url) */
int bookmarks_store_process_modify(BookmarkStore *store, const char *query_string);

/* Check if a modify request is valid */
int bookmarks_store_check_modify(BookmarkStore *store, const char *query_string);

/* Get the bookmarks file path */
const char *bookmarks_store_get_file_path(void);

#ifdef __cplusplus
}
#endif

#endif /* !__BOOKMARKS_STORE_H__ */