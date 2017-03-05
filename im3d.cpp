/*	CHANGE LOG
	==========
	2017-03-01 (v1.02) - Configurable VertexData alignment (IM3D_VERTEX_ALIGNMENT).
	2017-02-23 (v1.01) - Removed AppData::m_tanHalfFov, replaced with AppData::m_projScaleY. Added AppData::m_projOrtho.
*/
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

//#define IM3D_GIZMO_DEBUG

using namespace Im3d;

const Id    Im3d::Id_Invalid    = 0;
const Color Im3d::Color_Black   = Color(0x000000ff);
const Color Im3d::Color_White   = Color(0xffffffff);
const Color Im3d::Color_Red     = Color(0xff0000ff);
const Color Im3d::Color_Green   = Color(0x00ff00ff);
const Color Im3d::Color_Blue    = Color(0x0000ffff);
const Color Im3d::Color_Magenta = Color(0xff00ffff);
const Color Im3d::Color_Yellow  = Color(0xffff00ff);
const Color Im3d::Color_Cyan    = Color(0x00ffffff);

static const Color Color_GizmoHighlight = Color(0xffc745ff);

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

void Im3d::MulMatrix(const Mat4& _mat4)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * _mat4);
}
void Im3d::Translate(float _x, float _y, float _z)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Translation(Vec3(_x, _y, _z)));
}
void Im3d::Rotate(const Vec3& _axis, float _angle)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(Rotation(_axis, _angle)));
}
void Im3d::Rotate(const Mat3& _rotation)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(_rotation));
}
void Im3d::Scale(float _x, float _y, float _z)
{
	Context& ctx = GetContext();
	ctx.setMatrix(ctx.getMatrix() * Mat4(Scale(Vec3(_x, _y, _z))));
}


