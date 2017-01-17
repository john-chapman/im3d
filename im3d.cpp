#include "im3d.h"
#include "im3d_math.h"

#include <cstdlib>
#include <cstring>
#include <cfloat>

#if defined(IM3D_MALLOC) && !defined(IM3D_FREE)
		#error im3d: IM3D_MALLOC defined without IM3D_FREE; define both or neither
#endif
#if defined(IM3D_FREE) && !defined(IM3D_MALLOC)
	#error im3d: IM3D_FREE defined without IM3D_MALLOC; define both or neither
#endif
#ifndef IM3D_MALLOC
	#define IM3D_MALLOC(size) malloc(size)
#endif
#ifndef IM3D_FREE
	#define IM3D_FREE(ptr) free(ptr)
#endif

// Compiler
#if defined(__GNUC__)
	#define IM3D_COMPILER_GNU
#elif defined(_MSC_VER)
	#define IM3D_COMPILER_MSVC
#else
	#error im3d: Compiler not defined
#endif

// Platform 
#if defined(_WIN32) || defined(_WIN64)
	#define IM3D_PLATFORM_WIN
#else
	#error im3d: Platform not defined
#endif

#if defined(IM3D_COMPILER_GNU)
	#define if_likely(e)   if ( __builtin_expect(!!(e), 1) )
	#define if_unlikely(e) if ( __builtin_expect(!!(e), 0) )
//#elif defined(IM3D_COMPILER_MSVC)
  // not defined for MSVC
#else
	#define if_likely(e)   if(!!(e))
	#define if_unlikely(e) if(!!(e))
#endif



using namespace Im3d;

const Id    Im3d::Id_Invalid    = 0;
const Color Im3d::Color_Black   = Color(0.0f, 0.0f, 0.0f);
const Color Im3d::Color_White   = Color(1.0f, 1.0f, 1.0f);
const Color Im3d::Color_Red     = Color(1.0f, 0.0f, 0.0f);
const Color Im3d::Color_Green   = Color(0.0f, 1.0f, 0.0f);
const Color Im3d::Color_Blue    = Color(0.0f, 0.0f, 1.0f);
const Color Im3d::Color_Magenta = Color(1.0f, 0.0f, 1.0f);
const Color Im3d::Color_Yellow  = Color(1.0f, 1.0f, 0.0f);
const Color Im3d::Color_Cyan    = Color(0.0f, 1.0f, 1.0f);

static const Color Color_GizmoHighlight = Color(1.0f, 0.78f, 0.27f);

void Im3d::MulMatrix(const Mat4& _mat)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * _mat);
}

Vec3::Vec3(const Vec4& _v)
	: x(_v.x)
	, y(_v.y)
	, z(_v.z)
{
}

Vec4::Vec4(Color _rgba)
	: x(_rgba.getR())
	, y(_rgba.getG())
	, z(_rgba.getB())
	, w(_rgba.getA())
{
}

