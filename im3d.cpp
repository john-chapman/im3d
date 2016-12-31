#include "im3d.h"

using namespace Im3d;

Mat4::Mat4(float _diagonal)
{
	m[ 0] = _diagonal; m[ 1] = 0.0f;      m[ 2] = 0.0f;      m[ 3] = 0.0f;
	m[ 4] = 0.0f;      m[ 5] = _diagonal; m[ 6] = 0.0f;      m[ 7] = 0.0f;
	m[ 8] = 0.0f;      m[ 9] = 0.0f;      m[10] = _diagonal; m[11] = 0.0f;
	m[12] = 0.0f;      m[13] = 0.0f;      m[14] = 0.0f;      m[15] = _diagonal;
}

Mat4::Mat4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
	)
{
	m[ 0] = m00; m[ 1] = m01; m[ 2] = m02; m[ 3] = m03;
	m[ 4] = m10; m[ 5] = m11; m[ 6] = m12; m[ 7] = m13;
	m[ 8] = m20; m[ 9] = m21; m[10] = m22; m[11] = m23;
	m[12] = m30; m[13] = m31; m[14] = m32; m[15] = m33;
}

Color::Color(const Vec4& _rgba)
{
	v  = (U32)(_rgba.x * 255.0f) << 24;
	v |= (U32)(_rgba.y * 255.0f) << 16;
	v |= (U32)(_rgba.z * 255.0f) << 8;
	v |= (U32)(_rgba.w * 255.0f);
}
Color::Color(float _r, float _g, float _b, float _a)
{
	v  = (U32)(_r * 255.0f) << 24;
	v |= (U32)(_g * 255.0f) << 16;
	v |= (U32)(_b * 255.0f) << 8;
	v |= (U32)(_a * 255.0f);
}
