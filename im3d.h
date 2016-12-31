/*	Todo: use a set of flags to know whether or not you need to scale the alpha or apply the matrix per vertex.
*/
#pragma once
#ifndef im3d_h
#define im3d_h

#ifndef IM3D_API
	#define IM3D_API
#endif

namespace Im3d {

typedef unsigned int U32;

struct Vec2;
struct Vec3;
struct Vec4;
struct Mat4;
struct Color;
class  Context;

/// Get/set current context. 
IM3D_API Context& GetCurrentContext();
IM3D_API void     SetCurrentContext(Context& _ctx);

/// Begin primitive. End() *must* be called before starting a new primitive type.
IM3D_API void BeginPoints();
IM3D_API void BeginLines();
IM3D_API void BeginLineLoop();
IM3D_API void BeginLineStrip();
IM3D_API void BeginTriangles();
IM3D_API void BeginTriangleStrip();

/// End the current primitive.
IM3D_API void End();

/// Add a vertex to the current primitive (call between Begin*() and End()).
IM3D_API void Vertex(const Vec3& _position);
IM3D_API void Vertex(const Vec3& _position, Color _color);
IM3D_API void Vertex(const Vec3& _position, float _size);
IM3D_API void Vertex(const Vec3& _position, float _size, Color _color);
IM3D_API void Vertex(float _x, float _y, float _z);
IM3D_API void Vertex(float _x, float _y, float _z, Color _color);
IM3D_API void Vertex(float _x, float _y, float _z, float _size);
IM3D_API void Vertex(float _x, float _y, float _z, float _size, Color _color);

/// Current draw state (affects all subsequent primitives).
IM3D_API void  PushDrawState();
IM3D_API void  PopDrawState();
IM3D_API void  SetColor(Color _color);
IM3D_API Color GetColor();
IM3D_API void  SetAlpha(float _alpha);
IM3D_API float GetAlpha();
IM3D_API void  SetSize(float _size);
IM3D_API float GetSize();

/// Current transform state (affects all subsequent primitives).
IM3D_API void PushMatrix();
IM3D_API void PopMatrix();
IM3D_API void SetMatrix(const Mat4& _mat);
IM3D_API void SetIdentity();
IM3D_API void MulMatrix(const Mat4& _mat);
IM3D_API void Translate(float _x, float _y, float _z);
IM3D_API void Scale(float _x, float _y, float _z);

typedef U32  Id;
extern const Id kInvalidId;
IM3D_API Id MakeId(const char* _str);

/// Manipulate position/orientation/scale via a gizmo. Return true if the gizmo was used (modified its output).
IM3D_API bool Gizmo(const char* _id, Mat4* _mat_);
IM3D_API bool GizmoPosition(const char* _id, Vec3* _position_);

struct Vec2
{
	float x; float y; 
	Vec2()                                                                   {}
	Vec2(float _xy): x(_xy), y(_xy)                                          {}
	Vec2(float _x, float _y): x(_x), y(_y)                                   {}
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
};
struct Vec3
{ 
	float x; float y; float z; 
	Vec3()                                                                   {}
	Vec3(float _xyz): x(_xyz), y(_xyz), z(_xyz)                              {}
	Vec3(float _x, float _y, float _z): x(_x), y(_y), z(_z)                  {}
	Vec3(const Vec2& _xy, float _z): x(_xy.x), y(_xy.y), z(_z)               {}
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
};
struct Vec4
{ 
	float x; float y; float z; float w;
	Vec4()                                                                   {}
	Vec4(float _xyzw): x(_xyzw), y(_xyzw), z(_xyzw), w(_xyzw)                {}
	Vec4(float _x, float _y, float _z, float _w): x(_x), y(_y), z(_z), w(_w) {}
	Vec4(const Vec3& _xyz, float _w): x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
};
struct Mat4
{
	float m[16];
	Mat4()                                                                   {}
	Mat4(float _diagonal);
	Mat4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
		);
	operator float*()                                                        { return m; }
	operator const float*() const                                            { return m; }
};
struct Color
{
	U32 v;
	Color(): v(0)                                                            {}
	Color(U32 _rgba): v(_rgba)                                               {}
	Color(const Vec4& _rgba);
	Color(float _r, float _g, float _b, float _a);
	operator U32() const                                                     { return v; }
};
struct Vertex
{
	Vec4   m_positionSize; //< xyz = position, w = size
	Color  m_color;        //< rgba (MSB = r)
	
	Vertex(const Vec3& _position, float _size, Color _color)
		: m_positionSize(_position, _size)
		, m_color(_color)
	{
	}
};

////////////////////////////////////////////////////////////////////////////////
/// \class Context
////////////////////////////////////////////////////////////////////////////////
class Context
{
public:

private:

}; // class Context


inline void Vertex(float _x, float _y, float _z)                             { Vertex(Vec3(_x, _y, _z)); }
inline void Vertex(float _x, float _y, float _z, Color _color)               { Vertex(Vec3(_x, _y, _z), _color); }
inline void Vertex(float _x, float _y, float _z, float _size)                { Vertex(Vec3(_x, _y, _z), _size); }
inline void Vertex(float _x, float _y, float _z, float _size, Color _color)  { Vertex(Vec3(_x, _y, _z), _size, _color); }

} // namespac Im3d

#endif // im3d_h
