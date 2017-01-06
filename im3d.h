#pragma once
#ifndef im3d_h
#define im3d_h

#ifndef IM3D_ASSERT
	#include <assert.h>
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

extern const Id    kId_Invalid;
extern const Color kColor_Black;
extern const Color kColor_White;
extern const Color kColor_Red;
extern const Color kColor_Green;
extern const Color kColor_Blue;
extern const Color kColor_Magenta;
extern const Color kColor_Yellow;
extern const Color kColor_Cyan;



// Begin primitive. End() *must* be called before starting each new primitive type.
void  BeginPoints();
void  BeginLines();
void  BeginLineLoop();
void  BeginLineStrip();
void  BeginTriangles();
void  BeginTriangleStrip();
// End the current primitive.
void  End();

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
void  EnableSorting(bool _enable);
void  PushDrawState();
void  PopDrawState();
void  SetColor(Color _color);
Color GetColor();
void  SetAlpha(float _alpha);
float GetAlpha();
void  SetSize(float _size);
float GetSize();

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

// Manipulate position/orientation/scale via a gizmo. Return true if the gizmo was used (it modified its output).
bool  Gizmo(const char* _id, Mat4* _mat_);
bool  GizmoPosition(const char* _id, Vec3* _position_);

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
	Vec4(Color _rgba);
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
	kDrawPrimitive_Points,
	kDrawPrimitive_Lines,
	kDrawPrimitive_Triangles
};
typedef void (DrawPrimitivesCallback)(DrawPrimitiveType _primType, const VertexData* _data, U32 _count);

enum Key
{
	kMouseLeft,
	kMouseRight,
	kMouseMiddle,
		
	kKey_Ctrl,
	kKey_Shift,
	kKey_Alt,

	kKey_T,           // Select translation gizmo.
	kKey_R,           // Select rotation gizmo.
	kKey_S,           // Select scale gizmo.

	kKey_Count
};
struct AppData
{
	bool  m_keyDown[kKey_Count]; // Application-provided key states.

	Vec3  m_cursorRayOrigin;     // World space cursor ray origin.
	Vec3  m_cursorRayDirection;  // World space cursor ray direction.
	Vec3  m_viewOrigin;          // World space render origin (camera position).
	Vec2  m_displaySize;         // Viewport size (pixels).
	float m_tanHalfFov;          // tan(fov/2); fov = vertical field of view of the current projection.
	float m_deltaTime;           // Time since previous frame (seconds).

	DrawPrimitivesCallback* drawPrimitives;
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
};

// Context stores all relevant state - main interface affects the context currently bound via SetCurrentContext().
class Context
{
public:
	enum PrimitiveMode
	{
		kPrimitiveMode_None,
		kPrimitiveMode_Points,
		kPrimitiveMode_Lines,
		kPrimitiveMode_LineStrip,
		kPrimitiveMode_LineLoop,
		kPrimitiveMode_Triangles,
		kPrimitiveMode_TriangleStrip
	};
	void        begin(PrimitiveMode _mode);
	void        end();
	void        vertex(const Vec3& _position, float _size, Color _color);

	void        reset();
	void        draw();

	void        enableSorting(bool _enable);

	void        pushColor(Color _color)          { m_colorStack.push_back(_color); }
	void        popColor()                       { m_colorStack.pop_back();        }
	void        setColor(Color _color)           { m_colorStack.back() = _color;   }
	Color       getColor() const                 { return m_colorStack.back();     }

	void        pushAlpha(float _alpha)          { m_alphaStack.push_back(_alpha); }
	void        popAlpha()                       { m_alphaStack.pop_back();        }
	void        setAlpha(float _alpha)           { m_alphaStack.back() = _alpha;   }
	float       getAlpha() const                 { return m_alphaStack.back();     }

	void        pushSize(float _size)            { m_sizeStack.push_back(_size);   }
	void        popSize()                        { m_sizeStack.pop_back();         }
	void        setSize(float _size)             { m_sizeStack.back() = _size;     }
	float       getSize() const                  { return m_sizeStack.back();      }

	void        pushMatrix(const Mat4& _mat)     { m_matrixStack.push_back(_mat);  }
	void        popMatrix()                      { m_matrixStack.pop_back();       }
	void        setMatrix(const Mat4& _mat)      { m_matrixStack.back() = _mat;    }
	const Mat4& getMatrix() const                { return m_matrixStack.back();    }

	void        pushId(Id _id)                   { m_idStack.push_back(_id);       }
	void        popId()                          { m_idStack.pop_back();           }
	void        setId(Id _id)                    { m_idStack.back() = _id;         }
	Id          getId() const                    { return m_idStack.back();        }

