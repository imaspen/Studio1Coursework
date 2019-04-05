/*---------------------------------------------------------------------
*
* Copyright © 2018  Minsi Chen
* E-mail: m.chen@hud.ac.uk
*
* The source is written for the CSwGP related modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#if defined(WIN32) || defined(_WINDOWS)
#include <Windows.h>
#include <gl/GL.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#include "RayTracer.h"
#include "Ray.h"
#include "Scene.h"
#include "Camera.h"

RayTracer::RayTracer()
{
	m_buffHeight = m_buffWidth = 0.0;
	m_renderCount = 0;
	SetTraceLevel(5);
	m_traceflag = (TraceFlags)(TRACE_AMBIENT | TRACE_DIFFUSE_AND_SPEC |
		TRACE_SHADOW | TRACE_REFLECTION | TRACE_REFRACTION);

}

RayTracer::RayTracer(int Width, int Height)
{
	m_buffWidth = Width;
	m_buffHeight = Height;
	m_renderCount = 0;
	SetTraceLevel(5);

	m_framebuffer = new Framebuffer(Width, Height);

	//default set default trace flag, i.e. no lighting, non-recursive
	m_traceflag = (TraceFlags)(TRACE_AMBIENT);
}

RayTracer::~RayTracer()
{
	delete m_framebuffer;
}

void RayTracer::DoRayTrace( Scene* pScene )
{
	Camera* cam = pScene->GetSceneCamera();
	
	Vector3 camRightVector = cam->GetRightVector();
	Vector3 camUpVector = cam->GetUpVector();
	Vector3 camViewVector = cam->GetViewVector();
	Vector3 centre = cam->GetViewCentre();
	Vector3 camPosition = cam->GetPosition();

	double sceneWidth = pScene->GetSceneWidth();
	double sceneHeight = pScene->GetSceneHeight();

	double pixelDX = sceneWidth / m_buffWidth;
	double pixelDY = sceneHeight / m_buffHeight;
	
	int total = m_buffHeight*m_buffWidth;
	int done_count = 0;
	
	Vector3 start;

	start[0] = centre[0] - ((sceneWidth * camRightVector[0])
		+ (sceneHeight * camUpVector[0])) / 2.0;
	start[1] = centre[1] - ((sceneWidth * camRightVector[1])
		+ (sceneHeight * camUpVector[1])) / 2.0;
	start[2] = centre[2] - ((sceneWidth * camRightVector[2])
		+ (sceneHeight * camUpVector[2])) / 2.0;
	
	Colour scenebg = pScene->GetBackgroundColour();

	if (m_renderCount == 0)
	{
		fprintf(stdout, "Trace start.\n");

		Colour colour;
//TinyRay on multiprocessors using OpenMP!!!
#pragma omp parallel for schedule (dynamic, 1) private(colour)
		for (int i = 0; i < m_buffHeight; i+=1) {
			for (int j = 0; j < m_buffWidth; j+=1) {

				//calculate the metric size of a pixel in the view plane (e.g. framebuffer)
				Vector3 pixel;

				pixel[0] = start[0] + (i + 0.5) * camUpVector[0] * pixelDY
					+ (j + 0.5) * camRightVector[0] * pixelDX;
				pixel[1] = start[1] + (i + 0.5) * camUpVector[1] * pixelDY
					+ (j + 0.5) * camRightVector[1] * pixelDX;
				pixel[2] = start[2] + (i + 0.5) * camUpVector[2] * pixelDY
					+ (j + 0.5) * camRightVector[2] * pixelDX;

				/*
				* setup first generation view ray
				* In perspective projection, each view ray originates from the eye (camera) position 
				* and pierces through a pixel in the view plane
				*/
				Ray viewray;
				viewray.SetRay(camPosition,	(pixel - camPosition).Normalise());
				
				double u = (double)j / (double)m_buffWidth;
				double v = (double)i / (double)m_buffHeight;

				scenebg = pScene->GetBackgroundColour();

				//trace the scene using the view ray
				//default colour is the background colour, unless something is hit along the way
				colour = this->TraceScene(pScene, viewray, scenebg, m_traceLevel);

				/*
				* Draw the pixel as a coloured rectangle
				*/
				m_framebuffer->WriteRGBToFramebuffer(colour, j, i);
			}
		}

		fprintf(stdout, "Done!!!\n");
		m_renderCount++;
	}
}

