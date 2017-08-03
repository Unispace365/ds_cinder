#include "stdafx.h"

#include "editor_item.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"

#include <cinder/TriMesh.h>
#include <cinder/Triangulate.h>

#include "ds/ui/sprite/circle.h"

namespace ds{
namespace cfg{

EditorItem::EditorItem(ds::ui::SpriteEngine& e)
	: ds::ui::Sprite(e)
	, mTheSetting(nullptr)
{
}


void EditorItem::setSetting(Settings::Setting* theSettings){

}

}
}
