#include "im3d.h"
#include "im3d_math.h"

#include <cstring>

using namespace Im3d;

const Id    Im3d::kId_Invalid    = 0;
const Color Im3d::kColor_Black   = Color(0.0f, 0.0f, 0.0f);
const Color Im3d::kColor_White   = Color(1.0f, 1.0f, 1.0f);
const Color Im3d::kColor_Red     = Color(1.0f, 0.0f, 0.0f);
const Color Im3d::kColor_Green   = Color(0.0f, 1.0f, 0.0f);
const Color Im3d::kColor_Blue    = Color(0.0f, 0.0f, 1.0f);
const Color Im3d::kColor_Magenta = Color(1.0f, 0.0f, 1.0f);
const Color Im3d::kColor_Yellow  = Color(1.0f, 1.0f, 0.0f);
const Color Im3d::kColor_Cyan    = Color(0.0f, 1.0f, 1.0f);

void Im3d::MulMatrix(const Mat4& _mat)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * _mat);
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

template class Vector<Color>;
template class Vector<float>;
template class Vector<Mat4>;
template class Vector<Id>;

/*******************************************************************************

                                 Context

*******************************************************************************/

static Context s_DefaultContext;
Context* Im3d::internal::g_CurrentContext = &s_DefaultContext;

void Context::begin(PrimitiveMode _mode)
{
	IM3D_ASSERT(m_primMode == kPrimitiveMode_None);
	m_primMode = _mode;
	m_vertCountThisPrim = 0;
	switch (m_primMode) {
	case kPrimitiveMode_Points:
		m_firstVertThisPrim = m_points[m_primList].size();
		break;
	case kPrimitiveMode_Lines:
	case kPrimitiveMode_LineStrip:
	case kPrimitiveMode_LineLoop:
		m_firstVertThisPrim = m_lines[m_primList].size();
		break;
	case kPrimitiveMode_Triangles:
	case kPrimitiveMode_TriangleStrip:
		m_firstVertThisPrim = m_triangles[m_primList].size();
		break;
	default:
		break;
	};
}

void Context::end()
{
	IM3D_ASSERT(m_primMode != kPrimitiveMode_None);
	switch (m_primMode) {
	case kPrimitiveMode_Points:
		break;

	case kPrimitiveMode_Lines:
		IM3D_ASSERT(m_vertCountThisPrim % 2 == 0);
		break;
	case kPrimitiveMode_LineStrip:
		IM3D_ASSERT(m_vertCountThisPrim > 1);
		break;
	case kPrimitiveMode_LineLoop:
		IM3D_ASSERT(m_vertCountThisPrim > 1);
		m_lines[m_primList].push_back(m_lines[m_primList].back());
		m_lines[m_primList].push_back(m_lines[m_primList][m_firstVertThisPrim]);
		break;
	case kPrimitiveMode_Triangles:
		IM3D_ASSERT(m_vertCountThisPrim % 3 == 0);
		break;
	case kPrimitiveMode_TriangleStrip:
		IM3D_ASSERT(m_vertCountThisPrim >= 3);
		break;
	default:
		break;
	};
	m_primMode = kPrimitiveMode_None;
}

void Context::vertex(const Vec3& _position, float _size, Color _color)
{	
	IM3D_ASSERT(m_primMode != kPrimitiveMode_None);

	VertexData vd(m_matrixStack.back() * _position, _size, _color);
	vd.m_color.setA(vd.m_color.getA() * m_alphaStack.back());
	
	switch (m_primMode) {
	case kPrimitiveMode_Points:
		m_points[m_primList].push_back(vd);
		break;
	case kPrimitiveMode_Lines:
		m_lines[m_primList].push_back(vd);
		break;
	case kPrimitiveMode_LineStrip:
	case kPrimitiveMode_LineLoop:
		if (m_vertCountThisPrim >= 2) {
			m_lines[m_primList].push_back(m_lines[m_primList].back());
			++m_vertCountThisPrim;
		}
		m_lines[m_primList].push_back(vd);
		break;
	case kPrimitiveMode_Triangles:
		m_triangles[m_primList].push_back(vd);
		break;
	case kPrimitiveMode_TriangleStrip:
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
	IM3D_ASSERT(m_primMode == kPrimitiveMode_None);
	m_primMode = kPrimitiveMode_None;

	for (int i = 0; i < 2; ++i) {
		m_points[i].clear();
		m_lines[i].clear();
		m_triangles[i].clear();
	}

 // copy keydown array internally so that we can make a delta to detect key presses
	memcpy(m_keyDownPrev, m_keyDownCurr,       kKey_Count); // \todo avoid this copy, use an index
	memcpy(m_keyDownCurr, m_appData.m_keyDown, kKey_Count); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)
}

void Context::draw()
{
	IM3D_ASSERT(m_appData.drawPrimitives);

 // draw unsorted prims first
	if (m_triangles[0].size() > 0) {
		m_appData.drawPrimitives(kDrawPrimitive_Triangles, m_triangles[0].data(), m_triangles[0].size());
	}
	if (m_lines[0].size() > 0) {
		m_appData.drawPrimitives(kDrawPrimitive_Lines, m_lines[0].data(), m_lines[0].size());
	}
	if (m_points[0].size() > 0) {
		m_appData.drawPrimitives(kDrawPrimitive_Points, m_points[0].data(), m_points[0].size());
	}

 // draw sorted primitives on top
 // \todo need to sort *all* primitives together
}

void Context::enableSorting(bool _enable)
{
	IM3D_ASSERT(m_primMode == kPrimitiveMode_None);
	m_primList  = _enable ? 1 : 0;
}

Context::Context()
{
	m_primMode = kPrimitiveMode_None;
	m_primList = 0; // sorting disabled by default
	m_firstVertThisPrim = 0;
	m_vertCountThisPrim = 0;
	memset(&m_appData, 0, sizeof(m_appData));
	memset(&m_keyDownCurr, 0, sizeof(m_keyDownCurr));
	memset(&m_keyDownPrev, 0, sizeof(m_keyDownPrev));

	pushMatrix(Mat4(1.0f));
	pushColor(kColor_White);
	pushAlpha(1.0f);
	pushSize(1.0f);
	pushId(0x811C9DC5u); // fnv1 hash base
}

Context::~Context()
{
}

float Context::pixelsToWorldSize(const Vec3& _position, float _pixels)
{
	float d = length(_position - m_appData.m_viewOrigin);
	return m_appData.m_tanHalfFov * 2.0f * d * (_pixels / m_appData.m_displaySize.y);
}
