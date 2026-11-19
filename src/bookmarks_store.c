/*
 * File: bookmarks_store.c
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

#ifdef DISABLE_COOKIES

void bookmarks_store_init(void) { MSG("Bookmarks: disabled at compile time.\n"); }
void bookmarks_store_free(void *store) { (void)store; }
void bookmarks_store_add(void *store, const char *url, const char *title) { (void)store; (void)url; (void)title; }
char *bookmarks_store_get_page(void *store) { (void)store; return dStrdup(""); }
char *bookmarks_store_get_modify_page(void *store) { (void)store; return dStrdup(""); }
char *bookmarks_store_get_add_section_page(void *store) { (void)store; return dStrdup(""); }
char *bookmarks_store_get_add_url_page(void *store) { (void)store; return dStrdup(""); }
char *bookmarks_store_get_update_page(void *store, const char *q) { (void)store; (void)q; return dStrdup(""); }
int bookmarks_store_process_modify(void *store, const char *q) { (void)store; (void)q; return -1; }
int bookmarks_store_check_modify(void *store, const char *q) { (void)store; (void)q; return -1; }
const char *bookmarks_store_get_file_path(void) { return ""; }

#else

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <stddef.h>

#include "msg.h"
#include "list.h"
#include "bookmarks_store.h"
#include "dlib/dlib.h"


/* Opaque structures */
struct BookmarkStore {
    char *BmFile;
    time_t BmFileTimeStamp;
    Dlist *B_bms;
    int bm_key;
    Dlist *B_secs;
    int sec_key;
    int modify_page_num;
};

typedef struct {
    int key;
    int section;
    char *url;
    char *title;
} BmRec;

typedef struct {
    int section;
    char *title;
    int o_sec;
} BmSec;

/* Forward declarations */
static int Bms_node_by_key_cmp(const void *node, const void *key);
static int Bms_node_by_section_cmp(const void *node, const void *key);
static int Bms_sec_by_number_cmp(const void *node, const void *key);
static BmRec *Bms_get(BookmarkStore *store, int key);
static BmSec *Bms_get_sec(BookmarkStore *store, int key);
static void Bms_add(BookmarkStore *store, int section, char *url, char *title);
static void Bms_sec_add(BookmarkStore *store, char *title);
static void Bms_del(BookmarkStore *store, int key);
static void Bms_sec_del(BookmarkStore *store, int section);
static void Bms_move(BookmarkStore *store, int key, int target_section);
static void Bms_update_title(BookmarkStore *store, int key, char *n_title);
static void Bms_update_sec_title(BookmarkStore *store, int key, char *n_title);
static void Bms_free(BookmarkStore *store);
static void Bms_normalize(BookmarkStore *store);
static void Bms_check_import(BookmarkStore *store);
static int Bms_load(BookmarkStore *store);
static int Bms_cond_load(BookmarkStore *store);
static int Bms_save(BookmarkStore *store);
static int Bmsrv_add_bm(BookmarkStore *store, char *url, char *title);
static void Bmsrv_count_urls_and_sections(char *url, int *n_sec, int *n_url);
static char *make_one_line_str(char *str);
static void Unencode_str(char *e_str);
static char *Escape_html_str(const char *str);
static char *Unescape_html_str(const char *str);
static char *Escape_uri_str(const char *url, const char *chars);
static int Bmsrv_modify_delete(BookmarkStore *store, char *url);
static int Bmsrv_modify_move(BookmarkStore *store, char *url);
static int Bmsrv_modify_update(BookmarkStore *store, char *url);
static int Bmsrv_modify_add_section(BookmarkStore *store, char *url);
static int Bmsrv_modify_add_url(BookmarkStore *store, char *url);
static int Bmsrv_check_modify_request(BookmarkStore *store, char *url);
static int Bmsrv_process_modify_request(BookmarkStore *store, char *url);
static char *send_bm_page(BookmarkStore *store);
static char *send_modify_page(BookmarkStore *store);
static char *send_modify_add_section_page(void);
static char *send_modify_add_url_page(void);
static char *send_modify_update_page(BookmarkStore *store, char *url);

/* HTML templates */
#define DOCTYPE "<!DOCTYPE html>\n"

#define CSS_STYLE     "<style type='text/css'>\n"     "body { margin: 2em; background: light-dark(white, #111111); font-size: 14px }\n"     "ul { font-size: inherit; margin-left: 1em; margin-top: 0.5em; list-style-type: none; }\n"     "ul.main { list-style-type: none; }\n"     "li { min-height: 1.6em; }\n"     "div.modify-menu { background-color: light-dark(#f0f0f0, #333); border: solid 1px light-dark(#ccc, #666); padding: 1em }\n"     "nav ul { margin-top: 1em }\n"     "nav li { display: inline; }\n"     "a { color: light-dark(black, #88aaff); }\n"     "input[type=text] { background: light-dark(white, #222); color: light-dark(black, #eee); border: 1px solid light-dark(#ccc, #666); }\n"     "input[type=submit] { background: light-dark(#ddd, #444); color: light-dark(black, #eee); border: 1px solid light-dark(#ccc, #666); padding: 0.2em 0.5em; }\n"     "select { background: light-dark(white, #222); color: light-dark(black, #eee); border: 1px solid light-dark(#ccc, #666); }\n"     "option { background: light-dark(white, #222); color: light-dark(black, #eee); }\n"     "</style>\n"