	AppData&    getAppData()                     { return m_appData;               }

	Context();
	~Context();

private:
 // state stacks
	Vector<Color>      m_colorStack;
	Vector<float>      m_alphaStack;
	Vector<float>      m_sizeStack;
	Vector<Mat4>       m_matrixStack;
	Vector<Id>         m_idStack;

 // primitive data: [0] unsorted, [1] sorted
	Vector<VertexData> m_points[2];
	Vector<VertexData> m_lines[2];
	Vector<VertexData> m_triangles[2];

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
	bool               m_keyDownCurr[kKey_Count];  // Key state captured during reset().
	bool               m_keyDownPrev[kKey_Count];  // Key state from previous frame.

	// Interpret key state.
	bool isKeyDown(Key _key) const     { return m_keyDownCurr[_key]; }
	bool wasKeyPressed(Key _key) const { return m_keyDownCurr[_key] && !m_keyDownPrev[_key]; }

	// Convert pixels -> world space size based on distance between _position and view origin.
	float pixelsToWorldSize(const Vec3& _position, float _pixels);
};

namespace internal {
	extern Context* g_CurrentContext;
}

inline Context& GetContext()                                                 { return *internal::g_CurrentContext; }
inline void     SetContext(Context& _ctx)                                    { internal::g_CurrentContext = &_ctx; }
inline AppData& GetAppData()                                                 { return GetContext().getAppData();   }

inline void  EnableSorting(bool _enable)                                     { GetContext().enableSorting(_enable);                       }
inline void  BeginPoints()                                                   { GetContext().begin(Context::kPrimitiveMode_Points);        }
inline void  BeginLines()                                                    { GetContext().begin(Context::kPrimitiveMode_Lines);         }
inline void  BeginLineLoop()                                                 { GetContext().begin(Context::kPrimitiveMode_LineLoop);      }
inline void  BeginLineStrip()                                                { GetContext().begin(Context::kPrimitiveMode_LineStrip);     }
inline void  BeginTriangles()                                                { GetContext().begin(Context::kPrimitiveMode_Triangles);     }
inline void  BeginTriangleStrip()                                            { GetContext().begin(Context::kPrimitiveMode_TriangleStrip); }
inline void  End()                                                           { GetContext().end(); }

inline void  Vertex(const Vec3& _position)                                   { GetContext().vertex(_position, GetContext().getSize(), GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, Color _color)                     { GetContext().vertex(_position, GetContext().getSize(), _color); }
inline void  Vertex(const Vec3& _position, float _size)                      { GetContext().vertex(_position, _size, GetContext().getColor()); }
inline void  Vertex(const Vec3& _position, float _size, Color _color)        { GetContext().vertex(_position, _size, _color); }
inline void  Vertex(float _x, float _y, float _z)                            { Vertex(Vec3(_x, _y, _z)); }
inline void  Vertex(float _x, float _y, float _z, Color _color)              { Vertex(Vec3(_x, _y, _z), _color); }
inline void  Vertex(float _x, float _y, float _z, float _size)               { Vertex(Vec3(_x, _y, _z), _size); }
inline void  Vertex(float _x, float _y, float _z, float _size, Color _color) { Vertex(Vec3(_x, _y, _z), _size, _color); }

inline void  PushColor()                                                     { GetContext().pushColor(GetContext().getColor()); }
inline void  PopColor()                                                      { GetContext().popColor();        }
inline void  SetColor(Color _color)                                          { GetContext().setColor(_color);  }
inline Color GetColor()                                                      { return GetContext().getColor(); }

inline void  PushAlpha()                                                     { GetContext().pushAlpha(GetContext().getAlpha()); }
inline void  PopAlpha()                                                      { GetContext().popAlpha();        }
inline void  SetAlpha(float _alpha)                                          { GetContext().setAlpha(_alpha);  }
inline float GetAlpha()                                                      { return GetContext().getAlpha(); }

inline void  PushSize()                                                      { GetContext().pushSize(GetContext().getAlpha()); }
inline void  PopSize()                                                       { GetContext().popSize();         }
inline void  SetSize(float _size)                                            { GetContext().setSize(_size);    }
inline float GetSize()                                                       { return GetContext().getSize();  }

inline void  PushMatrix()                                                    { GetContext().pushMatrix(GetContext().getMatrix()); }
inline void  PopMatrix()                                                     { GetContext().popMatrix();           }
inline void  SetMatrix(const Mat4& _mat)                                     { GetContext().setMatrix(_mat);       }
inline void  SetIdentity()                                                   { GetContext().setMatrix(Mat4(1.0f)); }

} // namespac Im3d

#endif // im3d_h
