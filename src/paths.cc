/*
 * File: paths.cc
 *
 * Copyright 2006-2009 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "msg.h"
#include "../dlib/dlib.h"
#include "paths.hh"

/*
 * Local data
 */

// Dillo works from an unmounted directory (/tmp)
static char* oldWorkingDir = NULL;

/**
 * Changes current working directory to /tmp and creates ~/.config/vibedillo
 * if not exists.
 */
void Paths::init(void)
{
   char *path;
   struct stat st;
   int rc = 0;

   dFree(oldWorkingDir);
   oldWorkingDir = dGetcwd();
   rc = chdir("/tmp");
   if (rc == -1) {
      DBG("paths: Error changing directory to /tmp: %s\n",
          dStrerror(errno));
   }

   char *home = dGethomedir();
   DBG("paths: init: home dir = '%s'\n", home ? home : "(null)");
   path = dStrconcat(home, "/.config/vibedillo", NULL);
   DBG("paths: init: checking dir '%s'\n", path);
   if (stat(path, &st) == -1) {
      if (errno == ENOENT) {
         DBG("paths: Creating directory '%s/'\n", path);
         if (mkdir(path, 0700) < 0) {
            DBG("paths: Error creating directory %s: %s\n",
                path, dStrerror(errno));
         } else {
            DBG("paths: Directory created successfully\n");
         }
      } else {
         printf("Dillo: error reading %s: %s\n", path, dStrerror(errno));
      }
   } else {
      DBG("paths: Directory already exists\n");
   }

   dFree(path);
}

/**
 * Return the initial current working directory in a string.
 */
char *Paths::getOldWorkingDir(void)
{
   return oldWorkingDir;
}

/**
 * Free memory
 */
void Paths::free(void)
{
   dFree(oldWorkingDir);
}

/**
 * Examines the path for "rcFile" and assign its file pointer to "fp".
 * Creates default config files if they don't exist.
 */
FILE *Paths::getPrefsFP(const char *rcFile)
{
   FILE *fp;
   char *path = dStrconcat(dGethomedir(), "/.config/vibedillo/", rcFile, NULL);

   if (!(fp = fopen(path, "r"))) {
      /* Try to create default config file if it doesn't exist */
      if (errno == ENOENT) {
         Paths::create_default_config(rcFile, path);
         fp = fopen(path, "r");
      }
      if (!fp) {
         MSG("paths: Cannot open file '%s': %s\n", path, dStrerror(errno));

         char *path2 = dStrconcat(DILLO_SYSCONF, rcFile, NULL);
         if (!(fp = fopen(path2, "r"))) {
            MSG("paths: Cannot open file '%s': %s\n", path2, dStrerror(errno));
            MSG("paths: Using internal defaults...\n");
         } else {
            MSG("paths: Using %s\n", path2);
         }
         dFree(path2);
      }
   }

   dFree(path);
   return fp;
}

/**
 * Create default config file if it doesn't exist.
 */