void Im3d::DrawXyzAxes()
{
	Context& ctx = GetContext();
	ctx.pushColor(ctx.getColor());
	ctx.begin(PrimitiveMode_Lines);
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
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawQuad(const Vec3& _origin, const Vec3& _normal, const Vec2& _size)
{
	Context& ctx = GetContext();
	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
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
	ctx.begin(PrimitiveMode_Triangles);
		ctx.vertex(_a);
		ctx.vertex(_b);
		ctx.vertex(_c);
		ctx.vertex(_a);
		ctx.vertex(_c);
		ctx.vertex(_d);
	ctx.end();
}
void Im3d::DrawQuadFilled(const Vec3& _origin, const Vec3& _normal, const Vec2& _size)
{
	Context& ctx = GetContext();
	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	DrawQuadFilled(
		Vec3(-_size.x,  _size.y, 0.0f),
		Vec3( _size.x,  _size.y, 0.0f),
		Vec3( _size.x, -_size.y, 0.0f),
		Vec3(-_size.x, -_size.y, 0.0f)
		);
	ctx.popMatrix();
}
void Im3d::DrawCircle(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail)
{
	Context& ctx = GetContext();
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius);
	}
 	ctx.pushMatrix(ctx.getMatrix() * LookAt(_origin, _origin + _normal, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius, sinf(rad) * _radius, 0.0f));
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawSphere(const Vec3& _origin, float _radius, int _detail)
{
	Context& ctx = GetContext();
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(_origin, _radius);
	}
 // xy circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, sinf(rad) * _radius + _origin.y, 0.0f + _origin.z));
		}
	ctx.end();
 // xz circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(cosf(rad) * _radius + _origin.x, 0.0f + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
 // yz circle
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i < _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f + _origin.x, cosf(rad) * _radius + _origin.y, sinf(rad) * _radius + _origin.z));
		}
	ctx.end();
}
void Im3d::DrawAlignedBox(const Vec3& _min, const Vec3& _max)
{
	Context& ctx = GetContext();
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _min.y, _min.z)); 
		ctx.vertex(Vec3(_max.x, _min.y, _min.z));
		ctx.vertex(Vec3(_max.x, _min.y, _max.z));
		ctx.vertex(Vec3(_min.x, _min.y, _max.z));
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
		ctx.vertex(Vec3(_min.x, _max.y, _min.z)); 
		ctx.vertex(Vec3(_max.x, _max.y, _min.z));
		ctx.vertex(Vec3(_max.x, _max.y, _max.z));
		ctx.vertex(Vec3(_min.x, _max.y, _max.z));
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
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
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(org, _radius);
	}
	float ln  = Length(_end - _start) * 0.5f;
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _detail; ++i) {
			float rad = TwoPi * ((float)i / (float)_detail) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
		for (int i = 0; i <= 6; ++i) {
			float rad = TwoPi * ((float)i / 6.0f) - HalfPi;
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
	if (_detail < 0) {
		_detail = ctx.estimateLevelOfDetail(org, _radius);
	}
	float ln = Length(_end - _start) * 0.5f;
	int detail2 = _detail * 2; // force cap base detail to match ends
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
	 // yz silhoette + cap bases
		for (int i = 0; i <= detail2; ++i) {
			float rad = TwoPi * ((float)i / (float)detail2) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail) + Pi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(0.0f, cosf(rad), sinf(rad)) * _radius);
		}
		for (int i = 0; i <= detail2; ++i) {
			float rad = TwoPi * ((float)i / (float)detail2) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_LineLoop);
	 // xz silhoette
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail) + Pi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
		}
		for (int i = 0; i < _detail; ++i) {
			float rad = Pi * ((float)i / (float)_detail);
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), 0.0f, sinf(rad)) * _radius);
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawPrism(const Vec3& _start, const Vec3& _end, float _radius, int _sides)
{
	IM3D_ASSERT(_sides > 2);
	Context& ctx = GetContext();
	Vec3 org  = _start + (_end - _start) * 0.5f;
	float ln  = Length(_end - _start) * 0.5f;
	ctx.pushMatrix(ctx.getMatrix() * LookAt(org, _end, ctx.getAppData().m_worldUp));
	ctx.begin(PrimitiveMode_LineLoop);
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.begin(PrimitiveMode_Lines);
		for (int i = 0; i <= _sides; ++i) {
			float rad = TwoPi * ((float)i / (float)_sides) - HalfPi;
			ctx.vertex(Vec3(0.0f, 0.0f, -ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
			ctx.vertex(Vec3(0.0f, 0.0f,  ln) + Vec3(cosf(rad), sinf(rad), 0.0f) * _radius);
		}
	ctx.end();
	ctx.popMatrix();
}
void Im3d::DrawArrow(const Vec3& _start, const Vec3& _end, float _headFraction)
{
	Context& ctx = GetContext();
	Vec3 head = _start + (_end - _start) * (1.0f - _headFraction);
	ctx.begin(PrimitiveMode_Lines);
		ctx.vertex(_start);
		ctx.vertex(head);
		ctx.vertex(head, Max(ctx.getSize() * 2.0f, 4.0f), ctx.getColor());
		ctx.vertex(_end, 2.0f, ctx.getColor());
	ctx.end();
}


static const U32 kFnv1aPrime32 = 0x01000193u;
static U32 Hash(const char* _buf, int _buflen, U32 _base)
{
	IM3D_ASSERT(_buf);
	U32 ret = _base;
	const char* lim = _buf + _buflen;
	while (_buf < lim) {
		ret ^= (U32)*_buf++;
		ret *= kFnv1aPrime32;
	}
	return ret;
}
static U32 HashStr(const char* _str, U32 _base)
{
	IM3D_ASSERT(_str);
	U32 ret = _base;
	while (*_str) {
		ret ^= (U32)*_str++;
		ret *= kFnv1aPrime32;
	}
	return ret;
}
Im3d::Id Im3d::MakeId(const char* _str)
{
	return HashStr(_str, GetContext().getId());
}
Im3d::Id Im3d::MakeId(const void* _ptr)
{
	return Hash((const char*)&_ptr, sizeof(void*), GetContext().getId());
}
Im3d::Id Im3d::MakeId(int _i)
{
	return Hash((const char*)&_i, sizeof(int), GetContext().getId());
}


inline static float Snap(float _val, float _snap)
{
	if (_snap > 0.0f) {
		return floorf(_val / _snap) * _snap;
	}
	return _val;
}

inline static Vec3 Snap(const Vec3& _val, float _snap) {
	if (_snap > 0.0f) {
		return Vec3(floorf(_val.x / _snap) * _snap, floorf(_val.y / _snap) * _snap, floorf(_val.z / _snap) * _snap);
	}
	return _val;
}

inline static Vec3 Snap(const Vec3& _pos, const Plane& _plane, float _snap)
{
	if (_snap > 0.0f) {
	 // get basis vectors on the plane
		Mat3 basis = AlignZ(_plane.m_normal);
		Vec3 i = basis.getCol(0);
		Vec3 j = basis.getCol(1);

	 // decompose _pos in terms of the basis vectors
		i = i * Dot(_pos, i);
		j = j * Dot(_pos, j);

	 // snap the vector lengths
		float ilen = Length(i);
		i = i / ilen;
		ilen = floorf(ilen / _snap) * _snap;
		i = i * ilen;
		float jlen = Length(j);
		j = j / jlen;
		jlen = floorf(jlen / _snap) * _snap;
		j = j * jlen;

		return i + j;
	}
	return _pos;
}

bool Im3d::GizmoTranslation(const char* _id, float _translation_[3], bool _local)
{
	Context& ctx = GetContext();
	ctx.pushId(MakeId(_id));

	if (_local) {
		Mat4 localMatrix = ctx.getMatrix();
		localMatrix.setScale(Vec3(1.0f));
		ctx.pushMatrix(localMatrix);
	}
	
	bool ret = false;
	Vec3* outVec3 = (Vec3*)_translation_;
	Vec3 drawAt = *outVec3;

	float worldHeight = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoHeightPixels);
	float worldSize = ctx.pixelsToWorldSize(drawAt, ctx.m_gizmoSizePixels);	
	float planeSize = worldHeight * (0.5f * 0.5f);
	float planeOffset = worldHeight * 0.5f;
	
	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Vec3(1.0f, 0.0f, 0.0f), Color_Red   },
		{ MakeId("axisY"), Vec3(0.0f, 1.0f, 0.0f), Color_Green },
		{ MakeId("axisZ"), Vec3(0.0f, 0.0f, 1.0f), Color_Blue  }
	};
	struct PlaneG { Id m_id; Vec3 m_origin; };
	PlaneG planes[] = {
		{ MakeId("planeYZ"), Vec3(0.0f, planeOffset, planeOffset) },
		{ MakeId("planeXZ"), Vec3(planeOffset, 0.0f, planeOffset) },
		{ MakeId("planeXY"), Vec3(planeOffset, planeOffset, 0.0f) },
		{ MakeId("planeV"),  Vec3(0.0f, 0.0f, 0.0f) }
	};

 // planes
 	ctx.pushEnableSorting(true);
	if (_local) {
	 // local planes need to be drawn with the pushed matrix for correct orientation
		for (int i = 0; i < 3; ++i) {
			PlaneG& plane = planes[i];
			ctx.gizmoPlaneTranslation_Draw(plane.m_id, plane.m_origin, axes[i].m_axis, planeSize, Color_GizmoHighlight);
			axes[i].m_axis = Normalize(Vec3(ctx.getMatrix().getCol(i))); // if local, extract axes from the pushed matrix
			ret |= ctx.gizmoPlaneTranslation_Behavior(plane.m_id, ctx.getMatrix() * plane.m_origin, axes[i].m_axis, planeSize, outVec3);
		}

	} else {
		ctx.pushMatrix(Mat4(1.0f));
		for (int i = 0; i < 3; ++i) {
			PlaneG& plane = planes[i];
			ret |= ctx.gizmoPlaneTranslation_Behavior(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis, planeSize, outVec3);
			ctx.gizmoPlaneTranslation_Draw(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis, planeSize, Color_GizmoHighlight);
		}
		ctx.popMatrix();
	}
	 
	ctx.pushMatrix(Mat4(1.0f));
	
 // view plane (store the normal when the gizmo becomes active)
	Id currentId = ctx.m_activeId;
	Vec3& storedViewNormal= *((Vec3*)ctx.m_gizmoStateMat3.m);
	Vec3 viewNormal;
	if (planes[3].m_id == ctx.m_activeId) {
		viewNormal = storedViewNormal;
	} else {
		viewNormal = Normalize(drawAt - ctx.getAppData().m_viewOrigin);
	}
	ret |= ctx.gizmoPlaneTranslation_Behavior(planes[3].m_id, drawAt, viewNormal, worldSize, outVec3);
	if (currentId != ctx.m_activeId) {
	 // gizmo became active, store the view normal
		storedViewNormal = viewNormal;
	}
	ctx.begin(PrimitiveMode_Points);
		ctx.vertex(drawAt, ctx.m_gizmoSizePixels * 2.0f, planes[3].m_id == ctx.m_hotId ? Color_GizmoHighlight : Color_White);
	ctx.end();

 // highlight axes if the corresponding plane is hot
	if (planes[0].m_id == ctx.m_hotId) { // YZ
		axes[1].m_color = axes[2].m_color = Color_GizmoHighlight;
	} else if (planes[1].m_id == ctx.m_hotId) { // XZ
		axes[0].m_color = axes[2].m_color = Color_GizmoHighlight;
	} else if (planes[2].m_id == ctx.m_hotId) { // XY
		axes[0].m_color = axes[1].m_color = Color_GizmoHighlight;
	} else if (planes[3].m_id == ctx.m_hotId) {
		axes[0].m_color = axes[1].m_color = axes[2].m_color = Color_GizmoHighlight;
	}

 // axes
	for (int i = 0; i < 3; ++i) {
		AxisG& axis = axes[i];
		ret |= ctx.gizmoAxisTranslation_Behavior(axis.m_id, drawAt, axis.m_axis, worldHeight, worldSize, outVec3);
		ctx.gizmoAxisTranslation_Draw(axis.m_id, drawAt, axis.m_axis, worldHeight, worldSize, axis.m_color);
	}
	ctx.popMatrix();
	ctx.popEnableSorting();

	if (_local) {
		ctx.popMatrix();
	}
	
	ctx.popId();

	return ret;
}
bool Im3d::GizmoRotation(const char* _id, float _rotation_[3*3], bool _local)
{
	Context& ctx = GetContext();
	Id currentId = ctx.m_activeId; // store currentId to detect if the gizmo becomes active during this call
	ctx.pushId(MakeId(_id));
	
	bool ret = false;
	Mat3& storedRotation = ctx.m_gizmoStateMat3;
	Mat3* outMat3 = (Mat3*)_rotation_;
	Vec3 euler = ToEulerXYZ(*outMat3);
	Vec3 origin = ctx.getMatrix().getTranslation();

	float worldRadius = ctx.pixelsToWorldSize(origin, ctx.m_gizmoHeightPixels);
	float worldSize = ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels);
	
	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Vec3(1.0f, 0.0f, 0.0f), Color_Red   },
		{ MakeId("axisY"), Vec3(0.0f, 1.0f, 0.0f), Color_Green },
		{ MakeId("axisZ"), Vec3(0.0f, 0.0f, 1.0f), Color_Blue  }
	};
	Id viewId = MakeId("axisV");
 
	if (_local) {
	 // extract axes from the pushed matrix
		for (int i = 0; i < 3; ++i) {
			if (ctx.m_activeId == axes[i].m_id) {
			 // use the stored matrix where the id is active, avoid rotating the axis frame during interaction (cause numerical instability)
				axes[i].m_axis = Normalize(Vec3(storedRotation.getCol(i)));
			} else {
				axes[i].m_axis = Normalize(Vec3(ctx.getMatrix().getCol(i)));
			}
		}
	}

	ctx.pushMatrix(Mat4(1.0f));
	for (int i = 0; i < 3; ++i) {
		if (i == 0 && (ctx.m_activeId == axes[1].m_id || ctx.m_activeId == axes[2].m_id || ctx.m_activeId == viewId)) {
			continue;
		}
		if (i == 1 && (ctx.m_activeId == axes[2].m_id || ctx.m_activeId == axes[0].m_id || ctx.m_activeId == viewId)) {
			continue;
		}
		if (i == 2 && (ctx.m_activeId == axes[0].m_id || ctx.m_activeId == axes[1].m_id || ctx.m_activeId == viewId)) {
			continue;
		}
		
		AxisG& axis = axes[i];
		ctx.gizmoAxislAngle_Draw(axis.m_id, origin, axis.m_axis, worldRadius * 0.9f, euler[i], axis.m_color);
		if (ctx.gizmoAxislAngle_Behavior(axis.m_id, origin, axis.m_axis, worldRadius * 0.9f, worldSize, &euler[i])) {
			*outMat3 = Rotation(axis.m_axis, euler[i] - ctx.m_gizmoStateFloat) * storedRotation;
			ret = true;
		} 
	}
	if (!(ctx.m_activeId == axes[0].m_id || ctx.m_activeId == axes[1].m_id || ctx.m_activeId == axes[2].m_id)) {
		Vec3 viewNormal = Normalize(origin - ctx.getAppData().m_viewOrigin);
		float angle = 0.0f;
		if (ctx.gizmoAxislAngle_Behavior(viewId, origin, viewNormal, worldRadius, worldSize, &angle)) {
			*outMat3 = Rotation(viewNormal, angle) * storedRotation;
			ret = true;
		} 
		ctx.gizmoAxislAngle_Draw(viewId, origin, viewNormal, worldRadius, angle, viewId == ctx.m_activeId ? Color_GizmoHighlight : Color_White);
	}
	ctx.popMatrix();

	if (currentId != ctx.m_activeId) {
	 // gizmo became active, store rotation matrix
		storedRotation = *outMat3;
	}
	ctx.popId();
	return ret;
}
bool Im3d::GizmoScale(const char* _id, float _scale_[3])
{
	Context& ctx = GetContext();
	ctx.pushId(MakeId(_id));

	bool ret = false;
	Vec3* outVec3 = (Vec3*)_scale_;
	Vec3 origin = ctx.getMatrix().getTranslation();

	float worldHeight = ctx.pixelsToWorldSize(origin, ctx.m_gizmoHeightPixels);
	float worldSize = ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels);	
	float planeSize = worldHeight * (0.5f * 0.5f);
	float planeOffset = worldHeight * 0.5f;
	
	struct AxisG { Id m_id; Vec3 m_axis; Color m_color; };
	AxisG axes[] = {
		{ MakeId("axisX"), Normalize(ctx.getMatrix().getCol(0)), Color_Red   },
		{ MakeId("axisY"), Normalize(ctx.getMatrix().getCol(1)), Color_Green },
		{ MakeId("axisZ"), Normalize(ctx.getMatrix().getCol(2)), Color_Blue  }
	};
	
 	ctx.pushEnableSorting(true);
	ctx.pushMatrix(Mat4(1.0f));
	{ // uniform scale
		const AppData& appData = ctx.getAppData();
		Id uniformId = MakeId("uniform");
		Sphere handle(origin, ctx.pixelsToWorldSize(origin, ctx.m_gizmoSizePixels * 4.0f));
		Ray ray(ctx.getAppData().m_cursorRayOrigin, appData.m_cursorRayDirection);
		float t0, t1;
		bool intersects = Intersect(ray, handle, t0, t1);
		Vec3& storedScale = ctx.m_gizmoStateVec3;
		Vec3& storedPosition = *((Vec3*)ctx.m_gizmoStateMat3.m);
		if (uniformId == ctx.m_activeId) {
			if (ctx.isKeyDown(Action_Select)) {
				Plane plane(Normalize(origin - appData.m_viewOrigin), origin);
				Intersect(ray, plane, t0);
				Vec3 intersection = ray.m_origin + ray.m_direction * t0;
				float sign = Dot(intersection - origin, storedPosition - origin);
				float scale= copysignf(Length(intersection - origin), sign) / worldHeight;
				scale = Snap(scale, ctx.getAppData().m_snapScale);
				*outVec3 = storedScale * Vec3(Max(1.0f + copysignf(scale, sign), 1e-4f));
				ret = true;
			} else {
				ctx.m_activeId = Id_Invalid;
			}

		} else if (uniformId == ctx.m_hotId) {
			if (intersects) {
				if (ctx.isKeyDown(Action_Select)) {
					ctx.m_activeId = uniformId;
					storedScale = *outVec3;
					storedPosition = ray.m_origin + ray.m_direction * t0;
				}
			} else {
				ctx.resetId();
			}

		} else {
		 	float depth = Length2(origin - appData.m_viewOrigin);
			ctx.makeHot(uniformId, depth, intersects);
		}

		bool activeOrHot = ctx.m_activeId == uniformId || ctx.m_hotId == uniformId;
		if (activeOrHot) {
			for (int i = 0; i < 3; ++i) {
				axes[i].m_color = Color_GizmoHighlight;
			}
			ctx.pushColor(Color_GizmoHighlight);
			ctx.pushAlpha(1.0f);
			ctx.pushSize(2.0f);
				DrawCircle(origin, Normalize(origin - appData.m_viewOrigin), worldSize * 2.0f);
			ctx.popSize();
			ctx.popAlpha();
			ctx.popColor();
		}
		ctx.pushAlpha(ctx.m_hotId == uniformId ? 1.0f : ctx.getAlpha());
		ctx.begin(PrimitiveMode_Points);
			ctx.vertex(origin, ctx.m_gizmoSizePixels * 2.0f, activeOrHot ? Color_GizmoHighlight : Color_White);
		ctx.end();
		ctx.popAlpha();
	}

	for (int i = 0; i < 3; ++i) {
		AxisG& axis = axes[i];
		ret |= ctx.gizmoAxisScale_Behavior(axis.m_id, origin, axis.m_axis, worldHeight, worldSize, &(*outVec3)[i]);
		ctx.gizmoAxisScale_Draw(axis.m_id, origin, axis.m_axis, worldHeight, worldSize, axis.m_color);
	}

	ctx.popMatrix();
	ctx.popEnableSorting();
	
	ctx.popId();
	return ret;
}
bool Im3d::Gizmo(const char* _id, float _transform_[4*4])
{
	IM3D_ASSERT(_transform_);

	Context& ctx = GetContext();
 	Mat4* outMat4 = (Mat4*)_transform_;
	ctx.pushMatrix(*outMat4);
	
	bool ret = false;
	switch (ctx.m_gizmoMode) {
		case GizmoMode_Translation: {
			Vec3 translation = outMat4->getTranslation();
			if (GizmoTranslation(_id, translation, ctx.m_gizmoLocal)) {
				outMat4->setTranslation(translation);
				ret = true;
			}
			break;
		}
		case GizmoMode_Rotation: {
			Mat3 rotation = outMat4->getRotation();
			if (GizmoRotation(_id, rotation, ctx.m_gizmoLocal)) {
				outMat4->setRotation(rotation);
				ret = true;
			}
			break;
		}
		case GizmoMode_Scale: {
			Vec3 scale = outMat4->getScale();
			if (GizmoScale(_id, scale)) {
				outMat4->setScale(scale);
				ret = true;
			}
			break;
		}
		default:
			break;
	};

	ctx.popMatrix();
	
	return ret;
}

