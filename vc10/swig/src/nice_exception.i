// Handle exceptions better ------------
%feature("director:except") {
	if ($error != NULL) {
		throw Swig::DirectorMethodException();
	}
}
%exception {
	try { $action }
	catch (Swig::DirectorException &e) { e.getMessage(); SWIG_fail; }
}
