#pragma once
#ifndef im3d_h
#define im3d_h

#include "im3d_config.h"

#define IM3D_VERSION "1.10"

#ifndef IM3D_ASSERT
	#include <cassert>
	#define IM3D_ASSERT(e) assert(e)
#endif

#ifndef IM3D_VERTEX_ALIGNMENT
	#define IM3D_VERTEX_ALIGNMENT 4
#endif

namespace Im3d {

typedef unsigned int U32;
typedef U32 Id;
struct Vec2;
struct Vec3;
struct Vec4;
struct Mat3;
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

// Get AppData struct from the current context, fill before calling NewFrame().
AppData& GetAppData();

// Call at the start of each frame, after filling the AppData struct.
void  NewFrame();
// Call after all Im3d calls have been made for the current frame.
void  Draw();

// Begin/end primitive. End() must be called before starting each new primitive type.
void  BeginPoints();
void  BeginLines();
void  BeginLineLoop();
void  BeginLineStrip();
void  BeginTriangles();
void  BeginTriangleStrip();
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

// Color draw state (per vertex).
void  PushColor(); // push the stack top
void  PushColor(Color _color);
void  PopColor();
void  SetColor(Color _color);
void  SetColor(float _r, float _g, float _b, float _a = 1.0f);
Color GetColor();

// Alpha draw state, multiplies the alpha set by the color draw state (per vertex).
void  PushAlpha(); // push the stack top
void  PushAlpha(float _alpha);
void  PopAlpha();
void  SetAlpha(float _alpha);
float GetAlpha();

// Size draw state, for points/lines this is the radius/width in pixels (per vertex).
void  PushSize(); // push the stack top
void  PushSize(float _size);
void  PopSize();
void  SetSize(float _size);
float GetSize();

// Sorting draw state, enable depth sorting between primitives (per primitive).
void  PushEnableSorting(); // push the stack top
void  PushEnableSorting(bool _enable);
void  PopEnableSorting();
void  EnableSorting(bool _enable);

// Push/pop all draw states (color, alpha, size, sorting).
void  PushDrawState();
void  PopDrawState();

// Layer id state, subsequent primitives are added to a separate draw list associated with the id (per primitive).
void  PushLayerId(Id _layer);
void  PopLayerId();
Id    GetLayerId();

// Transform state (per vertex).
void  PushMatrix(); // push stack top
void  PushMatrix(const Mat4& _mat4);
void  PopMatrix();
void  SetMatrix(const Mat4& _mat4);
void  SetIdentity();
void  MulMatrix(const Mat4& _mat4);
void  Translate(float _x, float _y, float _z);
void  Rotate(const Vec3& _axis, float _angle);
void  Rotate(const Mat3& _rotation);
void  Scale(float _x, float _y, float _z);

// High order shapes. Where _detail = -1, an automatic level of detail is chosen based on the distance to the view origin (as specified via the AppData struct).
void  DrawXyzAxes();
void  DrawPoint(const Vec3& _position, float _size, Color _color);
void  DrawLine(const Vec3& _a, const Vec3& _b, float _size, Color _color);
void  DrawQuad(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d);
void  DrawQuad(const Vec3& _origin, const Vec3& _normal, const Vec2& _size);
void  DrawQuadFilled(const Vec3& _a, const Vec3& _b, const Vec3& _c, const Vec3& _d);
void  DrawQuadFilled(const Vec3& _origin, const Vec3& _normal, const Vec2& _size);
void  DrawCircle(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail = -1);
void  DrawCircleFilled(const Vec3& _origin, const Vec3& _normal, float _radius, int _detail = -1);
void  DrawSphere(const Vec3& _origin, float _radius, int _detail = -1);
void  DrawSphereFilled(const Vec3& _origin, float _radius, int _detail = -1);
void  DrawAlignedBox(const Vec3& _min, const Vec3& _max);
void  DrawAlignedBoxFilled(const Vec3& _min, const Vec3& _max);
void  DrawCylinder(const Vec3& _start, const Vec3& _end, float _radius, int _detail = -1);
void  DrawCapsule(const Vec3& _start, const Vec3& _end, float _radius, int _detail = -1);
void  DrawPrism(const Vec3& _start, const Vec3& _end, float _radius, int _sides);
void  DrawArrow(const Vec3& _start, const Vec3& _end, float _headLength = -1.0f, float _headThickness = -1.0f);

// Ids are used to uniquely identify gizmos. Each gizmo should have a unique Id during a frame.
Id    MakeId(const char* _str);
Id    MakeId(const void* _ptr);
Id    MakeId(int _i);

// PushId()/PopId() affect the result of subsequent calls to MakeId(), use when creating gizmos in a loop.
void  PushId(); // push stack top
void  PushId(Id _id);
void  PushId(const char* _str);
void  PushId(const void* _ptr);
void  PushId(int _i);
void  PopId();
Id    GetId();
Id    GetActiveId(); // GetActiveId() != Id_Invalid means that a gizmo is in use
Id    GetHotId();

// Manipulate translation/rotation/scale via a gizmo. Return true if the gizmo is 'active' (if it modified the output parameter).
// If _local is true, the Gizmo* functions expect that the local matrix is on the matrix stack; in general the application should
// push the local matrix before calling any of the following.
bool  GizmoTranslation(const char* _id, float _translation_[3], bool _local = false);
bool  GizmoRotation(const char* _id, float _rotation_[3*3], bool _local = false);
bool  GizmoScale(const char* _id, float _scale_[3]); // local scale only
// Unified gizmo, selects local/global, translation/rotation/scale based on the context-global gizmo modes. Return true if the gizmo is active.
bool  Gizmo(const char* _id, float _translation_[3], float _rotation_[3*3], float _scale_[3]); // any of _translation_/_rotation_/_scale_ may be null.
bool  Gizmo(const char* _id, float _transform_[4*4]);

// Gizmo* overloads which take an Id directly. In some cases the app may want to call MakeId() separately, usually to change the gizmo appearance if hot/active.
bool  GizmoTranslation(Id _id, float _translation_[3], bool _local = false);
bool  GizmoRotation(Id _id, float _rotation_[3*3], bool _local = false);
bool  GizmoScale(Id _id, float _scale_[3]);
bool  Gizmo(Id _id, float _transform_[4*4]);
bool  Gizmo(Id _id, float _translation_[3], float _rotation_[3*3], float _scale_[3]);

// Visibility tests. The application must set a culling frustum via AppData.
bool  IsVisible(const Vec3& _origin, float _radius); // sphere
bool  IsVisible(const Vec3& _min, const Vec3& _max); // axis-aligned bounding box

// Get/set the current context. All Im3d calls affect the currently bound context.
Context& GetContext();
void     SetContext(Context& _ctx);

struct Vec2
{
	float x, y;
	Vec2()                                                                   {}
	Vec2(float _xy): x(_xy), y(_xy)                                          {}
	Vec2(float _x, float _y): x(_x), y(_y)                                   {}
	operator float*()                                                        { return &x; }
	operator const float*() const                                            { return &x; }
	#ifdef IM3D_VEC2_APP
		IM3D_VEC2_APP
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
	#ifdef IM3D_VEC3_APP
		IM3D_VEC3_APP
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
	#ifdef IM3D_VEC4_APP
		IM3D_VEC4_APP
	#endif
};
struct Mat3
{
	float m[3*3]; // column-major unless IM3D_MATRIX_ROW_MAJOR defined
	Mat3()                                                                   {}
	Mat3(float _diagonal);
	Mat3(
		float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22
		);
	Mat3(const Vec3& _colX, const Vec3& _colY, const Vec3& _colZ);
	Mat3(const Mat4& _mat4); // extract upper 3x3
	operator float*()                                                        { return m; }
	operator const float*() const                                            { return m; }

