# PythonMonkey

[![Test and Publish Suite](https://github.com/Distributive-Network/PythonMonkey/actions/workflows/test-and-publish.yaml/badge.svg)](https://github.com/Distributive-Network/PythonMonkey/actions/workflows/test-and-publish.yaml)

## About
[PythonMonkey](https://pythonmonkey.io) is a Mozilla [SpiderMonkey](https://firefox-source-docs.mozilla.org/js/index.html) JavaScript engine embedded into the Python Runtime,
using the Python engine to provide the Javascript host environment.

We feature JavaScript Array and Object methods implemented on Python List and Dictionaries using the cPython C API, and the inverse using the Mozilla Firefox Spidermonkey JavaScript C++ API.

This project has reached MVP as of September 2024. It is under maintenance by [Distributive](https://distributive.network/).

External contributions and feedback are welcome and encouraged.

### tl;dr
```bash
$ pip install pythonmonkey
```
```python
from pythonmonkey import eval as js_eval

js_eval("console.log")('hello, world')
```

### Goals
- **Fast** and memory-efficient
- Make writing code in either JS or Python a developer preference
- Use JavaScript libraries from Python
- Use Python libraries from JavaScript
- The same process runs both JavaScript and Python VirtualMachines - no serialization, pipes, etc
- Python Lists and Dicts behave as Javacript Arrays and Objects, and vice-versa, fully adapting to the given context.

### Data Interchange
- Strings share immutable backing stores whenever possible (when allocating engine choses UCS-2 or Latin-1 internal string representation) to keep memory consumption under control, and to make it possible to move very large strings between JavaScript and Python library code without memory-copy overhead.
- TypedArrays share mutable backing stores.
- JS objects are represented by Python dicts through a Dict subclass for optimal compatibility. Similarly for JS arrays and Python lists.
- JS Date objects are represented by Python datetime.datetime objects
- Intrinsics (boolean, number, null, undefined) are passed by value
- JS Functions are automatically wrapped so that they behave like Python functions, and vice-versa
- Python Lists are represented by JS true arrays and support all Array methods through a JS API Proxy. Similarly for Python Dicts and JS objects.

### Roadmap
- [done] JS instrinsics coerce to Python intrinsics
- [done] JS strings coerce to Python strings
- [done] JS objects coerce to Python dicts [own-properties only]
- [done] JS functions coerce to Python function wrappers
- [done] JS exceptions propagate to Python
- [done] Implement `eval()` function in Python which accepts JS code and returns JS->Python coerced values
- [done] NodeJS+NPM-compatible CommonJS module system
- [done] Python strings coerce to JS strings
- [done] Python intrinsics coerce to JS intrinsics
- [done] Python dicts coerce to JS objects
- [done] Python `require` function, returns a coerced dict of module exports
- [done] Python functions coerce to JS function wrappers
- [done] CommonJS module system .py loader, loads Python modules for use by JS
- [done] Python host environment supplies event loop, including EventEmitter, setTimeout, etc.
- [done] Python host environment supplies XMLHttpRequest
- [done] Python TypedArrays coerce to JS TypeArrays
- [done] JS TypedArrays coerce to Python TypeArrays
- [done] Python lists coerce to JS Arrays
- [done] JS arrays coerce to Python lists
- [90%] PythonMonkey can run the dcp-client npm package from Distributive.

## Build Instructions

Read this if you want to build a local version.

1. You will need the following installed (which can be done automatically by running `./setup.sh`):
    - bash
    - cmake
    - Doxygen 1.9 series (if you want to build the docs)
    - graphviz  (if you want to build the docs)
    - llvm
    - rust
    - python3.8 or later with header files (python3-dev)
    - spidermonkey latest from mozilla-central
    - [Poetry](https://python-poetry.org/docs/#installation)
    - [poetry-dynamic-versioning](https://github.com/mtkennerly/poetry-dynamic-versioning)

2. Run `poetry install`. This command automatically compiles the project and installs the project as well as dependencies into the poetry virtualenv. If you would like to build the docs, set the `BUILD_DOCS` environment variable, like so: `BUILD_DOCS=1 poetry install`.
PythonMonkey supports multiple build types, which you can build by setting the `BUILD_TYPE` environment variable, like so: `BUILD_TYPE=Debug poetry install`. The build types are (case-insensitive):
- `Release`: stripped symbols, maximum optimizations (default)
- `DRelease`: same as `Release`, except symbols are not stripped
- `Debug`: minimal optimizations
- `Sanitize`: same as `Debug`, except with [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer) enabled
- `Profile`: same as `Debug`, except profiling is enabled 
- `None`: don't compile (useful if you only want to build the docs)

If you are using VSCode, you can just press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd> to [run build task](https://code.visualstudio.com/docs/editor/tasks#_custom-tasks) - We have [the `tasks.json` file configured for you](.vscode/tasks.json).

## Running tests
1. Compile the project
2. Install development dependencies: `poetry install --no-root --only=dev`
3. From the root directory, run `poetry run pytest ./tests/python`
4. From the root directory, run `poetry run bash ./peter-jr ./tests/js/`

For VSCode users, similar to the Build Task, we have a Test Task ready to use.

## Using the library

> npm (Node.js) is required **during installation only** to populate the JS dependencies.

### Install from [PyPI](https://pypi.org/project/pythonmonkey/)

```bash
$ pip install pythonmonkey
```

### Install the [nightly build](https://nightly.pythonmonkey.io/)

```bash
$ pip install --extra-index-url https://nightly.pythonmonkey.io/ --pre pythonmonkey
```

### Use local version

`pythonmonkey` is available in the poetry virtualenv once you compiled the project using poetry.

```bash
$ poetry run python
```
```py
Python 3.10.6 (main, Nov 14 2022, 16:10:14) [GCC 11.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import pythonmonkey as pm
>>> hello = pm.eval("() => {return 'Hello from Spidermonkey!'}")
>>> hello()
'Hello from Spidermonkey!'
```

Alternatively, you can build installable packages by running
```bash
$ cd python/pminit && poetry build --format=sdist && cd - && mv -v python/pminit/dist/* ./dist/
$ poetry build --format=wheel
```
and install them by `pip install ./dist/*`.

## Uninstallation

Installing `pythonmonkey` will also install the `pminit` package as a dependency. However, `pip uninstall`ing a package won't automatically remove its dependencies.  
If you want to cleanly remove `pythonmonkey` from your system, do the following:

```bash
$ pip uninstall pythonmonkey pminit
```

## Debugging Steps

1. [build the project locally](#build-instructions)
2. To use gdb, run `poetry run gdb python`.
See [Python Wiki: DebuggingWithGdb](https://wiki.python.org/moin/DebuggingWithGdb)

If you are using VSCode, it's more convenient to debug in [VSCode's built-in debugger](https://code.visualstudio.com/docs/editor/debugging). Simply press <kbd>F5</kbd> on an open Python file in the editor to start debugging - We have [the `launch.json` file configured for you](https://github.com/Distributive-Network/PythonMonkey/blob/main/.vscode/launch.json).

## Examples

* [examples/](https://github.com/Distributive-Network/PythonMonkey/tree/main/examples)
* https://github.com/Distributive-Network/PythonMonkey-examples
* https://github.com/Distributive-Network/PythonMonkey-Crypto-JS-Fullstack-Example

## Official API
These methods are exported from the pythonmonkey module. See definitions in [python/pythonmonkey/pythonmonkey.pyi](https://github.com/Distributive-Network/PythonMonkey/blob/main/python/pythonmonkey/pythonmonkey.pyi).

### eval(code, options)
Evaluate JavaScript code. The semantics of this eval are very similar to the eval used in JavaScript;
the last expression evaluated in the `code` string is used as the return value of this function. To
evaluate `code` in strict mode, the first expression should be the string `"use strict"`.

#### options
The eval function supports an options object that can affect how JS code is evaluated in powerful ways.
They are largely based on SpiderMonkey's `CompileOptions`. The supported option keys are:
- `filename`: set the filename of this code for the purposes of generating stack traces etc.
- `lineno`: set the line number offset of this code for the purposes of generating stack traces etc.
- `column`: set the column number offset of this code for the purposes of generating stack traces etc.
- `mutedErrors`: if set to `True`, eval errors or unhandled rejections are ignored ("muted"). Default `False`.
- `noScriptRval`: if `False`, return the last expression value of the script as the result value to the caller. Default `False`.
- `selfHosting`: *experimental*
- `strict`: forcibly evaluate in strict mode (`"use strict"`). Default `False`.
- `module`: indicate the file is an ECMAScript module (always strict mode code and disallow HTML comments). Default `False`.
- `fromPythonFrame`: generate the equivalent of filename, lineno, and column based on the location of
  the Python call to eval. This makes it possible to evaluate Python multiline string literals and
  generate stack traces in JS pointing to the error in the Python source file.

#### tricks
- function literals evaluate as `undefined` in JavaScript; if you want to return a function, you must
  evaluate an expression:
  ```python
  pythonmonkey.eval("myFunction() { return 123 }; myFunction")
  ```
  or
  ```python
  pythonmonkey.eval("(myFunction() { return 123 })")
  ```
- function expressions are a great way to build JS IIFEs that accept Python arguments
  ```python
  pythonmonkey.eval("(thing) => console.log('you said', thing)")("this string came from Python")
  ```

### require(moduleIdentifier)
Return the exports of a CommonJS module identified by `moduleIdentifier`, using standard CommonJS
semantics
 - modules are singletons and will never be loaded or evaluated more than once
 - moduleIdentifier is relative to the Python file invoking `require`
 - moduleIdentifier should not include a file extension
 - moduleIdentifiers which do not begin with ./, ../, or / are resolved by search require.path
   and module.paths.
 - Modules are evaluated immediately after loading
 - Modules are not loaded until they are required
 - The following extensions are supported:
  * `.js` - JavaScript module; source code decorates `exports` object
  * `.py` - Python module; source code decorates `exports` dict
  * `.json` - JSON module; exports are the result of parsing the JSON text in the file

### globalThis
A Python Dict which is equivalent to the globalThis object in JavaScript.

### createRequire(filename, extraPaths, isMain)
Factory function which returns a new require function
- filename: the pathname of the module that this require function could be used for
- extraPaths: [optional] a list of extra paths to search to resolve non-relative and non-absolute module identifiers
- isMain: [optional] True if the require function is being created for a main module

### runProgramModule(filename, argv, extraPaths)
Load and evaluate a program (main) module. Program modules must be written in JavaScript. Program modules are not
necessary unless the main entry point of your program is written in JavaScript.
- filename: the location of the JavaScript source code
- argv: the program's argument vector
- extraPaths: [optional] a list of extra paths to search to resolve non-relative and non-absolute module identifiers

Care should be taken to ensure that only one program module is run per JS context.

### isCompilableUnit(code)
Examines the string `code` and returns False if the string might become a valid JS statement with
the addition of more lines. This is used internally by pmjs and can be very helpful for building
JavaScript REPLs; the idea is to accumulate lines in a buffer until isCompilableUnit is true, then
evaluate the entire buffer.

### new(function)
Returns a Python function which invokes `function` with the JS new operator.
```python
import pythonmonkey as pm

>>> pm.eval("class MyClass { constructor() { console.log('ran ctor') }}")
>>> MyClass = pm.eval("MyClass")
>>> MyClass()
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
pythonmonkey.SpiderMonkeyError: TypeError: class constructors must be invoked with 'new'

>>> MyClassCtor = pm.new(MyClass)
>>> MyClassCtor()
ran ctor
{}
>>>
```

### typeof(value)
This is the JS `typeof` operator, wrapped in a function so that it can be used easily from Python.

### Standard Classes and Globals
All of the JS Standard Classes (Array, Function, Object, Date...) and objects (globalThis,
FinalizationRegistry...) are available as exports of the pythonmonkey module. These exports are
generated by enumerating the global variable in the current SpiderMonkey context. The current list is:
<blockquote>undefined, Boolean, JSON, Date, Math, Number, String, RegExp, Error, InternalError, AggregateError, EvalError, RangeError, ReferenceError, SyntaxError, TypeError, URIError, ArrayBuffer, Int8Array, Uint8Array, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array, Uint8ClampedArray, BigInt64Array, BigUint64Array, BigInt, Proxy, WeakMap, Map, Set, DataView, Symbol, Intl, Reflect, WeakSet, Promise, WebAssembly, WeakRef, Iterator, AsyncIterator, NaN, Infinity, isNaN, isFinite, parseFloat, parseInt, escape, unescape, decodeURI, encodeURI, decodeURIComponent, encodeURIComponent, Function, Object, debuggerGlobal, FinalizationRegistry, Array, globalThis</blockquote>

## Built-In Functions

See definitions in [python/pythonmonkey/global.d.ts](https://github.com/Distributive-Network/PythonMonkey/blob/main/python/pythonmonkey/global.d.ts).
Including:

- `console`
- `atob`
- `btoa`
- `setTimeout`
- `clearTimeout`

### CommonJS Subsystem Additions
The CommonJS subsystem is activated by invoking the `require` or `createRequire` exports of the (Python)
pythonmonkey module.

- `require`
- `exports`
- `module`
- `__filename`
- `__dirname`
- `python.print`  - the Python print function
- `python.getenv` - the Python getenv function
- `python.stdout` - an object with `read` and `write` methods, which read and write to stdout
- `python.stderr` - an object with `read` and `write` methods, which read and write to stderr
- `python.exec`   - the Python exec function
- `python.eval`   - the Python eval function
- `python.exit`   - exit via sys.exit(); the exit code is the function argument or `python.exit.code`.
- `python.paths`  - the Python sys.paths list, visible in JS as an Array

## Type Transfer (Coercion / Wrapping)
When sending variables from Python into JavaScript, PythonMonkey will intelligently coerce or wrap your
variables based on their type. PythonMonkey will share backing stores (use the same memory) for ctypes,
typed arrays, and strings; moving these types across the language barrier is extremely fast because
there is no copying involved.

*Note:* There are plans in Python 3.12 (PEP 623) to change the internal string representation so that
        every character in the string uses four bytes of memory. This will break fast string transfers
        for PythonMonkey, as it relies on the memory layout being the same in Python and JavaScript. As
        of this writing (July 2023), "classic" Python strings still work in the 3.12 beta releases.

Where shared backing store is not possible, PythonMonkey will automatically emit wrappers that use
the "real" data structure as its value authority. Only immutable intrinsics are copied. This means
that if you update an object in JavaScript, the corresponding Dict in Python will be updated, etc.

JavaScript Array and Object methods are implemented on Python List and Dictionaries, and vice-versa.

| Python Type | JavaScript Type |
|:------------|:----------------|
| String      | string
| Integer     | number
| Bool        | boolean
| Function    | function
| Dict        | object
| List        | Array
| datetime    | Date object
| awaitable   | Promise
| Error       | Error object
| Buffer      | ArrayBuffer

| JavaScript Type      | Python Type     |
|:---------------------|:----------------|
| string               | pythonmonkey.JSStringProxy (String)
| number               | Float
| bigint               | pythonmonkey.bigint (Integer)
| boolean              | Bool
| function             | pythonmonkey.JSFunctionProxy || pythonmonkey.JSMethodProxy (Function || Method)
| object - most        | pythonmonkey.JSObjectProxy (Dict)
| object - Date        | datetime
| object - Array       | pythonmonkey.JSArrayProxy (List)
| object - Promise     | awaitable
| object - ArrayBuffer | Buffer
| object - type arrays | Buffer
| object - Error       | Error

## Tricks
### Integer Type Coercion
You can force a number in JavaScript to be coerced as an integer by casting it to BigInt:
```javascript
function myFunction(a, b) {
  const result = calculate(a, b);
  return BigInt(Math.floor(result));
}
```

The `pythonmonkey.bigint` object works like an int in Python, but it will be coerced as a BigInt in JavaScript:
```python
import pythonmonkey

def fn myFunction()
  result = 5
  return pythonmonkey.bigint(result)
```

### Symbol injection via cross-language IIFE
You can use a JavaScript IIFE to create a scope in which you can inject Python symbols:
```python
globalThis.python.exit = pm.eval("""'use strict';
(exit) => function pythonExitWrapper(exitCode) {
  if (typeof exitCode === 'number')
    exitCode = BigInt(Math.floor(exitCode));
  exit(exitCode);
}
""")(sys.exit);
```

### Run Python event-loop

You need an event-loop running to use `setTimeout` and `Promise`<=>`awaitable` coercion.

```python
import asyncio

async def async_fn():
  await pm.eval("""
    new Promise((resolve) => setTimeout((...args) => {
        console.log(args);
        resolve();
      }, 1000, 42, "abc")
    )
  """)
  await pm.eval("async (x) => await x")(asyncio.sleep(0.5))

asyncio.run(async_fn())
```

# pmjs
A basic JavaScript shell, `pmjs`, ships with PythonMonkey. This shell can act as a REPL or run
JavaScript programs; it is conceptually similar to the `node` shell which ships with Node.js.

## Modules
Pmjs starts PythonMonkey's CommonJS subsystem, which allow it to use CommonJS modules, with semantics
that are similar to Node.js - e.g. searching module.paths, understanding package.json, index.js, and
so on. See the [ctx-module](https://www.npmjs.com/package/ctx-module) for a full list of supported
features.

In addition to CommonJS modules written in JavaScript, PythonMonkey supports CommonJS modules written
in Python. Simply decorate a Dict named `exports` inside a file with a `.py` extension, and it can be
loaded by `require()` -- in either JavaScript or Python.

### Program Module
The program module, or main module, is a special module in CommonJS. In a program module:
 - variables defined in the outermost scope are properties of `globalThis`
 - returning from the outermost scope is a syntax error
 - the `arguments` variable in an Array which holds your program's argument vector
   (command-line arguments)

```console
$ echo "console.log('hello world')" > my-program.js
$ pmjs my-program.js
hello world
$
```

### CommonJS Module: JavaScript language
```js
// date-lib.js - require("./date-lib")
const d = new Date();
exports.today = `${d.getFullYear()}-${String(d.getMonth()).padStart(2,'0')}-${String(d.getDay()).padStart(2,'0')}`
```

### CommonJS Module: Python language
```python
# date-lib.py - require("./date-lib")
from datetime import date # You can use Python libraries.
exports['today'] = date.today()
```

# Troubleshooting Tips

## CommonJS (require)
If you are having trouble with the CommonJS require function, set environment variable `DEBUG='ctx-module*'` and you can see the filenames it tries to load.

## pmdb

PythonMonkey has a built-in gdb-like JavaScript command-line debugger called **pmdb**, which would be automatically triggered on `debugger;` statements and uncaught exceptions.

To enable **pmdb**, simply call `from pythonmonkey.lib import pmdb; pmdb.enable()` before doing anything on PythonMonkey.

```py
import pythonmonkey as pm
from pythonmonkey.lib import pmdb

pmdb.enable()

pm.eval("...")
```

Run `help` command in **pmdb** to see available commands.

```console
(pmdb) > help
List of commands:
• ...
• ...
```

## pmjs
- there is a `.help` menu in the REPL
- there is a `--help` command-line option
- the `--inspect` option enables **pmdb**, a gdb-like JavaScript command-line debugger
- the `-r` option can be used to load a module before your program or the REPL runs
- the `-e` option can be used evaluate code -- e.g. define global variables -- before your program or the REPL runs
- The REPL can evaluate Python expressions, storing them in variables named `$1`, `$2`, etc.

```console
$ pmjs

Welcome to PythonMonkey v1.0.0.
Type ".help" for more information.
> .python import sys
> .python sys.path
$1 = { '0': '/home/wes/git/pythonmonkey2',
  '1': '/usr/lib/python310.zip',
  '2': '/usr/lib/python3.10',
  '3': '/usr/lib/python3.10/lib-dynload',
  '4': '/home/wes/.cache/pypoetry/virtualenvs/pythonmonkey-StuBmUri-py3.10/lib/python3.10/site-packages',
  '5': '/home/wes/git/pythonmonkey2/python' }
> $1[3]
'/usr/lib/python3.10/lib-dynload'
>
```