static const char *mainpage_header =
DOCTYPE
"<html>\n"
"<head>\n"
"<title>Bookmarks</title>\n"
"<meta name=\"color-scheme\" content=\"dark light\">\n"
CSS_STYLE
"</head>\n"
"<body id='dillo_bm'>\n"
"\n"
"  <h1>Bookmarks</h1>\n"
"  <div style='float: right; padding: 1em'>\n"
"    [<a href='dpi:/bm/modify'>Modify</a>]\n"
"  </div>\n";

static const char *modifypage_header =
DOCTYPE
"<html>\n"
"<head>\n"
"<title>Bookmarks</title>\n"
"<meta name=\"color-scheme\" content=\"dark light\">\n"
CSS_STYLE
"</head>\n"
"<body id='dillo_bm'>\n"
"  <h1>Bookmarks</h1>\n"
"\n"
"<form action='modify'>\n"
"<div class='modify-menu'>\n"
"  <div style='float: right'>\n"
"    [<a href='dpi:/bm/'>Dismiss</a>]\n"
"  </div>\n"
"  <p>Select an operation\n"
"  <select name='operation'>\n"
"    <option value='none' selected>--\n"
"    <option value='delete'>Delete\n"
"    <option value='move'>Move\n"
"    <option value='modify'>Modify\n"
"    <option value='add_sec'>Add Section\n"
"    <option value='add_url'>Add URL\n"
"  </select>\n"
"  , mark its operands and\n"
"  <input type='submit' name='submit' value='Submit'>\n"
"  <p>You can also modify the bookmarks by editing the\n"
"  <a href='file:~/.config/vibedillo/bm.txt'><code>~/.config/vibedillo/bm.txt</code></a> file.\n"
"</div>\n";

static const char *mainpage_sections_header =
"<nav>\n"
"  <ul>\n";

#define modifypage_sections_header mainpage_sections_header

static const char *mainpage_sections_item =
"    <li><a href='#s%d'>%s</a></li>\n";

static const char *sections_sep =
" | \n";

static const char *modifypage_sections_item =
"    <li><input type='checkbox' name='s%d'><a href='#s%d'>%s</a></li>\n";

static const char *mainpage_sections_footer =
"  </ul>\n"
"</nav>\n";

#define modifypage_sections_footer mainpage_sections_footer

static const char *mainpage_middle1 = "\n";

static const char *modifypage_middle1 = "\n";

static const char *mainpage_section_card_header =
"<h2 id='s%d'>%s</h2>\n"
"<ul class='main'>\n";

static const char *modifypage_section_card_header =
"<h2 id='s%d'>%s</h2>\n"
"<ul class='main'>\n";

static const char *mainpage_section_card_item =
"  <li>\n"
"    <a href='%s'>%s</a>\n"
"  </li>\n";

static const char *modifypage_section_card_item =
"  <li>\n"
"    <input type='checkbox' name='url%d'>\n"
"    <a href='%s'>%s</a>\n"
"  </li>\n";

static const char *mainpage_section_card_footer =
"</ul>\n";

#define modifypage_section_card_footer mainpage_section_card_footer

static const char *mainpage_footer =
"</body>\n"
"</html>\n";

static const char *modifypage_footer =
"</form>\n"
"</body>\n"
"</html>\n";

static const char *modifypage_add_section_page =
DOCTYPE
"<html>\n"
"<head>\n"
"<title>Bookmarks</title>\n"
"<meta name=\"color-scheme\" content=\"dark light\">\n"
CSS_STYLE
"</head>\n"
"<body id='dillo_bm'>\n"
"<h1>Bookmarks: New section</h1>\n"
"<form action='modify'>\n"
"  <input type='hidden' name='operation' value='add_section'>\n"
"  <p>\n"
"   New section title:\n"
"   <input type='text' name='title' size='32'>\n"
"   <input type='submit' name='submit' value='Submit'>\n"
"   or <a href='dpi:/bm/'>Cancel</a>\n"
"  </p>\n"
"</form>\n"
"</body>\n"
"</html>\n";

static const char *modifypage_update_header =
DOCTYPE
"<html>\n"
"<head>\n"
"<title>Bookmarks</title>\n"
"<meta name=\"color-scheme\" content=\"dark light\">\n"
CSS_STYLE
"</head>\n"
"<body id='dillo_bm'>\n"
"<h1>Bookmarks: Update</h1>\n"
"<form action='modify'>\n"
"<input type='hidden' name='operation' value='modify2'>\n";

