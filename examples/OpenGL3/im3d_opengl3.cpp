#include "im3d_example.h"

static GLuint g_vaIm3d;           // vertex array object
static GLuint g_vbIm3d;           // vertex buffer
static GLuint g_shIm3dPoints;
static GLuint g_shIm3dLines;
static GLuint g_shIm3dTriangles;

using namespace Im3d;

bool Im3d_Init()
{
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0POINTS\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0POINTS\0");
		if (vs && fs) {
			glAssert(s_shIm3dPoints = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dPoints, vs));
			glAssert(glAttachShader(s_shIm3dPoints, fs));
			bool ret = LinkShaderProgram(s_shIm3dPoints);
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
			glAssert(s_shIm3dLines = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dLines, vs));
			glAssert(glAttachShader(s_shIm3dLines, gs));
			glAssert(glAttachShader(s_shIm3dLines, fs));
			bool ret = LinkShaderProgram(s_shIm3dLines));
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(gs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
		} else {
			return false;
		}
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0TRIANGLES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0TRIANGLES\0");
		if (vs && fs) {
			glAssert(s_shIm3dTriangles = glCreateProgram());
			glAssert(glAttachShader(s_shIm3dTriangles, vs));
			glAssert(glAttachShader(s_shIm3dTriangles, fs));
			bool ret = LinkShaderProgram(s_shIm3dTriangles));
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}		
		} else {
			return false;
		}
	}

	glAssert(glCreateBuffers(1, &s_vbIm3d));;
	glAssert(glCreateVertexArrays(1, &s_vaIm3d));	
	glAssert(glBindVertexArray(s_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbIm3d));
	glAssert(glEnableVertexAttribArray(0));
    glAssert(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize)));
	glAssert(glEnableVertexAttribArray(1));
    glAssert(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color)));
	glAssert(glBindVertexArray(0));

	GetAppData().drawPrimitives = &Im3d_Draw;

	return true;
}

void Im3d_Shutdown()
{
	glAsset(glDeleteVertexArrays(1, &g_vaIm3d));
	glAsset(glDeleteBuffers(1, &g_vbIm3d));
	glAsset(glDeleteProgram(g_shIm3dPoints));
	glAsset(glDeleteProgram(g_shIm3dLines));
	glAsset(glDeleteProgram(g_shIm3dTriangles));
}

void Im3d_Update()
{
	AppData& ad = GetAppData();
	ad.m_deltaTime = g_Example->m_deltaTime;
	ad.m_viewportSize = Vec2((float)g_Example->m_width, (float)g_Example->m_height);
	ad.m_viewOrigin = g_Example->m_camPos;
	ad.m_tanHalfFov = tanf(fovRads * 0.5f);

	Vec2 cursorPos = g_Example->getWindowRelativeCursor();
	cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	ad.m_cursorRayOrigin = ad.m_viewOrigin;
	float aspect = (float)m_width / (float)m_height;
	ad.m_cursorRayDirection = m_camWorld * Normalize(Vec4(cursorPos.x * ad.m_tanHalfFov * aspect, cursorPos.y * ad.m_tanHalfFov, -1.0f, 0.0f));

	Im3d::NewFrame();
}

void Im3d_Draw(Im3d::DrawPrimitiveType _primType, const Im3d::VertexData* _data, Im3d::U32 _count)
{
	glAssert(glEnable(GL_BLEND));
	glAssert(glBlendEquation(GL_FUNC_ADD));
	glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glAssert(glEnable(GL_PROGRAM_POINT_SIZE));
	
	GLenum prim;
	GLuint sh;
	switch (_primType) {
	case Im3d::DrawPrimitive_Points:
		prim = GL_POINTS;
		sh = s_shIm3dPoints;
		//ImGui::Text("Points (%d)", _count);
		glAssert(glDisable(GL_CULL_FACE)); // points are view-aligned
		break;
	case Im3d::DrawPrimitive_Lines:
		prim = GL_LINES;
		sh = s_shIm3dLines;
		//ImGui::Text("Lines (%d)", _count / 2);
		glAssert(glDisable(GL_CULL_FACE)); // lines are view-aligned
		break;
	case Im3d::DrawPrimitive_Triangles:
		prim = GL_TRIANGLES;
		sh = s_shIm3dTriangles;
		//ImGui::Text("Tris (%d)", _count / 3);
		//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
		break;
	default:
		IM3D_ASSERT(false);
		return;
	};

	glAssert(glBindVertexArray(s_vaIm3d));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, s_vbIm3d));
	glAssert(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)_count * sizeof(Im3d::VertexData), (GLvoid*)_data, GL_STREAM_DRAW));
	glAssert(glUseProgram(sh));
	glAssert(glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*)s_testApp->m_camViewProj));
	glAssert(glUniform2f(glGetUniformLocation(sh, "uViewport"), (float)s_testApp->getWidth(), (float)s_testApp->getHeight()));
	glAssert(glDrawArrays(prim, 0, (GLsizei)_count));

	glAssert(glDisable(GL_PROGRAM_POINT_SIZE));
}
