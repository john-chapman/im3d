#pragma once
#ifndef im3d_h
#define im3d_h

#include "im3d_config.h"

#define IM3D_VERSION "0.5"

#ifndef IM3D_ASSERT
	#include <cassert>
	#define IM3D_ASSERT(e) assert(e)
#endif

namespace Im3d {

typedef unsigned int U32;
typedef U32  Id;
struct Vec2;
struct Vec3;
struct Vec4;
struct Mat4;
struct Color;
struct VertexData;
struct AppData;
class  Context;

extern const Id    Id_Invalid;
extern const Color Color_Black;
extern const Color Color_White;
extern const Color Color_Red;
extern const Color Color_Green;
extern const Color Color_Blue;
extern const Color Color_Magenta;
extern const Color Color_Yellow;
extern const Color Color_Cyan;

Context& GetContext();
void     SetContext(Context& _ctx);
AppData& GetAppData();

// Call at the start of each frame, after filling the AppData struct.
void  NewFrame();
// Call after all Im3d calls have been made for the current frame.
void  Draw();

// Begin primitive. End() *must* be called before starting each new primitive type.
void  BeginPoints();
void  BeginLines();
void  BeginLineLoop();
void  BeginLineStrip();
void  BeginTriangles();
void  BeginTriangleStrip();
// End the current primitive.
void  End();

// Higher-order shapes.
void  DrawXyzAxes();
void  DrawSphere(const Vec3& _origin, float _radius, int _detail = 24);
void  DrawAlignedBox(const Vec3& _min, const Vec3& _max);
void  DrawCylinder(const Vec3& _start, const Vec3& _end, float _radius, int _detail = 24);
void  DrawCapsule(const Vec3& _start, const Vec3& _end, float _radius, int _detail = 12);

// Add a vertex to the current primitive (call between Begin*() and End()).
void  Vertex(const Vec3& _position);
void  Vertex(const Vec3& _position, Color _color);
void  Vertex(const Vec3& _position, float _size);
void  Vertex(const Vec3& _position, float _size, Color _color);
void  Vertex(float _x, float _y, float _z);
void  Vertex(float _x, float _y, float _z, Color _color);
void  Vertex(float _x, float _y, float _z, float _size);
void  Vertex(float _x, float _y, float _z, float _size, Color _color);

// Current draw state (affects all subsequent primitives).
void  PushDrawState(); // color, alpha, size, sorting
void  PopDrawState();
void  SetColor(Color _color);
void  SetColor(float _r, float _g, float _b, float _a = 1.0f);
Color GetColor();
void  SetAlpha(float _alpha);
float GetAlpha();
void  SetSize(float _size);
float GetSize();
void  EnableSorting(bool _enable);

// Current transform state (affects all subsequent primitives).
void  PushMatrix();
void  PopMatrix();
void  SetMatrix(const Mat4& _mat);
void  SetIdentity();
void  MulMatrix(const Mat4& _mat);
void  Translate(float _x, float _y, float _z);
void  Scale(float _x, float _y, float _z);

// Generate an Id from a null-terminated string.
Id    MakeId(const char* _str);

// Manipulate position/orientation/scale via a gizmo. Return true if the gizmo was used (if it modified its output).
bool  Gizmo(const char* _id, Mat4* _mat_);
bool  GizmoPosition(const char* _id, Vec3* _position_);

struct Vec2
{
	float x, y; 
	Vec2()                                                                   {}
	Vec2(float _xy): x(_xy), y(_xy)                                          {}
	Vec2(float _x, float _y): x(_x), y(_y)                                   {}
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
	#ifdef IM3D_VEC2_USER
		IM3D_VEC2_USER
	#endif
};
struct Vec3
{ 
	float x, y, z;
	Vec3()                                                                   {}
	Vec3(float _xyz): x(_xyz), y(_xyz), z(_xyz)                              {}
	Vec3(float _x, float _y, float _z): x(_x), y(_y), z(_z)                  {}
	Vec3(const Vec2& _xy, float _z): x(_xy.x), y(_xy.y), z(_z)               {}
	Vec3(const Vec4& _v); // discards w
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
	#ifdef IM3D_VEC3_USER
		IM3D_VEC3_USER
	#endif
};
struct Vec4
{ 
	float x, y, z, w;
	Vec4()                                                                   {}
	Vec4(float _xyzw): x(_xyzw), y(_xyzw), z(_xyzw), w(_xyzw)                {}
	Vec4(float _x, float _y, float _z, float _w): x(_x), y(_y), z(_z), w(_w) {}
	Vec4(const Vec3& _xyz, float _w): x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
	Vec4(Color _rgba);
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
	#ifdef IM3D_VEC4_USER
		IM3D_VEC4_USER
	#endif
};
struct Mat4
{
	float m[16]; // column-major unless IM3D_MATRIX_ROW_MAJOR defined
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