bool Im3d::Gizmo(const char* _id, float _translation_[3], float _rotation_[3*3], float _scale_[3])
{
	Context& ctx = GetContext();
 	
	Mat4 transform(
		_translation_ ? *((Vec3*)_translation_) : Vec3(0.0f), 
		_rotation_    ? *((Mat3*)_rotation_)    : Mat3(1.0f),
		_scale_       ? *((Vec3*)_scale_)       : Vec3(1.0f)
		);
	ctx.pushMatrix(transform);
	
	bool ret = false;
	switch (ctx.m_gizmoMode) {
		case GizmoMode_Translation: 
			if (_translation_) {
				if (GizmoTranslation(_id, _translation_, ctx.m_gizmoLocal)) {
					ret = true;
				}
			}
			break;
		case GizmoMode_Rotation: 
			if (_rotation_) {
				if (GizmoRotation(_id, _rotation_, ctx.m_gizmoLocal)) {
					ret = true;
				}
			}
			break;
		case GizmoMode_Scale: 
			if (_scale_) {
				if (GizmoScale(_id, _scale_)) {
					ret = true;
				}
			}
			break;
		default:
			break;
	};

	ctx.popMatrix();
	
	return ret;
}

/*******************************************************************************

                                  Vector

*******************************************************************************/