Mat4::Mat4(float _diagonal)
{
	(*this)(0, 0) = _diagonal; (*this)(0, 1) = 0.0f;      (*this)(0, 2) = 0.0f;      (*this)(0, 3) = 0.0f;
	(*this)(1, 0) = 0.0f;      (*this)(1, 1) = _diagonal; (*this)(1, 2) = 0.0f;      (*this)(1, 3) = 0.0f;
	(*this)(2, 0) = 0.0f;      (*this)(2, 1) = 0.0f;      (*this)(2, 2) = _diagonal; (*this)(2, 3) = 0.0f;
	(*this)(3, 0) = 0.0f;      (*this)(3, 1) = 0.0f;      (*this)(3, 2) = 0.0f;      (*this)(3, 3) = _diagonal;
}
Mat4::Mat4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
	)
{
	(*this)(0, 0) = m00; (*this)(0, 1) = m01; (*this)(0, 2) = m02; (*this)(0, 3) = m03;
	(*this)(1, 0) = m10; (*this)(1, 1) = m11; (*this)(1, 2) = m12; (*this)(1, 3) = m13;
	(*this)(2, 0) = m20; (*this)(2, 1) = m21; (*this)(2, 2) = m22; (*this)(2, 3) = m23;
	(*this)(3, 0) = m30; (*this)(3, 1) = m31; (*this)(3, 2) = m32; (*this)(3, 3) = m33;
}
Vec4 Mat4::getCol(int _i) const
{
	return Vec4((*this)(0, _i), (*this)(1, _i), (*this)(2, _i), (*this)(3, _i));
}
Vec4 Mat4::getRow(int _i) const
{
	return Vec4((*this)(_i, 0), (*this)(_i, 1), (*this)(_i, 2), (*this)(_i, 3));
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

Im3d::Id Im3d::MakeId(const char* _str)
{
	static const U32 kFnv1aPrime32 = 0x01000193u;

	IM3D_ASSERT(_str);
	U32 ret = (U32)GetContext().getId(); // top of Id stack
	while (*_str) {
		ret ^= (U32)*_str++;
		ret *= kFnv1aPrime32;
	}
	return (Id)ret;
}

// declared in im3d_math.h
static const float kPi     = 3.14159265359f;
static const float kTwoPi  = 2.0f * kPi;
static const float kHalfPi = 0.5f * kPi;

static inline bool Equal(const Vec3& _a, const Vec3& _b)
{
	if (!(fabs(_a.x - _b.x) < FLT_EPSILON)) return false;
	if (!(fabs(_a.y - _b.y) < FLT_EPSILON)) return false;
	if (!(fabs(_a.z - _b.z) < FLT_EPSILON)) return false;
	return true;
}
static inline float Determinant(const Mat4& _m)
{
	return 
		_m(0, 3) * _m(1, 2) * _m(2, 1) * _m(3, 0) - _m(0, 2) * _m(1, 3) * _m(2, 1) * _m(3, 0) - _m(0, 3) * _m(1, 1) * _m(2, 2) * _m(3, 0) + _m(0, 1) * _m(1, 3) * _m(2, 2) * _m(3, 0) +
		_m(0, 2) * _m(1, 1) * _m(2, 3) * _m(3, 0) - _m(0, 1) * _m(1, 2) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(1, 2) * _m(2, 0) * _m(3, 1) + _m(0, 2) * _m(1, 3) * _m(2, 0) * _m(3, 1) +
		_m(0, 3) * _m(1, 0) * _m(2, 2) * _m(3, 1) - _m(0, 0) * _m(1, 3) * _m(2, 2) * _m(3, 1) - _m(0, 2) * _m(1, 0) * _m(2, 3) * _m(3, 1) + _m(0, 0) * _m(1, 2) * _m(2, 3) * _m(3, 1) +
		_m(0, 3) * _m(1, 1) * _m(2, 0) * _m(3, 2) - _m(0, 1) * _m(1, 3) * _m(2, 0) * _m(3, 2) - _m(0, 3) * _m(1, 0) * _m(2, 1) * _m(3, 2) + _m(0, 0) * _m(1, 3) * _m(2, 1) * _m(3, 2) +
		_m(0, 1) * _m(1, 0) * _m(2, 3) * _m(3, 2) - _m(0, 0) * _m(1, 1) * _m(2, 3) * _m(3, 2) - _m(0, 2) * _m(1, 1) * _m(2, 0) * _m(3, 3) + _m(0, 1) * _m(1, 2) * _m(2, 0) * _m(3, 3) +
		_m(0, 2) * _m(1, 0) * _m(2, 1) * _m(3, 3) - _m(0, 0) * _m(1, 2) * _m(2, 1) * _m(3, 3) - _m(0, 1) * _m(1, 0) * _m(2, 2) * _m(3, 3) + _m(0, 0) * _m(1, 1) * _m(2, 2) * _m(3, 3)
		;
}
Mat4 Im3d::Inverse(const Mat4& _m)
{
	Mat4 ret;
	ret(0, 0) = _m(1, 2) * _m(2, 3) * _m(3, 1) - _m(1, 3) * _m(2, 2) * _m(3, 1) + _m(1, 3) * _m(2, 1) * _m(3, 2) - _m(1, 1) * _m(2, 3) * _m(3, 2) - _m(1, 2) * _m(2, 1) * _m(3, 3) + _m(1, 1) * _m(2, 2) * _m(3, 3);
	ret(0, 1) = _m(0, 3) * _m(2, 2) * _m(3, 1) - _m(0, 2) * _m(2, 3) * _m(3, 1) - _m(0, 3) * _m(2, 1) * _m(3, 2) + _m(0, 1) * _m(2, 3) * _m(3, 2) + _m(0, 2) * _m(2, 1) * _m(3, 3) - _m(0, 1) * _m(2, 2) * _m(3, 3);
	ret(0, 2) = _m(0, 2) * _m(1, 3) * _m(3, 1) - _m(0, 3) * _m(1, 2) * _m(3, 1) + _m(0, 3) * _m(1, 1) * _m(3, 2) - _m(0, 1) * _m(1, 3) * _m(3, 2) - _m(0, 2) * _m(1, 1) * _m(3, 3) + _m(0, 1) * _m(1, 2) * _m(3, 3);
	ret(0, 3) = _m(0, 3) * _m(1, 2) * _m(2, 1) - _m(0, 2) * _m(1, 3) * _m(2, 1) - _m(0, 3) * _m(1, 1) * _m(2, 2) + _m(0, 1) * _m(1, 3) * _m(2, 2) + _m(0, 2) * _m(1, 1) * _m(2, 3) - _m(0, 1) * _m(1, 2) * _m(2, 3);
	ret(1, 0) = _m(1, 3) * _m(2, 2) * _m(3, 0) - _m(1, 2) * _m(2, 3) * _m(3, 0) - _m(1, 3) * _m(2, 0) * _m(3, 2) + _m(1, 0) * _m(2, 3) * _m(3, 2) + _m(1, 2) * _m(2, 0) * _m(3, 3) - _m(1, 0) * _m(2, 2) * _m(3, 3);
	ret(1, 1) = _m(0, 2) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(2, 2) * _m(3, 0) + _m(0, 3) * _m(2, 0) * _m(3, 2) - _m(0, 0) * _m(2, 3) * _m(3, 2) - _m(0, 2) * _m(2, 0) * _m(3, 3) + _m(0, 0) * _m(2, 2) * _m(3, 3);
	ret(1, 2) = _m(0, 3) * _m(1, 2) * _m(3, 0) - _m(0, 2) * _m(1, 3) * _m(3, 0) - _m(0, 3) * _m(1, 0) * _m(3, 2) + _m(0, 0) * _m(1, 3) * _m(3, 2) + _m(0, 2) * _m(1, 0) * _m(3, 3) - _m(0, 0) * _m(1, 2) * _m(3, 3);
	ret(1, 3) = _m(0, 2) * _m(1, 3) * _m(2, 0) - _m(0, 3) * _m(1, 2) * _m(2, 0) + _m(0, 3) * _m(1, 0) * _m(2, 2) - _m(0, 0) * _m(1, 3) * _m(2, 2) - _m(0, 2) * _m(1, 0) * _m(2, 3) + _m(0, 0) * _m(1, 2) * _m(2, 3);
	ret(2, 0) = _m(1, 1) * _m(2, 3) * _m(3, 0) - _m(1, 3) * _m(2, 1) * _m(3, 0) + _m(1, 3) * _m(2, 0) * _m(3, 1) - _m(1, 0) * _m(2, 3) * _m(3, 1) - _m(1, 1) * _m(2, 0) * _m(3, 3) + _m(1, 0) * _m(2, 1) * _m(3, 3);
	ret(2, 1) = _m(0, 3) * _m(2, 1) * _m(3, 0) - _m(0, 1) * _m(2, 3) * _m(3, 0) - _m(0, 3) * _m(2, 0) * _m(3, 1) + _m(0, 0) * _m(2, 3) * _m(3, 1) + _m(0, 1) * _m(2, 0) * _m(3, 3) - _m(0, 0) * _m(2, 1) * _m(3, 3);
	ret(2, 2) = _m(0, 1) * _m(1, 3) * _m(3, 0) - _m(0, 3) * _m(1, 1) * _m(3, 0) + _m(0, 3) * _m(1, 0) * _m(3, 1) - _m(0, 0) * _m(1, 3) * _m(3, 1) - _m(0, 1) * _m(1, 0) * _m(3, 3) + _m(0, 0) * _m(1, 1) * _m(3, 3);
	ret(2, 3) = _m(0, 3) * _m(1, 1) * _m(2, 0) - _m(0, 1) * _m(1, 3) * _m(2, 0) - _m(0, 3) * _m(1, 0) * _m(2, 1) + _m(0, 0) * _m(1, 3) * _m(2, 1) + _m(0, 1) * _m(1, 0) * _m(2, 3) - _m(0, 0) * _m(1, 1) * _m(2, 3);
	ret(3, 0) = _m(1, 2) * _m(2, 1) * _m(3, 0) - _m(1, 1) * _m(2, 2) * _m(3, 0) - _m(1, 2) * _m(2, 0) * _m(3, 1) + _m(1, 0) * _m(2, 2) * _m(3, 1) + _m(1, 1) * _m(2, 0) * _m(3, 2) - _m(1, 0) * _m(2, 1) * _m(3, 2);
	ret(3, 1) = _m(0, 1) * _m(2, 2) * _m(3, 0) - _m(0, 2) * _m(2, 1) * _m(3, 0) + _m(0, 2) * _m(2, 0) * _m(3, 1) - _m(0, 0) * _m(2, 2) * _m(3, 1) - _m(0, 1) * _m(2, 0) * _m(3, 2) + _m(0, 0) * _m(2, 1) * _m(3, 2);
	ret(3, 2) = _m(0, 2) * _m(1, 1) * _m(3, 0) - _m(0, 1) * _m(1, 2) * _m(3, 0) - _m(0, 2) * _m(1, 0) * _m(3, 1) + _m(0, 0) * _m(1, 2) * _m(3, 1) + _m(0, 1) * _m(1, 0) * _m(3, 2) - _m(0, 0) * _m(1, 1) * _m(3, 2);
	ret(3, 3) = _m(0, 1) * _m(1, 2) * _m(2, 0) - _m(0, 2) * _m(1, 1) * _m(2, 0) + _m(0, 2) * _m(1, 0) * _m(2, 1) - _m(0, 0) * _m(1, 2) * _m(2, 1) - _m(0, 1) * _m(1, 0) * _m(2, 2) + _m(0, 0) * _m(1, 1) * _m(2, 2);

	float det = 1.0f / Determinant(_m);
	for (int i = 0; i < 16; ++i) {
		ret[i] *= det;
	}
	return ret;
}
Mat4 Im3d::Transpose(const Mat4& _m)
{
	return Mat4(
		_m(0, 0), _m(1, 0), _m(2, 0), _m(3, 0),
		_m(0, 1), _m(1, 1), _m(2, 1), _m(3, 1),
		_m(0, 2), _m(1, 2), _m(2, 2), _m(3, 2),
		_m(0, 3), _m(1, 3), _m(2, 3), _m(3, 3)
		);
}
Mat4 Im3d::Translate(const Mat4& _m, const Vec3& _t)
{
	return _m * Mat4(
		1.0f, 0.0f, 0.0f, _t.x,
		0.0f, 1.0f, 0.0f, _t.y,
		0.0f, 0.0f, 1.0f, _t.z,
		0.0f, 0.0f, 0.0f, 1.0f
		);
}
Mat4 Im3d::Rotate(const Mat4& _m, const Vec3& _axis, float _rads)
{
	float c  = cosf(_rads);
	float rc = 1.0f - c;
	float s  = sinf(_rads);
	return _m * Mat4(
		_axis.x * _axis.x + (1.0f - _axis.x * _axis.x) * c, _axis.x * _axis.y * rc - _axis.z * s,                _axis.x * _axis.z * rc + _axis.y * s,                0.0f,
		_axis.x * _axis.y * rc + _axis.z * s,               _axis.y * _axis.y + (1.0f - _axis.y * _axis.y) * c,  _axis.y * _axis.z * rc - _axis.x * s,                0.0f,
		_axis.x * _axis.z * rc - _axis.y * s,               _axis.y * _axis.z * rc + _axis.x * s,                _axis.z * _axis.z + (1.0f - _axis.z * _axis.z) * c,  0.0f,
		0.0f,                                               0.0f,                                                0.0f,                                                1.0f
		);
}
Mat4 Im3d::LookAt(const Vec3& _from, const Vec3& _to, const Vec3& _up)
{
	Vec3 z = Normalize(_to - _from);
	Vec3 x, y;
	if_unlikely (Equal(z, _up) || Equal(z, -_up)) { // prevent degenerate where z aligns with _up
		Vec3 k = _up + Vec3(FLT_EPSILON);
		y = Normalize(k - z * Dot(k, z));
	} else {
		y = Normalize(_up - z * Dot(_up, z));
	}
	x = Cross(y, z);

	return Mat4(
		x.x,    y.x,    z.x,    _from.x,
		x.y,    y.y,    z.y,    _from.y,
		x.z,    y.z,    z.z,    _from.z,
		0.0f,   0.0f,   0.0f,   1.0f
		);
}

Line::Line(const Vec3& _origin, const Vec3& _direction) 
	: m_origin(_origin)
	, m_direction(_direction)
{
}
Ray::Ray(const Vec3& _origin, const Vec3& _direction)
	: m_origin(_origin)
	, m_direction(_direction)
{
}
LineSegment::LineSegment(const Vec3& _start, const Vec3& _end)
	: m_start(_start)
	, m_end(_end)
{
}
Sphere::Sphere(const Vec3& _origin, float _radius)
	: m_origin(_origin)
	, m_radius(_radius)
{
}
Plane::Plane(const Vec3& _normal, float _offset)
	: m_normal(_normal)
	, m_offset(_offset)
{
}
Plane::Plane(const Vec3& _normal, const Vec3& _origin)
	: m_normal(_normal)
	, m_offset(Dot(_normal, _origin))
{
}
Capsule::Capsule(const Vec3& _start, const Vec3& _end, float _radius)
	: m_start(_start)
	, m_end(_end)
	, m_radius(_radius)
{
}

bool Im3d::Intersects(const Ray& _ray, const Plane& _plane)
{
	float x = Dot(_plane.m_normal, _ray.m_direction);
	return x <= 0.0f;
}
bool Im3d::Intersect(const Ray& _ray, const Plane& _plane, float& t0_)
{
	t0_ = Dot(_plane.m_normal, (_plane.m_normal * _plane.m_offset) - _ray.m_origin) / Dot(_plane.m_normal, _ray.m_direction);
	return t0_ >= 0.0f;
}
bool Im3d::Intersects(const Ray& _r, const Sphere& _s)
{
	Vec3 p = _s.m_origin - _r.m_origin;
	float p2 = Length2(p);
	float q = Dot(p, _r.m_direction);
	float r2 = _s.m_radius * _s.m_radius;
	if (q < 0.0f && p2 > r2) {
		return false;
	}
	return p2 - (q * q) <= r2;
}
bool Im3d::Intersect(const Ray& _r, const Sphere& _s, float& t0_, float& t1_)
{
	Vec3 p = _s.m_origin - _r.m_origin; 
	float q = Dot(p, _r.m_direction); 
	if (q < 0.0f) {
		return false;
	}
	float p2 = Length2(p) - q * q; 
	float r2 = _s.m_radius * _s.m_radius;
	if (p2 > r2) {
		return false;
	}
	float s = sqrt(r2 - p2); 
	t0_ = Max(q - s, 0.0f);
	t1_ = q + s;
	 
	return true;
}
bool Im3d::Intersects(const Ray& _ray, const Capsule& _capsule)
{
	return Distance2(_ray, LineSegment(_capsule.m_start, _capsule.m_end)) < _capsule.m_radius * _capsule.m_radius;
}
bool Im3d::Intersect(const Ray& _ray, const Capsule& _capsule, float& t0_, float& t1_)
{
	IM3D_ASSERT(false); // \todo implement
	return false;
}
Vec3 Im3d::Nearest(const Ray& _ray, const LineSegment& _segment, float& tr_)
{
	Vec3 ldir = _segment.m_end - _segment.m_start;
	Vec3 p = _segment.m_start - _ray.m_origin;
	float q = Length2(ldir);
	float r = Dot(ldir, _ray.m_direction);
	float s = Dot(ldir, p);
	float t = Dot(_ray.m_direction, p);

	float sn, sd, tn, td;
	float denom = q - r * r;
	if (denom < FLT_EPSILON) {
		sd = td = 1.0f;
		sn = 0.0f;
		tn = t;
	} else {
		sd = td = denom;
		sn = r * t - s;
		tn = q * t - r * s;
		if (sn < 0.0f) {
		    sn = 0.0f;
		    tn = t;
		    td = 1.0f;
		} else if (sn > sd) {
			sn = sd;
			tn = t + r;
			td = 1.0f;
		}
	}

	float ts;
	if (tn < 0.0f) {
		tr_ = 0.0f;
		if (r >= 0.0f) {
		    ts = 0.0f;
		} else if (s <= q) {
		    ts = 1.0f;
		} else {
		    ts = -s / q;
		}
	} else {
		tr_ = tn / td;
		ts = sn / sd;
	}
	return _segment.m_start + ldir * ts;
}
float Im3d::Distance2(const Ray& _ray, const LineSegment& _segment)
{
	float tr;
	Vec3 p = Nearest(_ray, _segment, tr);
	return Length2(_ray.m_origin + _ray.m_direction * tr - p);
}


void Im3d::DrawXyzAxes()
{
	Context& ctx = GetContext();
	ctx.pushColor(ctx.getColor());
		ctx.begin(Context::PrimitiveMode_Lines);
			ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Red);
			ctx.vertex(Vec3(1.0f, 0.0f, 0.0f), ctx.getSize(), Color_Red);
			ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Green);
			ctx.vertex(Vec3(0.0f, 1.0f, 0.0f), ctx.getSize(), Color_Green);
			ctx.vertex(Vec3(0.0f, 0.0f, 0.0f), ctx.getSize(), Color_Blue);
			ctx.vertex(Vec3(0.0f, 0.0f, 1.0f), ctx.getSize(), Color_Blue);
		ctx.end();
	ctx.popColor();

}
void Im3d::DrawSphere(const Vec3& _origin, float _radius, int _detail)
{
	Context& ctx = GetContext();
 // xy circle
	ctx.begin(Context::PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = kTwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, sinf(rad) * _radius + _origin.y, 0.0f + _origin.z));
		}
	ctx.end();
 // xz circle
	ctx.begin(Context::PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = kTwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, 0.0f + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
 // yz circle
	ctx.begin(Context::PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = kTwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f + _origin.x, cosf(rad) * _radius + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
}
void Im3d::DrawAlignedBox(const Vec3& _min, const Vec3& _max)
{
	Context& ctx = GetContext();
	ctx.begin(Context::PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _min.y, _min.z)); 
		ctx.vertex(Vec3(_max.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _max.z));
		ctx.vertex(Vec3(_min.x, _min.y, _max.z));
	ctx.end();
	ctx.begin(Context::PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _max.y, _min.z)); 
		ctx.vertex(Vec3(_max.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _max.z));
		ctx.vertex(Vec3(_min.x, _max.y, _max.z));
	ctx.end();
	ctx.begin(Context::PrimitiveMode_Lines);
		ctx.vertex(Vec3(_min.x, _min.y, _min.z));
		ctx.vertex(Vec3(_min.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _min.z));
		ctx.vertex(Vec3(_min.x, _min.y, _max.z));
		ctx.vertex(Vec3(_min.x, _max.y, _max.z));
		ctx.vertex(Vec3(_max.x, _min.y, _max.z));
		ctx.vertex(Vec3(_max.x, _max.y, _max.z));
	ctx.end();
}
void Im3d::DrawCylinder(const Vec3& _start, const Vec3& _end, float _radius, int _detail)
{
	Context& ctx = GetContext();
	Vec3 org  = _start + (_end - _start) * 0.5f;
	float ln  = Length(_end - _start) * 0.5f;
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end));
		ctx.begin(Context::PrimitiveMode_LineLoop);
			for (int i = 0; i <= _detail; ++i) {
				float rad = kTwoPi * ((float)i / (float)_detail) - kHalfPi;
				ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			}
			for (int i = 0; i <= _detail; ++i) {
				float rad = kTwoPi * ((float)i / (float)_detail) - kHalfPi;
				ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			}
		ctx.end();
		ctx.begin(Context::PrimitiveMode_Lines);
			for (int i = 0; i <= _detail; ++i) {
				float rad = kTwoPi * ((float)i / (float)_detail) - kHalfPi;
				ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
				ctx.vertex(Vec3(0.0f, 0.0f,  ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			}
		ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawCapsule(const Vec3& _start, const Vec3& _end, float _radius, int _detail)
{
	Context& ctx = GetContext();
	Vec3 org = _start + (_end - _start) * 0.5f;
	float ln = Length(_end - _start) * 0.5f;
	int detail2 = _detail * 2; // force cap base detail to match ends
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end));
	 // yz silhoette + cap bases
		ctx.begin(Context::PrimitiveMode_LineLoop);
			for (int i = 0; i <= detail2; ++i) {
				float rad = kTwoPi * ((float)i / (float)detail2) - kHalfPi;
				ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			}
			for (int i = 0; i < _detail; ++i) {
				float rad = kPi * ((float)i / (float)_detail) + kPi;
				ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
			}
			for (int i = 0; i < _detail; ++i) {
				float rad = kPi * ((float)i / (float)_detail);
				ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
			}
			for (int i = 0; i <= detail2; ++i) {
				float rad = kTwoPi * ((float)i / (float)detail2) - kHalfPi;
				ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			}
		ctx.end();
	 // xz silhoette
		ctx.begin(Context::PrimitiveMode_LineLoop);
			for (int i = 0; i < _detail; ++i) {
				float rad = kPi * ((float)i / (float)_detail) + kPi;
				ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
			}
			for (int i = 0; i < _detail; ++i) {
				float rad = kPi * ((float)i / (float)_detail);
				ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
			}
		ctx.end();
	ctx.popMatrix();
}

