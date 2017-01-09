#include "im3d.h"
#include "im3d_math.h"

#include <cstring>
#include <cfloat>

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
	U32 ret = (U32)GetContext().getId(); // i.e. top of Id stack
	while (*_str) {
		ret ^= (U32)*_str++;
		ret *= kFnv1aPrime32;
	}
	return (Id)ret;
}

// declared in im3d_math.h
static inline bool Equal(const Vec3& _a, const Vec3& _b)
{
	if (fabs(_a.x - _b.x) < FLT_EPSILON) return false;
	if (fabs(_a.y - _b.y) < FLT_EPSILON) return false;
	if (fabs(_a.z - _b.z) < FLT_EPSILON) return false;
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
	Mat4 ret(
		1.0f, 0.0f, 0.0f, _t.x,
		0.0f, 1.0f, 0.0f, _t.y,
		0.0f, 0.0f, 1.0f, _t.z,
		0.0f, 0.0f, 0.0f, 1.0f
		);
	return _m * ret;
}
Mat4 Im3d::Rotate(const Mat4& _m, const Vec3& _axis, float _rads)
{
	float c  = cosf(_rads);
	float rc = 1.0f - c;
	float s  = sinf(_rads);
	return Mat4(
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

/*******************************************************************************

                                  Vector

*******************************************************************************/

template <typename T>
Vector<T>::~Vector()
{
	if (m_data) {
		delete[] m_data;
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
	T* data = new T[_capacity];
	if (m_data) {
		memcpy(data, m_data, sizeof(T) * m_size);
		delete[] m_data;
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

template class Vector<Color>;
template class Vector<float>;
template class Vector<Mat4>;
template class Vector<Id>;
template class Vector<char>;

/*******************************************************************************

                                 Context

*******************************************************************************/

static Context s_DefaultContext;
Context* Im3d::internal::g_CurrentContext = &s_DefaultContext;

void Context::begin(PrimitiveMode _mode)
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None);
	m_primMode = _mode;
	m_vertCountThisPrim = 0;
	switch (m_primMode) {
	case PrimitiveMode_Points:
		m_firstVertThisPrim = m_points[m_primList].size();
		break;
	case PrimitiveMode_Lines:
	case PrimitiveMode_LineStrip:
	case PrimitiveMode_LineLoop:
		m_firstVertThisPrim = m_lines[m_primList].size();
		break;
	case PrimitiveMode_Triangles:
	case PrimitiveMode_TriangleStrip:
		m_firstVertThisPrim = m_triangles[m_primList].size();
		break;
	default:
		break;
	};
}

void Context::end()
{
	IM3D_ASSERT(m_primMode != PrimitiveMode_None);
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
		m_lines[m_primList].push_back(m_lines[m_primList].back());
		m_lines[m_primList].push_back(m_lines[m_primList][m_firstVertThisPrim]);
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
	IM3D_ASSERT(m_primMode != PrimitiveMode_None);

	VertexData vd(m_matrixStack.back() * _position, _size, _color);
	vd.m_color.setA(vd.m_color.getA() * m_alphaStack.back());
	
	switch (m_primMode) {
	case PrimitiveMode_Points:
		m_points[m_primList].push_back(vd);
		break;
	case PrimitiveMode_Lines:
		m_lines[m_primList].push_back(vd);
		break;
	case PrimitiveMode_LineStrip:
	case PrimitiveMode_LineLoop:
		if (m_vertCountThisPrim >= 2) {
			m_lines[m_primList].push_back(m_lines[m_primList].back());
			++m_vertCountThisPrim;
		}
		m_lines[m_primList].push_back(vd);
		break;
	case PrimitiveMode_Triangles:
		m_triangles[m_primList].push_back(vd);
		break;
	case PrimitiveMode_TriangleStrip:
		if (m_vertCountThisPrim >= 3) {
			m_triangles[m_primList].push_back(*(m_triangles[m_primList].end() - 2));
			m_triangles[m_primList].push_back(*(m_triangles[m_primList].end() - 2));
			m_vertCountThisPrim += 2;
		}
		m_triangles[m_primList].push_back(vd);
		break;
	default:
		break;
	};
	++m_vertCountThisPrim;
}

void Context::reset()
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None);
	m_primMode = PrimitiveMode_None;

	for (int i = 0; i < 2; ++i) {
		m_points[i].clear();
		m_lines[i].clear();
		m_triangles[i].clear();
	}

 // copy keydown array internally so that we can make a delta to detect key presses
	memcpy(m_keyDownPrev, m_keyDownCurr,       Key_Count); // \todo avoid this copy, use an index
	memcpy(m_keyDownCurr, m_appData.m_keyDown, Key_Count); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)
}

void Context::draw()
{
	IM3D_ASSERT(m_appData.drawPrimitives);

 // draw unsorted prims first
	if (m_triangles[0].size() > 0) {
		m_appData.drawPrimitives(DrawPrimitive_Triangles, m_triangles[0].data(), m_triangles[0].size());
	}
	if (m_lines[0].size() > 0) {
		m_appData.drawPrimitives(DrawPrimitive_Lines, m_lines[0].data(), m_lines[0].size());
	}
	if (m_points[0].size() > 0) {
		m_appData.drawPrimitives(DrawPrimitive_Points, m_points[0].data(), m_points[0].size());
	}

 // draw sorted primitives on top
 // \todo need to sort *all* primitives together
}

void Context::enableSorting(bool _enable)
{
	IM3D_ASSERT(m_primMode == PrimitiveMode_None);
	m_primList  = _enable ? 1 : 0;
}

Context::Context()
{
	m_primMode = PrimitiveMode_None;
	m_primList = 0; // sorting disabled by default
	m_firstVertThisPrim = 0;
	m_vertCountThisPrim = 0;
	memset(&m_appData, 0, sizeof(m_appData));
	memset(&m_keyDownCurr, 0, sizeof(m_keyDownCurr));
	memset(&m_keyDownPrev, 0, sizeof(m_keyDownPrev));

	pushMatrix(Mat4(1.0f));
	pushColor(Color_White);
	pushAlpha(1.0f);
	pushSize(1.0f);
	pushId(0x811C9DC5u); // fnv1 hash base
}

Context::~Context()
{
}

float Context::pixelsToWorldSize(const Vec3& _position, float _pixels)
{
	float d = Length(_position - m_appData.m_viewOrigin);
	return m_appData.m_tanHalfFov * 2.0f * d * (_pixels / m_appData.m_viewportSize.y);
}
