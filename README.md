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
*A key point to note is that there is no view-projection matrix here - the requirement for VR support precludes this. Instead, all vertices are specified in world space and the view-projection transform is applied at draw time (in the shader).*

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
Im3d has no dependencies other than the C standard lib. A C++11 compatible compiler is required.

Integration is fairly straightforward:

- Copy the files from the root of this repo and add them to the application project.
- Modify `im3d_config.h` if necessary (provide a custom `malloc`/`free`, set the vertex data alignment, matrix layout, etc.).

At runtime, the application should then proceed as follows:

- At startup, load the graphics resources (shaders, etc.) required to actually draw the Im3d vertex buffers.
- Each frame, fill the `Im3d::AppData` struct, providing user input and other context data, then call `Im3d::NewFrame()`.
- Towards the end of the frame, call `Im3d::Draw()` once for each view/projection to be rendered. Im3d calls an application-defined callback to actually execute rendering commands and draw the points/lines/triangles pushed during the frame.

More detailed and API-specific integration examples are available in [examples/](https://github.com/john-chapman/im3d/tree/master/examples).

### Frequently Asked Questions (FAQ)

**Where is the documentation?**

- [im3d.h](https://github.com/john-chapman/im3d/tree/master/im3d.h) contains the main API documentation.
- [examples/common/main.cpp](https://github.com/john-chapman/im3d/tree/master/examples/common/main.cpp) contains usage examples for most features, especially how to use the `Gizmo*()` API.
- [examples/](https://github.com/john-chapman/im3d/tree/master/examples) contain reference implementations with lots of comments.


**Are geometry shaders required?**

No, the application is free to render the vertex data in any conceivable manner. Geometry shaders are the easiest way to expand points/lines into triangle strips for rendering, but there are alternatives:

- Use instanced rendering to draw 1 triangle strip per point/line primitive and do point/line expansion in the vertex shader, manually fetching the vertex data from a constant buffer. See [examples/OpenGL31](https://github.com/john-chapman/im3d/tree/master/examples/OpenGL31) for a reference implementation.
- Rasterize points/lines directly. If the target graphics API doesn't support per-vertex point size/line width this won't draw as nicely but may be good enough.
- Expand points/lines manually on the CPU and draw the converted vertex data as a single large triangle strip. This would obviate the need for shaders entirely, however the performance may be suboptimal.


**How can I update to the latest version?**

- Check which version you currently have; `IM3D_VERSION` is defined at the top of [im3d.h](https://github.com/john-chapman/im3d/tree/master/im3d.h).
- Examine the change log in the latest version of [im3d.cpp](https://github.com/john-chapman/im3d/tree/master/im3d.cpp) to see if there have been any API-breaking changes which require modifications on the application side.
- Overwrite everything *except* `im3d_config.h` if you have made any changes to your copy.

**Is Im3d thread safe?**

Im3d provides no thread safety mechanism per se, however per-thread contexts are fully supported and can be used to achieve thread safety.