bool Im3d::GizmoPosition(const char* _id, Vec3* _position_)
{
	Context& ctx = GetContext();
	ctx.pushId(MakeId(_id));
		float worldSize = ctx.pixelsToWorldSize(*_position_, 64.0f);
		Id xaxisId = MakeId("xaxis");
		ctx.axisGizmo(xaxisId, _position_, Vec3(1.0f, 0.0f, 0.0f), Color_Red, worldSize);
	ctx.popId();
	return false;
}

/*******************************************************************************

                                  Vector

*******************************************************************************/

template <typename T>
Vector<T>::~Vector()
{
	if (m_data) {
		IM3D_FREE(m_data);
		m_data = 0;
	}
}

template <typename T>
void Vector<T>::reserve(U32 _capacity)
{
	_capacity = _capacity < 8 ? 8 : _capacity;
	if (_capacity < m_capacity) {
		return;
	}
	T* data = (T*)IM3D_MALLOC(sizeof(T) * _capacity);
	if (m_data) {
		memcpy(data, m_data, sizeof(T) * m_size);
		IM3D_FREE(m_data);
	}
	m_data = data;
	m_capacity = _capacity;
}

template <typename T>
void Vector<T>::resize(U32 _size, const T& _val)
{
	reserve(_size);
	while (m_size < _size) {
		push_back(_val);
	}
}

