/*	OpenGL 3.1 example
	This example demonstrates a method for integrating Im3d without geometry shaders, instead using the 
	vertex shader to expand points/lines into triangle strips.
*/
#include "im3d_example.h"

static GLuint g_Im3dVertexArray;
static GLuint g_Im3dVertexBuffer;
static GLuint g_Im3dUniformBuffer;
static GLuint g_Im3dShaderPoints;
static GLuint g_Im3dShaderLines;
static GLuint g_Im3dShaderTriangles;

using namespace Im3d;

// The draw callback is where Im3d draw lists are rendered by the application. Im3d::Draw can potentially
// call this function multiple times per primtive type.
// The example below shows the simplest type of draw callback. Variations on this are possible, for example
// using a depth buffer. See the shader source file for more details.
// For VR, the easiest option is to call Im3d::Draw() once per eye with the appropriate framebuffer bound,
// passing the eye view-projection matrix. A more efficient scheme would be to render to both eyes
// inside the draw callback to avoid uploading the vertex data twice.
// Note that there is no guarantee that the data in _drawList will exist after this function exits.
void Im3d_Draw(const Im3d::DrawList& _drawList)
{
	AppData& ad = GetAppData();
 // setting the framebuffer, viewport and pipeline states can (and should) be done prior to calling Im3d::Draw
	glAssert(glViewport(0, 0, (GLsizei)g_Example->m_width, (GLsizei)g_Example->m_height));
	glAssert(glEnable(GL_BLEND));
	glAssert(glBlendEquation(GL_FUNC_ADD));
	glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	
	GLenum prim;
	GLuint sh;
	int primVertexCount;
	switch (_drawList.m_primType) {
		case Im3d::DrawPrimitive_Points:
			prim = GL_TRIANGLE_STRIP;
			primVertexCount = 1;
			sh = g_Im3dShaderPoints;
			glAssert(glDisable(GL_CULL_FACE)); // points are view-aligned
			break;
		case Im3d::DrawPrimitive_Lines:
			prim = GL_TRIANGLE_STRIP;
			primVertexCount = 2;
			sh = g_Im3dShaderLines;
			glAssert(glDisable(GL_CULL_FACE)); // lines are view-aligned
			break;
		case Im3d::DrawPrimitive_Triangles:
			prim = GL_TRIANGLES;
			primVertexCount = 3;
			sh = g_Im3dShaderTriangles;
			//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
			break;
		default:
			IM3D_ASSERT(false);
			return;
	};

	glAssert(glBindVertexArray(g_Im3dVertexArray));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer));
	
	glAssert(glUseProgram(sh));
	glAssert(glUniform2f(glGetUniformLocation(sh, "uViewport"), ad.m_viewportSize.x, ad.m_viewportSize.y));
	glAssert(glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*)g_Example->m_camViewProj));

 // Uniform buffers have a size limit; split the vertex data into several passes.
 // Padding is also required to match alignment requirements.
	struct PaddedVertex { Im3d::VertexData m_vertexData; Im3d::U32 _pad[3]; };
	const int kMaxBufferSize = 64 * 1024; // assuming a 64kb max uniform buffer size
	const int kPrimsPerPass = kMaxBufferSize / (sizeof(PaddedVertex) * primVertexCount);
	PaddedVertex paddedVertexData[kMaxBufferSize / sizeof(PaddedVertex)];

	int remainingPrimCount = _drawList.m_vertexCount / primVertexCount;
	const Im3d::VertexData* vertexData = _drawList.m_vertexData;
	while (remainingPrimCount > 0) {
		int passPrimCount = remainingPrimCount < kPrimsPerPass ? remainingPrimCount : kPrimsPerPass;
		int passVertexCount = passPrimCount * primVertexCount;

	 // copy + pad vertex data into a local buffer, upload to the gpu buffer
		for (int i = 0; i < passVertexCount; ++i) {
			paddedVertexData[i].m_vertexData = vertexData[i];
		}
		glAssert(glBindBuffer(GL_UNIFORM_BUFFER, g_Im3dUniformBuffer));
		glAssert(glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)passVertexCount * sizeof(PaddedVertex), (GLvoid*)paddedVertexData, GL_DYNAMIC_DRAW));
		
	 // instanced draw call, 1 instance per prim
		glAssert(glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_Im3dUniformBuffer));
		glDrawArraysInstanced(prim, 0, prim == GL_TRIANGLES ? 3 : 4, passPrimCount); // for triangles just use the first 3 verts of the strip

		vertexData += passVertexCount;
		remainingPrimCount -= passPrimCount;
	}
}