static const char *modifypage_update_title =
"<h2>%s</h2>\n";

static const char *modifypage_update_item_header =
"<ul>\n";

static const char *modifypage_update_item =
"<li style='padding: 0.5em'>\n"
"  <input type='text' name='title%d' size='64' value='%s'>\n"
"  <br>URL: <code>%s</code>\n"
"</li>\n";

static const char *modifypage_update_item2 =
"<li style='padding: 0.5em'>\n"
"  <input type='text' name='s%d' size='64' value='%s'>\n"
"</li>\n";

static const char *modifypage_update_item_footer =
"</ul>\n";

static const char *modifypage_update_footer =
"<input type='submit' name='submit' value='Submit'>\n"
"</form>\n"
"</body>\n"
"</html>\n";

static const char *modifypage_add_url =
DOCTYPE
"<html>\n"
"<head>\n"
"<title>Bookmarks</title>\n"
"<meta name=\"color-scheme\" content=\"dark light\">\n"
CSS_STYLE
"</head>\n"
"<body id='dillo_bm'>\n"
"<h1>Bookmarks: Add URL</h1>\n"
"<form action='modify'>\n"
"<input type='hidden' name='operation' value='add_url2'>\n"
"<p>Title:\n"
"  <input type='text' name='title' size='64'>\n"
"</p>\n"
"<p>URL:\n"
"  <input type='text' name='url' size='64'>\n"
"</p>\n"
"<p>\n"
"  <input type='submit' name='submit' value='Submit'>\n"
"  or <a href='dpi:/bm/'>Cancel</a>\n"
"</p>\n"
"</form>\n"
"</body>\n"
"</html>\n";

/* -- ADT for bookmarks ---------------------------------------------------- */

static int Bms_node_by_key_cmp(const void *node, const void *key)
{
   return ((BmRec *)node)->key - VOIDP2INT(key);
}

static int Bms_node_by_section_cmp(const void *node, const void *key)
{
   return ((BmRec *)node)->section - VOIDP2INT(key);
}

static int Bms_sec_by_number_cmp(const void *node, const void *key)
{
   return ((BmSec *)node)->section - VOIDP2INT(key);
}

static BmRec *Bms_get(BookmarkStore *store, int key)
{
   return dList_find_custom(store->B_bms, INT2VOIDP(key), Bms_node_by_key_cmp);
}

static BmSec *Bms_get_sec(BookmarkStore *store, int key)
{
   return dList_find_custom(store->B_secs, INT2VOIDP(key), Bms_sec_by_number_cmp);
}

static void Bms_add(BookmarkStore *store, int section, char *url, char *title)
{
   BmRec *bm_node = dNew(BmRec, 1);
   bm_node->key = ++store->bm_key;
   bm_node->section = section;
   bm_node->url = Escape_uri_str(url, "'");
   bm_node->title = Escape_html_str(title);
   dList_append(store->B_bms, bm_node);
}

static void Bms_sec_add(BookmarkStore *store, char *title)
{
   BmSec *sec_node = dNew(BmSec, 1);
   sec_node->section = store->sec_key++;
   sec_node->title = Escape_html_str(title);
   dList_append(store->B_secs, sec_node);
}

static void Bms_del(BookmarkStore *store, int key)
{
   BmRec *bm_node = dList_find_custom(store->B_bms, INT2VOIDP(key), Bms_node_by_key_cmp);
   if (bm_node) {
      dList_remove(store->B_bms, bm_node);
      dFree(bm_node->title);
      dFree(bm_node->url);
      dFree(bm_node);
   }
   if (dList_length(store->B_bms) == 0)
      store->bm_key = 0;
}

static void Bms_sec_del(BookmarkStore *store, int section)
{
   BmSec *sec_node = dList_find_custom(store->B_secs, INT2VOIDP(section), Bms_sec_by_number_cmp);
   if (sec_node) {
      dList_remove(store->B_secs, sec_node);
      dFree(sec_node->title);
      dFree(sec_node);

      BmRec *bm_node;
      while ((bm_node = dList_find_custom(store->B_bms, INT2VOIDP(section), Bms_node_by_section_cmp))) {
         Bms_del(store, bm_node->key);
      }
   }
   if (dList_length(store->B_secs) == 0)
      store->sec_key = 0;
}

static void Bms_move(BookmarkStore *store, int key, int target_section)
{
   BmRec *bm_node = dList_find_custom(store->B_bms, INT2VOIDP(key), Bms_node_by_key_cmp);
   if (bm_node) {
      bm_node->section = target_section;
   }
}

static void Bms_update_title(BookmarkStore *store, int key, char *n_title)
{
   BmRec *bm_node = dList_find_custom(store->B_bms, INT2VOIDP(key), Bms_node_by_key_cmp);
   if (bm_node) {
      dFree(bm_node->title);
      bm_node->title = Escape_html_str(n_title);
   }
}

