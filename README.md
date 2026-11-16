# what is this repo
Official dillo site: https://dillo-browser.org/

Cloned from original https://github.com/dillo-browser/dillo, detached, 
and applied patches from https://walley.org/dillo/ (26/11/10).
AI continued working on this fork.
Renamed to **vibedillo** (v3.3.2).

## Recent changes (v3.3.2)
- **Config directory**: Changed from `~/.vibedillo` to `~/.config/vibedillo` (XDG standard)
- **Version**: Bumped to 3.3.2

## Recent changes (v3.3.1)
- **CSS Grid Layout**: Full support for `display: grid` with `grid-template-columns/rows`, `grid-column/row`, `grid-gap`, named lines, `minmax()`, `repeat()`, `fr` units, negative line numbers, and auto-placement
- **Dark Mode**: Support for `<meta name="color-scheme" content="dark light">` and `@media (prefers-color-scheme: dark/light)` media queries
- **Font Family**: Explicit `inherit` handling, console warnings for uninstalled fonts
- **About Protocol**: `about:vibedillo`, `about:settings` (full settings table), `about:about`, `about:cache`, `about:dicache`
- **Rename**: dillo → vibedillo, dilloc → vibedilloc, all paths updated (~/.dillo → ~/.vibedillo, /etc/dillo → /etc/vibedillo)

# vibedillo web browser

vibedillo is a multi-platform graphical web browser, known for its speed and
small footprint, that is developed with a focus on personal security and
privacy. It is built with the [FLTK 1.3 GUI toolkit](http://fltk.org).

Screenshot of the [vibedillo Website][vibedillo] rendered in vibedillo:

[vibedillo]: https://walley.org/

![Screenshot of vibedillo website rendered in vibedillo](doc/vibedillo.png)

To install vibedillo follow the [installation guide](doc/install.md).

You can clone the latest version of vibedillo directly from our git server:

- <https://github.com/walley/vibedillo>

The commits are signed with the same OpenPGP we use for the releases.

See also other related forks: [dillo-plus][dillo-plus],
[dilloNG][dilloNG], [D+ browser][dplus-browser] and [Mobilized
Dillo][mobilized].

[dillo-plus]: https://github.com/crossbowerbt/dillo-plus
[dilloNG]: https://github.com/w00fpack/dilloNG
[dplus-browser]: https://sourceforge.net/projects/dplus-browser/
[mobilized]: https://www.toomanyatoms.com/software/mobilized_dillo.html

> **Warning**:
> As of December 2023, the host `dillo.org` is [no longer under control][gone]
> of Dillo developers. A copy of the old website is archived in
> [our website][old] and the [Wayback Machine (May 2022)][may].

[gone]: https://walley.org/dillo.org.html
[old]: https://walley.org/old/
[may]: http://web.archive.org/web/20220508022123/https://www.dillo.org/