	Vec3 getCol(int _i) const;
	Vec3 getRow(int _i) const;
	void setCol(int _i, const Vec3& _v);
	void setRow(int _i, const Vec3& _v);

	Vec3 getScale() const;
	void setScale(const Vec3& _scale);

	float operator()(int _row, int _col) const
	{
		#ifdef IM3D_MATRIX_ROW_MAJOR
			int i = _row * 3 + _col;
		#else
			int i = _col * 3 + _row;
		#endif
		return m[i];
	}
	float& operator()(int _row, int _col)
	{
		#ifdef IM3D_MATRIX_ROW_MAJOR
			int i = _row * 3 + _col;
		#else
			int i = _col * 3 + _row;
		#endif
		return m[i];
	}

	#ifdef IM3D_MAT3_APP
		IM3D_MAT3_APP
	#endif
};
struct Mat4
{
	float m[4*4]; // column-major unless IM3D_MATRIX_ROW_MAJOR defined
	Mat4()                                                                   {}
	Mat4(float _diagonal);
	Mat4(
		float m00,        float m01,        float m02,        float m03,
		float m10,        float m11,        float m12,        float m13,
		float m20,        float m21,        float m22,        float m23,
		float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 1.0f
		);
	Mat4(const Mat3& _mat3);
	Mat4(const Vec3& _translation, const Mat3& _rotation, const Vec3& _scale);
	operator float*()                                                        { return m; }
	operator const float*() const                                            { return m; }