static void Bms_update_sec_title(BookmarkStore *store, int key, char *n_title)
{
   BmSec *sec_node = dList_find_custom(store->B_secs, INT2VOIDP(key), Bms_sec_by_number_cmp);
   if (sec_node) {
      dFree(sec_node->title);
      sec_node->title = Escape_html_str(n_title);
   }
}

static void Bms_free(BookmarkStore *store)
{
   BmRec *bm_node;
   BmSec *sec_node;

   while ((bm_node = dList_nth_data(store->B_bms, 0))) {
      Bms_del(store, bm_node->key);
   }
   while ((sec_node = dList_nth_data(store->B_secs, 0))) {
      Bms_sec_del(store, sec_node->section);
   }
}

static void Bms_normalize(BookmarkStore *store)
{
   BmRec *bm_node;
   BmSec *sec_node;
   int i, j;

   if (dList_length(store->B_secs) == 0)
      Bms_sec_add(store, "Unclassified");

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      sec_node->o_sec = sec_node->section;
      sec_node->section = i;
   }

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      if (sec_node->section != sec_node->o_sec) {
         for (j = 0; (bm_node = dList_nth_data(store->B_bms, j)); ++j) {
            if (bm_node->section == sec_node->o_sec)
               bm_node->section = sec_node->section;
         }
      }
   }
}

static void Bms_check_import(BookmarkStore *store)
{
   char *OldBmFile;
   char *cmd1 = "echo \":s0: Unclassified\" > %s";
   char *cmd2 = "grep -i \"href\" %s | sed -e 's/<li><A HREF=\"/s0 /' -e 's/\">/ /' -e 's/<.*$//' >> %s";
   Dstr *dstr = dStr_new("");
   int rc;

   if (access(store->BmFile, F_OK) != 0) {
      OldBmFile = dStrconcat(dGethomedir(), "/.dillo/bookmarks.html", NULL);
      if (access(OldBmFile, F_OK) == 0) {
         dStr_sprintf(dstr, cmd1, store->BmFile);
         rc = system(dstr->str);
         if (rc == 127) {
            MSG("Bookmarks: /bin/sh could not be executed\n");
         } else if (rc == -1) {
            MSG("Bookmarks: process creation failure: %s\n", dStrerror(errno));
         }
         dStr_sprintf(dstr, cmd2, OldBmFile, store->BmFile);
         rc = system(dstr->str);
         if (rc == 127) {
            MSG("Bookmarks: /bin/sh could not be executed\n");
         } else if (rc == -1) {
            MSG("Bookmarks: process creation failure: %s\n", dStrerror(errno));
         }
         dStr_free(dstr, TRUE);
         dFree(OldBmFile);
      }
   }
}

static int Bms_load(BookmarkStore *store)
{
   FILE *BmTxt;
   char *buf, *p, *url, *title, *u_title;
   int section;
   struct stat TimeStamp;

   Bms_free(store);

   if (!(BmTxt = fopen(store->BmFile, "r"))) {
      perror("[fopen]");
      return 1;
   }

   while ((buf = dGetline(BmTxt)) != NULL) {
      if (buf[0] == 's') {
         section = strtol(buf + 1, NULL, 10);
         p = strchr(buf, ' ');
         if (!p) { dFree(buf); continue; }
         *p = 0;
         url = ++p;
         p = strchr(p, ' ');
         if (!p) { dFree(buf); continue; }
         *p = 0;
         title = ++p;
         p = strchr(p, '\n');
         if (!p) { dFree(buf); continue; }
         *p = 0;
         u_title = Unescape_html_str(title);
         Bms_add(store, section, url, u_title);
         dFree(u_title);

      } else if (buf[0] == ':' && buf[1] == 's') {
         p = strchr(buf + 2, ' ');
         if (!p) { dFree(buf); continue; }
         title = ++p;
         p = strchr(p, '\n');
         if (!p) { dFree(buf); continue; }
         *p = 0;
         Bms_sec_add(store, title);

      } else {
         /* skip */
      }
      dFree(buf);
   }
   fclose(BmTxt);

   stat(store->BmFile, &TimeStamp);
   store->BmFileTimeStamp = TimeStamp.st_mtime;

   return 0;
}

static int Bms_cond_load(BookmarkStore *store)
{
   struct stat TimeStamp;

   if (stat(store->BmFile, &TimeStamp) != 0) {
      Bms_check_import(store);
      if (stat(store->BmFile, &TimeStamp) != 0)
         TimeStamp.st_mtime = 0;
   }

   if (!store->BmFileTimeStamp || !dList_length(store->B_bms) || !dList_length(store->B_secs) ||
       store->BmFileTimeStamp < TimeStamp.st_mtime) {
      Bms_load(store);
      return 1;
   }
   return 0;
}

