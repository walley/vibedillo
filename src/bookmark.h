#ifndef __BOOKMARK_H__
#define __BOOKMARK_H__

#include "bw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void a_Bookmarks_add(BrowserWindow *bw, const DilloUrl *url);

/* In-process bookmarks store API */
void a_Bookmarks_init(void);
void a_Bookmarks_free(void);
char *a_Bookmarks_get_page(void);
char *a_Bookmarks_get_modify_page(void);
char *a_Bookmarks_get_add_section_page(void);
char *a_Bookmarks_get_add_url_page(void);
char *a_Bookmarks_get_update_page(const char *query_string);
int a_Bookmarks_process_modify(const char *query_string);
int a_Bookmarks_check_modify(const char *query_string);
const char *a_Bookmarks_get_file_path(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOKMARK_H__ */