	Vec4 getCol(int _i) const;
	Vec4 getRow(int _i) const;
	void setCol(int _i, const Vec4& _v);
	void setRow(int _i, const Vec4& _v);

	Vec3 getTranslation() const;
	void setTranslation(const Vec3& _translation);
	Mat3 getRotation() const;
	void setRotation(const Mat3& _rotation);
	Vec3 getScale() const;
	void setScale(const Vec3& _scale);

	float operator()(int _row, int _col) const
	{
		#ifdef IM3D_MATRIX_ROW_MAJOR
			int i = _row * 4 + _col;
		#else
			int i = _col * 4 + _row;
		#endif
		return m[i];
	}
	float& operator()(int _row, int _col)
	{
		#ifdef IM3D_MATRIX_ROW_MAJOR
			int i = _row * 4 + _col;
		#else
			int i = _col * 4 + _row;
		#endif
		return m[i];
	}

	#ifdef IM3D_MAT4_APP
		IM3D_MAT4_APP
	#endif
};
struct Color
{
	U32 v; // rgba8 (MSB = r)
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

struct alignas(IM3D_VERTEX_ALIGNMENT) VertexData
{
	Vec4   m_positionSize; // xyz = position, w = size
	Color  m_color;        // rgba8 (MSB = r)

	VertexData() {}
	VertexData(const Vec3& _position, float _size, Color _color): m_positionSize(_position, _size), m_color(_color) {}
};

enum DrawPrimitiveType
{
 // order here determines the order in which unsorted primitives are drawn
	DrawPrimitive_Triangles,
	DrawPrimitive_Lines,
	DrawPrimitive_Points,

	DrawPrimitive_Count
};

struct DrawList
{
	Id                m_layerId;
	DrawPrimitiveType m_primType;
	const VertexData* m_vertexData;
	U32               m_vertexCount;
};
typedef void (DrawPrimitivesCallback)(const DrawList& _drawList);

enum Key
{
	Mouse_Left,
	Key_L,
	Key_R,
	Key_S,
	Key_T,

	Key_Count,

// the following map keys -> 'action' states which may be more intuitive
	Action_Select           = Mouse_Left,
	Action_GizmoLocal       = Key_L,
	Action_GizmoRotation    = Key_R,
	Action_GizmoScale       = Key_S,
	Action_GizmoTranslation = Key_T,

	Action_Count
};

enum FrustumPlane
{
	FrustumPlane_Near,
	FrustumPlane_Far,
	FrustumPlane_Top,
	FrustumPlane_Right,
	FrustumPlane_Bottom,
	FrustumPlane_Left,