static int Bms_save(BookmarkStore *store)
{
   FILE *BmTxt;
   BmRec *bm_node;
   BmSec *sec_node;
   struct stat BmStat;
   char *u_title;
   int i, j;
   Dstr *dstr = dStr_new("");

   if (stat(store->BmFile, &BmStat) == 0 && BmStat.st_size > 256) {
      char *BmFileBak = dStrconcat(store->BmFile, ".bak", NULL);
      rename(store->BmFile, BmFileBak);
      dFree(BmFileBak);
   }

   if (!(BmTxt = fopen(store->BmFile, "w"))) {
      perror("[fopen]");
      dStr_free(dstr, TRUE);
      return 1;
   }

   Bms_normalize(store);

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      u_title = Unescape_html_str(sec_node->title);
      dStr_sprintf(dstr, ":s%d: %s\n", sec_node->section, u_title);
      fwrite(dstr->str, (size_t)dstr->len, 1, BmTxt);
      dFree(u_title);
   }

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      for (j = 0; (bm_node = dList_nth_data(store->B_bms, j)); ++j) {
         if (bm_node->section == sec_node->section) {
            u_title = Unescape_html_str(bm_node->title);
            dStr_sprintf(dstr, "s%d %s %s\n",
                         bm_node->section, bm_node->url, u_title);
            fwrite(dstr->str, (size_t)dstr->len, 1, BmTxt);
            dFree(u_title);
         }
      }
   }

   dStr_free(dstr, TRUE);
   fclose(BmTxt);

   stat(store->BmFile, &BmStat);
   store->BmFileTimeStamp = BmStat.st_mtime;

   return 0;
}

static int Bmsrv_add_bm(BookmarkStore *store, char *url, char *title)
{
   char *u_title = Unescape_html_str(title);
   Bms_add(store, 0, url, u_title);
   dFree(u_title);
   Bms_save(store);
   return 0;
}

static void Bmsrv_count_urls_and_sections(char *url, int *n_sec, int *n_url)
{
   char *p, *q;
   int i;

   *n_sec = *n_url = 0;
   if ((p = strchr(url, '?'))) {
      for (q = p; (q = strstr(q, "&url")); ++q) {
         for (i = 0; dIsdigit(q[4+i]); ++i);
         *n_url += (q[4+i] == '=') ? 1 : 0;
      }
      for (q = p; (q = strstr(q, "&s")); ++q) {
         for (i = 0; dIsdigit(q[2+i]); ++i);
         *n_sec += (q[2+i] == '=') ? 1 : 0;
      }
   }
}

static char *make_one_line_str(char *str)
{
   char *new_str;
   int i, j, n;

   for (i = 0, n = 0; str[i]; ++i)
      if (str[i] == ' ')
         ++n;

   new_str = dNew(char, strlen(str) + 6*n + 1);
   new_str[0] = 0;

   for (i = 0, j = 0; str[i]; ++i) {
      if (str[i] == ' ') {
         strcpy(new_str + j, "&nbsp;");
         j += 6;
      } else {
         new_str[j] = str[i];
         new_str[++j] = 0;
      }
   }
   return new_str;
}

static void Unencode_str(char *e_str)
{
   char *p, *e;

   for (p = e = e_str; *e; e++, p++) {
      if (*e == '+') {
         *p = ' ';
      } else if (*e == '%') {
         if (dStrnAsciiCasecmp(e, "%0D%0A", 6) == 0) {
            *p = '\n';
            e += 5;
         } else {
            *p = (dIsdigit(e[1]) ? (e[1] - '0') : (e[1] - 'A' + 10)) * 16 +
                 (dIsdigit(e[2]) ? (e[2] - '0') : (e[2] - 'A' + 10));
            e += 2;
         }
      } else {
         *p = *e;
      }
   }
   *p = 0;
}

static char *Escape_html_str(const char *str)
{
   if (!str) return dStrdup("");
   size_t len = strlen(str);
   char *out = dNew(char, len * 6 + 1);
   size_t j = 0;
   for (size_t i = 0; i < len; i++) {
      switch (str[i]) {
         case '&': strcpy(out + j, "&"); j += 5; break;
         case '<': strcpy(out + j, "<"); j += 4; break;
         case '>': strcpy(out + j, ">"); j += 4; break;
         case '"': strcpy(out + j, "&quot;"); j += 6; break;
         case '\'': strcpy(out + j, "'"); j += 5; break;
         default: out[j++] = str[i]; break;
      }
   }
   out[j] = 0;
   return out;
}

