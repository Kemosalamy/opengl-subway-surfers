#ifndef CLASSES_H
#define CLASSES_H

//=======================================================================
// `INCLUDES`
//=======================================================================

#include "includes.h"

//=======================================================================
// `STRUCT DECLARATIONS`
//=======================================================================

struct Vector2d {
	double x, y;

	constexpr Vector2d();
	constexpr Vector2d(const double _x, const double _y);

	Vector2d operator + (Vector2d& v);
	Vector2d operator - (Vector2d& v);
	Vector2d operator * (const double n);
	Vector2d operator / (const double n);

	double dot(Vector2d& v);

	Vector2d unit();
	double magnitude();
	double angle(Vector2d& v);

};

struct Vector3d {
	double x, y, z;

	constexpr Vector3d();
	constexpr Vector3d(const double _x, const double _y, const double _z);

	Vector3d operator + (Vector3d& v);
	Vector3d operator - (Vector3d& v);
	Vector3d operator * (const double n);
	Vector3d operator / (const double n);

	double dot(Vector3d& v);
	Vector3d cross(Vector3d& v);

	Vector3d unit();
	double magnitude();
	double angle(Vector3d& v);

};

struct Vector4d {
	double x, y, z, w;

	constexpr Vector4d();
	constexpr Vector4d(const double _x, const double _y, const double _z, const double _w);

	Vector4d operator + (Vector4d& v);
	Vector4d operator - (Vector4d& v);
	Vector4d operator * (const double n);
	Vector4d operator / (const double n);

	double dot(Vector4d& v);
	
	Vector4d unit();
	double magnitude();
	double angle(Vector4d& v);

};

struct Camera {
	Vector3d position, target, up;

	Camera();
	Camera(const Vector3d& _position, const Vector3d& _target, const Vector3d& _up);
	
	void lookAt(const Vector3d& _target);
	void project();

};


struct GameObject {
	Vector3d position;
	string object_type;

	GameObject();
	GameObject(Vector3d _position);
	GameObject(Vector3d _position, string _object_type);

	void render(Model_3DS& model);
};

#endif