	FrustumPlane_Count
};

struct AppData
{
	bool   m_keyDown[Key_Count];               // Key states.
	Vec4   m_cullFrustum[FrustumPlane_Count];  // Frustum planes for culling (if culling enabled).
	Vec3   m_cursorRayOrigin;                  // World space cursor ray origin.
	Vec3   m_cursorRayDirection;               // World space cursor ray direction.
	Vec3   m_worldUp;                          // World space 'up' vector.
	Vec3   m_viewOrigin;                       // World space render origin (camera position).
	Vec3   m_viewDirection;                    // World space view direction.
	Vec2   m_viewportSize;                     // Viewport size (pixels).
	float  m_projScaleY;                       // Scale factor used to convert from pixel size -> world scale; use tan(fov) for perspective projections, far plane height for ortho.
	bool   m_projOrtho;                        // If the projection matrix is orthographic.
	float  m_deltaTime;                        // Time since previous frame (seconds).
	float  m_snapTranslation;                  // Snap value for translation gizmos (world units). 0 = disabled.
	float  m_snapRotation;                     // Snap value for rotation gizmos (radians). 0 = disabled.
	float  m_snapScale;                        // Snap value for scale gizmos. 0 = disabled.
	void*  m_appData;                          // App-specific data.

	DrawPrimitivesCallback* drawCallback; // e.g. void Im3d_Draw(const DrawList& _drawList)

	// Extract cull frustum planes from the view-projection matrix.
	// Set _ndcZNegativeOneToOne = true if the proj matrix maps z from [-1,1] (OpenGL style).
	void setCullFrustum(const Mat4& _viewProj, bool _ndcZNegativeOneToOne);
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

	void     push_back(const T& _v)               { T tmp = _v; if (m_size == m_capacity) { reserve(m_capacity + m_capacity / 2); } m_data[m_size++] = tmp; }
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

	static void swap(Vector<T>& _a_, Vector<T>& _b_);
};


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
enum GizmoMode
{
	GizmoMode_Translation,
	GizmoMode_Rotation,
	GizmoMode_Scale
};

// Context stores all relevant state - main interface affects the context currently bound via SetCurrentContext().
class Context
{
public:
	void        begin(PrimitiveMode _mode);
	void        end();
	void        vertex(const Vec3& _position, float _size, Color _color);
	void        vertex(const Vec3& _position )   { vertex(_position, getSize(), getColor()); }

	void        reset();
	void        draw();


	void        setColor(Color _color)           { m_colorStack.back() = _color;   }
	Color       getColor() const                 { return m_colorStack.back();     }
	void        pushColor(Color _color)          { m_colorStack.push_back(_color); }
	void        popColor()                       { IM3D_ASSERT(m_colorStack.size() > 1); m_colorStack.pop_back(); }

	void        setAlpha(float _alpha)           { m_alphaStack.back() = _alpha;   }
	float       getAlpha() const                 { return m_alphaStack.back();     }
	void        pushAlpha(float _alpha)          { m_alphaStack.push_back(_alpha); }
	void        popAlpha()                       { IM3D_ASSERT(m_alphaStack.size() > 1); m_alphaStack.pop_back(); }

	void        setSize(float _size)             { m_sizeStack.back() = _size;     }
	float       getSize() const                  { return m_sizeStack.back();      }
	void        pushSize(float _size)            { m_sizeStack.push_back(_size);   }
	void        popSize()                        { IM3D_ASSERT(m_sizeStack.size() > 1); m_sizeStack.pop_back(); }

	void        setEnableSorting(bool _enable);
	bool        getEnableSorting() const         { return m_enableSortingStack.back(); }
	void        pushEnableSorting(bool _enable);
	void        popEnableSorting();

	Id          getLayerId() const               { return m_layerIdStack.back(); }
	void        pushLayerId(Id _layer);
	void        popLayerId();

