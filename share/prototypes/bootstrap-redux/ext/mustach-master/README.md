Introduction to Mustach 0.99
============================

`mustach` is a C implementation of the [mustache](http://mustache.github.io "main site for mustache")
template specification.

The main site for `mustach` is on [gitlab](https://gitlab.com/jobol/mustach).

The best way to use mustach is to copy the files **mustach.h** and **mustach.c**
directly into your project and use it.

Alternatively, make and meson files are provided for building `mustach` and 
`libmustach.so` shared library.

The file **mustach.h** is the main documentation. Look at it.

The current source files are:

- **mustach.c** core implementation of mustache in C
- **mustach.h** header file for core definitions
- **mustach-json-c.c** tiny json wrapper of mustach using [json-c](https://github.com/json-c/json-c)
- **mustach-json-c.h** header file for using the tiny JSON wrapper
- **mustach-tool.c** simple tool for applying template files to a JSON file

The file **mustach-json-c.c** is the main example of use of **mustach** core
and it is also a practical implementation that can be used. It uses the library
json-c. (NOTE for Mac OS: available through homebrew).

HELP REQUESTED TO GIVE EXAMPLE BASED ON OTHER LIBRARIES (ex: janson, ...).

The tool **mustach** is build using `make`,  its usage is:

    mustach json template [template]...

It then outputs the result of applying the templates files to the JSON file.

Portability
===========

Some system does not provide *open_memstream*. In that case, tell your
prefered compiler to declare the preprocessor symbol **NO_OPEN_MEMSTREAM**.
Example:

	gcc -DNO_OPEN_MEMSTREAM

Integration
===========

The file **mustach.h** is the main documentation. Look at it.

The file **mustach-json-c.c** provides a good example of integration.

If you intend to use basic HTML/XML escaping and standard C FILE, the callbacks
of the interface **mustach_itf** that you have to implement are:
`enter`, `next`, `leave`, `get`.

If you intend to use specific escaping and/or specific output, the callbacks
of the interface **mustach_itf** that you have to implement are:
`enter`, `next`, `leave`, `get` and `emit`.

Extensions
==========

By default, the current implementation provides the following extensions:

Explicit Substitution
---------------------

This is a core extension implemented in file **mustach.c**.

In somecases the name of the key used for substition begins with a
character reserved for mustach: one of `#`, `^`, `/`, `&`, `{`, `>` and `=`.
This extension introduces the special character `:` to explicitly
tell mustach to just substitute the value. So `:` becomes a new special
character.

Value Testing and Comparing
---------------------------

This are a tool extension implemented in file **mustach-json-c.c**.

These extensions allows you to test the value of the selected key.
They allow to write `key=value` (matching test) or `key=!value`
(not matching test) in any query.

The specific comparison extension also allows to compare if greater,
lesser, etc.. than a value. It allows to write `key>value`.

It the comparator sign appears in the first column it is ignored
as if it was escaped.

Access to current value
-----------------------

The value of the current field can be accessed using single dot like
in `{{#key}}{{.}}{{/key}}` that applied to `{"key":3.14}` produces `3.14`
and `{{#array}} {{.}}{{/array}}` applied to `{"array":[1,2]}` produces
` 1 2`.

Iteration on objects
--------------------

Using the pattern `{{#X.*}}...{{/X.*}}` it is possible to iterate on
fields of `X`. Example: `{{s.*}} {{*}}:{{.}}{{/s.*}}` applied on
`{"s":{"a":1,"b":true}}` produces ` a:1 b:true`. Here the single star
`{{*}}` is replaced by the iterated key and the single dot `{{.}}` is
replaced by its value.

Removing Extensions
-------------------

When compiling mustach.c or mustach-json-c.c,
extensions can be removed by defining macros
using option -D.

The possible macros are of 3 categories, the global,
the mustach core specific and the mustach-json-c example
of implementation specific.

### Global macro

- `NO_EXTENSION_FOR_MUSTACH`

  This macro disables any current or future
  extensions for the core or the example.

### Macros for the core mustach engine (mustach.c)

- `NO_COLON_EXTENSION_FOR_MUSTACH`

  This macro remove the ability to use colon (:)
  as explicit command for variable substituion.
  This extension allows to have name starting
  with one of the mustach character `:#^/&{=>`

- `NO_ALLOW_EMPTY_TAG`

  Generate the error MUSTACH_ERROR_EMPTY_TAG automatically
  when an empty tag is encountered.

### Macros for the implementation example (mustach-json-c.c)

- `NO_EQUAL_VALUE_EXTENSION_FOR_MUSTACH`

  This macro allows the program to check whether
  the actual value is equal to an expected value.
  This is useful in `{{#key=val}}` or `{{^key=val}}`
  with the corresponding `{{/key=val}}`.
  It can also be used in `{{key=val}}` but this
  doesn't seem to be useful.

- `NO_COMPARE_VALUE_EXTENSION_FOR_MUSTACH`

  This macro allows the program to compare the actual
  value with an expected value. The comparison operators
  are `=`, `>`, `<`, `>=`, `<=`. The meaning of the
  comparison numeric or alphabetic depends on the type
  of the inspected value. Also the result of the comparison
  can be inverted if the value starts with `!`.
  Example of use: `{{key>=val}}`, or `{{#key>=val}}` and
  `{{^key>=val}}` with their matching `{{/key>=val}}`.

- `NO_USE_VALUE_ESCAPE_FIRST_EXTENSION_FOR_MUSTACH`

  This macro fordids automatic escaping of coparison
  sign appearing at first column.

- `NO_JSON_POINTER_EXTENSION_FOR_MUSTACH`

  This macro removes the possible use of JSON pointers.
  JSON pointers are defined in IETF RFC 6901.
  If not set, any key starting with "/" is a JSON pointer.
  This implies to use the colon to introduce keys.
  So `NO_COLON_EXTENSION_FOR_MUSTACH` implies
  `NO_JSON_POINTER_EXTENSION_FOR_MUSTACH`.
  A special escaping is used for `=`, `<`, `>` signs when
  values comparisons are enabled: `~=` gives `=` in the key.

- `NO_OBJECT_ITERATION_FOR_MUSTACH`

  Disable the object iteration extension. That extension allows
  to iterate over the keys of an object. The iteration on object
  is selected by using the selector `{{#key.*}}`. In the context
  of iterating over object keys, the single key `{{*}}` returns the
  key and `{{.}}` returns the value.

- `NO_SINGLE_DOT_EXTENSION_FOR_MUSTACH`

  Disable access to current object value using single dot
  like in `{{.}}`.

- `NO_INCLUDE_PARTIAL_FALLBACK`

  Disable include of file by partial pattern like `{{> name}}`.
  By default if a such pattern is found, **mustach** search
  for `name` in the current json context. This what is done
  historically and when `NO_INCLUDE_PARTIAL_FALLBACK` is defined.
  When `NO_INCLUDE_PARTIAL_FALLBACK` is defined, if the value is
  found in the json context, the files `name` and `name.mustache`
  are searched in that order and the first file found is used
  as partial content. The macro `INCLUDE_PARTIAL_EXTENSION` can
  be use for changing the extension added.

