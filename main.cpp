#include"raytracer.h"
int main() {
	Raytracer* raytracer = new Raytracer;
	raytracer->SetInput( "scene.txt" );
	raytracer->SetOutput( "picture.bmp" );
	raytracer->Run();
	delete raytracer;
	//system("Pause");
	return 0;
}
