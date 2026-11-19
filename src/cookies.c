/*
 * File: cookies.c
 * Client-side cookie handling for vibedillo.
 * Now uses the in-process cookies_store module.
 *
 * Copyright 2001 Lars Clausen   <lrclause@cs.uiuc.edu>
 *                Jörgen Viksell <jorgen.viksell@telia.com>
 * Copyright 2025 Rodrigo Arias Mallo <rodarima@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "msg.h"

#ifdef DISABLE_COOKIES

/**
 * Initialize the cookies module
 */
void a_Cookies_init(void)
{
   MSG("Cookies: absolutely disabled at compilation time.\n");
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "IO/Url.h"
#include "list.h"
#include "cookies.h"
#include "cookies_store.h"


/**
 * Initialize the cookies module
 * (The 'disabled' variable is writable only within a_Cookies_init)
 */
void a_Cookies_init(void)
{
   cookies_store_init();
}

/**
 * Flush cookies to disk and free all the memory allocated.
 */
void a_Cookies_freeall(void)
{
   cookies_store_freeall();
}

/**
 * Set the value corresponding to the cookie string
 */
void a_Cookies_set(Dlist *cookie_strings, const DilloUrl *set_url,
                   const char *date)
{
   if (!cookies_store_check_host(URL_HOST(set_url)))
      return;

   const char *path = URL_PATH_(set_url);
   if (!path)
      path = "/";

   for (int i = 0; i < dList_length(cookie_strings); ++i) {
      const char *cookie_string = dList_nth_data(cookie_strings, i);
      cookies_store_set(cookie_string, URL_HOST(set_url), path, date);
   }
}

/**
 * Return a string containing cookie data for an HTTP query.
 */
char *a_Cookies_get_query(const DilloUrl *query_url, const DilloUrl *requester,
                          int is_root_url)
{
   if (!cookies_store_check_host(URL_HOST(query_url)))
      return dStrdup("");

   if (requester != NULL && !is_root_url &&
       !a_Url_same_organization(query_url, requester)) {
      MSG("Cookies: not sent for non-root request by '%s' for '%s'\n",
          URL_HOST(requester), URL_HOST(query_url));
      return dStrdup("");
   }

   const char *path = URL_PATH_(query_url);
   if (!path)
      path = "/";

   return cookies_store_get(URL_HOST(query_url), path, URL_SCHEME(query_url));
}

#endif /* !DISABLE_COOKIES */