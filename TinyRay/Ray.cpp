/*---------------------------------------------------------------------
*
* Copyright © 2018  Minsi Chen
* E-mail: m.chen@hud.ac.uk
*
* The source is written for the CSwGP related modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include "Ray.h"

RayHitResult Ray::s_defaultHitResult;

Ray::Ray()
{
	s_defaultHitResult.data = nullptr;
	s_defaultHitResult.t = FARFAR_AWAY;
}


Ray::~Ray()
{
}
