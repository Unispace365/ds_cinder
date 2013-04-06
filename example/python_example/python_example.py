import sys, os
if "DS_PLATFORM" in os.environ:
    sys.path.append( os.path.join( os.environ["DS_PLATFORM"], "vc10/swig/lib") )

import ds_cinder_swig
from ds_cinder_swig import App, Sprite

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

	def add_rect( self, w, h ):
		sprite = Sprite( self.engine, w, h )
		sprite.setTransparent( False )
		sprite.enable( True )
		sprite.enableMultiTouch( ds_cinder_swig.MULTITOUCH_NO_CONSTRAINTS );
		self.sprites.append( sprite )
		self.rootSprite.addChild( sprite )
		return sprite
	
	#def draw( self ):
		#super().draw()
		#print( "OMG DRAWING!!!" )

def run():
	ds_cinder_swig.runApp( MyApp() )

if not sys.flags.interactive:
    run()
