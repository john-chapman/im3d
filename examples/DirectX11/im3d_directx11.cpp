#include "im3d_example.h"

struct D3DShader
{
	ID3DBlob*             m_vsBlob;
	ID3D11VertexShader*   m_vs;
	ID3DBlob*             m_gsBlob;
	ID3D11GeometryShader* m_gs;
	ID3DBlob*             m_psBlob;
	ID3D11PixelShader*    m_ps;

	void Release()
	{
		if (m_vsBlob) m_vsBlob->Release();
		if (m_vs)     m_vs->Release();
		if (m_gsBlob) m_gsBlob->Release();
		if (m_gs)     m_gs->Release();
		if (m_psBlob) m_psBlob->Release();
		if (m_ps)     m_ps->Release();
	}
};

static D3DShader                 g_Im3dShaderPoints;
static D3DShader                 g_Im3dShaderLines;
static D3DShader                 g_Im3dShaderTriangles;
static ID3D11InputLayout*        g_Im3dInputLayout;
static ID3D11RasterizerState*    g_Im3dRasterizerState;
static ID3D11BlendState*         g_Im3dBlendState;
static ID3D11DepthStencilState*  g_Im3dDepthStencilState;
static ID3D11Buffer*             g_Im3dConstantBuffer;
static ID3D11Buffer*             g_Im3dVertexBuffer;

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
	/*AppData& ad = GetAppData();

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
	glAssert(glDrawArrays(prim, 0, (GLsizei)_drawList.m_vertexCount));*/
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
	ad.m_worldUp = Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases

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
	ID3D11Device* d3d = g_Example->m_d3dDevice;
	#define SHADER_VERSION "4_0"
	{ // points shader
		g_Im3dShaderPoints.m_vsBlob = LoadCompileShader("vs_" SHADER_VERSION, "im3d.hlsl", "VERTEX_SHADER\0POINTS\0");
		if (!g_Im3dShaderPoints.m_vsBlob) {
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderPoints.m_vsBlob->GetBufferPointer(), g_Im3dShaderPoints.m_vsBlob->GetBufferSize(), NULL, &g_Im3dShaderPoints.m_vs));
	
		g_Im3dShaderPoints.m_psBlob = LoadCompileShader("ps_" SHADER_VERSION, "im3d.hlsl", "PIXEL_SHADER\0POINTS\0");
		if (!g_Im3dShaderPoints.m_psBlob) {
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderPoints.m_psBlob->GetBufferPointer(), g_Im3dShaderPoints.m_psBlob->GetBufferSize(), NULL, &g_Im3dShaderPoints.m_ps));
	}
	{ // lines shader
		g_Im3dShaderLines.m_vsBlob = LoadCompileShader("vs_" SHADER_VERSION, "im3d.hlsl", "VERTEX_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_vsBlob) {
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderLines.m_vsBlob->GetBufferPointer(), g_Im3dShaderLines.m_vsBlob->GetBufferSize(), NULL, &g_Im3dShaderLines.m_vs));
		
		g_Im3dShaderLines.m_gsBlob = LoadCompileShader("gs_" SHADER_VERSION, "im3d.hlsl", "GEOMETRY_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_gsBlob) {
			return false;
		}
		dxAssert(d3d->CreateGeometryShader((DWORD*)g_Im3dShaderLines.m_gsBlob->GetBufferPointer(), g_Im3dShaderLines.m_gsBlob->GetBufferSize(), NULL, &g_Im3dShaderLines.m_gs));
		
		g_Im3dShaderLines.m_psBlob = LoadCompileShader("ps_" SHADER_VERSION, "im3d.hlsl", "PIXEL_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_psBlob) {
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderLines.m_psBlob->GetBufferPointer(), g_Im3dShaderLines.m_psBlob->GetBufferSize(), NULL, &g_Im3dShaderLines.m_ps));
	}
	{ // triangles shader
		g_Im3dShaderTriangles.m_vsBlob = LoadCompileShader("vs_" SHADER_VERSION, "im3d.hlsl", "VERTEX_SHADER\0TRIANGLES\0");
		if (!g_Im3dShaderTriangles.m_vsBlob) {
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderTriangles.m_vsBlob->GetBufferPointer(), g_Im3dShaderTriangles.m_vsBlob->GetBufferSize(), NULL, &g_Im3dShaderTriangles.m_vs));
	
		g_Im3dShaderTriangles.m_psBlob = LoadCompileShader("ps_" SHADER_VERSION, "im3d.hlsl", "PIXEL_SHADER\0TRIANGLES\0");
		if (!g_Im3dShaderTriangles.m_psBlob) {
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderTriangles.m_psBlob->GetBufferPointer(), g_Im3dShaderTriangles.m_psBlob->GetBufferSize(), NULL, &g_Im3dShaderTriangles.m_ps));
	}

	{
		D3D11_INPUT_ELEMENT_DESC desc[] = {
			{ "POSITION_SIZE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, (UINT)offsetof(Im3d::VertexData, m_positionSize), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",         0, DXGI_FORMAT_R8G8B8A8_UNORM,       0, (UINT)offsetof(Im3d::VertexData, m_color),        D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		dxAssert(d3d->CreateInputLayout(desc, 3, g_Im3dShaderPoints.m_vsBlob->GetBufferPointer(), g_Im3dShaderPoints.m_vsBlob->GetBufferSize(), &g_Im3dInputLayout));
	}

	GetAppData().drawCallback = &Im3d_Draw;

	return true;
}

void Im3d_Shutdown()
{
	g_Im3dShaderPoints.Release();
	g_Im3dShaderLines.Release();
	g_Im3dShaderTriangles.Release();

	if (g_Im3dInputLayout)        g_Im3dInputLayout->Release();
	if (g_Im3dRasterizerState)    g_Im3dRasterizerState->Release();
	if (g_Im3dBlendState)         g_Im3dBlendState->Release();
	if (g_Im3dDepthStencilState)  g_Im3dDepthStencilState->Release();
	if (g_Im3dConstantBuffer)     g_Im3dConstantBuffer->Release();
	if (g_Im3dVertexBuffer)       g_Im3dVertexBuffer->Release();
}
