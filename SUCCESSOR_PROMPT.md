# Vibedillo Browser - Successor Prompt

## Project Overview
Vibedillo is a fork of Dillo 3.x - a lightweight, fast web browser written in C++ using FLTK. Key changes from Dillo:
- **Package name**: `vibedillo` (was `dillo`)
- **Config dir**: `~/.config/vibedillo` (XDG standard, was `~/.dillo`)
- **Config file**: `vibedillorc` (was `dillorc`)
- **Version**: 3.3.4
- **Window title**: "vibedillo:" (was "Dillo:")

## Major Features Implemented
1. **CSS Grid Layout** - Full support (display: grid, grid-template-columns/rows, grid-column/row, grid-gap, named lines, minmax, repeat, fr units, negative lines, auto-placement)
2. **Dark Mode** - Complete implementation:
   - Config option `color_scheme` in vibedillorc (auto/light/dark)
   - `<meta name="color-scheme" content="dark light">` parsing in `html.cc`
3. **about: protocol** - about:about, about:dillo, about:settings, about:cache, about:dicache

## Key Source Locations
git at /home/walley/git/vibedillo
```
src/
├── styleengine.cc       # User agent stylesheet with @media (prefers-color-scheme: dark)
├── html.cc              # Html_tag_open_meta (color-scheme), Html_tag_open_body (dark bg)
├── html_common.hh       # DilloHtmlState.color_scheme field
├── cssparser.cc         # Media query evaluation for prefers-color-scheme
├── prefs.h/.c           # color_scheme config option
├── prefsparser.cc       # Parses color_scheme from vibedillorc
├── paths.cc             # Config dir ~/.config/vibedillo, loads config files
├── paths.hh             # PATHS_RC_PREFS = "vibedillorc"
├── configure.ac         # Version 3.3.4, package name vibedillo
doc/
├── user_help.html       # Documentation for about: protocol and color_scheme
├── install.md           # Updated for vibedillo
├── vibedillo.1.in       # Man page
```

## Build & Test
```bash
cd /home/walley/git/vibedillo
./configure && make -C src -j4
src/vibedillo file:///tmp/fl/color-scheme-ttt.htm  # Test dark mode
```