static char *Unescape_html_str(const char *str)
{
   if (!str) return dStrdup("");
   size_t len = strlen(str);
   char *out = dNew(char, len + 1);
   size_t j = 0;
   for (size_t i = 0; i < len; i++) {
      if (str[i] == '&' && i + 5 < len) {
         if (strncmp(str + i, "&", 5) == 0) { out[j++] = '&'; i += 4; continue; }
         if (strncmp(str + i, "<", 4) == 0) { out[j++] = '<'; i += 3; continue; }
         if (strncmp(str + i, ">", 4) == 0) { out[j++] = '>'; i += 3; continue; }
         if (strncmp(str + i, "&quot;", 6) == 0) { out[j++] = '"'; i += 5; continue; }
         if (strncmp(str + i, "'", 5) == 0) { out[j++] = '\''; i += 4; continue; }
      }
      out[j++] = str[i];
   }
   out[j] = 0;
   return out;
}

static char *Escape_uri_str(const char *url, const char *chars)
{
   if (!url) return dStrdup("");
   static const char hex[] = "0123456789ABCDEF";
   size_t len = strlen(url);
   char *out = dNew(char, len * 3 + 1);
   size_t j = 0;
   for (size_t i = 0; i < len; i++) {
      unsigned char c = url[i];
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || strchr("-_.~", c) ||
          (chars && strchr(chars, c))) {
         out[j++] = c;
      } else {
         out[j++] = '%';
         out[j++] = hex[c >> 4];
         out[j++] = hex[c & 0xF];
      }
   }
   out[j] = 0;
   return out;
}

static int Bmsrv_modify_delete(BookmarkStore *store, char *url)
{
   char *p;
   int key;

   p = strchr(url, '?');
   for (; (p = strstr(p, "&s")); ++p) {
      if (dIsdigit(p[2])) {
         key = strtol(p + 2, NULL, 10);
         Bms_sec_del(store, key);
      }
   }
   p = strchr(url, '?');
   for (; (p = strstr(p, "&url")); ++p) {
      if (dIsdigit(p[4])) {
         key = strtol(p + 4, NULL, 10);
         Bms_del(store, key);
      }
   }

   Bms_save(store);
   return 0;
}

static int Bmsrv_modify_move(BookmarkStore *store, char *url)
{
   char *p;
   int n = 0, section = 0, key;

   for (p = url; (p = strstr(p, "&s")); ++p) {
      if (dIsdigit(p[2])) {
         section = strtol(p + 2, NULL, 10);
         break;
      }
   }
   if (!p) return 1;

   p = strchr(url, '?');
   for (; (p = strstr(p, "&url")); ++p) {
      if (dIsdigit(p[4])) {
         key = strtol(p + 4, NULL, 10);
         Bms_move(store, key, section);
         ++n;
      }
   }

   if (n) Bms_save(store);
   return 0;
}

static int Bmsrv_modify_update(BookmarkStore *store, char *url)
{
   char *p, *q, *title;
   int i, key;

   p = strchr(url, '?');
   for (; (p = strstr(p, "s")); ++p) {
      if (p[-1] == '&' || p[-1] == '?' ) {
         for (i = 0; dIsdigit(p[1 + i]); ++i);
         if (i && p[1 + i] == '=') {
            key = strtol(p + 1, NULL, 10);
            if ((q = strchr(p + 1, '&')))
               title = dStrndup(p + 2 + i, (uint_t)(q - (p + 2 + i)));
            else
               title = dStrdup(p + 2 + i);
            Unencode_str(title);
            Bms_update_sec_title(store, key, title);
            dFree(title);
         }
      }
   }

   p = strchr(url, '?');
   for (; (p = strstr(p, "title")); ++p) {
      if (p[-1] == '&' || p[-1] == '?' ) {
         for (i = 0; dIsdigit(p[5 + i]); ++i);
         if (i && p[5 + i] == '=') {
            key = strtol(p + 5, NULL, 10);
            if ((q = strchr(p + 5, '&')))
               title = dStrndup(p + 6 + i, (uint_t)(q - (p + 6 + i)));
            else
               title = dStrdup(p + 6 + i);
            Unencode_str(title);
            Bms_update_title(store, key, title);
            dFree(title);
         }
      }
   }

   Bms_save(store);
   return 0;
}

static int Bmsrv_modify_add_section(BookmarkStore *store, char *url)
{
   char *p, *title = NULL;

   if ((p = strstr(url, "&title="))) {
      title = dStrdup(p + 7);
      if ((p = strchr(title, '&'))) *p = 0;
      Unencode_str(title);
   } else return 1;

   Bms_sec_add(store, title);
   dFree(title);
   Bms_save(store);
   return 0;
}

static int Bmsrv_modify_add_url(BookmarkStore *store, char *url)
{
   char *p, *q, *title, *u_title, *url_str;
   int i, section = 0;

   for (q = url; (q = strstr(q, "&s")); ++q) {
      for (i = 0; dIsdigit(q[2+i]); ++i);
      if (q[2+i] == '=') section = strtol(q + 2, NULL, 10);
   }

   if (!(p = strstr(url, "&title=")) || !(q = strstr(url, "&url="))) return 1;

   title = dStrdup(p + 7);
   if ((p = strchr(title, '&'))) *p = 0;
   url_str = dStrdup(q + 5);
   if ((p = strchr(url_str, '&'))) *p = 0;

   if (strlen(title) && strlen(url_str)) {
      Unencode_str(title);
      Unencode_str(url_str);
      u_title = Unescape_html_str(title);
      Bms_add(store, section, url_str, u_title);
      dFree(u_title);
   }
   dFree(title);
   dFree(url_str);

   Bms_save(store);
   return 0;
}

