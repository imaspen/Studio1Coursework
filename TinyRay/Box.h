/*---------------------------------------------------------------------
*
* Copyright � 2018  Minsi Chen
* E-mail: m.chen@hud.ac.uk
*
* The source is written for the CSwGP related modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#pragma	once
#include "Primitive.h"
#include "Vector3.h"
#include "Triangle.h"

//Class represents an axis-aligned box

class Box : public Primitive
{
	private:
		Triangle m_triangles[12]; //12 triangles forming the 6 faces of the box

	public:
		Box();
		Box(Vector3 position, double width, double height, double depth);
		~Box();

		void SetBox(Vector3 position, double width, double height, double depth);

		RayHitResult IntersectByRay(Ray& ray);

};