template <typename T>
void Im3d::swap(Vector<T>& _a_, Vector<T>& _b_)
{
	T* data        = _a_.m_data;
	U32 capacity   = _a_.m_capacity;
	U32 size       = _a_.m_size;
	_a_.m_data     = _b_.m_data;
	_a_.m_capacity = _b_.m_capacity;
	_a_.m_size     = _b_.m_size;
	_b_.m_data     = data;
	_b_.m_capacity = capacity;
	_b_.m_size     = size;
}

template class Vector<bool>;
template class Vector<char>;
template class Vector<float>;
template class Vector<Id>;
template class Vector<Mat4>;
template class Vector<Color>;
template class Vector<DrawList>;

/*******************************************************************************

                                 Context

*******************************************************************************/

static Context s_DefaultContext;
Context* Im3d::internal::g_CurrentContext = &s_DefaultContext;

void Context::begin(PrimitiveMode _mode)
{
	IM3D_ASSERT(!m_drawCalled); // Begin*() called after Draw() but before NewFrame()
	IM3D_ASSERT(m_primMode == PrimitiveMode_None); // forgot to call End()
	m_primMode = _mode;
	m_vertCountThisPrim = 0;
	switch (m_primMode) {
	case PrimitiveMode_Points:
		m_firstVertThisPrim = m_vertexData[DrawPrimitive_Points][m_primList].size();
		break;
	case PrimitiveMode_Lines:
	case PrimitiveMode_LineStrip:
	case PrimitiveMode_LineLoop:
		m_firstVertThisPrim = m_vertexData[DrawPrimitive_Lines][m_primList].size();
		break;
	case PrimitiveMode_Triangles:
	case PrimitiveMode_TriangleStrip:
		m_firstVertThisPrim = m_vertexData[DrawPrimitive_Triangles][m_primList].size();
		break;
	default:
		break;
	};
}

