#include "run_app.h"

namespace ds {
	int runApp( App *app ) {
		cinder::app::AppBasic::prepareLaunch();
		cinder::app::Renderer *ren = new RendererGl();
		cinder::app::AppBasic::executeLaunch( app, ren, "CinderApp" );
		cinder::app::AppBasic::cleanupLaunch();
		return 0;
	}

	int go() {
		cinder::app::AppBasic::prepareLaunch();
		cinder::app::AppBasic *app = new App();
		cinder::app::Renderer *ren = new RendererGl();
		cinder::app::AppBasic::executeLaunch( app, ren, "CinderApp" );
		cinder::app::AppBasic::cleanupLaunch();
		return 0;
	}

	namespace ui {

	ShaderSprite::ShaderSprite( SpriteEngine &engine,
			float w, float h, const std::string &filename,
			bool applyToChildren )
		: Sprite(engine, w, h)
	{
		setTransparent( false );
		if ( filename != "" )
			setBaseShader( "data/shaders/", filename, applyToChildren );
	}

	void ShaderSprite::drawLocalClient() {
		ci::gl::GlslProg &shaderBase = mSpriteShader.getShader();
		if (shaderBase) {
			shaderBase.bind();
			onBindShader( shaderBase );
		}
		Sprite::drawLocalClient();
	}

	void ShaderSprite::onBindShader( ci::gl::GlslProg &shader ) {
	}

	cinder::gl::GlslProg * ShaderSprite::getShader() {
		return &getBaseShader().getShader();
	}

	} // namespace ui
} //namespace ds

