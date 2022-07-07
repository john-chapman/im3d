/*	DirectX 11 example
	This is a standard 'modern' example which implements point/line expansion via a geometry shader.
	See examples/OpenGL31 for a reference which doesn't require geometry shaders.
*/
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

// Resource init/shutdown will be app specific. In general you'll need one shader for each of the 3
// draw primitive types (points, lines, triangles), plus some number of vertex buffers.
bool Im3d_Init()
{
	ID3D11Device* d3d = g_Example->m_d3dDevice;
	{ // points shader
		g_Im3dShaderPoints.m_vsBlob = LoadCompileShader("vs_" IM3D_DX11_VSHADER, "im3d.hlsl", "VERTEX_SHADER\0POINTS\0");
		if (!g_Im3dShaderPoints.m_vsBlob)
		{
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderPoints.m_vsBlob->GetBufferPointer(), g_Im3dShaderPoints.m_vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_vs));

		g_Im3dShaderPoints.m_gsBlob = LoadCompileShader("gs_" IM3D_DX11_VSHADER, "im3d.hlsl", "GEOMETRY_SHADER\0POINTS\0");
		if (!g_Im3dShaderPoints.m_gsBlob)
		{
			return false;
		}
		dxAssert(d3d->CreateGeometryShader((DWORD*)g_Im3dShaderPoints.m_gsBlob->GetBufferPointer(), g_Im3dShaderPoints.m_gsBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_gs));

		g_Im3dShaderPoints.m_psBlob = LoadCompileShader("ps_" IM3D_DX11_VSHADER, "im3d.hlsl", "PIXEL_SHADER\0POINTS\0");
		if (!g_Im3dShaderPoints.m_psBlob)
		{
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderPoints.m_psBlob->GetBufferPointer(), g_Im3dShaderPoints.m_psBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_ps));
	}

	{ // lines shader
		g_Im3dShaderLines.m_vsBlob = LoadCompileShader("vs_" IM3D_DX11_VSHADER, "im3d.hlsl", "VERTEX_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_vsBlob)
		{
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderLines.m_vsBlob->GetBufferPointer(), g_Im3dShaderLines.m_vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_vs));

		g_Im3dShaderLines.m_gsBlob = LoadCompileShader("gs_" IM3D_DX11_VSHADER, "im3d.hlsl", "GEOMETRY_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_gsBlob)
		{
			return false;
		}
		dxAssert(d3d->CreateGeometryShader((DWORD*)g_Im3dShaderLines.m_gsBlob->GetBufferPointer(), g_Im3dShaderLines.m_gsBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_gs));

		g_Im3dShaderLines.m_psBlob = LoadCompileShader("ps_" IM3D_DX11_VSHADER, "im3d.hlsl", "PIXEL_SHADER\0LINES\0");
		if (!g_Im3dShaderLines.m_psBlob)
		{
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderLines.m_psBlob->GetBufferPointer(), g_Im3dShaderLines.m_psBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_ps));
	}

	{ // triangles shader
		g_Im3dShaderTriangles.m_vsBlob = LoadCompileShader("vs_" IM3D_DX11_VSHADER, "im3d.hlsl", "VERTEX_SHADER\0TRIANGLES\0");
		if (!g_Im3dShaderTriangles.m_vsBlob)
		{
			return false;
		}
		dxAssert(d3d->CreateVertexShader((DWORD*)g_Im3dShaderTriangles.m_vsBlob->GetBufferPointer(), g_Im3dShaderTriangles.m_vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderTriangles.m_vs));

		g_Im3dShaderTriangles.m_psBlob = LoadCompileShader("ps_" IM3D_DX11_VSHADER, "im3d.hlsl", "PIXEL_SHADER\0TRIANGLES\0");
		if (!g_Im3dShaderTriangles.m_psBlob)
		{
			return false;
		}
		dxAssert(d3d->CreatePixelShader((DWORD*)g_Im3dShaderTriangles.m_psBlob->GetBufferPointer(), g_Im3dShaderTriangles.m_psBlob->GetBufferSize(), nullptr, &g_Im3dShaderTriangles.m_ps));
	}

	{	D3D11_INPUT_ELEMENT_DESC desc[] =
			{
				{ "POSITION_SIZE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, (UINT)offsetof(Im3d::VertexData, m_positionSize), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",         0, DXGI_FORMAT_R8G8B8A8_UNORM,       0, (UINT)offsetof(Im3d::VertexData, m_color),        D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
		dxAssert(d3d->CreateInputLayout(desc, 2, g_Im3dShaderPoints.m_vsBlob->GetBufferPointer(), g_Im3dShaderPoints.m_vsBlob->GetBufferSize(), &g_Im3dInputLayout));
	}

	g_Im3dConstantBuffer = CreateConstantBuffer(sizeof(Mat4) + sizeof(Vec4), D3D11_USAGE_DYNAMIC);

	// Typical pipeline states: enable alpha blending, disable depth test and backface culling.
	{	D3D11_BLEND_DESC desc = {};
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dxAssert(d3d->CreateBlendState(&desc, &g_Im3dBlendState));
	}
	{	D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		dxAssert(d3d->CreateRasterizerState(&desc, &g_Im3dRasterizerState));
	}
	{	D3D11_DEPTH_STENCIL_DESC desc = {};
		dxAssert(d3d->CreateDepthStencilState(&desc, &g_Im3dDepthStencilState));
	}

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

// At the top of each frame, the application must fill the Im3d::AppData struct and then call Im3d::NewFrame().
// The example below shows how to do this, in particular how to generate the 'cursor ray' from a mouse position
// which is necessary for interacting with gizmos.
void Im3d_NewFrame()
{
	AppData& ad = GetAppData();

	ad.m_deltaTime     = g_Example->m_deltaTime;
	ad.m_viewportSize  = Vec2((float)g_Example->m_width, (float)g_Example->m_height);
	ad.m_viewOrigin    = g_Example->m_camPos; // for VR use the head position
	ad.m_viewDirection = g_Example->m_camDir;
	ad.m_worldUp       = Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
	ad.m_projOrtho     = g_Example->m_camOrtho;

 // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
	ad.m_projScaleY = g_Example->m_camOrtho
		? 2.0f / g_Example->m_camProj(1, 1) // use far plane height for an ortho projection
		: tanf(g_Example->m_camFovRad * 0.5f) * 2.0f // or vertical fov for a perspective projection
		;

 // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
	Vec2 cursorPos = g_Example->getWindowRelativeCursor();
	cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	Vec3 rayOrigin, rayDirection;
	if (g_Example->m_camOrtho)
	{
		rayOrigin.x  = cursorPos.x / g_Example->m_camProj(0, 0);
		rayOrigin.y  = cursorPos.y / g_Example->m_camProj(1, 1);
		rayOrigin.z  = 0.0f;
		rayOrigin    = g_Example->m_camWorld * Vec4(rayOrigin, 1.0f);
		rayDirection = g_Example->m_camWorld * Vec4(0.0f, 0.0f, -1.0f, 0.0f);

	}
	else
	{
		rayOrigin = ad.m_viewOrigin;
		rayDirection.x  = cursorPos.x / g_Example->m_camProj(0, 0);
		rayDirection.y  = cursorPos.y / g_Example->m_camProj(1, 1);
		rayDirection.z  = -1.0f;
		rayDirection    = g_Example->m_camWorld * Vec4(Normalize(rayDirection), 0.0f);
	}
	ad.m_cursorRayOrigin = rayOrigin;
	ad.m_cursorRayDirection = rayDirection;

 // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via
 // im3d_config.h, or if any of the IsVisible() functions are called.
	ad.setCullFrustum(g_Example->m_camViewProj, true);

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

 // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
	ad.m_snapTranslation = ctrlDown ? 0.1f : 0.0f;
	ad.m_snapRotation    = ctrlDown ? Im3d::Radians(30.0f) : 0.0f;
	ad.m_snapScale       = ctrlDown ? 0.5f : 0.0f;

	Im3d::NewFrame();
}

// After all Im3d calls have been made for a frame, the user must call Im3d::EndFrame() to finalize draw data, then
// access the draw lists for rendering. Draw lists are only valid between calls to EndFrame() and NewFrame().
// The example below shows the simplest approach to rendering draw lists; variations on this are possible. See the 
// shader source file for more details.
void Im3d_EndFrame()
{
	Im3d::EndFrame();

 // Primitive rendering
	
	AppData& ad = GetAppData();

	ID3D11Device* d3d = g_Example->m_d3dDevice;
	ID3D11DeviceContext* ctx = g_Example->m_d3dDeviceCtx;

	ctx->OMSetBlendState(g_Im3dBlendState, nullptr, 0xffffffff);
	ctx->OMSetDepthStencilState(g_Im3dDepthStencilState, 0);
	ctx->RSSetState(g_Im3dRasterizerState);
	
	D3D11_VIEWPORT viewport =
		{
			0.0f, 0.0f, // TopLeftX, TopLeftY
			(float)g_Example->m_width, (float)g_Example->m_height,
			0.0f, 1.0f // MinDepth, MaxDepth
		};
	ctx->RSSetViewports(1, &viewport);

	for (U32 i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		auto& drawList = Im3d::GetDrawLists()[i];
 
		if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
		{
		 // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}
	
	 // upload view-proj matrix/viewport size
		struct Layout { Mat4 m_viewProj; Vec2 m_viewport; };
		Layout* layout = (Layout*)MapBuffer(g_Im3dConstantBuffer, D3D11_MAP_WRITE_DISCARD);
		layout->m_viewProj = g_Example->m_camViewProj;
		layout->m_viewport = ad.m_viewportSize;
		UnmapBuffer(g_Im3dConstantBuffer);
	
	 // upload vertex data
		static U32 s_vertexBufferSize = 0;
		if (!g_Im3dVertexBuffer || s_vertexBufferSize < drawList.m_vertexCount)
		{
			if (g_Im3dVertexBuffer)
			{
				g_Im3dVertexBuffer->Release();
				g_Im3dVertexBuffer = nullptr;
			}
			s_vertexBufferSize = drawList.m_vertexCount;
			g_Im3dVertexBuffer = CreateVertexBuffer(s_vertexBufferSize * sizeof(Im3d::VertexData), D3D11_USAGE_DYNAMIC);
		}
		memcpy(MapBuffer(g_Im3dVertexBuffer, D3D11_MAP_WRITE_DISCARD), drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));
		UnmapBuffer(g_Im3dVertexBuffer);
	
	 // select shader/primitive topo
		switch (drawList.m_primType)
		{
			case Im3d::DrawPrimitive_Points:
				ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
				ctx->VSSetShader(g_Im3dShaderPoints.m_vs, nullptr, 0);
				ctx->GSSetShader(g_Im3dShaderPoints.m_gs, nullptr, 0);
				ctx->GSSetConstantBuffers(0, 1, &g_Im3dConstantBuffer);
				ctx->PSSetShader(g_Im3dShaderPoints.m_ps, nullptr, 0);
				break;
			case Im3d::DrawPrimitive_Lines:
				ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
				ctx->VSSetShader(g_Im3dShaderLines.m_vs, nullptr, 0);
				ctx->GSSetShader(g_Im3dShaderLines.m_gs, nullptr, 0);
				ctx->GSSetConstantBuffers(0, 1, &g_Im3dConstantBuffer);
				ctx->PSSetShader(g_Im3dShaderLines.m_ps, nullptr, 0);
				break;
			case Im3d::DrawPrimitive_Triangles:
				ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				ctx->VSSetShader(g_Im3dShaderTriangles.m_vs, nullptr, 0);
				ctx->PSSetShader(g_Im3dShaderTriangles.m_ps, nullptr, 0);
				break;
			default:
				IM3D_ASSERT(false);
				return;
		};
	
		UINT stride = sizeof(Im3d::VertexData);
		UINT offset = 0;
		ctx->IASetVertexBuffers(0, 1, &g_Im3dVertexBuffer, &stride, &offset);
		ctx->IASetInputLayout(g_Im3dInputLayout);
		ctx->VSSetConstantBuffers(0, 1, &g_Im3dConstantBuffer);
		ctx->Draw(drawList.m_vertexCount, 0);
		
		ctx->VSSetShader(nullptr, nullptr, 0);
		ctx->GSSetShader(nullptr, nullptr, 0);
		ctx->PSSetShader(nullptr, nullptr, 0);
	}

 // Text rendering.
 // This is common to all examples since we're using ImGui to draw the text lists, see im3d_example.cpp.
	g_Example->drawTextDrawListsImGui(Im3d::GetTextDrawLists(), Im3d::GetTextDrawListCount());
}