static void* AlignedMalloc(size_t _size, size_t _align)
{
	IM3D_ASSERT(_size > 0);
	IM3D_ASSERT(_align > 0);
	size_t grow = (_align - 1) + sizeof(void*);
	size_t mem = (size_t)IM3D_MALLOC(_size + grow);
	if (mem) {
		size_t ret = (mem + grow) / _align * _align;
		IM3D_ASSERT(ret % _align == 0); // aligned correctly
		IM3D_ASSERT(ret >= mem + sizeof(void*)); // header large enough to store a ptr
		*((void**)(ret - sizeof(void*))) = (void*)mem;
		return (void*)ret;
	} else {
		return nullptr;
	}
}
static void AlignedFree(void* _ptr_)
{
	void* mem = *((void**)((size_t)_ptr_ - sizeof(void*)));
	IM3D_FREE(mem);
}

template <typename T>
Vector<T>::~Vector()
{
	if (m_data) {
		AlignedFree(m_data);
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
	T* data = (T*)AlignedMalloc(sizeof(T) * _capacity, alignof(T));
	if (m_data) {
		memcpy(data, m_data, sizeof(T) * m_size);
		AlignedFree(m_data);;
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
void Vector<T>::swap(Vector<T>& _a_, Vector<T>& _b_)
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

static Context g_DefaultContext;
Context* Im3d::internal::g_CurrentContext = &g_DefaultContext;

void Context::begin(PrimitiveMode _mode)
{
	IM3D_ASSERT(!m_drawCalled); // Begin*() called after Draw() but before NewFrame(), or forgot to call NewFrame()
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

	VertexData vd(_position, _size, _color);
	if (m_matrixStack.size() > 1) { // optim, skip the matrix multiplication when the stack size is 1
		vd.m_positionSize = Vec4(m_matrixStack.back() * _position, _size);
	}
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
 // all state stacks should be default here, else there was a mismatched Push*()/Pop*()
	IM3D_ASSERT(m_colorStack.size() == 1);
	IM3D_ASSERT(m_alphaStack.size() == 1);
	IM3D_ASSERT(m_sizeStack.size() == 1);
	IM3D_ASSERT(m_enableSortingStack.size() == 1);
	IM3D_ASSERT(m_matrixStack.size() == 1);
	IM3D_ASSERT(m_idStack.size() == 1);
	
	IM3D_ASSERT(m_primMode == PrimitiveMode_None);
	m_primMode = PrimitiveMode_None;

	for (int i = 0; i < DrawPrimitive_Count; ++i) {
		m_vertexData[i][0].clear();
		m_vertexData[i][1].clear();
	}
	m_sortedDrawLists.clear();
	m_sortCalled = false;
	m_drawCalled = false;
	
 // copy keydown array internally so that we can make a delta to detect key presses
	memcpy(m_keyDownPrev, m_keyDownCurr,       Key_Count); // \todo avoid this copy, use an index
	memcpy(m_keyDownCurr, m_appData.m_keyDown, Key_Count); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)

 // update gizmo modes
	if (wasKeyPressed(Action_GizmoTranslation)) {
		m_gizmoMode = GizmoMode_Translation;
		resetId();
	} else if (wasKeyPressed(Action_GizmoRotation)) {
		m_gizmoMode = GizmoMode_Rotation;
		resetId();
	} else if (wasKeyPressed(Action_GizmoScale)) {
		m_gizmoMode = GizmoMode_Scale;
		resetId();
	}
	if (wasKeyPressed(Action_GizmoLocal)) {
		m_gizmoLocal = !m_gizmoLocal;
		resetId();
	}
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
	for (Im3d::DrawList* dl = m_sortedDrawLists.begin(); dl != m_sortedDrawLists.end(); ++dl) {
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

	m_gizmoLocal = false;
	m_gizmoMode = GizmoMode_Translation;
	m_hotId = Id_Invalid;
	m_activeId = Id_Invalid;
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
		Vector<VertexData>::swap(_data_, ret);
	}
}

void Context::sort()
{
	Vector<SortData> sortData[DrawPrimitive_Count];
	Vec3 viewOrigin = m_appData.m_viewOrigin;

 // sort each primitive list internally
	for (int i = 0 ; i < DrawPrimitive_Count; ++i) {
		Vector<VertexData>& vd = m_vertexData[i][1];
		if (!vd.empty()) {
			sortData[i].reserve(vd.size() / DrawPrimitiveSize[i]);
			for (VertexData* v = vd.begin(); v != vd.end(); ) {
				sortData[i].push_back(SortData(0.0f, v));
				IM3D_ASSERT(v < vd.end());
				for (int j = 0; j < DrawPrimitiveSize[i]; ++j, ++v) {
				 // sort key is the primitive midpoint distance to view origin
					sortData[i].back().m_key += Length2(Vec3(v->m_positionSize) - viewOrigin);
				}
				sortData[i].back().m_key /= (float)DrawPrimitiveSize[i];
			}
		 // qsort is not necessarily stable but it doesn't matter assuming the prims are pushed in roughly the same order each frame
			qsort(sortData[i].data(), sortData[i].size(), sizeof(SortData), SortCmp);
			Reorder(m_vertexData[i][1], sortData[i].data(), sortData[i].size(), DrawPrimitiveSize[i]);
		}
	}

 // construct draw lists - partition sort data into non-overlapping lists
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
			dl.m_vertexData = m_vertexData[cprim][1].data() + (search[cprim] - sortData[cprim].data()) * DrawPrimitiveSize[cprim];
			dl.m_vertexCount= 0;
			m_sortedDrawLists.push_back(dl);
		}

	 // increment the vertex count for the current draw list
		m_sortedDrawLists.back().m_vertexCount += DrawPrimitiveSize[cprim];
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
	float d = m_appData.m_projOrtho ? 1.0f : Length(_position - m_appData.m_viewOrigin);
	return m_appData.m_projScaleY * d * (_pixels / m_appData.m_viewportSize.y);
}

int Context::estimateLevelOfDetail(const Vec3& _position, float _worldSize, int _min, int _max)
{
	float d = Length(_position - m_appData.m_viewOrigin);
	float x = Clamp(2.0f * atanf(_worldSize / (2.0f * d)), 0.0f, 1.0f);
	float fmin = (float)_min;
	float fmax = (float)_max;
	return (int)(fmin + (fmax - fmin) * x);
}

bool Context::gizmoAxisTranslation_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Vec3* _out_)
{
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Line axisLine(_origin, _axis);
	Capsule axisCapsule(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight, _worldSize);

	#ifdef IM3D_GIZMO_DEBUG
		if (_id == m_hotId) {
			PushDrawState();
			EnableSorting(false);
			SetColor(Color_Magenta);
			SetAlpha(1.0f);
			DrawCapsule(axisCapsule.m_start, axisCapsule.m_end, axisCapsule.m_radius);
			PopDrawState();
		}
	#endif

	Vec3& storedPosition = m_gizmoStateVec3;
	
	if (_id == m_activeId) {
		if (isKeyDown(Action_Select)) {
			float tr, tl;
			Nearest(ray, axisLine, tr, tl);
			tl = Snap(tl, m_appData.m_snapTranslation);
			Vec3 snappedOrigin = Snap(storedPosition, m_appData.m_snapTranslation); // always snap the origin = prevent issues when enabling snap after the gizmo became hot
			*_out_ = *_out_ + _axis * tl - snappedOrigin;
			return true;
		} else {
			m_activeId = Id_Invalid;
		}
	} else if (_id == m_hotId) {
		if (Intersects(ray, axisCapsule)) {
			if (isKeyDown(Action_Select)) {
				m_activeId = _id;
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);
				storedPosition = _axis * tl;
			}
		} else {
			resetId();
		}
	} else {
		float depth = Length2(axisCapsule.m_end - m_appData.m_viewOrigin);
		bool intersects = Intersects(ray, axisCapsule);
		makeHot(_id, depth, intersects);
	}

	return false;
}

