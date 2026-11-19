/*
 * File: bookmark.c
 * Bookmark handling for vibedillo - now uses in-process bookmarks_store.
 *
 * Copyright 2002-2007 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>

#include "msg.h"
#include "history.h"
#include "capi.h"
#include "bookmark.h"
#include "bookmarks_store.h"


/* Global store instance */
static BookmarkStore *g_bookmarks_store = NULL;

/**
 * Add a bookmark through the bookmarks store
 */
void a_Bookmarks_add(BrowserWindow *bw, const DilloUrl *url)
{
   (void)bw; /* unused now */
   const char *title;

   dReturn_if_fail(url != NULL);

   /* if the page has no title, we'll use the url string */
   title = a_History_get_title_by_url(url, 1);

   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();

   bookmarks_store_add(g_bookmarks_store, URL_STR(url), title);
}

/* DPI URL handlers for about: bookmarks */
char *a_Bookmarks_get_page(void)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_get_page(g_bookmarks_store);
}

char *a_Bookmarks_get_modify_page(void)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_get_modify_page(g_bookmarks_store);
}

char *a_Bookmarks_get_add_section_page(void)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_get_add_section_page(g_bookmarks_store);
}

char *a_Bookmarks_get_add_url_page(void)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_get_add_url_page(g_bookmarks_store);
}

char *a_Bookmarks_get_update_page(const char *query_string)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_get_update_page(g_bookmarks_store, query_string);
}

int a_Bookmarks_process_modify(const char *query_string)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_process_modify(g_bookmarks_store, query_string);
}

int a_Bookmarks_check_modify(const char *query_string)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
   return bookmarks_store_check_modify(g_bookmarks_store, query_string);
}

const char *a_Bookmarks_get_file_path(void)
{
   return bookmarks_store_get_file_path();
}

void a_Bookmarks_init(void)
{
   if (!g_bookmarks_store)
      g_bookmarks_store = bookmarks_store_init();
}

void a_Bookmarks_free(void)
{
   if (g_bookmarks_store) {
      bookmarks_store_free(g_bookmarks_store);
      g_bookmarks_store = NULL;
   }
}