	Vec4   getCol(int _i) const;
	Vec4   getRow(int _i) const;
	
	float  operator()(int _row, int _col) const
	{
	#ifdef IM3D_MATRIX_ROW_MAJOR
		return m[_row * 4 + _col];
	#else
		return m[_col * 4 + _row];
	#endif
	}
	float& operator()(int _row, int _col)
	{ 
	#ifdef IM3D_MATRIX_ROW_MAJOR
		return m[_row * 4 + _col];
	#else
		return m[_col * 4 + _row];
	#endif
	}
	
	#ifdef IM3D_MAT4_USER
		IM3D_MAT4_USER
	#endif
};
struct Color
{
	U32 v;
	Color(): v(0)                                                            {}
	Color(U32 _rgba): v(_rgba)                                               {}
	Color(const Vec4& _rgba);
	Color(float _r, float _g, float _b, float _a = 1.0f);
	operator U32() const                                                     { return v; }

	void set(int _i, float _val)
	{
		_i *= 8;
		U32 mask = 0xff << _i;
		v = (v & ~mask) | ((U32)(_val * 255.0f) << _i);
	}
	void setR(float _val)                                                    { set(3, _val); }
	void setG(float _val)                                                    { set(2, _val); }
	void setB(float _val)                                                    { set(1, _val); }
	void setA(float _val)                                                    { set(0, _val); }

	float get(int _i) const
	{
		_i *= 8;
		U32 mask = 0xff << _i;
		return (float)((v & mask) >> _i) / 255.0f;
	}
	float getR() const                                                       { return get(3); }
	float getG() const                                                       { return get(2); }
	float getB() const                                                       { return get(1); }
	float getA() const                                                       { return get(0); }
};

struct VertexData
{
	Vec4   m_positionSize; // xyz = position, w = size
	Color  m_color;        // rgba8 (MSB = r)
	
	VertexData() {}
	VertexData(const Vec3& _position, float _size, Color _color): m_positionSize(_position, _size), m_color(_color) {}
};

enum DrawPrimitiveType
{
	DrawPrimitive_Points,
	DrawPrimitive_Lines,
	DrawPrimitive_Triangles,

	DrawPrimitive_Count
};
struct DrawList
{
	DrawPrimitiveType m_primType;
	const VertexData* m_vertexData;
	U32               m_vertexCount;
};
typedef void (DrawPrimitivesCallback)(const DrawList& _drawList);

enum Key
{
	MouseLeft,
	MouseRight,
	MouseMiddle,
		
	Key_Ctrl,
	Key_Shift,
	Key_Alt,

	Key_T,           // Select translation gizmo.
	Key_R,           // Select rotation gizmo.
	Key_S,           // Select scale gizmo.

	Key_Count
};
struct AppData
{
	bool  m_keyDown[Key_Count];  // Application-provided key states.

	Vec3  m_cursorRayOrigin;     // World space cursor ray origin.
	Vec3  m_cursorRayDirection;  // World space cursor ray direction.
	Vec3  m_viewOrigin;          // World space render origin (camera position).
	Vec2  m_viewportSize;        // Viewport size (pixels).
	float m_tanHalfFov;          // tan(fov/2); fov = vertical field of view of the current projection.
	float m_deltaTime;           // Time since previous frame (seconds).
	void* m_userData;            // App-specific data (useful for passing app context to drawPrimitives).

	DrawPrimitivesCallback* drawCallback;
};

// Minimal vector.
template <typename T>
class Vector
{
	T*   m_data;
	U32  m_size;
	U32  m_capacity;

public:
	Vector(): m_size(0), m_capacity(0), m_data(0) {}
	~Vector();

	T&       operator[](U32 _i)                   { IM3D_ASSERT(_i < m_size); return m_data[_i]; }
	const T& operator[](U32 _i) const             { IM3D_ASSERT(_i < m_size); return m_data[_i]; }
	T*       data()                               { return m_data; }
	const T* data() const                         { return m_data; }

	void     push_back(const T& _v)               { if (m_size == m_capacity) { reserve(m_capacity + m_capacity / 2); } m_data[m_size++] = _v; }
	void     pop_back()                           { IM3D_ASSERT(m_size > 0); --m_size; }