void Context::gizmoAxisTranslation_Draw(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Color _color)
{
	Color color = _color;
	float aligned = 1.0f - fabs(Dot(_axis, Normalize(m_appData.m_viewOrigin - _origin)));
	aligned = Remap(aligned, 0.05f, 0.1f);
	if (_id == m_activeId) {
		color = Color_GizmoHighlight;
		pushEnableSorting(false);
		begin(PrimitiveMode_Lines);
			vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
			vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
		end();
		popEnableSorting();
	} else if (_id == m_hotId) {
		color = Color_GizmoHighlight;
		aligned = 1.0f;
	}
	color.setA(color.getA() * aligned);
	pushColor(color);
	pushSize(m_gizmoSizePixels);
		DrawArrow(
			_origin + _axis * (0.2f * _worldHeight), 
			_origin + _axis * _worldHeight, 
			0.3f
			);
	popSize();
	popColor();
}

bool Context::gizmoPlaneTranslation_Behavior(Id _id, const Vec3& _origin, const Vec3& _normal, float _worldSize, Vec3* _out_)
{
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Plane plane(_normal, _origin);

	#ifdef IM3D_GIZMO_DEBUG
		if (_id == m_hotId) {
			PushDrawState();
			EnableSorting(false);
			SetColor(Color_Magenta);
			SetAlpha(0.1f);
			DrawQuadFilled(_origin, _normal, Vec2(2.0f));
			SetAlpha(0.75f);
			SetSize(1.0f);
			DrawQuad(_origin, _normal, Vec2(2.0f));
			SetSize(2.0f);
			DrawCircle(_origin, _normal, 2.0f);
			PopDrawState();
		}
	#endif
	
	float tr;
	bool intersects = Intersect(ray, plane, tr);
	if (!intersects) {
		return false;
	}
	Vec3 intersection = ray.m_origin + ray.m_direction * tr;
	intersects &= AllLess(Abs(intersection - _origin), Vec3(_worldSize));
	intersection = Snap(intersection, plane, m_appData.m_snapTranslation);
	
	Vec3& storedPosition = m_gizmoStateVec3;
	
	if (_id == m_activeId) {
		if (isKeyDown(Action_Select)) {
			*_out_ = intersection + storedPosition;
			return true;
		} else {
			m_activeId = Id_Invalid;
		}
	} else if (_id == m_hotId) {
		if (intersects) {
			if (isKeyDown(Action_Select)) {
				m_activeId = _id;
				storedPosition = *_out_ - intersection;
			}
		} else {
			resetId();
		}
	} else {
		float depth = Length2(_origin - m_appData.m_viewOrigin);
		makeHot(_id, depth, intersects);
	}

	return false;
}
void Context::gizmoPlaneTranslation_Draw(Id _id, const Vec3& _origin, const Vec3& _normal, float _worldSize, Color _color)
{
	Color color = _color;
	float aligned = fabs(Dot(_normal, Normalize(getAppData().m_viewOrigin - _origin)));
	aligned = Remap(aligned, 0.1f, 0.2f);
	color.setA(color.getA() * aligned);
	pushColor(color);
		pushAlpha(_id == m_hotId ? 0.7f : 0.1f * getAlpha());
			DrawQuadFilled(_origin, _normal, Vec2(_worldSize));
		popAlpha();
		DrawQuad(_origin, _normal, Vec2(_worldSize));
	popColor();
}

