#include "im3d.h"
#include "im3d_math.h"

					#include "imgui/imgui.h"
					#define IM3D_LOG_DBG(...) ImGui::Text(__VA_ARGS__)

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
void Mat4::setCol(int _i, const Vec4& _v)
{
	(*this)(0, _i) = _v.x;
	(*this)(1, _i) = _v.y;
	(*this)(2, _i) = _v.z;
	(*this)(3, _i) = _v.w;
}
void Mat4::setRow(int _i, const Vec4& _v)
{
	(*this)(_i, 0) = _v.x;
	(*this)(_i, 1) = _v.y;
	(*this)(_i, 2) = _v.z;
	(*this)(_i, 3) = _v.w;
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
Vec3 Im3d::ToEulerXYZ(const Mat4& _m)
{
	// see http://www.staff.city.ac.uk/~sbbh653/publications/euler.pdf
	if (fabs(_m(2, 0)) < 1.0f) {
	} else {
	}
	//return Vec3(
	//	atan2f(_m(3, 2), _m(3, 3)),
	//	atan2f(-_m(3, 1), sqrtf(_m(3, 2) * _m(3, 2) + _m(3, 3) * _m(3, 3))),
	//	atan2f(_m(2, 1), _m(1, 1))
	//	);
}
Mat4 Im3d::FromEulerXYZ(Vec3& _euler)
{
	float c, s;

	c = cosf(_euler.x);
	s = sinf(_euler.x);
	Mat4 mx(
		1.0f,  0.0f,  0.0f,  0.0f,
		0.0f,     c,    -s,  0.0f,
		0.0f,     s,     c,  0.0f,
		0.0f,  0.0f,  0.0f,  1.0f
		);
	c = cosf(_euler.y);
	s = sinf(_euler.y);
	Mat4 my(
		   c,  0.0f,     s,  0.0f,
		0.0f,  1.0f,  0.0f,  0.0f,
		  -s,  0.0f,     c,  0.0f,
		0.0f,  0.0f,  0.0f,  1.0f
		);
	c = cosf(_euler.z);
	s = sinf(_euler.z);
	Mat4 mz(
		   c,    -s,  0.0f,  0.0f,
		   s,     c,  0.0f,  0.0f,
		0.0f,  0.0f,  1.0f,  0.0f,
		0.0f,  0.0f,  0.0f,  1.0f
		);

 // \todo conflate matrix multiplications
	return mz * my * mx;
}
Mat4 Im3d::AlignZ(const Vec3& _axis, const Vec3& _up)
{
	Vec3 x, y;
	y = _up - _axis * Dot(_up, _axis);
	float ylen = Length(y);
	if_unlikely (!(ylen > FLT_EPSILON)) {
		Vec3 k = _up + Vec3(FLT_EPSILON);
		y = k - _axis * Dot(k, _axis);
		ylen = Length(y);
	}
	y = y / ylen;
	x = Cross(y, _axis);

	return Mat4(
		x.x,    y.x,    _axis.x,    0.0f,
		x.y,    y.y,    _axis.y,    0.0f,
		x.z,    y.z,    _axis.z,    0.0f,
		0.0f,   0.0f,      0.0f,    1.0f
		);
}
Mat4 Im3d::LookAt(const Vec3& _from, const Vec3& _to, const Vec3& _up)
{
	Mat4 ret = AlignZ(Normalize(_to - _from), _up);
	ret.setCol(3, Vec4(_from, 1.0f)); // inject translation
	return ret;
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
void Im3d::Nearest(const Line& _line0, const Line& _line1, float& t0_, float& t1_)
{
	Vec3 p = _line0.m_origin - _line1.m_origin;
	float q = Dot(_line0.m_direction, _line1.m_direction);
	float s = Dot(_line1.m_direction, p);

	float d = 1.0f - q * q;
	if (d < FLT_EPSILON) { // lines are parallel
		t0_ = 0.0f;
		t1_ = s;
	} else {
		float r = Dot(_line0.m_direction, p);
		t0_ = (q * s - r) / d;
		t1_ = (s - q * r) / d;
	}
}
void Im3d::Nearest(const Ray& _ray, const Line& _line, float& tr_, float& tl_)
{
	Nearest(Line(_ray.m_origin, _ray.m_direction), _line, tr_, tl_);
	tr_ = Max(tr_, 0.0f);
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
void Im3d::DrawQuad(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d)
{
	Context& ctx = GetContext();
	ctx.begin(Context::PrimitiveMode_LineLoop);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawQuad(const Vec3& _origin, const Vec3& _normal, const Vec2& _size)
{
	Context& ctx = GetContext();
	ctx.pushMatrix(LookAt(_origin, _origin + _normal));
		DrawQuad(
			Vec3(-_size.x,  _size.y, 0.0f),
			Vec3( _size.x,  _size.y, 0.0f),
			Vec3( _size.x, -_size.y, 0.0f),
			Vec3(-_size.x, -_size.y, 0.0f)
			);
	ctx.popMatrix();
}
void Im3d::DrawQuadFilled(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d)
{
	Context& ctx = GetContext();
	ctx.begin(Context::PrimitiveMode_Triangles);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_a);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawCircle(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail)
{
	Context& ctx = GetContext();
 	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal));
	ctx.begin(Context::PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = kTwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius, sinf(rad) * _radius, 0.0f));
		}
	ctx.end();
	ctx.popMatrix();
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
void Im3d::DrawArrow(const Vec3& _start, const Vec3& _end, float _headLength)
{
	Context& ctx = GetContext();

	float normHeadLength = _headLength / Length(_end - _start);
	Vec3 head = _start + (_end - _start) * (1.0f - normHeadLength);
	ctx.begin(Context::PrimitiveMode_Lines);
		ctx.vertex(_start);
		ctx.vertex(head);
		ctx.vertex(head, Max(ctx.getSize() * 2.0f, 4.0f), ctx.getColor());
		ctx.vertex(_end, 2.0f, ctx.getColor());
	ctx.end();
}


bool Im3d::Gizmo(const char* _id, Mat4* _mat_)
{
	Context& ctx = GetContext();
	if (ctx.wasKeyPressed(Action_TransformPosition)) {
		ctx.m_transformMode = Context::TransformMode_Position;
	} else if (ctx.wasKeyPressed(Action_TransformRotation)) {
		ctx.m_transformMode = Context::TransformMode_Rotation;
	} else if (ctx.wasKeyPressed(Action_TransformScale)) {
		ctx.m_transformMode = Context::TransformMode_Scale;
	}

	switch (ctx.m_transformMode) {
	case Context::TransformMode_Position: {
		Vec3 pos = _mat_->getCol(3);
		if (GizmoPosition(_id, &pos)) {
			_mat_->setCol(3, Vec4(pos, 1.0f));
			return true;
		}
		break;
	}
	case Context::TransformMode_Rotation: {
		Vec3 euler = ToEulerXYZ(*_mat_);
		ImGui::Text("EULER %.3f, %.3f, %.3f", euler.x, euler.y, euler.z);
		if (GizmoRotation(_id, _mat_->getCol(3), &euler.x, &euler.y, &euler.z)) {
			Mat4 rm = FromEulerXYZ(euler);
			(*_mat_)(0, 0) = rm(0, 0);  (*_mat_)(0, 1) = rm(0, 1);  (*_mat_)(0, 2) = rm(0, 2);
			(*_mat_)(1, 0) = rm(1, 0);  (*_mat_)(1, 1) = rm(1, 1);  (*_mat_)(1, 2) = rm(1, 2);
			(*_mat_)(2, 0) = rm(2, 0);  (*_mat_)(2, 1) = rm(2, 1);  (*_mat_)(2, 2) = rm(2, 2);
			return true;
		}
		break;
	}
		break;
	case Context::TransformMode_Scale:
		break;
	default:
		break;
	};

	return false;
}

bool Im3d::GizmoPosition(const char* _id, Vec3* _position_)
{
	Vec3 drawAt = *_position_; // copy to prevent lag in the sub-gizmos 
	bool ret = false;

	Context& ctx = GetContext();
	ctx.pushId(MakeId(_id));
	ctx.pushEnableSorting(false);
	ctx.pushColor(Color_GizmoHighlight);
		float worldHeight = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoHeightPixels);
		float worldSize = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoSizePixels);
		float planeSize = worldHeight * (0.5f * 0.5f);
		float planeOffset = worldHeight * 0.5f;
		
		Color colorX = Color_Red;
		Color colorY = Color_Green;
		Color colorZ = Color_Blue;
		
		{ // XZ plane
			Id planeId = MakeId("xzplane");
			Vec3 planeNormal = Vec3(0.0f, 1.0f, 0.0f);
			Vec3 planeOrigin = drawAt + Vec3(planeOffset, 0.0f, planeOffset);
			float alignedAlpha = fabs(Dot(planeNormal, Normalize(ctx.getAppData().m_viewOrigin - planeOrigin)));
			alignedAlpha = Remap(alignedAlpha, 0.1f, 0.2f);
			if (alignedAlpha > 0.0f || ctx.m_idHot == planeId) {
				ret |= ctx.gizmoPlanePosition(planeId, planeOrigin, _position_, planeNormal, Color_GizmoHighlight, planeSize); 
				if (ctx.m_idHot == planeId) {
					colorX = colorZ = Color_GizmoHighlight;
				}
				ctx.pushAlpha(ctx.getAlpha() * alignedAlpha);
					ctx.pushAlpha(ctx.m_idHot == planeId ? 1.0f : 0.1f * ctx.getAlpha());
						DrawQuadFilled(
							planeOrigin + Vec3(-1.0f,  0.0f,  1.0f) * planeSize,
							planeOrigin + Vec3( 1.0f,  0.0f,  1.0f) * planeSize,
							planeOrigin + Vec3( 1.0f,  0.0f, -1.0f) * planeSize,
							planeOrigin + Vec3(-1.0f,  0.0f, -1.0f) * planeSize
							);
					ctx.popAlpha();
					DrawQuad(
						planeOrigin + Vec3(-1.0f,  0.0f,  1.0f) * planeSize,
						planeOrigin + Vec3( 1.0f,  0.0f,  1.0f) * planeSize,
						planeOrigin + Vec3( 1.0f,  0.0f, -1.0f) * planeSize,
						planeOrigin + Vec3(-1.0f,  0.0f, -1.0f) * planeSize
						);
				ctx.popAlpha();
			}
		}
		{ // XY plane
			Id planeId = MakeId("xyplane");
			Vec3 planeNormal = Vec3(0.0f, 0.0f, 1.0f);
			Vec3 planeOrigin = drawAt + Vec3(planeOffset, planeOffset, 0.0f);
			float alignedAlpha = fabs(Dot(planeNormal, Normalize(ctx.getAppData().m_viewOrigin - planeOrigin)));
			alignedAlpha = Remap(alignedAlpha, 0.1f, 0.2f);
			if (alignedAlpha > 0.0f || ctx.m_idHot == planeId) {
				ret |= ctx.gizmoPlanePosition(planeId, planeOrigin, _position_, planeNormal, Color_GizmoHighlight, planeSize); 
				if (ctx.m_idHot == planeId) {
					colorX = colorY = Color_GizmoHighlight;
				}
				ctx.pushAlpha(ctx.getAlpha() * alignedAlpha);
					ctx.pushAlpha(ctx.m_idHot == planeId ? 1.0f : 0.1f * ctx.getAlpha());
						DrawQuadFilled(
							planeOrigin + Vec3(-1.0f,  1.0f,  0.0f) * planeSize,
							planeOrigin + Vec3( 1.0f,  1.0f,  0.0f) * planeSize,
							planeOrigin + Vec3( 1.0f, -1.0f,  0.0f) * planeSize,
							planeOrigin + Vec3(-1.0f, -1.0f,  0.0f) * planeSize
							);
					ctx.popAlpha();
					DrawQuad(
						planeOrigin + Vec3(-1.0f,  1.0f,  0.0f) * planeSize,
						planeOrigin + Vec3( 1.0f,  1.0f,  0.0f) * planeSize,
						planeOrigin + Vec3( 1.0f, -1.0f,  0.0f) * planeSize,
						planeOrigin + Vec3(-1.0f, -1.0f,  0.0f) * planeSize
						);
				ctx.popAlpha();
			}
		}
		{ // YZ plane
			Id planeId = MakeId("yzplane");
			Vec3 planeNormal = Vec3(1.0f, 0.0f, 0.0f);
			Vec3 planeOrigin = drawAt + Vec3(0.0f, planeOffset, planeOffset);
			float alignedAlpha = fabs(Dot(planeNormal, Normalize(ctx.getAppData().m_viewOrigin - planeOrigin)));
			alignedAlpha = Remap(alignedAlpha, 0.1f, 0.2f);
			if (alignedAlpha > 0.0f || ctx.m_idHot == planeId) {
				ret |= ctx.gizmoPlanePosition(planeId, planeOrigin, _position_, planeNormal, Color_GizmoHighlight, planeSize); 
				if (ctx.m_idHot == planeId) {
					colorY = colorZ = Color_GizmoHighlight;
				}
				ctx.pushAlpha(ctx.getAlpha() * alignedAlpha);
					ctx.pushAlpha(ctx.m_idHot == planeId ? 1.0f : 0.1f * ctx.getAlpha());
						DrawQuadFilled(
							planeOrigin + Vec3(0.0f, -1.0f,  1.0f) * planeSize,
							planeOrigin + Vec3(0.0f,  1.0f,  1.0f) * planeSize,
							planeOrigin + Vec3(0.0f,  1.0f, -1.0f) * planeSize,
							planeOrigin + Vec3(0.0f, -1.0f, -1.0f) * planeSize
							);
					ctx.popAlpha();
					DrawQuad(
						planeOrigin + Vec3(0.0f, -1.0f,  1.0f) * planeSize,
						planeOrigin + Vec3(0.0f,  1.0f,  1.0f) * planeSize,
						planeOrigin + Vec3(0.0f,  1.0f, -1.0f) * planeSize,
						planeOrigin + Vec3(0.0f, -1.0f, -1.0f) * planeSize
						);
				ctx.popAlpha();
			}
		}
		{ // view plane
			Id planeId = MakeId("viewplane");
			Vec3 planeNormal = Normalize(drawAt - ctx.getAppData().m_viewOrigin);
			Vec3 planeOrigin = drawAt;
			ret |= ctx.gizmoPlanePosition(planeId, planeOrigin, _position_, planeNormal, Color_GizmoHighlight, planeSize * 0.5f); 
			if (ctx.m_idActive == planeId || ctx.m_idHot == planeId) {
				colorX = colorY = colorZ = Color_GizmoHighlight;
			}
			
			ctx.pushAlpha(ctx.m_idHot == planeId ? 1.0f : ctx.getAlpha());
				ctx.begin(Context::PrimitiveMode_Points);
					ctx.vertex(planeOrigin, ctx.m_gizmoSizePixels * 2.0f, (ctx.m_idActive == planeId || ctx.m_idHot == planeId) ? Color_GizmoHighlight : Color_White);
				ctx.end();
			ctx.popAlpha();
		}

		ctx.setEnableSorting(true);
		ret |= ctx.gizmoAxisPosition(MakeId("xaxis"), drawAt, _position_, Vec3(1.0f, 0.0f, 0.0f), colorX, worldHeight, worldSize);
		ret |= ctx.gizmoAxisPosition(MakeId("yaxis"), drawAt, _position_, Vec3(0.0f, 1.0f, 0.0f), colorY, worldHeight, worldSize);
		ret |= ctx.gizmoAxisPosition(MakeId("zaxis"), drawAt, _position_, Vec3(0.0f, 0.0f, 1.0f), colorZ, worldHeight, worldSize);
	ctx.popColor();
	ctx.popEnableSorting();
	ctx.popId();

	return ret;
}
bool Im3d::GizmoRotation(const char* _id, const Vec3& _origin, float* _x_, float* _y_, float* _z_)
{
	bool ret = false;
	Context& ctx = Im3d::GetContext();
	ctx.pushId(MakeId(_id));
	ctx.pushEnableSorting(false);
		float worldHeight = ctx.pixelsToWorldSize(_origin, ctx.m_gizmoHeightPixels);
		float worldSize = ctx.pixelsToWorldSize(_origin, ctx.m_gizmoSizePixels);
		
		Id idXy = MakeId("xyplane");
		Id idXz = MakeId("xzplane");
		Id idYz = MakeId("yzplane");
		Id idV  = MakeId("viewplane");
		if (ctx.m_idActive != idXz && ctx.m_idActive != idYz && ctx.m_idActive != idV) {
			ret |= ctx.gizmoAxisAngle(idXy, _origin, Vec3(0.0f, 0.0f, 1.0f), _x_, Color_Blue,  worldHeight, worldSize); 
		}
		if (ctx.m_idActive != idXy && ctx.m_idActive != idYz && ctx.m_idActive != idV) {
			ret |= ctx.gizmoAxisAngle(idXz, _origin, Vec3(0.0f, 1.0f, 0.0f), _y_, Color_Green, worldHeight, worldSize);
		}
		if (ctx.m_idActive != idXy && ctx.m_idActive != idXz && ctx.m_idActive != idV) {
			ret |= ctx.gizmoAxisAngle(idYz, _origin, Vec3(1.0f, 0.0f, 0.0f), _z_, Color_Red,   worldHeight, worldSize);
		}
		// \todo efficient conversion from an arbitrary axis-angle to XYZ rotation
		//if (ctx.m_idActive != idXy && ctx.m_idActive != idXz && ctx.m_idActive != idYz) {
		//	Vec3 planeNormal = Normalize(_origin - ctx.getAppData().m_viewOrigin);
		//	float v = 0.0f;
		//	ret |= ctx.gizmoAxisAngle(idV, _origin, planeNormal, &v, Color_White, worldHeight * 1.2f, worldSize);
		//}

	ctx.popEnableSorting();
	ctx.popId();
	return ret;
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

	m_transformMode = TransformMode_Position;
	m_idHot = Id_Invalid;
	m_idActive = Id_Invalid;
	m_hotDepth = FLT_MAX;
	m_gizmoHeightPixels = 64.0f;
	m_gizmoSizePixels = 5.0f;

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

bool Context::gizmoAxisPosition(Id _id, const Vec3& _drawAt, Vec3* _out_, const Vec3& _axis, Color _color, float _worldHeight, float _worldSize)
{
	Vec3& storedPosition = m_gizmoStateVec3;

	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Line axisLine(_drawAt, _axis);
	Capsule axisCapsule(
		_drawAt + _axis * _worldHeight * 0.2f, // *.2f = leave a small space around the origin
		_drawAt + _axis * _worldHeight,
		_worldSize
		);
	bool ret = false;


	Color color = _color;
	if (_id == m_idActive) {
		color = Color_GizmoHighlight;
		if (isKeyDown(Action_Select)) {
			float tr, tl;
			Nearest(ray, axisLine, tr, tl);
			*_out_ = *_out_ + _axis * tl - storedPosition;
			ret = true;			

			begin(PrimitiveMode_Lines);
				vertex(_drawAt - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
				vertex(_drawAt + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
			end();
		} else {
			m_idActive = Id_Invalid;
		}

	} else if (_id == m_idHot) {
		color = Color_GizmoHighlight;

		if (m_idActive == Id_Invalid && Intersects(ray, axisCapsule)) {
			if (isKeyDown(Action_Select)) {
				m_idActive = _id;
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);
				storedPosition = _axis * tl;
			}
		} else {
			makeCold();
		}

	} else {
	 	float depth = Length2(axisCapsule.m_end - m_appData.m_viewOrigin);
		bool intersects = Intersects(ray, axisCapsule);
		makeHot(_id, depth, intersects);
	}

	float alignedAlpha = 1.0f - fabs(Dot(_axis, Normalize(m_appData.m_viewOrigin - _drawAt)));
	alignedAlpha = getAlpha() * Remap(alignedAlpha, 0.05f, 0.1f);
	if (m_idHot == _id) {
		alignedAlpha = 1.0f;
	}

	pushColor(color);
	pushAlpha(alignedAlpha);
	pushSize(m_gizmoSizePixels);
		DrawArrow(axisCapsule.m_start, axisCapsule.m_end, _worldSize * 4.0f);
	popSize();
	popAlpha();
	popColor();
	return ret;
}

bool Context::gizmoPlanePosition(Id _id, const Vec3& _drawAt, Vec3* _out_, const Vec3& _normal, Color _color, float _worldSize)
{
	Vec3& storedPosition = m_gizmoStateVec3;
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Plane plane(_normal, _drawAt);
	float tr;
	bool intersects = Intersect(ray, plane, tr);
	if (!intersects) {
		return false;
	}
	Vec3 intersection = ray.m_origin + ray.m_direction * tr;
	intersects &= AllLess(Abs(intersection - _drawAt), Vec3(_worldSize));	
	bool ret = false;
	
	if (_id == m_idActive) {
		if (isKeyDown(Action_Select)) {
			*_out_ = intersection + storedPosition;
			ret = true;
		} else {
			m_idActive = Id_Invalid;
		}
	} else if (_id == m_idHot) {
		if (intersects) {
			if (isKeyDown(Action_Select)) {
				m_idActive = _id;
				storedPosition = *_out_ - intersection;
			}
		} else {
			makeCold();
		}
	} else {
		float depth = Length2(_drawAt - m_appData.m_viewOrigin);
		makeHot(_id, depth, intersects);
	}

	return ret;
}

bool Context::gizmoAxisAngle(Id _id, const Vec3& _drawAt, const Vec3& _axis, float* _out_, Color _color, float _worldRadius, float _worldSize)
{
	Vec3& storedVec = m_gizmoStateVec3;
	float& storedAngle = m_gizmoStateFloat;

 // \note using the plane produces an intersection blind spot at grazing angles
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Plane plane(_axis, _drawAt);
	float tr;
	bool intersects = Intersect(ray, plane, tr);
	Vec3 intersection = ray.m_origin + ray.m_direction * tr;
	float dist = Length(intersection - _drawAt);
	intersects &= fabs(dist - _worldRadius) < _worldSize;
	bool ret = false;
	
	Color color = _color;
	if (_id == m_idActive) {
		color = Color_GizmoHighlight;
		if (isKeyDown(Action_Select)) {
			Vec3 v = Normalize(intersection - _drawAt);
			float sign = SignOf(Dot(Cross(storedVec, v), plane.m_normal));
			ImGui::Text("%.3f + %.3f", storedAngle, acosf(Dot(v, storedVec)) * sign);
			*_out_ = storedAngle + acosf(Dot(v, storedVec)) * sign;
			ret = true;

			begin(PrimitiveMode_Lines);
				vertex(_drawAt, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				vertex(_drawAt + storedVec * _worldRadius, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				vertex(_drawAt, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				vertex(_drawAt + v * _worldRadius, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);

				vertex(_drawAt - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
				vertex(_drawAt + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
			end();
		} else {
			m_idActive = Id_Invalid;
		}

	} else if (_id == m_idHot) {
		color = Color_GizmoHighlight;

		if (m_idActive == Id_Invalid && intersects) {
			if (isKeyDown(Action_Select)) {
				m_idActive = _id;
				storedVec = Normalize(intersection - _drawAt);
				storedAngle = *_out_;
			}
		} else {
			makeCold();
		}

	} else {
	 	float depth = Length2(intersection - m_appData.m_viewOrigin);
		makeHot(_id, depth, intersects);
	}
	Vec3 viewDir = Normalize(m_appData.m_viewOrigin - _drawAt);
	float alignedAlpha = fabs(Dot(_axis, viewDir));
	alignedAlpha = Max(Remap(alignedAlpha, 0.9f, 1.0f), 0.1f);
	if (m_idHot == _id) {
		alignedAlpha = 1.0f;
	}
	pushColor(color);
	pushSize(m_gizmoSizePixels);
		pushMatrix(getMatrix() * LookAt(_drawAt, _drawAt + _axis));
		begin(Context::PrimitiveMode_LineLoop);
			const int _detail = 32; // \todo dynamically select detail based on worldRadius
			for (int i = 0; i < _detail; ++i) {
				float rad = kTwoPi * ((float)i / (float)_detail);
				vertex(Vec3(cosf(rad) * _worldRadius, sinf(rad) * _worldRadius, 0.0f));

			 // post-modify the alpha for parts of the ring occluded by the sphere
				VertexData& vd = m_vertexData[DrawPrimitive_Lines][m_primList].back();
				Vec3 v = vd.m_positionSize;
				float d = Dot(Normalize(v - _drawAt), viewDir); 
				d = Max(Remap(d, 0.1f, 0.2f), alignedAlpha);
				vd.m_color.setA(vd.m_color.getA() * d);
			}
		end();
		popMatrix();
	popSize();
	popColor();
	return ret;
}

bool Context::makeHot(Id _id, float _depth, bool _intersects)
{
	if (m_idActive == Id_Invalid && _depth < m_hotDepth && _intersects) {
		m_idHot = _id;
		m_hotDepth = _depth;
		return true;
	}
	return false;
}

void Context::makeCold()
{
	m_idActive = m_idHot = Id_Invalid;
	m_hotDepth = FLT_MAX;
}
