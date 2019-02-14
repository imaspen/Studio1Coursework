/*---------------------------------------------------------------------
*
* Copyright © 2018  Minsi Chen
* E-mail: m.chen@hud.ac.uk
*
* The source is written for CSwGP modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#pragma once


//This is an optmised Vector3 class implemented using the SSE 128 intrinsics.

#include <immintrin.h>

typedef __m128 Vec4;

class Vector3
{
private:
	Vec4	mVector;

public:
	Vector3();
	
	Vector3(const Vector3& rhs);

	Vector3(float x, float y, float z);

	Vector3(__m128 &vector);
	
	~Vector3() {;}

	float operator [] (const int i) const;
	float& operator [] (const int i);
	Vector3 operator + (const Vector3& rhs) const;	//overloaded operator for computing vector addition
	Vector3 operator - (const Vector3& rhs) const;	//overloaded operator for computing vector subtraction
	Vector3 operator * (const Vector3& rhs) const;	//overloaded operator for computing element-wise multiplication e.g. (x1*x2, y1*y2, z1*z2)
	Vector3 operator * (float scale) const; //overloaded operator for computing vector multiplied by a scalar
	Vector3 operator = (const Vector3& rhs); //overloaded assignment operator.

	float Norm()	const;
	float Norm_Sqr() const;
	Vector3 Normalise();

	float DotProduct(const Vector3& rhs) const;		//method for computing dot product between this and rhs
	Vector3 CrossProduct(const Vector3& rhs) const; //method for computing cross product between this and rhs
	
	Vector3 Reflect(const Vector3& n) const; //Reflect this vector about the given normal n
	Vector3 Refract(const Vector3& n, float r_index) const; //Compute the refraction of this vector about the given normal n and refraction index

	void SetZero();
	
	inline void SetVector(float x, float y, float z)
	{ 
		mVector = _mm_set_ps(0.0f, z, y, x);
	}
};