void Context::end()
{
	IM3D_ASSERT(m_primMode != PrimitiveMode_None); // End() called without Begin*()
	switch (m_primMode) {
	case PrimitiveMode_Points:
		break;
	case PrimitiveMode_Lines:
		IM3D_ASSERT(m_vertCountThisPrim % 2 == 0);
		break;
	case PrimitiveMode_LineStrip:
		IM3D_ASSERT(m_vertCountThisPrim > 1);
		break;
	case PrimitiveMode_LineLoop:
		IM3D_ASSERT(m_vertCountThisPrim > 1);
		m_vertexData[DrawPrimitive_Lines][m_primList].push_back(m_vertexData[DrawPrimitive_Lines][m_primList].back());
		m_vertexData[DrawPrimitive_Lines][m_primList].push_back(m_vertexData[DrawPrimitive_Lines][m_primList][m_firstVertThisPrim]);
		break;
	case PrimitiveMode_Triangles:
		IM3D_ASSERT(m_vertCountThisPrim % 3 == 0);
		break;
	case PrimitiveMode_TriangleStrip:
		IM3D_ASSERT(m_vertCountThisPrim >= 3);
		break;
	default:
		break;
	};
	m_primMode = PrimitiveMode_None;
}

void Context::vertex(const Vec3& _position, float _size, Color _color)
{	
	IM3D_ASSERT(m_primMode != PrimitiveMode_None); // Vertex() called without Begin*()

	// \todo optim: force alpha/matrix stack bottom to be 1/identity, then skip the transform if the stack size == 1
	VertexData vd(m_matrixStack.back() * _position, _size, _color);
	vd.m_color.setA(vd.m_color.getA() * m_alphaStack.back());
	
	switch (m_primMode) {
	case PrimitiveMode_Points:
		m_vertexData[DrawPrimitive_Points][m_primList].push_back(vd);
		break;
	case PrimitiveMode_Lines:
		m_vertexData[DrawPrimitive_Lines][m_primList].push_back(vd);
		break;
	case PrimitiveMode_LineStrip:
	case PrimitiveMode_LineLoop:
		if (m_vertCountThisPrim >= 2) {
			m_vertexData[DrawPrimitive_Lines][m_primList].push_back(m_vertexData[DrawPrimitive_Lines][m_primList].back());
			++m_vertCountThisPrim;
		}
		m_vertexData[DrawPrimitive_Lines][m_primList].push_back(vd);
		break;
	case PrimitiveMode_Triangles:
		m_vertexData[DrawPrimitive_Triangles][m_primList].push_back(vd);
		break;
	case PrimitiveMode_TriangleStrip:
		if (m_vertCountThisPrim >= 3) {
			m_vertexData[DrawPrimitive_Triangles][m_primList].push_back(*(m_vertexData[DrawPrimitive_Triangles][m_primList].end() - 2));
			m_vertexData[DrawPrimitive_Triangles][m_primList].push_back(*(m_vertexData[DrawPrimitive_Triangles][m_primList].end() - 2));
			m_vertCountThisPrim += 2;
		}
		m_vertexData[DrawPrimitive_Triangles][m_primList].push_back(vd);
		break;
	default:
		break;
	};
	++m_vertCountThisPrim;
}