	T*       begin()                              { return m_data; }
	const T* begin() const                        { return m_data; }
	T*       end()                                { return m_data + m_size; }
	const T* end() const                          { return m_data + m_size; }
	T&       front()                              { IM3D_ASSERT(m_size > 0); return m_data[0]; }
	const T& front() const                        { IM3D_ASSERT(m_size > 0); return m_data[0]; }
	T&       back()                               { IM3D_ASSERT(m_size > 0); return m_data[m_size - 1]; }
	const T& back() const                         { IM3D_ASSERT(m_size > 0); return m_data[m_size - 1]; }

	U32      size() const                         { return m_size; }
	U32      capacity() const                     { return m_capacity; }
	bool     empty() const                        { return m_size == 0; }

	void     clear()                              { m_size = 0; }
	void     reserve(U32 _capacity);
	void     resize(U32 _size, const T& _val);

	template <typename T>
	friend void swap(Vector<T>& _a_, Vector<T>& _b_);
};

// Context stores all relevant state - main interface affects the context currently bound via SetCurrentContext().
class Context
{
public:
	enum PrimitiveMode
	{
		PrimitiveMode_None,
		PrimitiveMode_Points,
		PrimitiveMode_Lines,
		PrimitiveMode_LineStrip,
		PrimitiveMode_LineLoop,
		PrimitiveMode_Triangles,
		PrimitiveMode_TriangleStrip
	};
	void        begin(PrimitiveMode _mode);
	void        end();
	void        vertex(const Vec3& _position, float _size, Color _color);
	void        vertex(const Vec3& _position )   { vertex(_position, getSize(), getColor()); }

	void        reset();
	void        draw();


	void        pushColor(Color _color)          { m_colorStack.push_back(_color); }
	void        popColor()                       { m_colorStack.pop_back();        }
	void        setColor(Color _color)           { m_colorStack.back() = _color;   }
	Color       getColor() const                 { return m_colorStack.back();     }

	void        pushAlpha(float _alpha)          { m_alphaStack.push_back(_alpha); }
	void        popAlpha()                       { m_alphaStack.pop_back();        }
	void        setAlpha(float _alpha)           { m_alphaStack.back() = _alpha;   }
	float       getAlpha() const                 { return m_alphaStack.back();     }

	void        pushSize(float _size)            { m_sizeStack.push_back(_size); }
	void        popSize()                        { m_sizeStack.pop_back();       }
	void        setSize(float _size)             { m_sizeStack.back() = _size;   }
	float       getSize() const                  { return m_sizeStack.back();    }

	void        pushEnableSorting(bool _enable);
	void        popEnableSorting();
	void        setEnableSorting(bool _enable);
	bool        getEnableSorting() const         { return m_enableSortingStack.back();      }

	void        pushMatrix(const Mat4& _mat)     { m_matrixStack.push_back(_mat); }
	void        popMatrix()                      { m_matrixStack.pop_back();      }
	void        setMatrix(const Mat4& _mat)      { m_matrixStack.back() = _mat;   }
	const Mat4& getMatrix() const                { return m_matrixStack.back();   }

	void        pushId(Id _id)                   { m_idStack.push_back(_id); }
	void        popId()                          { m_idStack.pop_back();     }
	void        setId(Id _id)                    { m_idStack.back() = _id;   }
	Id          getId() const                    { return m_idStack.back();  }

	AppData&    getAppData()                     { return m_appData; }

	Context();
	~Context();

private:
 // state stacks
	Vector<Color>      m_colorStack;
	Vector<float>      m_alphaStack;
	Vector<float>      m_sizeStack;
	Vector<bool>       m_enableSortingStack;
	Vector<Mat4>       m_matrixStack;
	Vector<Id>         m_idStack;

 // primitive data: [0] unsorted, [1] sorted
	Vector<VertexData> m_vertexData[DrawPrimitive_Count][2];
	Vector<DrawList>   m_sortedDrawLists;
	bool               m_sortCalled;               // Prevent sorting during every call to draw().
	bool               m_drawCalled;               // For assert if primitives are pushed after draw() was called.

 // primitive state
	PrimitiveMode      m_primMode;   
	int                m_primList;                 // 1 if sorting enabled, else 0.
	U32                m_firstVertThisPrim;        // Index of the first vertex pushed during this primitive.
	U32                m_vertCountThisPrim;        // # calls to vertex() since the last call to begin().

 // gizmo state
	Id                 m_idActive;
	Id                 m_idHot;

 // app data
	AppData            m_appData;
	bool               m_keyDownCurr[Key_Count];  // Key state captured during reset().
	bool               m_keyDownPrev[Key_Count];  // Key state from previous frame.

