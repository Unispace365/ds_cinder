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

#include <cinder/CinderMath.h>
#include <cinder/Font.h>
#include <cinder/Perlin.h>
#include <cinder/app/AppBase.h>
#include <cinder/app/MouseEvent.h>
#include <cinder/app/Window.h>
#include <cinder/gl/Fbo.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/gl/TextureFont.h>
#include <cinder/gl/gl.h>
#include <cinder/params/Params.h>

// ds_cinder
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/engine/engine_settings.h>
#include <ds/app/environment.h>
#include <ds/app/event_client.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/circle.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>

// Poco
#include <Poco/Condition.h>
#include <Poco/Foundation.h>
#include <Poco/Thread.h>

// Std C++ Library
#include <functional>
#include <queue>
#include <regex>
#include <string>
#include <vector>
