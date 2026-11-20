# AGENTS.md - Vibedillo Browser

## Project Overview
Vibedillo is a fork of Dillo 3.x - a lightweight, fast web browser written in C++ using FLTK.

### Key Changes from Dillo
- **Package name**: `vibedillo` (was `dillo`)
- **Config dir**: `~/.config/vibedillo` (XDG standard, was `~/.dillo`)
- **Config file**: `vibedillorc` (was `dillorc`)
- **Version**: 3.3.4
- **Window title**: "vibedillo:" (was "Dillo:")

## Major Features Implemented

1. **CSS Grid Layout** - Full support:
   - `display: grid`, `grid-template-columns/rows`, `grid-column/row`, `grid-gap`
   - Named lines, `minmax()`, `repeat()`, `fr` units
   - Negative line numbers, auto-placement algorithm

2. **Dark Mode** - Complete implementation:
   - Config option `color_scheme` in vibedillorc (`auto`/`light`/`dark`)
   - `<meta name="color-scheme" content="dark light">` parsing in `html.cc`
Do NOT change user config, ask user if you need change

3. **about: protocol** - `about:about`, `about:dillo`, `about:settings`, `about:cache`, `about:dicache`

4. **All dillo references renamed** to vibedillo in strings/UI

## Key Source Locations

| Feature | Files |
|---------|-------|
| **User Agent Stylesheet / Dark Mode CSS** | `src/styleengine.cc` - `StyleEngine::init()` |
| **Meta tag parsing (color-scheme)** | `src/html.cc` - `Html_tag_open_meta()` |
| **Body/Layout dark background** | `src/html.cc` - `Html_tag_open_body()` |
| **Color scheme state** | `src/html_common.hh` - `DilloHtmlState.color_scheme` |
| **Media query evaluation** | `src/cssparser.cc` - `parseMediaExpression()` (prefers-color-scheme) |
| **Config option (color_scheme)** | `src/prefs.h`, `src/prefs.c`, `src/prefsparser.cc` |
| **Config directory (XDG)** | `src/paths.cc`, `src/paths.hh` |
| **Version / Package name** | `configure.ac` |
| **Documentation** | `doc/user_help.html`, `doc/install.md`, `doc/vibedillo.1.in` |

## Build & Test

```bash
cd /home/walley/git/vibedillo
./configure && make -C src -j4
src/vibedillo file:///tmp/fl/color-scheme-ttt.htm  # Test dark mode
```


Options: `auto` (default), `light`, `dark`

## Regression Tests

All pass: CSS Grid, Flexbox, Floats, ACID1, etc.

## Architecture Notes

- **Style Engine**: Parses user agent, user, and author stylesheets with cascade
- **CSS Parser**: Handles @media rules, media queries, selectors, properties
- **HTML Parser**: Tokenizes HTML, builds DOM, handles meta tags
- **Layout Engine**: `dw::core::Layout` (in `../dw/libDw-core.a`) - handles box model, grid, flexbox
- **Rendering**: Cairo-based via FLTK widgets
