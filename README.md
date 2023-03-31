# PythonMonkey

![Testing Suite](https://github.com/Kings-Distributed-Systems/PythonMonkey/actions/workflows/tests.yaml/badge.svg)

## About
PythonMonkey is a Mozilla [SpiderMonkey](https://firefox-source-docs.mozilla.org/js/index.html) JavaScript engine embedded into the Python VM,
using the Python engine to provide the JS host environment.

This product is in an early stage, approximately 65% to MVP as of March 2023. It is under active development by Distributive Corp.,
https://distributive.network/. External contributions and feedback are welcome and encouraged.

The goal is to make writing code in either JS or Python a developer preference, with libraries commonly used in either language
available eveywhere, with no significant data exchange or transformation penalties. For example, it should be possible to use NumPy 
methods from a JS library, or to refactor a slow "hot loop" written in Python to execute in JS instead, taking advantage of 
SpiderMonkey's JIT for near-native speed, rather than writing a C-language module for Python. At Distributive, we intend to use 
this package to execute our complex `dcp-client` library, which is written in JS and enables distributed computing on the web stack.

### Data Interchange
- Strings share immutable backing stores whenever possible (when allocating engine choses UCS-2 or Latin-1 internal string representation) to keep memory consumption under control, and to make it possible to move very large strings between JS and Python library code without memory-copy overhead.
- TypedArrays to share mutable backing stores; if this is not possible we will implement a copy-on-write (CoW) solution.
- JS objects are represented by Python dicts
- JS Date objects are represented by Python datetime.datetime objects
- Intrinsics (boolean, number, null, undefined) are passed by value
- JS Functions are automatically wrapped so that they behave like Python functions, and vice-versa

### Roadmap
- [done] JS instrinsics coerce to Python intrinsics
- [done] JS strings coerce to Python strings
- JS objects coerce to Python dicts [own-properties only]
- [done] JS functions coerce to Python function wrappers
- [done] JS exceptions propagate to Python
- [done] Implement `eval()` function in Python which accepts JS code and returns JS->Python coerced values
- [underway] NodeJS+NPM-compatible CommonJS module system
- [underway] Python strings coerce to JS strings
- [done] Python intrinsics coerce to JS intrinsics
- Python dicts coerce to JS objects
- Python `require` function, returns a coerced dict of module exports
- Python functions coerce to JS function wrappers
- CommonJS module system .py loader, loads Python modules for use by JS
- JS object->Python dict coercion supports inherited-property lookup (via __getattribute__?)
- Python host environment supplies event loop, including EventEmitter, setTimeout, etc.
- Python host environment supplies XMLHttpRequest (other project?)
- Python host environment supplies basic subsets of NodeJS's fs, path, process, etc, modules; as-needed by dcp-client (other project?)
- Python TypedArrays coerce to JS TypeArrays
- JS TypedArrays coerce to Python TypeArrays

## Build Instructions
1. You will need the following installed (which can be done automatically by running ``./setup.sh``):
    - pytest
    - cmake
    - doxygen 
    - python3-dev (python-dev)
    - graphviz
    - gcovr
    - llvm
    - rust
    - python3.9 or later
    - spidermonkey 102.2.0 or later

2. Compile pythonmonkey in ``/build`` (which can be done automatically by running ``./build_script.sh``)

## Running tests
1. Compile the project 
2. In the build folder `cd` into the `tests` directory and run `ctest`.
    ```bash
    # From the root directory we do the following (after compiling the project)
    $ cd buid/tests
    $ ctest
    ```
    Alternatively, from the root directory, run ``./test_script.sh``

## Using the library

### Method 1
After compiling the project in the `build/src` folder you will find a `.so` file named `pythonmonkey.so`. This is the shared object file that contains the pythonmonkey module.

If you wish to use the library you can simply copy the `.so` file into the directory that you wish to use python in.
```bash
# In the directory containg pythonmonkey.so
$ python
```
```py
Python 3.10.6 (main, Nov 14 2022, 16:10:14) [GCC 11.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import pythonmonkey as pm
>>> hello = pm.eval("() => {return 'Hello from Spidermonkey!'}")
>>> hello()
'Hello from Spidermonkey!'
```