static int Bmsrv_check_modify_request(BookmarkStore *store, char *url)
{
   char *p;
   int n_sec, n_url;

   Bmsrv_count_urls_and_sections(url, &n_sec, &n_url);

   p = strchr(url, '?');
   if (strstr(p, "operation=delete&")) {
      return (n_url || n_sec) ? 0 : 1;
   } else if (strstr(url, "operation=move&")) {
      return (n_url && n_sec) ? 0 : 1;
   } else if (strstr(url, "operation=modify&")) {
      return (n_url || n_sec) ? 0 : 1;
   } else if (strstr(url, "operation=add_sec&")) {
      return 0;
   } else if (strstr(url, "operation=add_section&")) {
      return 0;
   } else if (strstr(url, "operation=add_url&")) {
      return (n_sec <= 1) ? 0 : 1;
   } else if (strstr(url, "operation=add_url2&")) {
      return 0;
   } else if (strstr(url, "operation=none&")) {
      return 1;
   }
   return 1;
}

static int Bmsrv_process_modify_request(BookmarkStore *store, char *url)
{
   if (Bmsrv_check_modify_request(store, url) != 0)
      return 1;

   if (strstr(url, "operation=delete&")) {
      Bmsrv_modify_delete(store, url);
   } else if (strstr(url, "operation=move&")) {
      Bmsrv_modify_move(store, url);
   } else if (strstr(url, "operation=modify&")) {
      /* prepare update page */
   } else if (strstr(url, "operation=modify2&")) {
      Bmsrv_modify_update(store, url);
   } else if (strstr(url, "operation=add_sec&")) {
      /* prepare add section page */
   } else if (strstr(url, "operation=add_section&")) {
      Bmsrv_modify_add_section(store, url);
   } else if (strstr(url, "operation=add_url&")) {
      Bmsrv_modify_add_url(store, url);
   } else if (strstr(url, "operation=add_url2&")) {
      Bmsrv_modify_add_url(store, url);
   }
   return 0;
}

static char *send_bm_page(BookmarkStore *store)
{
   Dstr *dstr = dStr_new("");
   char *l_title;
   BmSec *sec_node;
   BmRec *bm_node;
   int i, j;

   Bms_cond_load(store);

   dStr_sprintfa(dstr, "%s", mainpage_header);

   dStr_sprintfa(dstr, "%s", mainpage_sections_header);
   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      if (i > 0) dStr_sprintfa(dstr, "%s", sections_sep);
      dStr_sprintf(dstr, mainpage_sections_item, sec_node->section, sec_node->title);
   }
   dStr_sprintfa(dstr, "%s", mainpage_sections_footer);

   dStr_sprintfa(dstr, "%s", mainpage_middle1);

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      l_title = make_one_line_str(sec_node->title);
      dStr_sprintf(dstr, mainpage_section_card_header, sec_node->section, l_title);
      dFree(l_title);

      for (j = 0; (bm_node = dList_nth_data(store->B_bms, j)); ++j) {
         if (bm_node->section == sec_node->section) {
            dStr_sprintf(dstr, mainpage_section_card_item, bm_node->url, bm_node->title);
         }
      }
      dStr_sprintfa(dstr, "%s", mainpage_section_card_footer);
   }

   dStr_sprintfa(dstr, "%s", mainpage_footer);

   return dstr->str;
}

static char *send_modify_page(BookmarkStore *store)
{
   Dstr *dstr = dStr_new("");
   char *l_title;
   BmSec *sec_node;
   BmRec *bm_node;
   int i, j;

   Bms_cond_load(store);

   dStr_sprintfa(dstr, "%s", modifypage_header);
   dStr_sprintfa(dstr, "%s", modifypage_sections_header);

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      if (i > 0) dStr_sprintfa(dstr, "%s", sections_sep);
      dStr_sprintf(dstr, modifypage_sections_item,
                   sec_node->section, sec_node->section, sec_node->title);
   }
   dStr_sprintfa(dstr, "%s", modifypage_sections_footer);
   dStr_sprintfa(dstr, "%s", modifypage_middle1);

   for (i = 0; (sec_node = dList_nth_data(store->B_secs, i)); ++i) {
      l_title = make_one_line_str(sec_node->title);
      dStr_sprintf(dstr, modifypage_section_card_header, sec_node->section, l_title);
      dFree(l_title);

      for (j = 0; (bm_node = dList_nth_data(store->B_bms, j)); ++j) {
         if (bm_node->section == sec_node->section) {
            dStr_sprintf(dstr, modifypage_section_card_item,
                         bm_node->key, bm_node->url, bm_node->title);
         }
      }
      dStr_sprintfa(dstr, "%s", modifypage_section_card_footer);
   }

   dStr_sprintfa(dstr, "%s", modifypage_footer);

   return dstr->str;
}

