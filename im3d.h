#pragma once
#ifndef im3d_h
#define im3d_h

#ifndef IM3D_API
	#define IM3D_API
#endif
#ifndef IM3D_ASSERT
	#include <assert.h>
	#define IM3D_ASSERT(e) assert(e)
#endif

namespace Im3d {

typedef unsigned int U32;

struct Vec2;
struct Vec3;
struct Vec4;
struct Mat4;
struct Color;
struct VertexData;
class  Context;

extern const Color kColorBlack;
extern const Color kColorWhite;
extern const Color kColorRed;
extern const Color kColorGreen;
extern const Color kColorBlue;
extern const Color kColorCyan;
extern const Color kColorMagenta;
extern const Color kColorYellow;

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


/// IDs are used to identify gizmos.
typedef U32  Id;
extern const Id kInvalidId;
IM3D_API Id MakeId(const char* _str);

/// Manipulate position/orientation/scale via a gizmo. Return true if the gizmo was used (modified its output).
IM3D_API bool Gizmo(const char* _id, Mat4* _mat_);
IM3D_API bool GizmoPosition(const char* _id, Vec3* _position_);


enum DrawPrimitiveType
{
	kDrawPrimitive_Points,
	kDrawPrimitive_Lines,
	kDrawPrimitive_Triangles
};
typedef void (DrawPrimitives)(DrawPrimitiveType _primType, const VertexData* _data, U32 _count);

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
	Color(float _r, float _g, float _b, float _a = 1.0f);
	operator U32() const                                                     { return v; }

	void set(int _i, float _val)
	{
		_i *= 8;
		U32 mask = ((1 << 8) - 1) << _i;
		v = (v & ~mask) | ((U32)(_val * 255.0f) << _i);
	}
	void setR(float _val)                                                    { set(3, _val); }
	void setG(float _val)                                                    { set(2, _val); }
	void setB(float _val)                                                    { set(1, _val); }
	void setA(float _val)                                                    { set(0, _val); }

	float get(int _i) const
	{
		_i *= 8;
		U32 mask = ((1 << 8) - 1) << _i;
		return (float)((v & ~mask) >> _i) / 255.0f;
	}
	float getR() const                                                       { return get(3); }
	float getG() const                                                       { return get(2); }
	float getB() const                                                       { return get(1); }
	float getA() const                                                       { return get(0); }
};
struct VertexData
{
	Vec4   m_positionSize; //< xyz = position, w = size
	Color  m_color;        //< rgba (MSB = r)
	
	VertexData() {}
	VertexData(const Vec3& _position, float _size, Color _color)
		: m_positionSize(_position, _size)
		, m_color(_color)
	{
	}
};


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
};


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

	Context();
	~Context();

private:
 // state stacks
	Vector<Color>      m_colorStack;
	Vector<float>      m_alphaStack;
	Vector<float>      m_sizeStack;
	Vector<Mat4>       m_matrixStack;
	Vector<Id>         m_idStack;

 // primitive data: 0 unsorted, 1 sorted
	Vector<VertexData> m_points[2];
	Vector<VertexData> m_lines[2];
	Vector<VertexData> m_triangles[2];

 // primitive state
	PrimitiveMode      m_primMode;   
	int                m_primList;             //< 0, or 1 if sorting enabled.
	U32                m_firstVertThisPrim;    //< Index of the first vertex pushed during this primitive.
	U32                m_vertCountThisPrim;    //< # calls to vertex() since the last call to begin().

 // callbacks
	DrawPrimitives*    drawPrimitives;

}; // class Context

namespace internal {
	extern Context* g_CurrentContext;
} // namespace internal



inline Context& GetCurrentContext()                                          { return *internal::g_CurrentContext; }
inline void     SetCurrentContext(Context& _ctx)                             { internal::g_CurrentContext = &_ctx; }

inline void BeginPoints()                                                    { GetCurrentContext().begin(Context::kPoints);        }
inline void BeginLines()                                                     { GetCurrentContext().begin(Context::kLines);         }
inline void BeginLineLoop()                                                  { GetCurrentContext().begin(Context::kLineLoop);      }
inline void BeginLineStrip()                                                 { GetCurrentContext().begin(Context::kLineStrip);     }
inline void BeginTriangles()                                                 { GetCurrentContext().begin(Context::kTriangles);     }
inline void BeginTriangleStrip()                                             { GetCurrentContext().begin(Context::kTriangleStrip); }
inline void End()                                                            { GetCurrentContext().end(); }

inline void Vertex(const Vec3& _position)                                    { GetCurrentContext().vertex(_position, GetCurrentContext().getSize(), GetCurrentContext().getColor()); }
inline void Vertex(const Vec3& _position, Color _color)                      { GetCurrentContext().vertex(_position, GetCurrentContext().getSize(), _color); }
inline void Vertex(const Vec3& _position, float _size)                       { GetCurrentContext().vertex(_position, _size, GetCurrentContext().getColor()); }
inline void Vertex(const Vec3& _position, float _size, Color _color)         { GetCurrentContext().vertex(_position, _size, _color); }
inline void Vertex(float _x, float _y, float _z)                             { Vertex(Vec3(_x, _y, _z)); }
inline void Vertex(float _x, float _y, float _z, Color _color)               { Vertex(Vec3(_x, _y, _z), _color); }
inline void Vertex(float _x, float _y, float _z, float _size)                { Vertex(Vec3(_x, _y, _z), _size); }
inline void Vertex(float _x, float _y, float _z, float _size, Color _color)  { Vertex(Vec3(_x, _y, _z), _size, _color); }

inline void  PushColor()                                                     { GetCurrentContext().pushColor(GetCurrentContext().getColor()); }
inline void  PopColor()                                                      { GetCurrentContext().popColor();        }
inline void  SetColor(Color _color)                                          { GetCurrentContext().setColor(_color);  }
inline Color GetColor()                                                      { return GetCurrentContext().getColor(); }

inline void  PushAlpha()                                                     { GetCurrentContext().pushAlpha(GetCurrentContext().getAlpha()); }
inline void  PopAlpha()                                                      { GetCurrentContext().popAlpha();        }
inline void  SetAlpha(float _alpha)                                          { GetCurrentContext().setAlpha(_alpha);  }
inline float GetAlpha()                                                      { return GetCurrentContext().getAlpha(); }

inline void  PushSize()                                                      { GetCurrentContext().pushSize(GetCurrentContext().getAlpha()); }
inline void  PopSize()                                                       { GetCurrentContext().popSize();         }
inline void  SetSize(float _size)                                            { GetCurrentContext().setSize(_size);    }
inline float GetSize()                                                       { return GetCurrentContext().getSize();  }

inline void  PushMatrix()                                                    { GetCurrentContext().pushMatrix(GetCurrentContext().getMatrix()); }
inline void  PopMatrix()                                                     { GetCurrentContext().popMatrix();           }
inline void  SetMatrix(const Mat4& _mat)                                     { GetCurrentContext().setMatrix(_mat);       }
inline void  SetIdentity()                                                   { GetCurrentContext().setMatrix(Mat4(1.0f)); }

} // namespac Im3d

#endif // im3d_h
