Observer Example
===============

This is a sample project showing you how to use the `Observer` API. `Observer` in its underlying class structure works on top of [AntTweakBar](http://anttweakbar.sourceforge.net/doc/) Therefore everything you may already know about the library or can find online, is applicable here.

Usage is really simple. The Observer class was designed in a way so it can be integrated in any class. A very good example of how to integrate Observer in a class can be found in `sprite.cpp`.

Important points to remember are:

 - In order to make your class `observable`, you have to inherit from `ds::Observer`.
 - You can extend `virtual void installObserver()` to add your class members to the observer GUI.
 - You may provide a hash function by extending `virtual std::string observerHashGenerator() const` to ensure your `Observers` have unique names
 - At any point, you can call `getObserver()` and that will return a pointer to your class' `TwBar*` object
 - There are templated wrappers around `TwAddVar**`  API's for convenience. They are all under `observe(...)`signature
 - For example calling `observe<bool>("bool member", setter, getter, this)` is identical to `TwAddVarCB(getObserver(), "bool member", TW_TYPE_CPPBOOL, setter, getter, this, "")`
 - You would want to usually use `observe` wrappers for callback access. For raw access use AntTweakBar API
 - `void TW_CALL setter(const void*, void*)` must be signature of setters
 - `void TW_CALL getter(void*, void*)` must be signature of getters
 - **Look at sprite.cpp**!

If you get fatal error C1001: An internal error has occurred in the compiler
------------------------------------------------------------------------
Try passing templates explicitly and pass default arguments. This is a known bug in VS compiler.