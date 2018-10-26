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

---

## EXCEPTIONS
Google doesn't allow them.  We don't really have a choice, since libraries we consume can throw
exceptions.


## FILE NAMES
they use .cc, we're using .cpp / .h


## FILE LOCATIONS
Currently src and header are being stuffed in the same folder.  It's a little weird to me, although less maintenance.  Little more standard to have separate src/ and include/ directories.  Opinions?


## HEADERS
When including paths, use forward slash for linux compatibility.  Admittedly, this is annoying, since VisualStudio defaults to backslash and apparently there's no way to change that currently.

When including ds_cinder/cinder/etc headers in your app, use angled includes: `#include <ds/thing.h>`
For includes within your app, use quoted includes: `#include "events/my_events.h"`



## NAMING
### VARIABLE NAMES
We use camelCase for variables

Class & struct member variables are prefixed with an 'm' to differentiate them from local variables & functions.
`int mLongMemberVariable;`


### FUNCTION NAMES
Function names are camelCase. Avoid adding free (non-member) functions to the root `ds::` namespace

### CLASS NAMES
Class names are MixedCase.

### GENERAL NAMES
Avoid long strings of capitals

Prefer: `JciApp`
instead of: `JCIApp`

## SPACING
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

I tend to be pretty extreme with this, aligning the tab location on all variables defined in a function, etc.  I don't
really care what people do but I think this is a lot easier to read and gives code an obvious structure.  For example,
in the functions of my code you can generally just scan vertically down a column to see all local variables that are defined.


## PUBLIC / PRIVATE SCOPE
I tend to mutliply define public and private in classes, in blocks that make sense, because it can be confusing to look at a long file and know the scope of what you're looking at.  For example, I usually have separate scope-declared sections for static variables, static functions, variables, functions, etc.  I don't care if other people do, but I find it helpful.


## CONSTRUCTOR INITIALIZER LISTS
Each initializer on its own line, with leading delimiters:

```cpp
Class::Class()
	: var0(0)
	, var(1)
```

## COMMENTS / DOXYGEN
Use three slashes for doxygen comments, and the backslash form for doxygen commands:
```cpp
/// Does the thing, this is the 'breif' description.
/// A longer winded explanation of the thing. What does it do?
/// Does it do things? Lets find out!
/// \param val an input to control the way the thing is done.
/// \return did the thing get done.
bool doTheThing(int val);

int mMemberThing; ///< inline comment about what mMemberThing is
```