void Context::reset()
{
 // All state stacks should be default here, else there was a mismatched Push*()/Pop*()
	IM3D_ASSERT(m_colorStack.size() == 1);
	IM3D_ASSERT(m_alphaStack.size() == 1);
	IM3D_ASSERT(m_sizeStack.size() == 1);
	IM3D_ASSERT(m_enableSortingStack.size() == 1);
	IM3D_ASSERT(m_matrixStack.size() == 1);
	IM3D_ASSERT(m_idStack.size() == 1);
	
	
	IM3D_ASSERT(m_primMode == PrimitiveMode_None);
	m_primMode = PrimitiveMode_None;

	for (int i = 0; i < DrawPrimitive_Count; ++i) {
		for (int j = 0; j < 2; ++j) {
			m_vertexData[i][j].clear();
			m_vertexData[i][j].clear();
			m_vertexData[i][j].clear();
		}
	}
	m_sortedDrawLists.clear();
	m_sortCalled = false;
	m_drawCalled = false;

	m_idActive = m_idHot = Id_Invalid;
	m_hotDepth = FLT_MAX;
	
 // copy keydown array internally so that we can make a delta to detect key presses
	memcpy(m_keyDownPrev, m_keyDownCurr,       Key_Count); // \todo avoid this copy, use an index
	memcpy(m_keyDownCurr, m_appData.m_keyDown, Key_Count); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)
}

