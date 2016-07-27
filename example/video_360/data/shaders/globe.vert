////////////////////////////////////////////////////////////////////////
// Vertex shader inputs (You will also need to set tex0 for frag shader)
////////////////////////////////////////////////////////////////////////

// ViewModel and Projection
// NOTE: order matters!
// Example for mv: ci::MayaCamUI::getCamera::getModelViewMatrix()
// Example for p:  ci::MayaCamUI::getCamera::getProjectionMatrix()
uniform mat4 mv,p;
// globe / sphere radius (passed to ci::gl::drawSphere for example)
uniform float radius;

////////////////////////////////////////////////////////////////////////
// Vertex shader outputs
////////////////////////////////////////////////////////////////////////

// calculated here and passed to fragment shader
varying vec4 texCoords;

////////////////////////////////////////////////////////////////////////
// Vertex shader main() vertex to world mapping
////////////////////////////////////////////////////////////////////////
 
void main(void) {
    // build the model view projection
	mat4 mvp = p*mv;
	// get the current world position
    gl_Position = mvp * gl_Vertex;
	// figure out where in the texture we are standing
    texCoords = gl_Vertex * (1.0 / radius);
}