	void        setMatrix(const Mat4& _mat4)     { m_matrixStack.back() = _mat4;   }
	const Mat4& getMatrix() const                { return m_matrixStack.back();    }
	void        pushMatrix(const Mat4& _mat4)    { m_matrixStack.push_back(_mat4); }
	void        popMatrix()                      { IM3D_ASSERT(m_matrixStack.size() > 1); m_matrixStack.pop_back(); }

	void        setId(Id _id)                    { m_idStack.back() = _id;   }
	Id          getId() const                    { return m_idStack.back();  }
	void        pushId(Id _id)                   { m_idStack.push_back(_id); }
	void        popId()                          { IM3D_ASSERT(m_idStack.size() > 1); m_idStack.pop_back(); }

	AppData&    getAppData()                     { return m_appData; }

	Context();
	~Context();

 // low-level interface for internal and app-defined gizmos, may be subject to breaking changes

	bool gizmoAxisTranslation_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Vec3* _out_);
	void gizmoAxisTranslation_Draw    (Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Color _color);

	bool gizmoPlaneTranslation_Behavior(Id _id, const Vec3& _origin, const Vec3& _normal, float _worldSize, Vec3* _out_);
	void gizmoPlaneTranslation_Draw    (Id _id, const Vec3& _origin, const Vec3& _normal, float _worldSize, Color _color);

	bool gizmoAxislAngle_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldRadius, float _worldSize, float* _out_);
	void gizmoAxislAngle_Draw    (Id _id, const Vec3& _origin, const Vec3& _axis, float _worldRadius, float _angle, Color _color);

	bool gizmoAxisScale_Behavior(Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, float *_out_);
	void gizmoAxisScale_Draw    (Id _id, const Vec3& _origin, const Vec3& _axis, float _worldHeight, float _worldSize, Color _color);

	// Convert pixels -> world space size based on distance between _position and view origin.
	float pixelsToWorldSize(const Vec3& _position, float _pixels);
	// Convert world space size -> pixels based on distance between _position and view origin.
	float worldSizeToPixels(const Vec3& _position, float _pixels);
	// Blend between _min and _max based on distance betwen _position and view origin.
	int estimateLevelOfDetail(const Vec3& _position, float _worldSize, int _min = 4, int _max = 256);

	// Make _id hot if _depth < m_hotDepth && _intersects.
	bool makeHot(Id _id, float _depth, bool _intersects);
	// Make _id active.
	void makeActive(Id _id);
	// Reset the acive/hot ids and the hot depth.
	void resetId();

	// Interpret key state.
	bool isKeyDown(Key _key) const     { return m_keyDownCurr[_key]; }
	bool wasKeyPressed(Key _key) const { return m_keyDownCurr[_key] && !m_keyDownPrev[_key]; }

	// Visibiity tests for culling.
	bool isVisible(const VertexData* _vdata, DrawPrimitiveType _prim); // per-vertex
	bool isVisible(const Vec3& _origin, float _radius);                // sphere
	bool isVisible(const Vec3& _min, const Vec3& _max);                // axis-aligned box

 // gizmo state
	bool               m_gizmoLocal;               // Global mode selection for gizmos.
	GizmoMode          m_gizmoMode;                //               "
	Id                 m_activeId;                 // Currently active gizmo. If set, this is the same as m_hotId.
	Id                 m_hotId;
	Id                 m_appId;
	Id                 m_appActiveId;
	Id                 m_appHotId;
	float              m_hotDepth;                 // Depth of the current hot gizmo along the cursor ray, for handling occlusion.
	Vec3               m_gizmoStateVec3;           // Stored state for the active gizmo.
	Mat3               m_gizmoStateMat3;           //               "
	float              m_gizmoStateFloat;          //               "
	float              m_gizmoHeightPixels;        // Height/radius of gizmos.
	float              m_gizmoSizePixels;          // Thickness of gizmo lines.


 // stats/debugging

