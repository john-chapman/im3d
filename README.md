Im3d is a small, self-contained library for immediate-mode rendering of simple primitives (points, lines, etc.), plus an immediate-mode UI providing 3d manipulation 'gizmos' and other tools. It is designed to be compatible with VR.

Im3d outputs vertex buffers for rendering by the application. Im3d does not affect the system graphics state directly, therefore Im3d calls can be made from anywhere inside the application code.

The design closely follows OpenGL immediate mode in that it functions as a state machine:

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
