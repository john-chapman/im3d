Im3d is a small, self-contained library for immediate mode rendering of basic primitives (points, lines, triangles), plus an immediate mode UI which provides 3d manipulation 'gizmos' and other tools. It is platform and graphics API agnostic and designed to be compatible with VR.

Im3d outputs vertex buffers for rendering by the application. Im3d does not affect the system graphics state directly, therefore Im3d calls can be made from anywhere in the application code. This is useful for graphics debugging, 3d data visualization, writing CAD & game development tools, etc.

![Demo Screenshot 1](https://github.com/john-chapman/im3d/wiki/images/im3d_demo1.jpg)
![Demo Screenshot 2](https://github.com/john-chapman/im3d/wiki/images/im3d_demo2.jpg)
![Demo Screenshot 3](https://github.com/john-chapman/im3d/wiki/images/im3d_demo3.jpg)

The API design follows OpenGL immediate mode in that it functions as a state machine:

```C++
Im3d::PushDrawState();
Im3d::SetSize(2.0f);
Im3d::BeginLineLoop();
	Im3d::Vertex(0.0f, 0.0f, 0.0f, Im3d::Color_Magenta);
	Im3d::Vertex(1.0f, 1.0f, 0.0f, Im3d::Color_Yellow);
	Im3d::Vertex(2.0f, 2.0f, 0.0f, Im3d::Color_Cyan);
Im3d::End();
Im3d::PopDrawState();
```
_A key point to note is that there is no view-projection matrix here - the requirement for VR support precludes this. Instead, all vertices are specified in world space and the view-projection transform is applied at draw time (in the shader)._

The UI system follows the immediate mode paradigm in that no UI state is retained; you can create gizmos from anywhere in the code:

```C++
static mat4 transform;
if (Im3d::Gizmo("UnifiedGizmo", &transform)) {
	// transform was modified, do something with the matrix
}
```
![Translation Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_translation.gif)
![Rotation Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_rotation.gif)
![Scale Gizmo](https://github.com/john-chapman/im3d/wiki/images/im3d_scale.gif)

See [here](https://github.com/john-chapman/im3d/blob/master/examples/common/main.cpp) for more complete examples.

### Integration
Integration is fairly straightforward: copy the files from the root of this repo and add them to the application project. The application should then proceed as follows:

- At startup, load the graphics resources (shaders, etc.) required to actually draw the Im3d vertex buffers.
- Each frame, the fill the `Im3d::AppData` struct, providing user input and other relevant context data, then call `Im3d::NewFrame()`.
- Towards the end of the frame, call `Im3d::Draw()` once for each view/projection to be rendered. Im3d calls an application-defined callback to actually execute rendering commands and draw the points/lines/triangles pushed during the frame.

More detailed and API-specific integration examples are available in [examples/](https://github.com/john-chapman/im3d/tree/master/examples).

