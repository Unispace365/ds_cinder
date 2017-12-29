
# Example Markdown stuff

This'll show what's supported in ds_cinder.  **Run the pango example and drop this on the window** 

To use markdown in your own app, you need to run the `ds::ui::markdown_to_pango` converter:

```c++
#include <ds/util/markdown_to_pango.h>

std::string theSourceText = "# Something with Markdown";
mTextSprite->setText(ds::ui::markdown_to_pango(theSourceText));
```

## Here's a second-level header

### And a third-level header

> **Block quote example**
>
> This shows things in a blockquote

* Bullet list
* Second bullet
  1. Indented numbered list
  2. Numbered list 2
  1. Numbered list 3
  19. The last numbered list
* **Bold text**
* *italic text*
* _**bold and italic**_
* Sometimes you want just a ^`little` bit of `code`

Superscript test for the 1^st time

~~This is a handy guide but it's been removed.~~

```
// code block
var theCode = new Code();
```



## Some stuff that's not supported

**Typographic replacements like copyright symbols**
Copyright Gordon (C)

**Tables**

The best thing to do would be wrap tables into code blocks like so:

```
| Table header |     second header | third header |
|--------------|:-----------------:|-------------:|
| column 1     | the middle center | right side   |
| this probly  | doesn't look      | correct      |
```

Otherwise they'd look like this:

| Table header |     second header | third header |
|--------------|:-----------------:|-------------:|
| column 1     | the middle center | right side   |
| this probly  | doesn't look      | correct      |

**Inline HTML**

<em>This will just show up as HTML markup</em>
<ol>This should be an ordered list, but it's just tags </ol>


**Horizontal rules**

These are just ignored

----------------------------------------

And nothing shows up

**Links**

Show the original link
[github](https://github.com)