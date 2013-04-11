#pragma once
#ifndef CINDER_AWESOMIUM_H
#define CINDER_AWESOMIUM_H

#include <Awesomium/WebCore.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/STLHelpers.h>

#include "cinder/Exception.h"
#include "cinder/Surface.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"

namespace ph { namespace awesomium {

// exceptions
class EmptyPointerException : public ci::Exception {
	virtual const char * what() const throw() { return "The specified pointer is empty."; }
};

class InvalidBufferException : public ci::Exception {
	virtual const char * what() const throw() { return "The specified buffer is empty or not initialized."; }
};

// A helper macro, used in 'getWebKeyFromKeyEvent'
#define mapKey(a, b) case ci::app::KeyEvent::KEY_##a: return Awesomium::KeyCodes::AK_##b;

// Translates a Cinder virtual key code to an Awesomium key code
int getWebKeyFromKeyEvent( ci::app::KeyEvent event );

// Conversion functions

ci::Surface toSurface( Awesomium::BitmapSurface* surface );

ci::Surface toSurface( Awesomium::WebView* webview );

ci::gl::Texture toTexture( Awesomium::BitmapSurface* surface, ci::gl::Texture::Format format=ci::gl::Texture::Format() );

ci::gl::Texture toTexture( Awesomium::WebView* webview, ci::gl::Texture::Format format=ci::gl::Texture::Format() );

bool isDirty( Awesomium::WebView* webview );

Awesomium::WebKeyboardEvent toKeyEvent( ci::app::KeyEvent event, Awesomium::WebKeyboardEvent::Type type );

Awesomium::WebKeyboardEvent toKeyChar( ci::app::KeyEvent event );

// Utility functions that take care of event handling

//! sends a Cinder KeyDown event to the WebView and handles Cut, Copy and Paste
void handleKeyDown( Awesomium::WebView *view, ci::app::KeyEvent event );

//! sends a Cinder KeyUp event to the WebView
void handleKeyUp( Awesomium::WebView *view, ci::app::KeyEvent event );

//! sends a Cinder MouseMove event to the WebView
void handleMouseMove( Awesomium::WebView *view, ci::app::MouseEvent event );

//! sends a Cinder MouseDown event to the WebView
void handleMouseDown( Awesomium::WebView *view, ci::app::MouseEvent event );

//! sends a Cinder MouseDrag event to the WebView
void handleMouseDrag( Awesomium::WebView *view, ci::app::MouseEvent event );

//! sends a Cinder MouseUp event to the WebView
void handleMouseUp( Awesomium::WebView *view, ci::app::MouseEvent event );

//! sends a Cinder MouseWheel event to the WebView
void handleMouseWheel( Awesomium::WebView *view, ci::app::MouseEvent event, int increment=150 );

} } // namespace ph::awesomium

#endif//CINDER_AWESOMIUM_H
