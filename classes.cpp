//=======================================================================
// `INCLUDES`
//=======================================================================

#include "includes.h"
#include "classes.h"

//=======================================================================
// `STRUCT DEFINITIONS`
//=======================================================================

constexpr Vector2d::Vector2d() : x(0), y(0) {}
constexpr Vector2d::Vector2d(const double _x, const double _y) : x(_x), y(_y) {}

Vector2d Vector2d::operator + (Vector2d& v) {
	return { x + v.x,y + v.y };
}

Vector2d Vector2d::operator - (Vector2d& v) {
	return { x - v.x,y - v.y };
}

Vector2d Vector2d::operator * (const double n) {
	return { x * n, y * n };
}

Vector2d Vector2d::operator / (const double n) {
	return { x / n, y / n };
}

double Vector2d::dot(Vector2d& v) {
	return x * v.x + y * v.x;
}

Vector2d Vector2d::unit() {
	return *this / sqrt(this->dot(*this));
}

double Vector2d::magnitude() {
	return sqrt(this->dot(*this));
}

double Vector2d::angle(Vector2d& v) {
	return acos((this->dot(v)) / (this->magnitude() * v.magnitude()));
}


constexpr Vector3d::Vector3d() : x(0), y(0), z(0) {}
constexpr Vector3d::Vector3d(const double _x, const double _y, const double _z) : x(_x), y(_y), z(_z) {}

Vector3d Vector3d::operator + (Vector3d& v) {
	return { x + v.x,y + v.y,z + v.z };
}

Vector3d Vector3d::operator - (Vector3d& v) {
	return { x - v.x,y - v.y,z - v.z };
}

Vector3d Vector3d::operator * (const double n) {
	return { x * n, y * n, z * n };
}

Vector3d Vector3d::operator / (const double n) {
	return { x / n, y / n, z / n };
}

double Vector3d::dot(Vector3d& v) {
	return x * v.x + y * v.x + z * v.z;
}

Vector3d Vector3d::cross(Vector3d& v) {
	return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
}

Vector3d Vector3d::unit() {
	return *this / sqrt(this->dot(*this));
}

double Vector3d::magnitude() {
	return sqrt(this->dot(*this));
}

double Vector3d::angle(Vector3d& v) {
	return acos((this->dot(v)) / (this->magnitude() * v.magnitude()));
}



constexpr Vector4d::Vector4d() : x(0), y(0), z(0), w(0) {}
constexpr Vector4d::Vector4d(const double _x, const double _y, const double _z, const double _w) : x(_x), y(_y), z(_z), w(_w) {}

Vector4d Vector4d::operator + (Vector4d& v) {
	return { x + v.x,y + v.y,z + v.z,w + v.w };
}

Vector4d Vector4d::operator - (Vector4d& v) {
	return { x - v.x,y - v.y,z - v.z,w - v.w };
}

Vector4d Vector4d::operator * (const double n) {
	return { x * n, y * n, z * n, w * n };
}

Vector4d Vector4d::operator / (const double n) {
	return { x / n, y / n, z / n, w / n };
}

double Vector4d::dot(Vector4d& v) {
	return x * v.x + y * v.x + z * v.z + w * v.w;
}

Vector4d Vector4d::unit() {
	return *this / sqrt(this->dot(*this));
}

double Vector4d::magnitude() {
	return sqrt(this->dot(*this));
}

double Vector4d::angle(Vector4d& v) {
	return acos((this->dot(v)) / (this->magnitude() * v.magnitude()));
}


Camera::Camera() {
	this->position = Vector3d::Vector3d();
	this->target = Vector3d::Vector3d();
	this->up = Vector3d::Vector3d();
}

Camera::Camera(const Vector3d& _position, const Vector3d& _target, const Vector3d& _up) {
	this->position = _position;
	this->target = _target;
	this->up = _up;
}

void Camera::lookAt(const Vector3d& _target) {
	this->target = target;
}

void Camera::project() {
	gluLookAt(
		position.x, position.y, position.z,
		target.x, target.y, target.z,
		up.x, up.y, up.z
	);
}



GameObject::GameObject() : position(Vector3d()) {}

GameObject::GameObject(Vector3d _position) {
	this->position = _position;
}

GameObject::GameObject(Vector3d _position, string _object_type) {
	this->position = _position;
	this->object_type = _object_type;
}

void GameObject::render(Model_3DS& model) {
	glPushMatrix();
		glTranslated(this->position.x, this->position.y, this->position.z);
		glPushMatrix();
			if (this->object_type == "coin") {
				double rotation_value = (chrono::steady_clock::now().time_since_epoch().count() / 3000000)%360ll;
				glScaled(3.5, 3.5, 3.5);
				glRotated(rotation_value, 0, 1, 0);
			}
			else if (this->object_type == "subway") {
				glScaled(0.1, 0.1, 0.1);
				glRotated(180, 0, 1, 0);
			}
			else if(this->object_type == "barricade") {
				glScaled(0.7, 0.7, 1);
				glRotated(90, 0, 1, 0);
			}
			else if (this->object_type == "ship") {
				glScaled(3.5, 8, 5);
				glRotated(0, 0, 1, 0);
			}
			else if (this->object_type == "swim_ring") {
				double rotation_value = ((chrono::steady_clock::now().time_since_epoch().count()) / 40000000ll)  % 360ll;
				glScaled(3, 3, 3);
				glRotated(rotation_value, 0, 1, 0);
				glTranslated(0, -0.2, 0);
			}

			model.Draw();
		glPopMatrix();
	glPopMatrix();
}

