#include "im3d_example.h"

static GLuint g_vaIm3d; // vertex array object
static GLuint g_vbIm3d; // vertex buffer
static GLuint g_shIm3dPoints;
static GLuint g_shIm3dLines;
static GLuint g_shIm3dTriangles;

using namespace Im3d;

// The draw callback is where Im3d draw lists are rendered by the application. Im3d::Draw can potentially
// call this function multiple times per primtive type.
// The example below shows the simplest possible draw callback. Variations on this are possible, for example
// using a depth buffer. See the shader source file for more details.
// For VR, the simplest option is to call Im3d::Draw() once per eye with the appropriate framebuffer bound,
// passing the appropriate view-projection matrix. A more efficient scheme would be to render to both eyes
// inside the draw callback to avoid uploading the vertex data twice.
// Note that there is no guarantee that the data in _drawList will exist after this function exits.
void Im3d_Draw(const Im3d::DrawList& _drawList)
{
	AppData& ad = GetAppData();

 // setting the framebuffer, viewport and pipeline states can (and should) be done prior to calling Im3d::Draw
	glAssert(glViewport(0, 0, (GLsizei)ad.m_viewportSize.x, (GLsizei)ad.m_viewportSize.y));
	glAssert(glEnable(GL_BLEND));
	glAssert(glBlendEquation(GL_FUNC_ADD));
	glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glAssert(glEnable(GL_PROGRAM_POINT_SIZE));
	
	GLenum prim;
	GLuint sh;
	switch (_drawList.m_primType) {
	case Im3d::DrawPrimitive_Points:
		prim = GL_POINTS;
		sh = g_shIm3dPoints;
		glAssert(glDisable(GL_CULL_FACE)); // points are view-aligned
		break;
	case Im3d::DrawPrimitive_Lines:
		prim = GL_LINES;
		sh = g_shIm3dLines;
		glAssert(glDisable(GL_CULL_FACE)); // lines are view-aligned
		break;
	case Im3d::DrawPrimitive_Triangles:
		prim = GL_TRIANGLES;
		sh = g_shIm3dTriangles;
		//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
		break;
	default:
		IM3D_ASSERT(false);
		return;
	};

	glAssert(glBindVertexArray(g_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_vbIm3d));
	glAssert(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)_drawList.m_vertexCount * sizeof(Im3d::VertexData), (GLvoid*)_drawList.m_vertexData, GL_STREAM_DRAW));

	glAssert(glUseProgram(sh));
	glAssert(glUniform2f(glGetUniformLocation(sh, "uViewport"), ad.m_viewportSize.x, ad.m_viewportSize.y));
	glAssert(glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*)g_Example->m_camViewProj));
	glAssert(glDrawArrays(prim, 0, (GLsizei)_drawList.m_vertexCount));
}

// At the top of each frame, the application must fill the Im3d::AppData struct and then call Im3d::NewFrame.
// The example below shows how to do this, in particular how to generate the 'cursor ray' from a mouse position
// which is necessary for interacting with gizmos.
void Im3d_Update()
{
	AppData& ad = GetAppData();
	ad.m_deltaTime = g_Example->m_deltaTime;
	ad.m_viewportSize = Vec2((float)g_Example->m_width, (float)g_Example->m_height);
	ad.m_tanHalfFov = tanf(g_Example->m_camFovRad * 0.5f); // vertical fov
	ad.m_viewOrigin = g_Example->m_camPos; // for VR use the head position

 // Cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller
	Vec2 cursorPos = g_Example->getWindowRelativeCursor();
	cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	ad.m_cursorRayOrigin = ad.m_viewOrigin;
	float aspect = ad.m_viewportSize.x / ad.m_viewportSize.y;
	ad.m_cursorRayDirection = g_Example->m_camWorld * Normalize(Vec4(cursorPos.x * ad.m_tanHalfFov * aspect, cursorPos.y * ad.m_tanHalfFov, -1.0f, 0.0f));

 // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
 // All key states have an equivalent (and more descriptive) 'Action_' enum.
	ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

 // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl
 // key as an additional predicate.
	bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
	ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
	ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
	ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

	Im3d::NewFrame();
}

// Resource init/shutdown will be app specific. In general you'll need one shader for each of the 3
// draw primitive types (points, lines, triangles), plus some number of vertex buffers.
bool Im3d_Init()
{
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0POINTS\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0POINTS\0");
		if (vs && fs) {
			glAssert(g_shIm3dPoints = glCreateProgram());
			glAssert(glAttachShader(g_shIm3dPoints, vs));
			glAssert(glAttachShader(g_shIm3dPoints, fs));
			bool ret = LinkShaderProgram(g_shIm3dPoints);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}			
		} else {
			return false;
		}
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0LINES\0");
		GLuint gs = LoadCompileShader(GL_GEOMETRY_SHADER, "im3d.glsl", "GEOMETRY_SHADER\0LINES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0LINES\0");
		if (vs && gs && fs) {
			glAssert(g_shIm3dLines = glCreateProgram());
			glAssert(glAttachShader(g_shIm3dLines, vs));
			glAssert(glAttachShader(g_shIm3dLines, gs));
			glAssert(glAttachShader(g_shIm3dLines, fs));
			bool ret = LinkShaderProgram(g_shIm3dLines);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(gs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}
		} else {
			return false;
		}
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0TRIANGLES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0TRIANGLES\0");
		if (vs && fs) {
			glAssert(g_shIm3dTriangles = glCreateProgram());
			glAssert(glAttachShader(g_shIm3dTriangles, vs));
			glAssert(glAttachShader(g_shIm3dTriangles, fs));
			bool ret = LinkShaderProgram(g_shIm3dTriangles);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}		
		} else {
			return false;
		}
	}

	glAssert(glCreateBuffers(1, &g_vbIm3d));;
	glAssert(glCreateVertexArrays(1, &g_vaIm3d));	
	glAssert(glBindVertexArray(g_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_vbIm3d));
	glAssert(glEnableVertexAttribArray(0));
    glAssert(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize)));
	glAssert(glEnableVertexAttribArray(1));
    glAssert(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color)));
	glAssert(glBindVertexArray(0));

	GetAppData().drawCallback = &Im3d_Draw;

	return true;
}

void Im3d_Shutdown()
{
	glAssert(glDeleteVertexArrays(1, &g_vaIm3d));
	glAssert(glDeleteBuffers(1, &g_vbIm3d));
	glAssert(glDeleteProgram(g_shIm3dPoints));
	glAssert(glDeleteProgram(g_shIm3dLines));
	glAssert(glDeleteProgram(g_shIm3dTriangles));
}