void Context::draw()
{
	IM3D_ASSERT(m_appData.drawCallback);
	
 // draw unsorted prims first
	for (int i = 0; i < DrawPrimitive_Count; ++i) {
		if (m_vertexData[i][0].size() > 0) {
			DrawList dl;
			dl.m_primType = (DrawPrimitiveType)i;
			dl.m_vertexData = m_vertexData[i][0].data();
			dl.m_vertexCount = m_vertexData[i][0].size();
			m_appData.drawCallback(dl);
		}
	}

 // draw sorted primitives on top
	if (!m_sortCalled) {
		sort();
		m_sortCalled = true;
	}
	for (auto dl = m_sortedDrawLists.begin(); dl != m_sortedDrawLists.end(); ++dl) {
		m_appData.drawCallback(*dl);
	}
	
	m_drawCalled = true;
}

void Context::pushEnableSorting(bool _enable)
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
	m_primList = _enable ? 1 : 0;
	m_enableSortingStack.push_back(_enable);
}
void Context::popEnableSorting()
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
	m_enableSortingStack.pop_back();
	m_primList = m_enableSortingStack.back() ? 1 : 0;
}
void Context::setEnableSorting(bool _enable)
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None); // can't change sort mode mid-primitive
	m_primList = _enable ? 1 : 0;
	m_enableSortingStack.back() = _enable;
}

Context::Context()
{
	m_sortCalled = false;
	m_drawCalled = false;
	m_primMode = PrimitiveMode_None;
	m_primList = 0; // = sorting disabled
	m_firstVertThisPrim = 0;
	m_vertCountThisPrim = 0;
	memset(&m_appData, 0, sizeof(m_appData));
	memset(&m_keyDownCurr, 0, sizeof(m_keyDownCurr));
	memset(&m_keyDownPrev, 0, sizeof(m_keyDownPrev));

	pushMatrix(Mat4(1.0f));
	pushColor(Color_White);
	pushAlpha(1.0f);
	pushSize(1.0f);
	pushEnableSorting(false);
	pushId(0x811C9DC5u); // fnv1 hash base
}

Context::~Context()
{
}

namespace {
	struct SortData
	{
		float       m_key;
		VertexData* m_start;
		SortData() {}
		SortData(float _key, VertexData* _start): m_key(_key), m_start(_start) {}
	};

	int SortCmp(const void* _a, const void* _b)
	{
		float ka = ((SortData*)_a)->m_key;
		float kb = ((SortData*)_b)->m_key;
		if (ka < kb) {
			return 1;
		} else if (ka > kb) {
			return -1;
		} else {
			return 0;
		}
	}

	void Reorder(Vector<VertexData>& _data_, const SortData* _sort, U32 _sortCount, U32 _primSize)
	{
		Vector<VertexData> ret;
		ret.reserve(_data_.size());
		for (U32 i = 0; i < _sortCount; ++i) {
			for (U32 j = 0; j < _primSize; ++j) {
				ret.push_back(*(_sort[i].m_start + j));
			}
		}
		Im3d::swap(_data_, ret);
	}
}