Colour RayTracer::TraceScene(Scene* pScene, Ray& ray, Colour incolour, int tracelevel, bool shadowray)
{
	RayHitResult result;

	Colour outcolour = incolour; //the output colour based on the ray-primitive intersection
	
	//obtain the list of light sources from the scene
	std::vector<Light*> *light_list = pScene->GetLightList();
	
	if (tracelevel <= 0)
	{
		return outcolour;
	}
	
	//Intersect the ray with the scene
	//TODO: Scene::IntersectByRay method needs to be implemented first
	result = pScene->IntersectByRay(ray);

	if (result.data) //the ray has hit something
	{
		//The origin of the ray currently being traced
		Vector3 start = ray.GetRayStart();
		
		//Determine surface colour from lights
		outcolour = CalculateLighting(light_list,
			&start,
			&result);

		if (m_traceflag & TRACE_REFLECTION)
		{
			if (((Primitive*)result.data)->m_primtype == Primitive::PRIMTYPE_Sphere ||
				((Primitive*)result.data)->m_primtype == Primitive::PRIMTYPE_Box)
			{
				//TODO: Adventurous Requirement
				//Trace the reflection on Spheres and Boxes
			}
		}

		if (m_traceflag & TRACE_REFRACTION)
		{
			if (((Primitive*)result.data)->m_primtype == Primitive::PRIMTYPE_Sphere ||
				((Primitive*)result.data)->m_primtype == Primitive::PRIMTYPE_Box)
			{
				//TODO: Adventurous Requirement
				//Trace the refraction on Spheres and Boxes
			}
		}

		if (m_traceflag & TRACE_SHADOW)
		{
			for (auto light_iter = light_list->begin(); light_iter != light_list->end(); ++light_iter)
			{
				//Trace the shadow ray
				Ray *shadowRay = new Ray();
				Vector3 start = result.point;
				Vector3 end = (*light_iter)->GetLightPosition();
				Vector3 direction = start - end;
				direction.Normalise();
				shadowRay->SetRay(end, direction);
				RayHitResult shadowRayHitResult = pScene->IntersectByRay(*shadowRay);
				if ((((Primitive*)shadowRayHitResult.data)->m_primtype == Primitive::PRIMTYPE_Sphere ||
				     ((Primitive*)shadowRayHitResult.data)->m_primtype == Primitive::PRIMTYPE_Box)
				     && shadowRayHitResult.data != result.data)
				{
					outcolour = outcolour * Colour(0.25, 0.25, 0.25);
				}
			}
		}
	}

	return outcolour;
}

Colour RayTracer::CalculateLighting(std::vector<Light*>* lights, Vector3* campos, RayHitResult* hitresult)
{
	Colour outcolour;
	std::vector<Light*>::iterator lit_iter = lights->begin();

	Primitive* prim = (Primitive*)hitresult->data;
	Material* mat = prim->GetMaterial();

	outcolour = mat->GetAmbientColour();
	
	//Generate the grid pattern on the plane
	if (((Primitive*)hitresult->data)->m_primtype == Primitive::PRIMTYPE_Plane)
	{
		Vector3 intersection = hitresult->point;

		int dx = (int)(fabsf(intersection[0]*0.5f)+0.5f);
		int dy = (int)(fabsf(intersection[1]*0.5f)+0.5f);
		int dz = (int)(fabsf(intersection[2]*0.5f)+0.5f);

		//If the intersection point is inside an "even" cell
		//Set the return colour to light grey [0.1,0.1,0.1]
		//Otherwise set the return outcolour to what material's diffuse colour.
		if (dx % 2 || dy % 2 || dz % 2 )
		{
			outcolour = Vector3(0.1f, 0.1f, 0.1f);
		}
		else
		{
			outcolour = mat->GetDiffuseColour();
		}

	}
	
	////Go through all lights in the scene
	////Note the default scene only has one light source
	if (m_traceflag & TRACE_DIFFUSE_AND_SPEC)
	{
		//TODO: Adventurous Requirement
		//Implement basic lighting normal
		//Calculate and apply the lighting of the intersected object using the illumination model covered in the lecture
		//i.e. diffuse using Lambertian model, for specular, you can use either Phong or Blinn-Phong model
		while (lit_iter != lights->end())
		{
			if (((Primitive*)hitresult->data)->m_primtype == Primitive::PRIMTYPE_Box
			 || ((Primitive*)hitresult->data)->m_primtype == Primitive::PRIMTYPE_Sphere)
                        {
				Vector3 light_vector = (*lit_iter)->GetLightPosition() - hitresult->point;
				light_vector.Normalise();
				Vector3 normal = hitresult->normal;
				Colour light_color = (*lit_iter)->GetLightColour();

				//Diffuse Reflection
				float diffuse_intensity = light_vector.DotProduct(normal);
				Vector3 diffuse_color = (mat->GetDiffuseColour() * light_color) * diffuse_intensity;

				//Specular Reflection
				Vector3 cam_vector = *campos - hitresult->point;
				cam_vector.Normalise();
				Vector3 half_vector = light_vector + cam_vector;
				half_vector = half_vector * (1/half_vector.Norm());
				half_vector.Normalise();
				float spec_angle = normal.DotProduct(half_vector);
				float spec_intensity = std::pow(spec_angle, mat->GetSpecPower() * 5);
				Vector3 specular_color = mat->GetSpecularColour() * light_color * spec_intensity;

				outcolour = outcolour + diffuse_color + specular_color;
			}
			lit_iter++;
		}
	}

	return outcolour;
}