	// Return the total number of primitives (sorted + unsorted) of the given _type in all layers.
	U32 getPrimitiveCount(DrawPrimitiveType _type) const;

	// Return the number of layers.
	U32 getLayerCount() const { return m_layerIdMap.size(); }

private:
 // state stacks
	Vector<Color>       m_colorStack;
	Vector<float>       m_alphaStack;
	Vector<float>       m_sizeStack;
	Vector<bool>        m_enableSortingStack;
	Vector<Mat4>        m_matrixStack;
	Vector<Id>          m_idStack;
	Vector<Id>          m_layerIdStack;

 // vertex data: one list per layer, per primitive type, *2 for sorted/unsorted
	typedef Vector<VertexData> VertexList;
	Vector<VertexList*> m_vertexData[2];            // Each layer is DrawPrimitive_Count consecutive lists.
	int                 m_vertexDataIndex;          // 0, or 1 if sorting enabled.
	Vector<Id>          m_layerIdMap;               // Map Id -> vertex data index.
	int                 m_layerIndex;               // Index of the currently active layer in m_layerIdMap.
	Vector<DrawList>    m_sortedDrawLists;          // Sorted draw lists are stored to avoid multiple calls to sort().
	bool                m_sortCalled;               // Avoid calling sort() during every call to draw().
	bool                m_drawCalled;               // For assert, if vertices are pushed after draw() was called.

 // primitive state
	PrimitiveMode       m_primMode;
	DrawPrimitiveType   m_primType;
	U32                 m_firstVertThisPrim;        // Index of the first vertex pushed during this primitive.
	U32                 m_vertCountThisPrim;        // # calls to vertex() since the last call to begin().
	Vec3                m_minVertThisPrim;
	Vec3                m_maxVertThisPrim;

 // app data
	AppData             m_appData;
	bool                m_keyDownCurr[Key_Count];   // Key state captured during reset().
	bool                m_keyDownPrev[Key_Count];   // Key state from previous frame.
	Vec4                m_cullFrustum[FrustumPlane_Count];  // Optimized frustum planes from m_appData.m_cullFrustum.
	int                 m_cullFrustumCount;         // # valid frustum planes in m_cullFrustum.


	// Sort primitive data.
	void sort();

	// Return -1 if _id not found.
	int  findLayerIndex(Id _id) const;

	VertexList* getCurrentVertexList();
};

namespace internal {
	extern Context* g_CurrentContext;
}

inline AppData& GetAppData()                                                 { return GetContext().getAppData();   }
inline void     NewFrame()                                                   { GetContext().reset();               }
inline void     Draw()                                                       { GetContext().draw();                }

inline void  BeginPoints()                                                   { GetContext().begin(PrimitiveMode_Points);        }
inline void  BeginLines()                                                    { GetContext().begin(PrimitiveMode_Lines);         }
inline void  BeginLineLoop()                                                 { GetContext().begin(PrimitiveMode_LineLoop);      }
inline void  BeginLineStrip()                                                { GetContext().begin(PrimitiveMode_LineStrip);     }
inline void  BeginTriangles()                                                { GetContext().begin(PrimitiveMode_Triangles);     }
inline void  BeginTriangleStrip()                                            { GetContext().begin(PrimitiveMode_TriangleStrip); }
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
inline void  PushColor(Color _color)                                         { GetContext().pushColor(_color);                  }
inline void  PopColor()                                                      { GetContext().popColor();                         }
inline void  SetColor(Color _color)                                          { GetContext().setColor(_color);                   }
inline void  SetColor(float _r, float _g, float _b, float _a)                { GetContext().setColor(Color(_r, _g, _b, _a));    }
inline Color GetColor()                                                      { return GetContext().getColor();                  }

inline void  PushAlpha()                                                     { GetContext().pushAlpha(GetContext().getAlpha()); }
inline void  PushAlpha(float _alpha)                                         { GetContext().pushAlpha(_alpha);                  }
inline void  PopAlpha()                                                      { GetContext().popAlpha();                         }
inline void  SetAlpha(float _alpha)                                          { GetContext().setAlpha(_alpha);                   }
inline float GetAlpha()                                                      { return GetContext().getAlpha();                  }