bool Context::gizmoAxislAngle_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldRadius, float _worldSize, float* _out_)
{
// \note using the plane produces an intersection blind spot at grazing angles, hence expand the ring size by 1-aligned
// \todo also this causes unintuitive behavior when the ray doesn't intersect the plane during interaction, could switch to a view-aligned plane?
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Plane plane(_axis, _origin);
	float tr;
	bool intersects = Intersect(ray, plane, tr);
	Vec3 intersection = ray.m_origin + ray.m_direction * tr;
	Vec3 viewDir = Normalize(m_appData.m_viewOrigin - _origin);
	float aligned = fabs(Dot(_axis, viewDir));
	float dist = Length(intersection - _origin);
	intersects &= fabs(dist - _worldRadius) < (_worldSize + _worldSize * (1.0f - aligned) * 2.0f);
	
	Vec3& storedVec = m_gizmoStateVec3;
	float& storedAngle = m_gizmoStateFloat;
	bool ret = false;
	
	if (_id == m_activeId) {
		if (isKeyDown(Action_Select)) {
			Vec3 delta = Normalize(intersection - _origin);
			float sign = Dot(Cross(storedVec, delta), plane.m_normal);
			float angle = acosf(Clamp(Dot(delta, storedVec), -1.0f, 1.0f));
			*_out_ = storedAngle + copysignf(Snap(angle, m_appData.m_snapRotation), sign);
			return true;
		} else {
			m_activeId = Id_Invalid;
		}

	} else if (_id == m_hotId) {
		if (intersects) {
			if (isKeyDown(Action_Select)) {
				m_activeId = _id;
				storedVec = Normalize(intersection - _origin);
				storedAngle = Snap(*_out_, m_appData.m_snapRotation);
			}
		} else {
			resetId();
		}
	} else {
	 	float depth = Length2(intersection - m_appData.m_viewOrigin);
		makeHot(_id, depth, intersects);
	}
	return false;
}
void Context::gizmoAxislAngle_Draw(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldRadius, float _angle, Color _color)
{
	Vec3 viewDir = Normalize(m_appData.m_viewOrigin - _origin);
	float aligned = fabs(Dot(_axis, viewDir));
	
	Vec3& storedVec = m_gizmoStateVec3;
	Color color = _color;
	
	if (_id == m_activeId) {
		color = Color_GizmoHighlight;
		if (isKeyDown(Action_Select)) {
			Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
			Plane plane(_axis, _origin);
			float tr;
			if (Intersect(ray, plane, tr)) {
				Vec3 intersection = ray.m_origin + ray.m_direction * tr;
				Vec3 delta = Normalize(intersection - _origin);
			
				pushAlpha(Remap(fabs(Dot(Normalize(_origin - m_appData.m_viewOrigin), _axis)), 1.0f, 0.99f));
				pushEnableSorting(false);
				begin(PrimitiveMode_Lines);
					vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
					vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
					vertex(_origin, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
					vertex(_origin + storedVec * _worldRadius, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				end();
				popEnableSorting();
				popAlpha();

				pushColor(Color_GizmoHighlight);
				pushSize(m_gizmoSizePixels);
					DrawArrow(_origin, _origin + delta * _worldRadius, 0.3f);
				popSize();
				popColor();
				begin(PrimitiveMode_Points);
					vertex(_origin, m_gizmoSizePixels * 2.0f, Color_GizmoHighlight);
				end();
			}
		} else {
			m_activeId = Id_Invalid;
		}

	} else if (_id == m_hotId) {
		color = Color_GizmoHighlight;
	}
	aligned = Max(Remap(aligned, 0.9f, 1.0f), 0.1f);
	if (m_activeId == _id) {
		aligned = 1.0f;
	}
	pushColor(color);
	pushSize(m_gizmoSizePixels);
	pushMatrix(getMatrix() * LookAt(_origin, _origin + _axis, m_appData.m_worldUp));
	begin(PrimitiveMode_LineLoop);
		const int detail = estimateLevelOfDetail(_origin, _worldRadius, 16, 128);
		for (int i = 0; i < detail; ++i) {
			float rad = TwoPi * ((float)i / (float)detail);
			vertex(Vec3(cosf(rad) * _worldRadius, sinf(rad) * _worldRadius, 0.0f));

		 // post-modify the alpha for parts of the ring occluded by the sphere
			VertexData& vd = m_vertexData[DrawPrimitive_Lines][m_primList].back();
			Vec3 v = vd.m_positionSize;
			float d = Dot(Normalize(v - _origin), viewDir); 
			d = Max(Remap(d, 0.1f, 0.2f), aligned);
			vd.m_color.setA(vd.m_color.getA() * d);
		}
	end();
	popMatrix();
	popSize();
	popColor();
}

bool Context::gizmoAxisScale_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, float *_out_)
{
	Ray ray(m_appData.m_cursorRayOrigin, m_appData.m_cursorRayDirection);
	Line axisLine(_origin, _axis);
	Capsule axisCapsule(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight, _worldSize);

	#ifdef IM3D_GIZMO_DEBUG
		if (_id == m_hotId) {
			PushDrawState();
			EnableSorting(false);
			SetColor(Color_Magenta);
			SetAlpha(1.0f);
			DrawCapsule(axisCapsule.m_start, axisCapsule.m_end, axisCapsule.m_radius);
			PopDrawState();
		}
	#endif

	Vec3& storedPosition = m_gizmoStateVec3;
	float& storedScale = m_gizmoStateFloat;

	if (_id == m_activeId) {
		if (isKeyDown(Action_Select)) {
			float tr, tl;
			Nearest(ray, axisLine, tr, tl);
			Vec3 intersection = _axis * tl;
			Vec3 delta = intersection - storedPosition;
			float scale = Snap(Length(delta) / _worldHeight, m_appData.m_snapTranslation);
			float sign = Dot(delta, _axis);
			*_out_ = storedScale * Max(1.0f + copysignf(scale, sign), 1e-3f);
			return true;
		} else {
			m_activeId = Id_Invalid;
		}

	} else if (_id == m_hotId) {
		if (Intersects(ray, axisCapsule)) {
			if (isKeyDown(Action_Select)) {
				m_activeId = _id;
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);
				storedPosition = _axis * tl;
				storedScale = *_out_;
			}
		} else {
			resetId();
		}
	} else {
	 	float depth = Length2(axisCapsule.m_end - m_appData.m_viewOrigin);
		bool intersects = Intersects(ray, axisCapsule);
		makeHot(_id, depth, intersects);
	}

	return false;
}
void Context::gizmoAxisScale_Draw(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Color _color)
{
	Color color = _color;
	float aligned = 1.0f - fabs(Dot(_axis, Normalize(m_appData.m_viewOrigin - _origin)));
	aligned = Remap(aligned, 0.05f, 0.1f);
	if (_id == m_activeId) {
		color = Color_GizmoHighlight;
		pushEnableSorting(false);
		begin(PrimitiveMode_Lines);
			vertex(_origin - _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
			vertex(_origin + _axis * 999.0f, m_gizmoSizePixels * 0.5f, _color);
		end();
		popEnableSorting();
	} else if (_id == m_hotId) {
		color = Color_GizmoHighlight;
		aligned = 1.0f;
	}
	color.setA(color.getA() * aligned);
	begin(PrimitiveMode_LineLoop);
		vertex(_origin + _axis * (0.2f * _worldHeight), m_gizmoSizePixels, color);
		vertex(_origin + _axis * _worldHeight, m_gizmoSizePixels, color);
	end();
	begin(PrimitiveMode_Points);
		vertex(_origin + _axis * _worldHeight, m_gizmoSizePixels * 2.0f, color);
	end();
}

bool Context::makeHot(Id _id, float _depth, bool _intersects)
{
	if (m_activeId == Id_Invalid &&	_depth < m_hotDepth && _intersects && !isKeyDown(Action_Select)) {
		m_hotId = _id;
		m_hotDepth = _depth;
		return true;
	}
	return false;
}

void Context::resetId()
{
	m_activeId = m_hotId = Id_Invalid; 
	m_hotDepth = FLT_MAX; 
}

U32 Context::getPrimitiveCount(DrawPrimitiveType _type) const
{
	U32 ret = m_vertexData[_type][0].size() + m_vertexData[_type][1].size();
	ret /= DrawPrimitiveSize[_type];
	return ret;
}

/******************************************************************************

                                 im3d_math

******************************************************************************/

const float Im3d::Pi     = 3.14159265359f;
const float Im3d::TwoPi  = 2.0f * Pi;
const float Im3d::HalfPi = 0.5f * Pi;

// Vec3
Vec3::Vec3(const Vec4& _v)
	: x(_v.x)
	, y(_v.y)
	, z(_v.z)
{
}

// Vec4
Vec4::Vec4(Color _rgba)
	: x(_rgba.getR())
	, y(_rgba.getG())
	, z(_rgba.getB())
	, w(_rgba.getA())
{
}

// Mat3
Mat3::Mat3(float _diagonal)
{
	(*this)(0, 0) = _diagonal; (*this)(0, 1) = 0.0f;      (*this)(0, 2) = 0.0f;
	(*this)(1, 0) = 0.0f;      (*this)(1, 1) = _diagonal; (*this)(1, 2) = 0.0f;
	(*this)(2, 0) = 0.0f;      (*this)(2, 1) = 0.0f;      (*this)(2, 2) = _diagonal;
}
Mat3::Mat3(
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22
	)
{
	(*this)(0, 0) = m00; (*this)(0, 1) = m01; (*this)(0, 2) = m02;
	(*this)(1, 0) = m10; (*this)(1, 1) = m11; (*this)(1, 2) = m12;
	(*this)(2, 0) = m20; (*this)(2, 1) = m21; (*this)(2, 2) = m22;
}
Mat3::Mat3(const Vec3& _colX, const Vec3& _colY, const Vec3& _colZ)
{
	(*this)(0, 0) = _colX.x; (*this)(0, 1) = _colY.x; (*this)(0, 2) = _colZ.x;
	(*this)(1, 0) = _colX.y; (*this)(1, 1) = _colY.y; (*this)(1, 2) = _colZ.y;
	(*this)(2, 0) = _colX.z; (*this)(2, 1) = _colY.z; (*this)(2, 2) = _colZ.z;
}
Mat3::Mat3(const Mat4& _mat4)
{
	(*this)(0, 0) = _mat4(0, 0); (*this)(0, 1) = _mat4(0, 1); (*this)(0, 2) = _mat4(0, 2);
	(*this)(1, 0) = _mat4(1, 0); (*this)(1, 1) = _mat4(1, 1); (*this)(1, 2) = _mat4(1, 2);
	(*this)(2, 0) = _mat4(2, 0); (*this)(2, 1) = _mat4(2, 1); (*this)(2, 2) = _mat4(2, 2);
}
Vec3 Mat3::getCol(int _i) const
{
	return Vec3((*this)(0, _i), (*this)(1, _i), (*this)(2, _i));
}
Vec3 Mat3::getRow(int _i) const
{
	return Vec3((*this)(_i, 0), (*this)(_i, 1), (*this)(_i, 2));
}
void Mat3::setCol(int _i, const Vec3& _v)
{
	(*this)(0, _i) = _v.x;
	(*this)(1, _i) = _v.y;
	(*this)(2, _i) = _v.z;
}
void Mat3::setRow(int _i, const Vec3& _v)
{
	(*this)(_i, 0) = _v.x;
	(*this)(_i, 1) = _v.y;
	(*this)(_i, 2) = _v.z;
}
Vec3 Mat3::getScale() const
{
	return Vec3(Length(getCol(0)), Length(getCol(1)), Length(getCol(2)));
}
void Mat3::setScale(const Vec3& _scale)
{
	Vec3 scale = _scale / getScale();
	setCol(0, getCol(0) * scale.x);
	setCol(1, getCol(1) * scale.y);
	setCol(2, getCol(2) * scale.z);
}
Vec3 Im3d::ToEulerXYZ(const Mat3& _m)
{
 // http://www.staff.city.ac.uk/~sbbh653/publications/euler.pdf
	Vec3 ret;
	if_likely (fabs(_m(2, 0)) < 1.0f) {
		ret.y = -asinf(_m(2, 0));
		float c = 1.0f / cosf(ret.y);
		ret.x = atan2f(_m(2, 1) * c, _m(2, 2) * c);
		ret.z = atan2f(_m(1, 0) * c, _m(0, 0) * c);
	} else {
		ret.z = 0.0f;
		if (!(_m(2, 0) > -1.0f)) {
			ret.x = ret.z + atan2f(_m(0, 1), _m(0, 2));
			ret.y = HalfPi;
		} else {
			ret.x = -ret.z + atan2f(-_m(0, 1), -_m(0, 2));			
			ret.y = -HalfPi;
		}
	}
	return ret;
}
Mat3 Im3d::FromEulerXYZ(Vec3& _euler)
{
	float cx = cosf(_euler.x);
	float sx = sinf(_euler.x);
	float cy = cosf(_euler.y);
	float sy = cosf(_euler.y);
	float cz = cosf(_euler.z);
	float sz = cosf(_euler.z);
	return Mat3(
		cy * cz, sz * sy * cz - cx * sz, cx * sy * cz + sx * sz,
		cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz,
		    -sz,                sx * cy,                cx * cy		
		);
}
Mat3 Im3d::Transpose(const Mat3& _m)
{
	return Mat3(
		_m(0, 0), _m(1, 0), _m(2, 0),
		_m(0, 1), _m(1, 1), _m(2, 1),
		_m(0, 2), _m(1, 2), _m(2, 2)
		);
}
Mat3 Im3d::Rotation(const Vec3& _axis, float _rads)
{
	float c  = cosf(_rads);
	float rc = 1.0f - c;
	float s  = sinf(_rads);
	return Mat3(
		_axis.x * _axis.x + (1.0f - _axis.x * _axis.x) * c, _axis.x * _axis.y * rc - _axis.z * s,                _axis.x * _axis.z * rc + _axis.y * s,
		_axis.x * _axis.y * rc + _axis.z * s,               _axis.y * _axis.y + (1.0f - _axis.y * _axis.y) * c,  _axis.y * _axis.z * rc - _axis.x * s,
		_axis.x * _axis.z * rc - _axis.y * s,               _axis.y * _axis.z * rc + _axis.x * s,                _axis.z * _axis.z + (1.0f - _axis.z * _axis.z) * c
		);
}
Mat3 Im3d::Scale(const Vec3& _s)
{
	return Mat3(
		_s.x,  0.0f,  0.0f,
		0.0f,  _s.y,  0.0f,
		0.0f,  0.0f,  _s.z
		);
}


// Mat4
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
Mat4::Mat4(const Mat3& _mat3)
{
	(*this)(0, 0) = _mat3(0, 0); (*this)(0, 1) = _mat3(0, 1); (*this)(0, 2) = _mat3(0, 2); (*this)(0, 3) = 0.0f;
	(*this)(1, 0) = _mat3(1, 0); (*this)(1, 1) = _mat3(1, 1); (*this)(1, 2) = _mat3(1, 2); (*this)(1, 3) = 0.0f;
	(*this)(2, 0) = _mat3(2, 0); (*this)(2, 1) = _mat3(2, 1); (*this)(2, 2) = _mat3(2, 2); (*this)(2, 3) = 0.0f;
	(*this)(3, 0) =        0.0f; (*this)(3, 1) =        0.0f; (*this)(3, 2) =        0.0f; (*this)(3, 3) = 1.0f;
}
Mat4::Mat4(const Vec3& _translation, const Mat3& _rotation, const Vec3& _scale)
{
	setCol(0, Vec4(_rotation.getCol(0) * _scale.x, 0.0f));
	setCol(1, Vec4(_rotation.getCol(1) * _scale.y, 0.0f));
	setCol(2, Vec4(_rotation.getCol(2) * _scale.z, 0.0f));
	setCol(3, Vec4(_translation, 1.0f));
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
Vec3 Mat4::getTranslation() const
{
	return Vec3((*this)(0, 3), (*this)(1, 3), (*this)(2, 3));
}
void Mat4::setTranslation(const Vec3& _translation)
{
	(*this)(0, 3) = _translation.x;
	(*this)(1, 3) = _translation.y;
	(*this)(2, 3) = _translation.z;
}
Mat3 Mat4::getRotation() const
{
	Mat3 ret(*this);
	ret.setCol(0, Normalize(ret.getCol(0)));
	ret.setCol(1, Normalize(ret.getCol(1)));
	ret.setCol(2, Normalize(ret.getCol(2)));
	return ret;
}
void Mat4::setRotation(const Mat3& _rotation)
{
	Vec3 scale = getScale();
	setCol(0, Vec4(_rotation.getCol(0) * scale.x, 0.0f));
	setCol(1, Vec4(_rotation.getCol(1) * scale.y, 0.0f));
	setCol(2, Vec4(_rotation.getCol(2) * scale.z, 0.0f));
}
Vec3 Mat4::getScale() const
{
	return Vec3(Length(getCol(0)), Length(getCol(1)), Length(getCol(2)));
}
void Mat4::setScale(const Vec3& _scale)
{
	Vec3 scale = _scale / getScale();
	setCol(0, getCol(0) * scale.x);
	setCol(1, getCol(1) * scale.y);
	setCol(2, getCol(2) * scale.z);
}
inline static float Determinant(const Mat4& _m)
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
Mat4 Im3d::Translation(const Vec3& _t)
{
	return Mat4(
		1.0f, 0.0f, 0.0f, _t.x,
		0.0f, 1.0f, 0.0f, _t.y,
		0.0f, 0.0f, 1.0f, _t.z,
		0.0f, 0.0f, 0.0f, 1.0f
		);
}
Mat4 Im3d::AlignZ(const Vec3& _axis, const Vec3& _up)
{
	Vec3 x, y;
	y = _up - _axis * Dot(_up, _axis);
	float ylen = Length(y);
	if_unlikely (ylen < FLT_EPSILON) {
		Vec3 k = Vec3(1.0f, 0.0f, 0.0f);
		y = k - _axis * Dot(k, _axis);
		ylen = Length(y);
		if_unlikely (ylen < FLT_EPSILON) {
			k = Vec3(0.0f, 0.0f, 1.0f);
			y = k - _axis * Dot(k, _axis);
			ylen = Length(y);
		}
	}
	y = y / ylen;
	x = Cross(y, _axis);

	return Mat4(
		x.x,    y.x,    _axis.x,    0.0f,
		x.y,    y.y,    _axis.y,    0.0f,
		x.z,    y.z,    _axis.z,    0.0f
		);
}
Mat4 Im3d::LookAt(const Vec3& _from, const Vec3& _to, const Vec3& _up)
{
	Mat4 ret = AlignZ(Normalize(_to - _from), _up);
	ret.setCol(3, Vec4(_from, 1.0f)); // inject translation
	return ret;
}

// Geometry
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
	float s = sqrtf(r2 - p2); 
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

#define IM3D_STATIC_ASSERT(e) { (void)sizeof(char[(e) ? 1 : -1]); }
static void StaticAsserts()
{
	IM3D_STATIC_ASSERT(sizeof (Vec2) == sizeof (float[2]));
	IM3D_STATIC_ASSERT(alignof(Vec2) == alignof(float[2]));
	IM3D_STATIC_ASSERT(sizeof (Vec3) == sizeof (float[3]));
	IM3D_STATIC_ASSERT(alignof(Vec3) == alignof(float[3]));
	IM3D_STATIC_ASSERT(sizeof (Vec4) == sizeof (float[4]));
	IM3D_STATIC_ASSERT(alignof(Vec4) == alignof(float[4]));
	IM3D_STATIC_ASSERT(sizeof (Mat3) == sizeof (float[9]));
	IM3D_STATIC_ASSERT(alignof(Mat3) == alignof(float[9]));
	IM3D_STATIC_ASSERT(sizeof (Mat4) == sizeof (float[16]));
	IM3D_STATIC_ASSERT(alignof(Mat4) == alignof(float[16]));
}
