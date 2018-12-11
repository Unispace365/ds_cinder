# C++ Style Guide

We're loosely following the [Google Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml)
as a starting point, with the changes noted below.

## Misc Best Practices

### Strings
* Prefer checking str.empty() over (str == "")

### Lambdas
* Capture as little as possible. Only capture 'this' if it will be used.
* Capture local variables by value rather than reference whenever possible
* Inline lambdas should be indented:

```cpp
setCallback([this]{
		aMemberFunction();
	});
```

### Enums
* Prefer class/struct enums.
```cpp
// Old Style (This can cause problems if any other enum or define uses the same names)
enum AppMode { AMBIENT, ENGAGE, NONE };
AppMode mode = 2; // This is legal!!!
AppMode other = NONE; // No way to be sure that this is the correct NONE!

// New Style:
enum class AppMode { AMBIENT, ENGAGE, NONE };
AppMode mode = 2; // Not legal, class enums are strongly typed!
AppMode other = NONE; // Nope!
auto good = AppMode::NONE; // No conflicts, and it's always clear what you are referring too
```

### Ifs and Loops
* Always use curly braces for conditions and loops, with the exception of single line ifs like:
	`if(!thing) return;`.
* Prefer `for(auto x : y)` whenever possible, and use descriptive iterator names (not just 'it')
* I like the `if(auto x = getSprite("my_sweet_child")){ ... }` structure when possible, since it limits the scope
	of the variable to the if block. Helps prevent checking if a pointer is null but still having a
	null pointer in scope somewhere further along.

---

## Exceptions
Google doesn't allow them.  We don't really have a choice, since libraries we consume can throw
exceptions. Try to be aware of functions that can throw execptions, and try to catch them as close
as possible to the call-site.


## File Names, Locations, and Includes
We're using .cpp / .h

Follow the existing pattern set up in ds_cinder and the full_starter project. All source & header
files are in the `src/` tree, and paired files should be in the same directory.

When including paths, use forward slash for linux compatibility.  Admittedly, this is annoying, since VisualStudio defaults to backslash and apparently there's no way to change that currently.

When including ds_cinder/cinder/etc headers in your app, use angled includes: `#include <ds/thing.h>`
For includes within your app, use quoted includes: `#include "events/my_events.h"`



## Naming
### Variable Names
We use camelCase for local variables. Toss a 'y' on the end if it shadows an existing name, or if ya
just wanna be cute. 

Class & struct member variables are prefixed with an 'm' to differentiate them from local variables & functions.
`int mLongMemberVariable;`


### Function Names
Function names are camelCase. Avoid adding free (non-member) functions to the root `ds::` namespace.

### Class Names
Class names are MixedCase.

### Namespace Names
We use lower_snake_case for namespace names, and don't indent namespace blocks. Avoid using
acronyms, but try to keep namespaces short and non-tedious.

```cpp
namespace my_cool_app {
class NotIndentedHere{...};
} // ! End my_cool_app
```

### Header Guards
```cpp
#pragma once
#ifndef NAMESPACENAME_FOLDER_SUBFOLDER_FILE_NAME
#define NAMESPACENAME_FOLDER_SUBFOLDER_FILE_NAME
// CODE GOES HERE
#endif
```

### General Names
Avoid long strings of capitals

Prefer: `JciApp`
instead of: `JCIApp`

## Spacing
The current convention is to run the entire function signature together, i.e.
```cpp
void aFunctionThatIsBoring(void);
int aFunctionThatLooksLikeItMightDoSomething(const int);
some_error_type aFunctionThatHasInfo(const int);
```
	
I prefer aligning the spaces on everything:
```cpp
void				aFunctionThatIsBoring(void);
int					aFunctionThatLooksLikeItMightDoSomething(const int);
some_error_type		aFunctionThatHasInfo(const int);
```

It's up to you how pedantic you want to be about this in application code. When working on existing
projects or ds_cinder, try to maintain consistency with the existing formatting. Or if you're
feeling extra saucy, reformat the file to be consistent.


## Constructor Initializer Lists
Each initializer on its own line, with leading delimiters:

```cpp
Class::Class()
	: var0(0)
	, var(1)
```

Avoid (or at least use caution) when initializing members using other members as input:
```cpp
class Classy {
public:
	// !!!! This will not produce the correct result!!!!
	// Members are initialized in the order they are declared in the class, not the order of
	// initialization
	Classy(): b(5), a(b+10){};
private:
int a;
int b;
};
```

Personally I also prefer to assign member variables default values in the header. It helps avoid
needing to duplicate all the initializers for every constructor. It also reduces the likelihood of
forgetting to initialize pointers to nullptr.

## Comments / Doxygen
Use three slashes for doxygen comments, and the backslash form for doxygen commands.
```cpp
/// Does the thing, this is the 'breif' description.
/// A longer winded explanation of the thing. What does it do?
/// Does it do things? Lets find out!
/// \param val an input to control the way the thing is done.
/// \return did the thing get done.
bool doTheThing(int val);

int mMemberThing; ///< inline comment about what mMemberThing is
```