static char *send_modify_add_section_page(void)
{
   return dStrdup(modifypage_add_section_page);
}

static char *send_modify_add_url_page(void)
{
   return dStrdup(modifypage_add_url);
}

static char *send_modify_update_page(BookmarkStore *store, char *url)
{
   static char *url1 = NULL;
   Dstr *dstr = dStr_new("");
   char *p, *q;
   int i, key, n_sec, n_url;
   BmRec *bm_node;
   BmSec *sec_node;

   if (url) {
      dFree(url1);
      url1 = dStrdup(url);
   }

   dStr_sprintfa(dstr, "%s", modifypage_update_header);

   Bmsrv_count_urls_and_sections(url1, &n_sec, &n_url);

   if (n_sec) {
      dStr_sprintf(dstr, modifypage_update_title, "Update sections");
      dStr_sprintfa(dstr, "%s", modifypage_update_item_header);
      p = strchr(url1, '?');
      for (q = p; (q = strstr(q, "&s")); ++q) {
         for (i = 0; dIsdigit(q[2+i]); ++i);
         if (q[2+i] == '=') {
            key = strtol(q + 2, NULL, 10);
            if ((sec_node = Bms_get_sec(store, key))) {
               dStr_sprintf(dstr, modifypage_update_item2,
                            sec_node->section, sec_node->title);
            }
         }
      }
      dStr_sprintfa(dstr, "%s", modifypage_update_item_footer);
   }

   if (n_url) {
      dStr_sprintf(dstr, modifypage_update_title, "Update titles");
      dStr_sprintfa(dstr, "%s", modifypage_update_item_header);
      p = strchr(url1, '?');
      for (q = p; (q = strstr(q, "&url")); ++q) {
         for (i = 0; dIsdigit(q[4+i]); ++i);
         if (q[4+i] == '=') {
            key = strtol(q + 4, NULL, 10);
            if ((bm_node = Bms_get(store, key))) {
               dStr_sprintf(dstr, modifypage_update_item,
                            bm_node->key, bm_node->title, bm_node->url);
            }
         }
      }
      dStr_sprintfa(dstr, "%s", modifypage_update_item_footer);
   }

   dStr_sprintfa(dstr, "%s", modifypage_update_footer);

   return dstr->str;
}

/* --- Public API --- */

BookmarkStore *bookmarks_store_init(void)
{
   BookmarkStore *store = dNew0(BookmarkStore, 1);
   store->B_bms = dList_new(512);
   store->B_secs = dList_new(32);
   store->BmFile = dStrconcat(dGethomedir(), "/.config/vibedillo/bm.txt", NULL);
   store->modify_page_num = 1;
   Bms_cond_load(store);
   return store;
}

void bookmarks_store_free(BookmarkStore *store)
{
   if (!store) return;
   Bms_save(store);
   Bms_free(store);
   dFree(store->BmFile);
   dList_free(store->B_bms);
   dList_free(store->B_secs);
   dFree(store);
}

void bookmarks_store_add(BookmarkStore *store, const char *url, const char *title)
{
   Bmsrv_add_bm(store, (char *)url, (char *)title);
}

char *bookmarks_store_get_page(BookmarkStore *store)
{
   Bms_cond_load(store);
   return send_bm_page(store);
}

char *bookmarks_store_get_modify_page(BookmarkStore *store)
{
   Bms_cond_load(store);
   return send_modify_page(store);
}

char *bookmarks_store_get_add_section_page(BookmarkStore *store)
{
   return send_modify_add_section_page();
}

char *bookmarks_store_get_add_url_page(BookmarkStore *store)
{
   return send_modify_add_url_page();
}

char *bookmarks_store_get_update_page(BookmarkStore *store, const char *query_string)
{
   Bms_cond_load(store);
   return send_modify_update_page(store, (char *)query_string);
}

int bookmarks_store_process_modify(BookmarkStore *store, const char *query_string)
{
   Bms_cond_load(store);
   return Bmsrv_process_modify_request(store, (char *)query_string);
}

int bookmarks_store_check_modify(BookmarkStore *store, const char *query_string)
{
   Bms_cond_load(store);
   return Bmsrv_check_modify_request(store, (char *)query_string) == 0 ? 0 : 1;
}

const char *bookmarks_store_get_file_path(void)
{
   static char *path = NULL;
   if (!path) path = dStrconcat(dGethomedir(), "/.config/vibedillo/bm.txt", NULL);
   return path;
}

#endif /* !DISABLE_COOKIES */
