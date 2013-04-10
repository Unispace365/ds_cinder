import sys, os

LIB_PATH = "vc10/swig/lib"
if "DS_PLATFORM" in os.environ:
		sys.path.append( os.path.join( os.environ["DS_PLATFORM"], LIB_PATH ) )
else:
		sys.path.append( os.path.join( "../..", LIB_PATH ) )

import ds_cinder_swig
from ds_cinder_swig import *

class MyApp(App):
	def __init__( self ):
		super().__init__( os.getcwd() )
	
	def setupServer( self ):
		self.enableCommonKeystrokes()
		self.engine = self.getSpriteEngine()
		self.rootSprite = self.getRootSprite()
		self.sprites = []

		self.add_rect( 300.0, 300.0 ).setColor( 0.1, 0.6, 0.8 )
		self.add_rect( 200.0, 200.0 ).setColor( 0.1, 0.5, 0.8 )
		self.add_rect( 100.0, 100.0 ).setColor( 0.1, 0.4, 0.8 )
		self.add_rect( 50.0, 50.0, self.sprites[0] ).setColor( 0.5, 0.1, 0.1 )

		cen = self.getWindowCenter()
		img = self.add_image( "data/images/kitty.jpg" )
		img.setCenter( 0.5, 0.5 )
		img.setPosition( cen.x, cen.y )
		#img.move( 100, 200 )
		self.rot = False

	def add_rect( self, w, h, parent=None ):
		sprite = Sprite( self.engine, w, h )
		sprite.setTransparent( False )
		sprite.enable( True )
		sprite.enableMultiTouch( ds_cinder_swig.MULTITOUCH_NO_CONSTRAINTS );
		self.sprites.append( sprite )
		print( "Added sprite: ", sprite )
		if parent is None:
			self.rootSprite.addChild( sprite )
		else:
			parent.addChild( sprite )
		return sprite

	def add_image( self, filename ):
		sprite = Image( self.engine, filename )
		#sprite.enable( True )
		#sprite.enableMultiTouch( ds_cinder_swig.MULTITOUCH_NO_CONSTRAINTS );
		self.sprites.append( sprite )
		self.rootSprite.addChild( sprite )
		return sprite

	def mouseDown( self, event ):
		super().mouseDown( event )
		pos = event.getPos()
		print( "OMG MOUSE DOWN: ", pos.x, pos.y )
		img = self.sprites[-1]
		#import ipdb; ipdb.set_trace()
		p = Vec3f( float(pos.x), float(pos.y), 0.0 )
		#img.tweenPositionTo( p, 1.0, EaseInCubic() )
		#img.tweenPositionTo( p, 0.4, EaseInCubic() )
		img.tweenPositionTo( p, 0.5, EaseInOutBack().func() )
		r = Vec3f( 0.0, (0.0 if self.rot else 180.0), 0.0 )
		self.rot = not self.rot
		#img.tweenRotationTo( r, 0.2 )
		img.tweenRotationTo( r, 0.5, EaseInOutBack().func() )
		#self.getEngine().getTweenline().apply(img, img.ANIM_POSITION(), p, 1.0, EaseInCubic() );
		#img.tween( tweenYpos, 80.0, EaseInCubic() )


def run():
	ds_cinder_swig.runApp( MyApp() )

if not sys.flags.interactive:
		run()