void Paths::create_default_config(const char *rcFile, const char *path)
{
   const char *default_content = NULL;

   if (strcmp(rcFile, "vibedillorc") == 0) {
      default_content =
         "# vibedillorc\n"
         "# Sample vibedillo initialization file.\n"
         "#\n"
         "# Lines that start with a '#' are comments.\n"
         "# \"#option=...\" shows the built-in default.\n"
         "# \"# option=...\" is an additional example.\n"
         "# \"option=...\" overrides the built-in value.\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                             FIRST SECTION                             :)\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Set the desired initial browser size\n"
         "# geometry=650x545+0+20\n"
         "#geometry=1024x768\n"
         "\n"
         "# Change this (and the following option) if you want to have text-only browsing\n"
         "# from the start. (While browsing, this can be changed from the tools/settings\n"
         "# menu.)\n"
         "#load_images=YES\n"
         "\n"
         "# A space separated list of image formats that will be ignored (not viewed).\n"
         "# An option to download the image will be offered instead.\n"
         "# Available formats: gif, png, webp, jpeg and svg.\n"
         "# ignore_image_formats=\"webp svg\"\n"
         "#ignore_image_formats=\"\"\n"
         "\n"
         "# Show a visible border on images that are not loaded yet. Makes it easier to\n"
         "# spot them, but may cause unwanted noise in some pages.\n"
         "#mark_unloaded_images=NO\n"
         "\n"
         "# Change this if you want background images to be loaded initially.\n"
         "# (While browsing, this can be changed from the tools/settings menu.)\n"
         "#load_background_images=NO\n"
         "\n"
         "# Change this if you want to disable loading of CSS stylesheets initially.\n"
         "# (While browsing, this can be changed from the tools/settings menu.)\n"
         "#load_stylesheets=YES\n"
         "\n"
         "# Change this if you want to disable parsing of embedded CSS initially.\n"
         "# (While browsing, this can be changed from the tools/settings menu.)\n"
         "#parse_embedded_css=YES\n"
         "\n"
         "# Change the buffering scheme for drawing\n"
         "# 0 no double buffering - useful for debugging\n"
         "# 1 light buffering using a single back buffer for all windows\n"
         "# 2 full fltk-based double buffering for all windows\n"
         "#buffered_drawing=1\n"
         "\n"
         "# Set your default directory for download/save operations\n"
         "#save_dir=/tmp\n"
         "\n"
         "# Set the increment in pixels that the page is moved up/down with each input\n"
         "# from the mouse wheel, keyboard arrow keys, or scrollbar arrow buttons.\n"
         "#scroll_step=100\n"
         "\n"
         "# Controls the overlap in pixels when scrolling to the next or previous page.\n"
         "# It is used to repeat the last line(s) to avoid losing the reading flow. It\n"
         "# controls the overlap in both the vertical and horizontal directions.\n"
         "#scroll_page_overlap=50\n"
         "\n"
         "# Place the vertical scrollbar on the left side (default right).\n"
         "#scrollbar_on_left=NO\n"
         "\n"
         "# Enable the page mode for the vertical scrollbar. When this mode is enabled,\n"
         "# clicking anywhere on the vertical scrollbar with the left button scrolls one\n"
         "# page down. With the right button, one page up. Click with the middle button to\n"
         "# jump to a given position.\n"
         "#scrollbar_page_mode=NO\n"
         "\n"
         "# Define custom actions for the link menu. The format is <label>:<cmd>. The\n"
         "# command will be executed in the system shell using the system() call. You can\n"
         "# implement your own handling logic in a script or program. The following\n"
         "# environment variables are set:\n"
         "#   $url:       URL being opened\n"
         "#   $origin:    URL of the current document\n"
         "#   $DILLO_PID: PID of vibedillo process\n"
         "# Examples:\n"
         "# link_action=\"Debug variables:echo url=$url origin=$origin\"\n"
         "# link_action=\"Open in MPV:mpv $url\"\n"
         "# link_action=\"Open in Firefox:firefox $url\"\n"
         "\n"
         "# Define custom actions for the page menu. The format is <label>:<cmd>. The\n"
         "# command will be executed in the system shell using the system() call. You can\n"
         "# implement your own handling logic in a script or program. The following\n"
         "# environment variables are set:\n"
         "#   $url:       URL of the current page\n"
         "#   $DILLO_PID: PID of vibedillo process\n"
         "# Examples:\n"
         "# page_action=\"Reader mode:vibedilloc dump | rdrview -H | vibedilloc load\"\n"
         "# page_action=\"Mimic Chrome:curl_chrome136 -i --raw $url | vibedilloc rawload\"\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                           RENDERING SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Default fonts:\n"
         "#\n"
         "# If FLTK has been configured with Xft enabled (the default), you can use\n"
         "# scalable fonts such as DejaVu or Liberation (try running\n"
         "# \"fc-list : family | cut -d ',' -f 2 | sort\").\n"
         "#font_serif=\"DejaVu Serif\"\n"
         "#font_sans_serif=\"DejaVu Sans\"\n"
         "#font_cursive=\"DejaVu Sans\"\n"
         "#font_fantasy=\"DejaVu Sans\"\n"
         "#font_monospace=\"DejaVu Sans Mono\"\n"
         "#\n"
         "# Otherwise, use bitmapped fonts like the following (for a list, try running\n"
         "# \"xlsfonts -fn *-iso10646-1 | grep -v -e -0-0 | cut -d - -f 3 | sort | uniq\").\n"
         "# font_serif=\"times\"\n"
         "# font_sans_serif=\"helvetica\"\n"
         "# font_cursive=\"helvetica\"\n"
         "# font_fantasy=\"helvetica\"\n"
         "# font_monospace=\"courier\"\n"
         "\n"
         "# All font sizes are scaled by this value\n"
         "# font_factor=1.5\n"
         "#font_factor=1.0\n"
         "\n"
         "# Maximum font size in pixels\n"
         "#font_max_size=100\n"
         "\n"
         "# Minimum font size in pixels\n"
         "#font_min_size=6\n"
         "\n"
         "# Show tooltip popups for HTML title attributes\n"
         "#show_tooltip=YES\n"
         "\n"
         "# Set this to YES to limit the word wrap width to the viewport width\n"
         "#limit_text_width=NO\n"
         "\n"
         "# If this is set to YES, all CSS size specifications are adjusted so that\n"
         "# all contents can be displayed. (Except for tables, see below.)\n"
         "#adjust_min_width=YES\n"
         "\n"
         "# If this is set to YES, all CSS size specifications for tables are\n"
         "# adjusted so that all contents can be displayed. This is separated\n"
         "# from \"adjust_min_width\" so that it is able to mimic Firefox, which\n"
         "# differentiates between tables and, say, textblocks (in some cases).\n"
         "#adjust_table_min_width=YES\n"
         "\n"
         "# Sets the initial zoom factor, which scales the size of all HTML elements.\n"
         "# zoom_factor=1.5\n"
         "#zoom_factor=1.0\n"
         "\n"
         "# Color scheme for CSS prefers-color-scheme media queries:\n"
         "# \"auto\" - follow system preference (default)\n"
         "# \"light\" - force light mode\n"
         "# \"dark\" - force dark mode\n"
         "#color_scheme=auto\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                               PENALTIES\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Penalties are used to control good and bad break points. The bigger\n"
         "# the penalty for a given break point, the less likely the line is\n"
         "# broken here. \"inf\" means that breaking is prohibited, \"-inf\" means\n"
         "# that a line *must* be broken here. (The latter should not be used\n"
         "# here, however.) Normal spaces get a penalty of 0. The exact\n"
         "# definition can be found in doc/dw-line-breaking.doc.\n"
         "\n"
         "# Penalties for hyphenation breaks; this covers automatic hyphenation,\n"
         "# soft hyphens, and unconditional hyphens. Since hyphenation should\n"
         "# rather be avoided, the default values are larger than 0.\n"
         "\n"
         "# This is used for hyphenation points, when there is no hyphen or dash\n"
         "# before:\n"
         "#penalty_hyphen = 1\n"
         "\n"
         "# This is used for hyphenation points, when the line before ends\n"
         "# already with a hyphen or a dash. Consequent lines ending with\n"
         "# hyphens or dashes should be avoided, so this value is bigger than\n"
         "# \"penalty_hyphen\":\n"
         "#penalty_hyphen_2 = 8\n"
         "\n"
         "# The same for a break right of an em-dash, when there are no spaces\n"
         "# surrounding it (as in English). The default values are the same as\n"
         "# for hyphens:\n"
         "#penalty_em_dash_right = 1\n"
         "#penalty_em_dash_right_2 = 8\n"
         "\n"
         "# Penalty for a break right of an em-dash, when there are no spaces\n"
         "# surrounding it (as in English). The default values are the same as\n"
         "# for hyphens:\n"
         "#penalty_em_dash_right = 1\n"
         "#penalty_em_dash_right_2 = 8\n"
         "\n"
         "# Penalty for a break *left* of an em-dash. Since a line ending with\n"
         "# an em-dash (and so breaking right of the em-dash) looks better than\n"
         "# a line beginning with an em-dash (breaking left of an em-dash), the\n"
         "# default value is bigger than \"penalty_em_dash_right\":\n"
         "#penalty_em_dash_left = 8\n"
         "\n"
         "# Notice that there is no \"penalty_em_dash_left_2\", since breaking\n"
         "# left of an em-dash makes the line *begin*, not *end* with a dash.\n"
         "\n"
         "# This factor is multiplied with the line height to get the\n"
         "# stretchability of a non-justified line. The larger this factor (and\n"
         "# thus, the stretchability), the less likely the words are hyphenated;\n"
         "# so you can use this value to control hyphenation of non-justified\n"
         "# text.\n"
         "#stretchability_factor=1\n"
         "\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                            NETWORK SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Set the start page.\n"
         "# start_page=\"about:blank\"\n"
         "# start_page=\"https://walley.org/\"\n"
         "# start_page=\"file:/home/jcid/custom_page.html\"\n"
         "#start_page=\"about:splash\"\n"
         "\n"
         "# Set the home location\n"
         "# home=\"file:/home/jcid/HomePage/Home.html\"\n"
         "#home=\"https://walley.org/\"\n"
         "\n"
         "# Set the new tab page.\n"
         "# new_tab_page=\"dpi:/bm/\"\n"
         "# new_tab_page=\"https://example.com/\"\n"
         "#new_tab_page=\"about:blank\"\n"
         "\n"
         "# Set the URLs used by the web search dialog.\n"
         "# \"%s\" is replaced with the search keywords separated by '+'.\n"
         "# Format: search_url=\"[prefix ][<label> ]<url>\"\n"
         "# You can enable multiple search_url strings at once and select from among\n"
         "# them at runtime, with the first being the default.\n"
         "# (the prefix serves to search from the Location Bar. e.g. \"dd vibedillo image\")\n"
         "search_url=\"dd DuckDuckGo https://duckduckgo.com/lite/?kp=-1&kd=-1&q=%s\"\n"
         "search_url=\"wi Wikipedia https://www.wikipedia.org/w/index.php?search=%s&go=Go\"\n"
         "search_url=\"di Free Dictionary https://www.thefreedictionary.com/%s\"\n"
         "search_url=\"sp Startpage https://www.startpage.com/do/search?query=%s\"\n"
         "search_url=\"mg Marginalia https://marginalia-search.com/search?query=%s\"\n"
         "search_url=\"wb Wiby https://wiby.org/?q=%s\"\n"
         "search_url=\"mg Mojeek https://www.mojeek.com/search?q=%s\"\n"
         "\n"
         "# If set, dillo will ask web servers to send pages in this language.\n"
         "# This setting does NOT change vibedillo's user interface.\n"
         "# Format explained: www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.4\n"
         "# Language-REGION values: www.iana.org/assignments/language-subtag-registry\n"
         "# (by default, no Accept-Language header is sent)\n"
         "# http_language=\"de\"\n"
         "# http_language=\"pt-BR\"\n"
         "# http_language=\"en-US,en;q=0.5\"\n"
         "\n"
         "# Maximum number of simultaneous TCP connections to a single server or proxy.\n"
         "# http_max_conns=6\n"
         "\n"
         "# If enabled, Dillo will reuse HTTP connections to a server or proxy when\n"
         "# possible rather than making a new connection for every request for a new\n"
         "# page/image/stylesheet.\n"
         "#http_persistent_conns=YES\n"
         "\n"
         "# This mechanism allows servers to specify that they are only to be contacted\n"
         "# through HTTPS and not HTTP.\n"
         "#\n"
         "# Overall, this is a valuable security measure against TLS stripping\n"
         "# attacks, etc., but in principle a site could contrive to use this as a\n"
         "# tracking mechanism. The term is \"HSTS super cookie\", although note that these\n"
         "# HSTS directives are not saved between browser sessions.\n"
         "#http_strict_transport_security=YES\n"
         "\n"
         "# If enabled, Dillo will force all HTTP connections to be upgraded to\n"
         "# a more secure HTTPS connection. This will prevent sites from loading\n"
         "# if they only support HTTP.\n"
         "#http_force_https=NO\n"
         "\n"
         "# Set the proxy information for http/https.\n"
         "# Note that the http_proxy environment variable overrides this setting.\n"
         "# WARNING: FTP and downloads plugins use wget. To use a proxy with them,\n"
         "#          you will need to configure wget accordingly. See\n"
         "#          http://www.gnu.org/software/wget/manual/html_node/Proxies.html\n"
         "# http_proxy=\"http://localhost:8080/\"\n"
         "#(by default, no proxy is used)\n"
         "\n"
         "# If you need to provide a  user/password pair for the proxy,\n"
         "# set the proxy user name here and Dillo will ask for the password later.\n"
         "# http_proxyuser=\"joe\"\n"
         "#(by default, no proxy is used)\n"
         "\n"
         "# Set the domains to access without proxy\n"
         "# no_proxy = \".hola.com .mynet.cl .hi.de\"\n"
         "#no_proxy=\"localhost 127.0.0.1\"\n"
         "\n"
         "# Set the HTTP Referer (sic) header.\n"
         "# Note that there is no option to reveal the page that you came from because it\n"
         "# would endanger your privacy. 'host' and 'path' allow you to pretend that the\n"
         "# link you followed was on the same site that you're going to.\n"
         "# none  : Don't send any Referer header at all.\n"
         "# host  : Send the requested URI's hostname.\n"
         "# path  : Send the requested URI's host and path.\n"
         "#http_referer=host\n"
         "\n"
         "# Set the HTTP User-Agent header.\n"
         "# This can be useful for privacy and for working around servers who think\n"
         "# vibedillo is less capable than it really is. However, if you pretend to use a\n"
         "# different browser, servers may send you pages that work with the features\n"
         "# and bugs of that other browser -- or even disallow access in cases like\n"
         "# wget or googlebot. Remember this before submitting bug reports.\n"
         "#\n"
         "# See http://zytrax.com/tech/web/browser_ids.htm for a compilation of strings.\n"
         "#\n"
         "# http_user_agent=\"Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0\"\n"
         "# http_user_agent=\"Wget/1.13.4 (linux-gnu)\"\n"
         "#The default is \"vibedillo/\"+current_version_number\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                            COLORS SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Set the page background color\n"
         "# bg_color=gray\n"
         "# bg_color=0xd6d6c0\n"
         "#bg_color=0xdcd1ba\n"
         "\n"
         "# If your eyes suffer with white backgrounds, change this.\n"
         "#allow_white_bg=YES\n"
         "\n"
         "# If allow_white_bg is set to NO, white backgrounds are replaced by\n"
         "# this color.\n"
         "#white_bg_replacement=0xe0e0a3\n"
         "\n"
         "# When set to YES, the page author's visited link color may be overridden\n"
         "# to allow better contrast with text/links/background\n"
         "#contrast_visited_color=YES\n"
         "\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                        USER INTERFACE SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# UI theme\n"
         "# \"none\" is the default FLTK appearance, which \"resembles old Windows...and\n"
         "# \"old GTK/KDE\".\n"
         "# \"plastic\" \"is inspired by the Aqua user interface on Mac OS X\".\n"
         "# \"gtk+\" \"is inspired by the Red Hat Bluecurve theme\".\n"
         "#\n"
         "# If you have fltk-1.3.3 or newer, you can specify \"gleam\", which\n"
         "# is \"a sort of Clearlooks Glossy scheme\".  (\"fltk-config --version\")\n"
         "#theme=none\n"
         "# theme=gtk+\n"
         "# theme=plastic\n"
         "\n"
         "# UI colors\n"
         "# Note that FLTK may sometimes override colors, generally for contrast and\n"
         "# readability.\n"
         "#\n"
         "# ui_fg_color, ui_main_bg_color, ui_text_bg_color, and ui_selection_color\n"
         "# map to concepts in the underlying FLTK toolkit which are described as:\n"
         "# \"the default foreground color...used for labels and text\", \"default\n"
         "# background color\", \"the default background color for text, list, and\n"
         "# valuator widgets\", and \"the default selection/highlight color\". They\n"
         "# sometimes have other uses in the more complex FLTK widgets.\n"
         "#\n"
         "# ui_button_highlight_color is the background used when the mouse cursor is\n"
         "# over a button. By default, this is a lightened version of the main\n"
         "# background color.\n"
         "\n"
         "# ui_tab_active_fg_color and ui_tab_active_bg_color are used for the current\n"
         "# tab. By default, they are the main foreground color and the text background\n"
         "# color, respectively.\n"
         "#\n"
         "# ui_tab_fg_color and ui_tab_bg_color are used for the other tabs. By default,\n"
         "# they are the main foreground color and the main background color,\n"
         "# respectively.\n"
         "#\n"
         "# ui_tab_height controls the height of the tabs (default 20).\n"
         "#\n"
         "# Note to packagers: leaving these variables for the system to guess\n"
         "# gives different results in different environments, so we played it safe\n"
         "# by defining the traditional colors.  Please choose the color theme that\n"
         "# better fits your distro.\n"
         "\n"
         "#\n"
         "# Gray theme (traditional)\n"
         "#\n"
         "ui_fg_color=black\n"
         "ui_main_bg_color=#c6c6c6\n"
         "ui_text_bg_color=#bfdabf\n"
         "ui_selection_color=#191970\n"
         "ui_button_highlight_color=#a9a9a9\n"
         "ui_tab_active_bg_color=#87aca7\n"
         "ui_tab_active_fg_color=black\n"
         "ui_tab_bg_color=#b7beb7\n"
         "\n"
         "#\n"
         "# Earthly theme:\n"
         "#\n"
         "#ui_fg_color=#100404\n"
         "#ui_main_bg_color=#c2a47b\n"
         "#ui_text_bg_color=#cdc9a5\n"
         "#ui_selection_color=#763024\n"
         "#ui_tab_active_bg_color=#af4b3f\n"
         "#ui_tab_active_fg_color=white\n"
         "#ui_tab_bg_color=#d2b48c\n"
         "\n"
         "#\n"
         "# Greenish theme:\n"
         "#\n"
         "#ui_fg_color=#100404\n"
         "#ui_main_bg_color=#c8d394\n"
         "#ui_text_bg_color=#bdd8b6\n"
         "#ui_selection_color=#7c5f42\n"
         "#ui_button_highlight_color=#adad70\n"
         "#ui_tab_active_bg_color=#b5b679\n"
         "#ui_tab_active_fg_color=#b60907\n"
         "#ui_tab_bg_color=#cac682\n"
         "\n"
         "\n"
         "# Size of vibedillo panel\n"
         "# tiny   : buttons, location, and progress boxes in one row\n"
         "# small  : location in one row, buttons + progress boxes in another\n"
         "# medium : adds text labels to buttons and boxes\n"
         "# panel_size=tiny\n"
         "# panel_size=small\n"
         "#panel_size=medium\n"
         "\n"
         "#small_icons=NO\n"
         "\n"
         "# Here you can choose to hide some widgets of the vibedillo panel...\n"
         "#show_back=YES\n"
         "#show_forw=YES\n"
         "#show_home=YES\n"
         "#show_reload=YES\n"
         "#show_save=YES\n"
         "#show_stop=YES\n"
         "#show_bookmarks=YES\n"
         "#show_tools=YES\n"
         "#show_filemenu=YES\n"
         "#show_clear_url=YES\n"
         "#show_url=YES\n"
         "#show_search=YES\n"
         "#show_help=YES\n"
         "#show_progress_box=YES\n"
         "\n"
         "# Show tooltip popups for the UI\n"
         "#show_ui_tooltip=YES\n"
         "\n"
         "# Start vibedillo with the panels hidden?\n"
         "#fullwindow_start=NO\n"
         "\n"
         "# When filling out forms, our default behaviour is to submit on enterpress,\n"
         "# but only when there's a single text entry (to avoid incomplete submits).\n"
         "# OTOH, if you have to fill out the same form repeatedly, you may find it\n"
         "# useful to keep away from the mouse by forcing enter to submit.\n"
         "#enterpress_forces_submit=NO\n"
         "\n"
         "# A mouse's middle click over a link opens a new Tab.\n"
         "# If you prefer to open a new Window instead, set it to NO.\n"
         "#middle_click_opens_new_tab=YES\n"
         "\n"
         "# A mouse's middle click over a tab closes the Tab.\n"
         "# With mousewheel mouses, right click feels way better (set to YES).\n"
         "#right_click_closes_tab=YES\n"
         "\n"
         "# Scroll over tabs (using the mouse wheel) to switch among tabs.\n"
         "# If set to NO, the page will be scrolled instead.\n"
         "#scroll_switches_tabs=YES\n"
         "\n"
         "# Reverse the direction of tab scrolling with mouse wheel.\n"
         "#scroll_switches_tabs_reverse=NO\n"
         "\n"
         "# Mouse middle click by default drives drag-scrolling.\n"
         "# To paste an URL into the window instead of scrolling, set it to NO.\n"
         "# Note: You could always paste the URL onto the URL box clear button.\n"
         "#middle_click_drags_page=YES\n"
         "\n"
         "# Focus follows new Tabs.\n"
         "# You can hold SHIFT to temporarily revert this behaviour.\n"
         "#focus_new_tab=NO\n"
         "\n"
         "# Ask before quitting Vibedillo with more than one window or tab open.\n"
         "#show_quit_dialog=NO\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                        DEBUG MESSAGES SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Soon we should add the \"show_debug_messages=NO\" option...\n"
         "\n"
         "# Generic messages (mainly for debugging specific parts)\n"
         "# Change this to disable them.\n"
         "#show_msg=YES\n"
         "#\n"
         "# Print HTTP traffic to stdout (before TLS encryption).\n"
         "# Useful for debugging but will print any user or password sent in forms!\n"
         "#trace_http=NO\n"
         "\n"
         "\n"
         "#-------------------------------------------------------------------------\n"
         "#                        HTML BUG MESSAGES SECTION\n"
         "#-------------------------------------------------------------------------\n"
         "\n"
         "# Accepted by the W3C validator but \"strongly discouraged\" by the SPEC.\n"
         "# (Such as \"TAB character inside <PRE>\").\n"
         "#show_extra_warnings=NO\n"
         "\n"
         "\n"
         "# -----------------------------------------------------------------------\n"
         "# vibedillorc ends here.\n";
   } else if (strcmp(rcFile, "keysrc") == 0) {
      default_content =
         "# vibedillo key bindings\n"
         "# Format: key = action\n"
         "# <modifier>key = action\n"
         "#\n"
         "# Modifiers: Shift, Ctrl, Alt, Meta\n"
         "#\n"
         "# <ctrl>l = location\n"
         "# <ctrl>f = find\n"
         "# <ctrl>s = search\n"
         "# <ctrl>r = reload\n"
         "# <ctrl>n = new-window\n"
         "# <ctrl>t = new-tab\n"
         "# <ctrl>w = close-tab\n"
         "# <ctrl>o = open-file\n"
         "# <ctrl>u = view-source\n"
         "# <ctrl>b = bookmarks\n"
         "# <ctrl>q = quit\n"
         "# <ctrl>c = copy\n"
         "# <ctrl>+ = zoom-in\n"
         "# <ctrl>- = zoom-out\n"
         "# <ctrl>0 = zoom-reset\n"
         "# back = back\n"
         "# shift-back = forward\n"
         "# <alt>f = file-menu\n"
         "# <ctrl>tab = next-tab\n"
         "# <ctrl><shift>tab = prev-tab\n"
         "# esc = escape\n";
   } else if (strcmp(rcFile, "cookiesrc") == 0) {
      default_content =
         "# vibedillo cookie configuration\n"
         "#\n"
         "# WARNING: this file is COMPLETELY vibedillo-specific (Dillo\n"
         "# legacy). It is not a web standard and no other browser has it.\n"
         "#\n"
         "# Format: host action\n"
         "# Action: DENY, ACCEPT_SESSION, ACCEPT\n"
         "#\n"
         "# Default policy (when no other match occurs).\n"
         "# Browsers accept cookies by default; use DENY rules below\n"
         "# to block specific hosts if you want to.\n"
         "DEFAULT ACCEPT\n";
   } else if (strcmp(rcFile, "domainrc") == 0) {
      default_content =
         "# vibedillo domain configuration\n"
         "#\n"
         "# WARNING: this file is COMPLETELY vibedillo-specific (Dillo\n"
         "# legacy). It is not a web standard; browsers load cross-domain\n"
         "# CSS/images/scripts by default and have no such blocker.\n"
         "#\n"
         "# Format: source destination\n"
         "# Default rule (sites usually load CSS/images from CDN domains\n"
         "# on a different public suffix, so the default must be accept):\n"
         "default accept\n"
         "\n"
         "# Exception rules flip the default for a source/destination pair.\n"
         "# With 'default accept' they DENY, e.g. to block trackers:\n"
         "# * .doubleclick.net\n"
         "# * .quantserve.com\n";
   } else if (strcmp(rcFile, "dpidrc") == 0) {
      default_content =
         "# vibedillo DPI configuration\n"
         "dpi_dir=/usr/lib/vibedillo/dpi\n"
         "\n"
         "proto.file=file/file.dpi\n"
         "proto.ftp=ftp/ftp.filter.dpi\n"
         "proto.data=datauri/datauri.filter.dpi\n";
   } else if (strcmp(rcFile, "style.css") == 0) {
      default_content =
         "/* User stylesheet for vibedillo */\n"
         "/* Add custom CSS rules here */\n";
   } else if (strcmp(rcFile, "bm.txt") == 0) {
      default_content =
         ":s0: Bookmarks\n"
         "\n"
         "s0 https://walley.org/ walley.org\n";
   }

   if (default_content) {
      FILE *fp = fopen(path, "w");
      if (fp) {
         fputs(default_content, fp);
         fclose(fp);
         MSG("paths: Created default config '%s'\n", path);
      }
   }
}

