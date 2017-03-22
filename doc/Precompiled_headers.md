## Adding precompiled headers to existing project

### Add stdafx.cpp/stdafx.h

* Add a stdafx.{cpp,h} to the src/app directory of the project

```c++
#include "stdafx.h"
```

* stdafx.h contains headers that are common / don't change often

```c++
#pragma once

// Cinder
#include <cinder/Cinder.h>
#include <cinder/Color.h>
#include <cinder/Easing.h>
#include <cinder/Function.h>
#include <cinder/Rand.h>
#include <cinder/Rect.h>
#include <cinder/TriMesh.h>
#include <cinder/Tween.h>
#include <cinder/Vector.h>
#include <cinder/Xml.h>
#include <cinder/app/App.h>
#include <cinder/gl/Vbo.h>

// Poco
#include <Poco/Condition.h>
#include <Poco/Foundation.h>
#include <Poco/Thread.h>

// ds_cinder
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/engine/engine_settings.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

// Std C++ Library
#include <functional>
#include <queue>
#include <regex>
#include <string>
#include <vector>

// Application specific
#include "app/globals.h"
#include "events/app_events.h"
```

### Project Configuration

The Visual Studio project file needs to be updated to use precompiled headers

* Each <ClCompile> group needs to be modified to include
```xml
<PrecompiledHeader>Use</PrecompiledHeader>
<AdditionalOptions>/Zm256 %(AdditionalOptions)</AdditionalOptions>
```

* stdafx.cpp needs to be added to the cpp `<ItemGroup>`
```xml
<ClCompile Include="..\src\app\stdafx.cpp">
  <PrecompiledHeader>Create</PrecompiledHeader>
</ClCompile>
```

* Precompiled headers should be disabled for generated model files
```xml
<ClCompile Include="..\src\model\generated\{NAME}_model.cpp" >
  <PrecompiledHeader>NotUsing</PrecompiledHeader>
</ClCompile>
```

### Updating Files

* All non-generated .cpp files need to include stdafx.h at the top


### Scripted Version (WIP)

* Running this script from the root of a project will do all of the project
    setup steps for using precompiled headers.

* Doesn't copy or create the stdafx header or source file

```bash
#!/usr/bin/bash
sed -i "s/<\/PrecompiledHeader>//g" vs2013/*.vcxproj
sed -i "s/<PrecompiledHeader>/<PrecompiledHeader>Use<\/PrecompiledHeader>\n<AdditionalOptions>\/Zm256 %(AdditionalOptions)<\/AdditionalOptions>/g" vs2013/*.vcxproj
sed -i "0,/<ItemGroup>/s/<ItemGroup>/<ItemGroup>\n    <ClCompile Include=\"..\\\src\\\app\\\stdafx.cpp\">\n      <PrecompiledHeader>Create<\/PrecompiledHeader>\n    <\/ClCompile>/" vs2013/*.vcxproj
sed -i "s/<ClInclude .*app_defs.h.*>/    <ClInclude Include=\"..\\\src\\\app\\\stdafx.h\" \/>\n    \0/" vs2013/*.vcxproj
sed -i "s/\(<ClCompile .*generated.*cpp\"\) \/>/\1>\n      <PrecompiledHeader>NotUsing<\/PrecompiledHeader>\n    <\/ClCompile>/g" vs2013/*.vcxproj
sed -i '1s/^/#include \"stdafx.h\"\n\n/' **/!(generated|stdafx)/*.cpp
```