void Context::sort()
{
	Vector<SortData> sortData[DrawPrimitive_Count];
	Vec3 viewOrigin = m_appData.m_viewOrigin;

 // sort each primitive list internally
	static const int kPrimCount[DrawPrimitive_Count] = { 1, 2, 3 };
	for (int i = 0 ; i < DrawPrimitive_Count; ++i) {
		Vector<VertexData>& vd = m_vertexData[i][1];
		if (!vd.empty()) {
			sortData[i].reserve(vd.size() / kPrimCount[i]);
			for (VertexData* v = vd.begin(); v != vd.end(); ) {
				sortData[i].push_back(SortData(0.0f, v));
				IM3D_ASSERT(v < vd.end());
				for (int j = 0; j < kPrimCount[i]; ++j, ++v) {
				 // sort key is the primitive midpoint distance to view origin
					sortData[i].back().m_key += Length2(Vec3(v->m_positionSize) - viewOrigin);
				}
				sortData[i].back().m_key /= (float)kPrimCount[i];
			}
		 // qsort is not necessarily stable but it doesn't matter assuming the prims are pushed in
		 //   roughly the same order each frame
			qsort(sortData[i].data(), sortData[i].size(), sizeof(SortData), SortCmp);
			Reorder(m_vertexData[i][1], sortData[i].data(), sortData[i].size(), kPrimCount[i]);
		}
	}

 // construct draw lists (split prim lists so that none overlap)
	int cprim = 0;
	SortData* search[DrawPrimitive_Count];
	int emptyCount = 0;
	for (int i = 0; i < DrawPrimitive_Count; ++i) {
		if (sortData[i].empty()) {
			search[i] = 0;
			++emptyCount;
		} else {
			search[i] = sortData[i].begin();
		}
	}
	#define modinc(v) ((v + 1) % DrawPrimitive_Count)
	while (emptyCount != DrawPrimitive_Count) {
		while (search[cprim] == 0) {
			cprim = modinc(cprim);
		}
	 // find the max key at the current position across all sort data
		float mxkey = search[cprim]->m_key;
		int mxprim = cprim;
		for (int p = modinc(cprim); p != cprim; p = modinc(p)) {
			if (search[p] != 0 && search[p]->m_key > mxkey) {
				mxkey = search[p]->m_key;
				mxprim = p;
			}
		}

	 // if draw list is empty or primitive changed start a new draw list
		if (m_sortedDrawLists.empty() || m_sortedDrawLists.back().m_primType != mxprim) {
			cprim = mxprim;
			DrawList dl;
			dl.m_primType = (DrawPrimitiveType)cprim;
			dl.m_vertexData = m_vertexData[cprim][1].data() + (search[cprim] - sortData[cprim].data()) * kPrimCount[cprim];
			dl.m_vertexCount= 0;
			m_sortedDrawLists.push_back(dl);
		}

	 // increment the vertex count for the current draw list
		m_sortedDrawLists.back().m_vertexCount += kPrimCount[cprim];
		++search[cprim];
		if (search[cprim] == sortData[cprim].end()) {
			search[cprim] = 0;
			++emptyCount;
		}

	}
	#undef modinc
}

float Context::pixelsToWorldSize(const Vec3& _position, float _pixels)
{
	float d = Length(_position - m_appData.m_viewOrigin);
	return m_appData.m_tanHalfFov * 2.0f * d * (_pixels / m_appData.m_viewportSize.y);
}

void Context::axisGizmo(Id _id, Vec3* _position_, const Vec3& _axis, Color _color, float _worldSize)
{
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Line axisLine(*_position_, _axis);
	Capsule axisCapsule(
		*_position_ + _axis * _worldSize * 0.2f, // *.2f = leave a small space around the origin
		*_position_ + _axis * _worldSize,
		0.05f * _worldSize
		);

	

	Color color = _color;
	if (_id == m_idActive) {
		color = Color_GizmoHighlight;
	} else if (_id == m_idHot) {
		if (m_idActive == Id_Invalid && Intersects(ray, axisCapsule)) {
			if (isKeyDown(MouseLeft)) {
				m_idActive = _id;
				float tr, tl;
				//Nearest(ray, axisLine, tr, tl);
				m_translationOffset = _axis * tl;
			}

		} else {
			m_idHot = Id_Invalid;
		}
		color = Color_GizmoHighlight;

	} else {
	 	float d2 = Length2(axisCapsule.m_end - m_appData.m_viewOrigin);
		if (m_idHot == Id_Invalid && d2 < m_hotDepth && Intersects(ray, axisCapsule)) {
			m_idHot = _id;
			m_hotDepth = d2;
		}
	}

	float alignedAlpha = 1.0f - fabs(Dot(_axis, Normalize(m_appData.m_viewOrigin - *_position_)));
	alignedAlpha = Remap(alignedAlpha, 0.1f, 0.2f);

	pushColor(color);
	pushAlpha(alignedAlpha);
	pushSize(4.0f);
		BeginLines();
			Vertex(axisCapsule.m_start);
			Vertex(axisCapsule.m_end);
			//DrawArrow(cp.m_start, cp.m_end, 0.2f * screenScale);
		End();
	popSize();
	popAlpha();
	popColor();

}
