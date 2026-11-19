/*
 * File: cookies_store.h
 * In-process cookie store for vibedillo.
 * Ported from dpi/cookies.c (original Dillo cookie DPI).
 *
 * Copyright 2001 Lars Clausen   <lrclause@cs.uiuc.edu>
 *                Jörgen Viksell <jorgen.viksell@telia.com>
 * Copyright 2002-2007 Jorge Arellano Cid <jcid@dillo.org>
 * Copyright 2026 Rodrigo Arias Mallo <rodarima@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __COOKIES_STORE_H__
#define __COOKIES_STORE_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the cookie store (load from disk, parse cookiesrc) */
void cookies_store_init(void);

/* Free all resources and save to disk */
void cookies_store_freeall(void);

/* Set a cookie from a Set-Cookie header string.
 * Returns 0 on success, negative on failure (disabled/denied/rejected). */
int cookies_store_set(const char *cookie_string,
                      const char *url_host,
                      const char *url_path,
                      const char *server_date);

/* Get cookies for an HTTP request.
 * Returns a malloc'd string like "Cookie: name=value; name2=value2\r\n"
 * or an empty string if none. Caller must dFree(). */
char *cookies_store_get(const char *url_host,
                        const char *url_path,
                        const char *url_scheme);

/* Check if cookies are enabled for a host (access control) */
bool_t cookies_store_check_host(const char *host);

/* Callback for cookies_store_iterate() */
typedef void (*cookies_store_iter_cb)(const char *domain, const char *path,
                                      const char *name, const char *value,
                                      bool_t host_only, bool_t secure,
                                      bool_t session_only, time_t expires_at,
                                      void *userdata);

/* Call 'cb' for every cookie currently in the store */
void cookies_store_iterate(cookies_store_iter_cb cb, void *userdata);

#ifdef __cplusplus
}
#endif

#endif /* !__COOKIES_STORE_H__ */