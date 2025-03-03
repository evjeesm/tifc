# TIFC - Infinite canvas for your terminal

<div style="text-align:center"><img src="./logo.svg" width="80%"/></div>

![version](https://img.shields.io/github/v/tag/evjeesm/tifc)

> 🚧 Warning
>
> This software is unfinished and can be changed any time with no warning.

## Concept

It is basically a TUI framework that supports infinite 2d space.
Can be thought of as a minimal version of today's mind mapping editors.

## Motivation

I wanted to introduce my self into event based systems and ui's,
as well as improve my C skills. Furthermore, I will test usability
of some other libraries that I am developing in parallel.

## Features
- Unicode support
- Mouse support


## Future Plans
- Make it generic library for tui applications as replacement for curses-like.

# Help

If you call build script without arguments it shows you list of commands.
```console
./build.sh
```

If you do:
```console
./build.sh help compile
```
will give you help info about compile sub-command, etc...


For generating of "compile_commands.json" file:
```console
./build.sh init_clangd
```

## Build

Build modules once:
```console
./rebuild-modules.sh update
./rebuild-modules.sh fresh
./rebuild-modules.sh config
./rebuild-modules.sh make
```
You can just use `config` and then `make` to perform light configuration change (compile flags).
If things went wrong, `remove-all` modules, to perform clean setup again.
`remake` is equivalent to `clean` + `make` for all submodules.

Build project:
```console
./build.sh compile
```
For cleaning:
```console
./build.sh clean
```

## TODO

Panel content is not yet functional.
What I doing now is making canvas to work inside one of the panels.
So basically it will use area of the panel as render area.
I already have two types of panels:
- PANEL_CONTENT_TYPE_RAW (this will be used for canvas)
- PANEL_CONTENT_TYPE_GRID (for ui layouts)
