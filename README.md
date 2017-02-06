Im3d is a small, self-contained library for immediate mode rendering of simple primitives (points, lines, triangles), plus an immediate mode UI which provides 3d manipulation 'gizmos' and other tools. It is platform and graphics API agnostic and designed to be compatible with VR.

Im3d outputs vertex buffers for rendering by the application. Im3d does not affect the system graphics state directly, therefore Im3d calls can be made from anywhere inside the application code.

The design follows OpenGL immediate mode in that it functions as a state machine:

TODO: Gif here showing rendering in action (code snippets above images).
```
Im3d::PushDrawState();
Im3d::SetSize(2.0f);
Im3d::SetColor(Im3d::kColor_Green);
Im3d::BeginLines();
	Im3d::Vertex(0.0f, 0.0f, 0.0f);
	Im3d::Vertex(1.0f, 0.0f, 0.0f);
Im3d::End();
Im3d::PopDrawState();
```
_A key point to note is that there is no view-projection matrix here - the requirement for VR support precludes this. Instead, all vertices are specified in world space and the view-projection transform is applied at draw time (in the shader)._

The UI system follows the immediate mode paradigm in that no UI state is retained; you can create gizmos from any place in the code:

```
static mat4 transform;
if (Im3d::Gizmo("UnifiedGizmo", &transform)) {
	// transform was modified
}
```
![Translation Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_translation.gif)
![Rotation Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_rotation.gif)
![Scale Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_scale.gif)


For a general-purpose (2d) immediate mode UI I highly recommend [dear ImGui](https://github.com/ocornut/imgui).