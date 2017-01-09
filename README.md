Im3d is a small, self-contained library for immediate-mode rendering of simple primitives (points, lines, etc.), plus an immediate mode UI which provides 3d manipulation 'gizmos' and other tools. It is platform and graphics API agnostic and designed to be compatible with VR.

Im3d outputs vertex buffers for rendering by the application. Im3d does not affect the system graphics state directly, therefore Im3d calls can be made from anywhere inside the application code.

The design follows OpenGL immediate mode in that it functions as a state machine:

TODO: Gif here showing rendering in action (code snippets above images).
```
Im3d::PushDrawState();
	Im3d::SetColor(Im3d::kColor_Green);
	Im3d::BeginLines();
		Im3d::Vertex(0.0f, 0.0f, 0.0f);
		Im3d::Vertex(1.0f, 0.0f, 0.0f);
	Im3d::End();
Im3d::PopDrawState();
```
_A key point to note is that there is no view-projection matrix here - the requirement for VR support precludes this. Instead, the view-projection transform should be applied at draw time (in the shader)._

Similarly, the UI system follows the immediate mode paradigm in that no UI state is retained. For example, you can create gizmos from any place in the code:


TODO: Gif here showing gizmos in action (code snippets above images).
```
vec3 position;
mat3 rotation;
vec3 scale;

// ...

Im3d::GizmoPosition("position", &position));
Im3d::GizmoRotation("rotation", &rotation));
Im3d::GizmoScale("scale", &scale));
```

For a general-purpose (2d) immediate mode UI I highly recommend [dear ImGui](https://github.com/ocornut/imgui).