	// Interpret key state.
	bool isKeyDown(Key _key) const     { return m_keyDownCurr[_key]; }
	bool wasKeyPressed(Key _key) const { return m_keyDownCurr[_key] && !m_keyDownPrev[_key]; }

	// Sort primitive data.
	void sort();

	// Convert pixels -> world space size based on distance between _position and view origin.
	float pixelsToWorldSize(const Vec3& _position, float _pixels);
};

namespace internal {
	extern Context* g_CurrentContext;
}

inline Context& GetContext()                                                 { return *internal::g_CurrentContext; }
inline void     SetContext(Context& _ctx)                                    { internal::g_CurrentContext = &_ctx; }
inline AppData& GetAppData()                                                 { return GetContext().getAppData();   }
inline void     NewFrame()                                                   { GetContext().reset();               }
inline void     Draw()                                                       { GetContext().draw();                }

inline void  BeginPoints()                                                   { GetContext().begin(Context::PrimitiveMode_Points);        }
inline void  BeginLines()                                                    { GetContext().begin(Context::PrimitiveMode_Lines);         }
inline void  BeginLineLoop()                                                 { GetContext().begin(Context::PrimitiveMode_LineLoop);      }
inline void  BeginLineStrip()                                                { GetContext().begin(Context::PrimitiveMode_LineStrip);     }
inline void  BeginTriangles()                                                { GetContext().begin(Context::PrimitiveMode_Triangles);     }
inline void  BeginTriangleStrip()                                            { GetContext().begin(Context::PrimitiveMode_TriangleStrip); }
inline void  End()                                                           { GetContext().end(); }

inline void  Vertex(const Vec3& _position)                                   { GetContext().vertex(_position, GetContext().getSize(), GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, Color _color)                     { GetContext().vertex(_position, GetContext().getSize(), _color); }
inline void  Vertex(const Vec3& _position, float _size)                      { GetContext().vertex(_position, _size, GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, float _size, Color _color)        { GetContext().vertex(_position, _size, _color); }
inline void  Vertex(float _x, float _y, float _z)                            { Vertex(Vec3(_x, _y, _z)); }
inline void  Vertex(float _x, float _y, float _z, Color _color)              { Vertex(Vec3(_x, _y, _z), _color); }
inline void  Vertex(float _x, float _y, float _z, float _size)               { Vertex(Vec3(_x, _y, _z), _size); }
inline void  Vertex(float _x, float _y, float _z, float _size, Color _color) { Vertex(Vec3(_x, _y, _z), _size, _color); }

inline void  PushDrawState()                                                 { Context& ctx = GetContext(); ctx.pushColor(ctx.getColor()); ctx.pushAlpha(ctx.getAlpha()); ctx.pushSize(ctx.getSize()); ctx.pushEnableSorting(ctx.getEnableSorting()); }
inline void  PopDrawState()                                                  { Context& ctx = GetContext(); ctx.popColor(); ctx.popAlpha(); ctx.popSize(); ctx.popEnableSorting(); }
inline void  PushColor()                                                     { GetContext().pushColor(GetContext().getColor()); }
inline void  PopColor()                                                      { GetContext().popColor();                         }
inline void  SetColor(Color _color)                                          { GetContext().setColor(_color);                   }
inline void  SetColor(float _r, float _g, float _b, float _a)                { GetContext().setColor(Color(_r, _g, _b, _a));    }
inline Color GetColor()                                                      { return GetContext().getColor();                  }
inline void  PushAlpha()                                                     { GetContext().pushAlpha(GetContext().getAlpha()); }
inline void  PopAlpha()                                                      { GetContext().popAlpha();        }
inline void  SetAlpha(float _alpha)                                          { GetContext().setAlpha(_alpha);  }
inline float GetAlpha()                                                      { return GetContext().getAlpha(); }
inline void  PushSize()                                                      { GetContext().pushSize(GetContext().getAlpha()); }
inline void  PopSize()                                                       { GetContext().popSize();         }
inline void  SetSize(float _size)                                            { GetContext().setSize(_size);    }
inline float GetSize()                                                       { return GetContext().getSize();  }
inline void  EnableSorting(bool _enable)                                     { GetContext().setEnableSorting(_enable); }

inline void  PushMatrix()                                                    { GetContext().pushMatrix(GetContext().getMatrix()); }
inline void  PopMatrix()                                                     { GetContext().popMatrix();           }
inline void  SetMatrix(const Mat4& _mat)                                     { GetContext().setMatrix(_mat);       }
inline void  SetIdentity()                                                   { GetContext().setMatrix(Mat4(1.0f)); }

} // namespac Im3d

#endif // im3d_h