inline void  PushSize()                                                      { GetContext().pushSize(GetContext().getAlpha());  }
inline void  PushSize(float _size)                                           { GetContext().pushSize(_size);                    }
inline void  PopSize()                                                       { GetContext().popSize();                          }
inline void  SetSize(float _size)                                            { GetContext().setSize(_size);                     }
inline float GetSize()                                                       { return GetContext().getSize();                   }

inline void  PushEnableSorting()                                             { GetContext().pushEnableSorting(GetContext().getEnableSorting()); }
inline void  PushEnableSorting(bool _enable)                                 { GetContext().pushEnableSorting(_enable); }
inline void  PopEnableSorting()                                              { GetContext().popEnableSorting();         }
inline void  EnableSorting(bool _enable)                                     { GetContext().setEnableSorting(_enable);  }

inline void  PushLayerId()                                                   { GetContext().pushLayerId(GetContext().getLayerId()); }
inline void  PushLayerId(Id _layer)                                          { GetContext().pushLayerId(_layer); }
inline void  PopLayerId()                                                    { GetContext().popLayerId();        }
inline Id    GetLayerId()                                                    { return GetContext().getLayerId(); }

inline void  PushMatrix()                                                    { GetContext().pushMatrix(GetContext().getMatrix()); }
inline void  PushMatrix(const Mat4& _mat4)                                   { GetContext().pushMatrix(_mat4);                    }
inline void  PopMatrix()                                                     { GetContext().popMatrix();                          }
inline void  SetMatrix(const Mat4& _mat4)                                    { GetContext().setMatrix(_mat4);                     }
inline void  SetIdentity()                                                   { GetContext().setMatrix(Mat4(1.0f));                }

inline void  PushId()                                                        { GetContext().pushId(GetContext().getId()); }
inline void  PushId(Id _id)                                                  { GetContext().pushId(_id);                  }
inline void  PushId(const char* _str)                                        { GetContext().pushId(MakeId(_str));         }
inline void  PushId(const void* _ptr)                                        { GetContext().pushId(MakeId(_ptr));         }
inline void  PushId(int _i)                                                  { GetContext().pushId(MakeId(_i));           }
inline void  PopId()                                                         { GetContext().popId();                      }
inline Id    GetId()                                                         { return GetContext().getId();               }
inline Id    GetActiveId()                                                   { return GetContext().m_appActiveId;         }
inline Id    GetHotId()                                                      { return GetContext().m_appHotId;            }

inline bool GizmoTranslation(const char* _id, float _translation_[3], bool _local)                   { return GizmoTranslation(MakeId(_id), _translation_);           }
inline bool GizmoRotation(const char* _id, float _rotation_[3*3], bool _local)                       { return GizmoRotation(MakeId(_id), _rotation_, _local);         }
inline bool GizmoScale(const char* _id, float _scale_[3])                                            { return GizmoScale(MakeId(_id), _scale_);                       }
inline bool Gizmo(const char* _id, float _translation_[3], float _rotation_[3*3], float _scale_[3])  { return Gizmo(MakeId(_id), _translation_, _rotation_, _scale_); }
inline bool Gizmo(const char* _id, float _transform_[4*4])                                           { return Gizmo(MakeId(_id), _transform_);                        }

inline bool IsVisible(const Vec3& _origin, float _radius)                    { return GetContext().isVisible(_origin, _radius); }
inline bool IsVisible(const Vec3& _min, const Vec3& _max)                    { return GetContext().isVisible(_min, _max);       }

inline Context& GetContext()                                                 { return *internal::g_CurrentContext; }
inline void     SetContext(Context& _ctx)                                    { internal::g_CurrentContext = &_ctx; }

} // namespac Im3d

#endif // im3d_h