// At the top of each frame, the application must fill the Im3d::AppData struct and then call Im3d::NewFrame().
// The example below shows how to do this, in particular how to generate the 'cursor ray' from a mouse position
// which is necessary for interacting with gizmos.
void Im3d_Update()
{
	AppData& ad = GetAppData();

	ad.m_deltaTime    = g_Example->m_deltaTime;
	ad.m_viewportSize = Vec2((float)g_Example->m_width, (float)g_Example->m_height);
	ad.m_viewOrigin   = g_Example->m_camPos; // for VR use the head position
	ad.m_worldUp      = Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
	ad.m_projOrtho    = g_Example->m_camOrtho; 
	
 // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
	ad.m_projScaleY   = g_Example->m_camOrtho
		? 2.0f / g_Example->m_camProj(1, 1) // use far plane height for an ortho projection
		: tanf(g_Example->m_camFovRad * 0.5f) * 2.0f // or vertical fov for a perspective projection
		;  

 // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
	Vec2 cursorPos = g_Example->getWindowRelativeCursor();
	cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	Vec3 rayOrigin, rayDirection;
	if (g_Example->m_camOrtho) {
		rayOrigin.x  = cursorPos.x / g_Example->m_camProj(0, 0);
		rayOrigin.y  = cursorPos.y / g_Example->m_camProj(1, 1);
		rayOrigin.z  = 0.0f;
		rayOrigin    = g_Example->m_camWorld * Vec4(rayOrigin, 1.0f);
		rayDirection = g_Example->m_camWorld * Vec4(0.0f, 0.0f, -1.0f, 0.0f);
		 
	} else {
		rayOrigin = ad.m_viewOrigin;
		rayDirection.x  = cursorPos.x / g_Example->m_camProj(0, 0);
		rayDirection.y  = cursorPos.y / g_Example->m_camProj(1, 1);
		rayDirection.z  = -1.0f;
		rayDirection    = g_Example->m_camWorld * Vec4(Normalize(rayDirection), 0.0f);
	}
	ad.m_cursorRayOrigin = rayOrigin;
	ad.m_cursorRayDirection = rayDirection;

 // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
 // All key states have an equivalent (and more descriptive) 'Action_' enum.
	ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

 // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl
 // key as an additional predicate.
	bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
	ad.m_keyDown[Im3d::Key_L/*Action_GizmoLocal*/]       = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
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
			glAssert(g_Im3dShaderPoints = glCreateProgram());
			glAssert(glAttachShader(g_Im3dShaderPoints, vs));
			glAssert(glAttachShader(g_Im3dShaderPoints, fs));
			bool ret = LinkShaderProgram(g_Im3dShaderPoints);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}			
		} else {
			return false;
		}
		GLuint blockIndex;
		glAssert(blockIndex = glGetUniformBlockIndex(g_Im3dShaderPoints, "VertexDataBlock"));
		glAssert(glUniformBlockBinding(g_Im3dShaderPoints, blockIndex, 0));
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0LINES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0LINES\0");
		if (vs && fs) {
			glAssert(g_Im3dShaderLines = glCreateProgram());
			glAssert(glAttachShader(g_Im3dShaderLines, vs));
			glAssert(glAttachShader(g_Im3dShaderLines, fs));
			bool ret = LinkShaderProgram(g_Im3dShaderLines);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}
		} else {
			return false;
		}
		GLuint blockIndex;
		glAssert(blockIndex = glGetUniformBlockIndex(g_Im3dShaderLines, "VertexDataBlock"));
		glAssert(glUniformBlockBinding(g_Im3dShaderLines, blockIndex, 0));
	}
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "im3d.glsl", "VERTEX_SHADER\0TRIANGLES\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "im3d.glsl", "FRAGMENT_SHADER\0TRIANGLES\0");
		if (vs && fs) {
			glAssert(g_Im3dShaderTriangles = glCreateProgram());
			glAssert(glAttachShader(g_Im3dShaderTriangles, vs));
			glAssert(glAttachShader(g_Im3dShaderTriangles, fs));
			bool ret = LinkShaderProgram(g_Im3dShaderTriangles);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret) {
				return false;
			}		
		} else {
			return false;
		}
		GLuint blockIndex;
		glAssert(blockIndex = glGetUniformBlockIndex(g_Im3dShaderTriangles, "VertexDataBlock"));
		glAssert(glUniformBlockBinding(g_Im3dShaderTriangles, blockIndex, 0));
	}

 // in this example we're using a static buffer as the vertex source with a uniform buffer to provide
 // the shader with the Im3d vertex data
	Im3d::Vec4 vertexData[] = {
		Im3d::Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
		Im3d::Vec4( 1.0f, -1.0f, 0.0f, 1.0f),
		Im3d::Vec4(-1.0f,  1.0f, 0.0f, 1.0f),
		Im3d::Vec4( 1.0f,  1.0f, 0.0f, 1.0f)
	};
	glAssert(glCreateBuffers(1, &g_Im3dVertexBuffer));;
	glAssert(glCreateVertexArrays(1, &g_Im3dVertexArray));	
	glAssert(glBindVertexArray(g_Im3dVertexArray));
	glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer));
	glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), (GLvoid*)vertexData, GL_STATIC_DRAW));
	glAssert(glEnableVertexAttribArray(0));
	glAssert(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec4), (GLvoid*)0));
	glAssert(glBindVertexArray(0));

	glAssert(glCreateBuffers(1, &g_Im3dUniformBuffer));
	

	GetAppData().drawCallback = &Im3d_Draw;

	return true;
}

void Im3d_Shutdown()
{
	glAssert(glDeleteVertexArrays(1, &g_Im3dVertexArray));
	glAssert(glDeleteBuffers(1, &g_Im3dUniformBuffer));
	glAssert(glDeleteBuffers(1, &g_Im3dVertexBuffer));
	glAssert(glDeleteProgram(g_Im3dShaderPoints));
	glAssert(glDeleteProgram(g_Im3dShaderLines));
	glAssert(glDeleteProgram(g_Im3dShaderTriangles));
}
