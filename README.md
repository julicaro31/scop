# scop: A 3D OBJ Model Viewer

`scop` is a small 3D engine written from scratch in C++ that loads a Wavefront `.obj`
file (a common 3D model format) and displays it in a window where you can rotate it
and move it around different directions, with each face shaded its own tone so the geometry reads
clearly. It uses **OpenGL 3.3** to talk to the graphics card, and
deliberately reimplements its own math library instead of depending on an external one.

This document is written for someone who is comfortable with programming but new to
graphics and OpenGL. It explains the concepts as it documents the code, so you can use
it both as a reference and as a study guide.

---

## Table of Contents

1. [What problem does this solve?](#1-what-problem-does-this-solve)
2. [Graphics crash course (read this first)](#2-graphics-crash-course-read-this-first)
3. [The libraries: GLFW and GLAD](#3-the-libraries-glfw-and-glad)
4. [Project layout](#4-project-layout)
5. [How to build and run](#5-how-to-build-and-run)
6. [The big picture: the data flow](#6-the-big-picture-the-data-flow)
7. [Component-by-component walkthrough](#7-component-by-component-walkthrough)
8. [The math library explained](#8-the-math-library-explained)
9. [The MVP pipeline (the heart of 3D)](#9-the-mvp-pipeline-the-heart-of-3d)
10. [The render loop, line by line](#10-the-render-loop-line-by-line)
11. [Controls](#11-controls)
12. [Current limitations / what's not done yet](#12-current-limitations--whats-not-done-yet)
13. [Glossary](#13-glossary)

---

## 1. What problem does this solve?

A 3D model on disk is just a list of numbers: points in space and instructions for how
to connect them into triangles. A screen, on the other hand, is a flat grid of pixels.

The job of this program is the bridge between those two: take a 3D model (the `.obj`
file), figure out where each of its points lands on the 2D screen given a virtual
camera, and color in the pixels. Doing that 60 times per second, while letting the user
spin the model and move it, is what "real-time 3D rendering" means.

---

## 2. Graphics crash course (read this first)

If you've never touched graphics programming, these ideas are the foundation. Everything
else in the codebase is an implementation detail of these concepts.

### The GPU is a separate computer

Your **CPU** runs your C++ code. The **GPU** (graphics card) is a separate processor
optimized for doing the *same simple math on thousands of data points in parallel*, exactly what rendering needs. The two have separate memory.

A huge part of graphics programming is just **shipping data from CPU memory to GPU
memory** and then telling the GPU "now draw with it." You don't draw pixels one at a
time yourself; you hand the GPU a description and it does the drawing.

### Everything is triangles

Any 3D shape (a sphere, a teapot, a dragon) is approximated as a mesh of flat
**triangles**. Triangles are used because they're always flat and always convex, which
makes the math simple and fast. A model is therefore just:

- a list of **vertices** (points in 3D space, each an `x, y, z`), and
- a list of **triangles**, where each triangle is three references into that vertex list.

### Vertices and indices (and why indices exist)

Imagine a cube. It has 8 corners, but 12 triangles (2 per face × 6 faces). If you stored
each triangle as three full `x,y,z` points, you'd repeat each corner many times.

Instead we store the 8 corners **once** in a "vertex array," and then store the triangles
as an "index array": small integers that point at corners. The cube becomes 8 vertices +
36 indices (12 triangles × 3) instead of 36 full vertices. This is called **indexed
drawing**, and it's why this project has both a `vertices` array and an `indices` array.

There's a tradeoff hiding here, though. Sharing vertices saves memory, but it means a
corner vertex belongs to *several* triangles at once. That's fine for position, but it
becomes a problem the moment you want to give each face its own color; a shared vertex
has only one slot and can't hold two different colors. The fix, used for per-face coloring
(see §9), is to *undo* the sharing, called **de-indexing**.

### The rendering pipeline and shaders

When you tell the GPU to draw, your data flows through a fixed sequence of stages called
the **rendering pipeline**. Two of those stages are programmable; you write tiny
programs for them called **shaders**, in a C-like language called **GLSL**:

1. **Vertex shader**: runs *once per vertex*. Its job is to decide where that vertex
   ends up on screen. This is where the 3D-to-2D projection math happens.
2. **Rasterizer** (not programmable): takes the three projected corners of a triangle
   and figures out which pixels fall inside it.
3. **Fragment shader**: runs *once per pixel* covered by a triangle. Its job is to
   decide the **color** of that pixel.

So: vertex shader = *position*, fragment shader = *color*. You'll see exactly these two
shaders in the `shaders/` folder.

### Shaders are compiled at runtime

Unlike your C++ (compiled once by your compiler), shaders are compiled **while the
program runs**, by the GPU driver. The reason: every GPU has a different internal
instruction set, so the GLSL source is compiled on the machine it actually runs on. This
is why the `Shader` class reads `.glsl` text files and compiles them during startup.

### OpenGL is a giant state machine

This is the single most surprising thing for newcomers. OpenGL functions mostly don't
take an object and operate on it. Instead, you **"bind"** an object to make it the
"currently active" one, and subsequent calls implicitly affect whatever is bound. For
example:

```cpp
glBindVertexArray(VAO);     // "this VAO is now the active one"
glDrawElements(...);        // draws using whatever VAO is currently bound
```

There's no `VAO.draw()`. The bound state is global and hidden. Keep this in mind whenever
the code calls a `glBind*` function, it's setting up context for the calls that follow.

---

## 3. The libraries: GLFW and GLAD

OpenGL by itself doesn't know how to open a window or read your keyboard. Those are
operating-system specific. Two small libraries fill the gaps:

- **GLFW**: creates the window, sets up the OpenGL "context" (the canvas OpenGL draws
  onto), and reports input (keyboard, mouse, window-close events). This is the only
  external dependency you have to install.
- **GLAD**: a "function loader." On modern systems, OpenGL functions aren't available
  directly; you have to ask the graphics driver for the address of each one at runtime.
  GLAD does this for you. It's **bundled** in the repo (`src/glad/glad.c`,
  `include/glad/glad.h`), so you don't install it. The line
  `gladLoadGLLoader(...)` in `Renderer` is what performs this loading; until it runs,
  calling any `gl*` function would crash.

---

## 4. Project layout

```
.
├── Makefile                      # Build script (auto-detects macOS vs Linux)
├── shaders/
│   ├── mvp.vert.glsl             # Vertex shader (applies MVP, passes color + UV)
│   └── basic.frag.glsl           # Fragment shader (blends per-face color with texture)
├── include/
│   ├── glad/  KHR/                # Bundled GLAD + Khronos platform headers
│   ├── math/
│   │   ├── Vec3.hpp               # 3D vector
│   │   ├── Vec4.hpp               # 4D vector
│   │   ├── Mat4.hpp               # 4×4 matrix
│   │   └── Math.hpp               # translate/rotate/scale/perspective/lookAt + helpers
│   ├── graphics/
│   │   ├── Renderer.hpp           # Window + OpenGL buffers + draw calls
│   │   ├── Shader.hpp             # GLSL loading/compiling/linking
|   |   ├── ShaderObject.hpp       # Creates shader
│   │   ├── Texture.hpp            # BMP loading + GPU texture object
│   │   ├── Camera.hpp             # Fly-camera (view matrix)
│   │   └── ModelTransform.hpp     # Per-model rotation/scale/centering
│   └── parser/
│       ├── Mesh.hpp               # The CPU-side model data + bounding-box helpers
│       └── ObjParser.hpp          # .obj file reader
├── textures/
│   └── default.bmp               # Fallback 24-bit BMP used when none is supplied
└── src/
    ├── main.cpp                   # Entry point + render loop
    ├── glad/glad.c                # Bundled GLAD implementation
    ├── graphics/                  # Renderer.cpp, Shader.cpp, Camera.cpp, Texture.cpp
    └── parser/ObjParser.cpp       # Parser implementation
```

The code is organized into three conceptual modules: **parser** (file → data),
**math** (linear algebra), and **graphics** (data → screen). `main.cpp` wires them
together.

---

## 5. How to build and run

```bash
make                  # compiles everything into a binary called "scop"
./scop models/teapot.obj   # run it on an .obj file
make re               # clean rebuild
make fclean           # remove the binary and all .o object files
make clean            # remove just the .o object files
```

**Requirements:** GLFW 3 (install via Homebrew on macOS, or your distro's package
manager on Linux) and a GPU that supports OpenGL 3.3.

The `Makefile` detects your OS with `uname -s` and links the right libraries:
on **macOS** it links GLFW from Homebrew plus the `OpenGL`, `Cocoa`, `IOKit`, and
`CoreVideo` frameworks; on **Linux** it links `-lGL -lglfw -ldl -lX11 -lpthread
-lXrandr -lXi`. It compiles the C++ sources and the single C source (`glad.c`)
separately, then links them. Compiler flags are strict: `-Wall -Wextra -Werror`.

---

## 6. The big picture: the data flow

Here is the entire journey of the model, from file to screen:

```
  model.obj  ──▶  ObjParser  ──▶  Mesh  ──▶  Renderer.uploadMesh()  ──▶  GPU memory
 (text file)     (parsing)    (CPU data)      (CPU → GPU transfer)      (VAO/VBO)

                                                          │
                                                          ▼  (every frame)
   Camera + ModelTransform + projection  ──▶  MVP matrix  ──▶  sent to vertex shader
                                                          │
                                                          ▼
                       GPU runs vertex shader → rasterizer → fragment shader
                                                          │
                                                          ▼
                                            pixels appear in the window
```

The first row (parse → upload) happens **once** at startup. The rest happens **every
single frame**, ~60 times a second. During upload the mesh is **de-indexed** (see §9) so
each triangle can carry its own color, which is why the GPU side uses just a VAO and VBO
with no index buffer.

---

## 7. Component-by-component walkthrough

### 7.1 `Mesh` (`include/parser/Mesh.hpp`)

The CPU-side representation of a model. It's a plain struct with two arrays:

```cpp
struct Mesh {
    std::vector<float>        vertices;  // flat: [x0,y0,z0, x1,y1,z1, ...]
    std::vector<unsigned int> indices;   // 3 per triangle, indexing into vertices
};
```

Note that `vertices` is a **flat array of floats**, not an array of `Vec3` objects.
That's because the GPU wants a contiguous block of raw floats; that's the format
`uploadMesh` ships over. Three floats = one vertex.

Helper methods:
- `vertexCount()` → `vertices.size() / 3`
- `triangleCount()` → `indices.size() / 3`
- `center()` → finds the model's **axis-aligned bounding box** (the smallest box that
  contains all points) and returns its center point. Used to recenter the model on the
  origin so it rotates around its middle instead of an arbitrary corner.
- `extent()` → returns half the longest side of that bounding box. Used to scale any
  model down to roughly the same on-screen size.
- `expandedColoredVertices()` → produces the GPU-ready array `uploadMesh` actually ships.
  It **de-indexes** the mesh (writes a private copy of each triangle's three vertices),
  tags every triangle with a single grey, and attaches a generated texture coordinate,
  laid out as `x, y, z, r, g, b, u, v` per vertex (eight floats). The greys are explained
  in §9; the UVs are explained in §7.5.

A **bounding box** is just `(minX, minY, minZ)` to `(maxX, maxY, maxZ)` computed by
sweeping through every vertex. Both helpers do that sweep.

### 7.2 `ObjParser` (`src/parser/ObjParser.cpp`)

Reads a `.obj` text file and produces a `Mesh`. The `.obj` format is line-based plain
text. This parser handles two line types and ignores the rest:

- `v x y z`: a vertex position. Pushed into a temporary `positions` list.
- `f a b c ...`: a face, listing vertices that form a polygon.
- `#`: a comment, skipped. Anything else (normals `vn`, texture coords `vt`,
  materials, groups) is silently ignored.

Two details worth understanding:

**1-based and negative indices.** In `.obj`, vertex indices start at **1**, not 0. They
can also be **negative**, meaning "count backwards from the most recent vertex." The
parser normalizes both into 0-based indices:

```cpp
idx = idx >= 0 ? idx - 1 : idx + positions.size();
```

A face token can also look like `3/1/2` (position/texture/normal). `parseFaceVertex`
just reads the part before the first `/` and throws away the rest, because this viewer
only cares about positions.

**Fan triangulation.** A face can have more than 3 vertices (a quad has 4, etc.), but the
GPU only draws triangles. The parser splits an N-sided polygon into a "fan" of triangles
all sharing the first vertex:

```
polygon  v0 v1 v2 v3 v4   →   triangles (v0,v1,v2) (v0,v2,v3) (v0,v3,v4)
```

That's the loop `for (i = 1; i + 1 < faceIndices.size(); ++i)`. It works correctly for
**convex** polygons (the usual case in `.obj` files).

The parser throws a `std::runtime_error` if the file can't be opened or has no geometry,
and prints warnings for malformed lines without crashing.

### 7.3 `Renderer` (`src/graphics/Renderer.cpp`)

Owns the window and all GPU resources. This is where most of the raw OpenGL lives.

**Constructor**: initializes GLFW, requests an OpenGL **3.3 Core** context (Core =
modern OpenGL, no deprecated functions), creates the window, loads OpenGL functions via
GLAD, and enables **depth testing**:

```cpp
glEnable(GL_DEPTH_TEST);
```

Depth testing is critical for 3D: it makes the GPU remember how far each pixel is from
the camera and only draw a new pixel if it's *closer* than what's already there.
Without it, triangles drawn later would paint over closer ones and the model would look
inside-out and scrambled.

**`uploadMesh`**: the CPU→GPU transfer. It first calls `mesh.expandedColoredVertices()`
to get a **de-indexed**, per-face-colored float array (see §9), then creates two objects:

- **VBO (Vertex Buffer Object)**: a block of GPU memory holding the raw vertex floats,
  now eight per vertex: position `x, y, z` followed by color `r, g, b` and texture `u, v`.
- **VAO (Vertex Array Object)**: *not* data, but a small recording of "how to interpret
  the VBO." It remembers which buffer to read and the layout of the data. Once set up,
  you just bind the VAO before drawing and OpenGL recalls all those settings.

The layout describes **three** attributes packed into each vertex:

```cpp
const GLsizei stride = 8 * sizeof(float);

// location 0: position (3 floats, offset 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
glEnableVertexAttribArray(0);

// location 1: color (3 floats, offset 3 floats in)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// location 2: texture coordinate (2 floats, offset 6 floats in)
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
glEnableVertexAttribArray(2);
```

The **stride** is eight floats: position, then color, then UV.
The `location` numbers match the `layout (location = ...)` lines in the vertex shader; they must agree.

`GL_STATIC_DRAW` is a hint telling the driver "this data won't change after upload," so
it can optimize storage.

**`beginFrame`**: clears the screen to dark gray and clears the depth buffer, wiping the
previous frame so you start fresh:

```cpp
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

**`draw`**: binds the VAO and issues the actual draw command:

```cpp
glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
```

**`endFrame`**: `glfwSwapBuffers` shows the finished frame, and `glfwPollEvents`
processes keyboard/window events. (The window is **double-buffered**: you draw to a
hidden "back buffer" and swap it to the visible "front buffer" at the end, so you never
see a half-drawn frame.)

**Destructor**: deletes the VAO/VBO and terminates GLFW. The class also deletes its
copy constructor and assignment operator, because copying GPU handles would be a bug
(two objects would try to free the same GPU resource).

### 7.4 `Shader` (`src/graphics/Shader.cpp`)

Turns GLSL text files into a usable GPU **shader program**. The constructor:

1. Reads the vertex and fragment shader source from disk (`readFile`).
2. Compiles each one (`compileShader` → `glCreateShader`, `glShaderSource`,
   `glCompileShader`).
3. Links them together into one **program** (`linkProgram` → `glCreateProgram`,
   `glAttachShader`, `glLinkProgram`). A program is the combined, ready-to-run
   vertex+fragment pair.
4. Deletes the individual shader objects (the linked program no longer needs them).

It checks for errors at each step (`checkCompileErrors`, `checkLinkErrors`) and prints
the GPU's error log.

**`use()`** calls `glUseProgram` to make this the active program (state machine again).

**Uniform setters** (`setMat4`, `setFloat`, `setInt`, `setBool`).
A a **uniform** is a
variable you send from C++ into a shader that stays constant for an entire draw call
(as opposed to per-vertex attributes). The MVP matrix is sent this way. Each setter
finds the variable's location by name with `glGetUniformLocation` and uploads the value
with a `glUniform*` call. `setMat4` is the one this project actually uses every frame.

### 7.5 `Texture` (`src/graphics/Texture.cpp`)

Turns an image file on disk into a GPU **texture object**. Like `Shader` and `Renderer`,
it owns a GPU handle, so it forbids copying and frees the handle in its destructor.

The project pulls in no image library, so the loader decodes a **24-bit uncompressed BMP** by hand. Three properties of
the BMP format shape the code:

- **Little-endian header fields.** Multi-byte numbers are stored least-significant-byte
  first. The loader reads them byte-by-byte rather than casting a struct, so it's correct
  on any CPU.
- **Bottom-up rows.** A BMP stores its last scanline first, which happens to match
  OpenGL's convention that the first row of pixel data is the *bottom* of the texture, so
  the image comes out upright with no flip. (Top-down BMPs, marked by a negative height,
  are handled too.)
- **`BGR` order and 4-byte row padding.** The loader swaps channels to `RGB` and repacks
  rows tightly, then sets `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)` so OpenGL doesn't assume
  4-byte-aligned rows and read the image skewed.

After decoding, it uploads with `glTexImage2D` and sets two behaviors: **wrapping**
(`GL_REPEAT` — what to sample when a UV leaves `[0,1]`) and **filtering** (`GL_LINEAR` plus
**mipmaps** via `glGenerateMipmap`, so distant surfaces don't shimmer). `bind(unit)` makes
the texture active on a texture unit so the fragment shader's sampler can read it.

**Where do the texture coordinates come from?** The `.obj` provides none (the parser drops
`vt` lines), so they're *generated*, exactly as the per-face greys are. `Mesh` uses a
**planar projection**: each vertex's X/Y position, normalized into `[0,1]` by the bounding
box, becomes its `(u, v)`. This is a projection, not an unwrap, so faces that turn edge-on
to the projection get stretched into streaks. On the other hand, a spherical
mapping (longitude/latitude from the model center) would wrap fully.

### 7.6 `Camera` (`src/graphics/Camera.cpp`)

A first-person "fly" camera. It stores a `_position` and two angles: **`_yaw`** (turning
left/right) and **`_pitch`** (looking up/down). From those angles `getFront()` computes a
direction vector using trigonometry, the standard spherical-to-Cartesian conversion.

`processInput` reads WASD/QE keys and moves the position along the camera's own axes (inverted so it's the object the one that looks like it's moving).
The `right` vector is computed as `front × up` (a **cross product**, which gives a vector
perpendicular to both). Movement is scaled by `deltaTime` so it's frame-rate independent
(more on that below).

`getViewMatrix()` calls `Math::lookAt(position, position + front, up)` to build the
**view matrix**: the transform that moves the whole world so the camera sits at the
origin looking down −Z. (See the MVP section.)

### 7.7 `ModelTransform` (`include/graphics/ModelTransform.hpp`)

A header-only struct controlling the *model's* orientation, separate from the camera. It
stores `yaw`, `pitch`, `roll` (rotations about the Y, X, and Z axes), driven by the arrow
keys and Z/X.

`getModelMatrix(center, scale)` builds the **model matrix** by composing several
transforms. Because matrix multiplication applies **right-to-left** to a vertex, the
operations actually happen to the model in this order:

1. **Translate by center** → move the model so its bounding-box center sits on the origin.
2. **Scale** → shrink/grow it to unit size.
3. **Rotate** by roll, then pitch, then yaw.

Recentering first is what makes the model spin around its own middle rather than drifting
in a circle around some far-off point.

---

## 8. The math library explained

The project rolls its own linear algebra (a stripped-down clone of the popular GLM
library) so it has zero math dependencies. Everything is **column-major**, the
convention OpenGL expects.

### `Vec3` (`include/math/Vec3.hpp`)

A 3D vector with `x, y, z` and the usual operations:
- `+`, `-`, `* scalar`: component-wise arithmetic.
- `dot(v)`: the **dot product**, a single number measuring how aligned two vectors are.
- `cross(v)`: the **cross product**, a vector perpendicular to both inputs (used to find
  the camera's "right" direction).
- `length()`, `normalize()`, `normalized()`: magnitude and scaling to unit length. A
  *normalized* vector keeps direction but has length 1, which is what you want for pure
  direction vectors.

### `Vec4` (`include/math/Vec4.hpp`)

A 4D vector `x, y, z, w`. The fourth component `w` is the key to 3D graphics math (see
"homogeneous coordinates" below). It can be built from a `Vec3` plus a `w` value.

### `Mat4` (`include/math/Mat4.hpp`)

A 4×4 matrix stored as **four `Vec4` columns**: `Vec4 m[4]`. Why 4×4 and not 3×3? Because
a 3×3 matrix can rotate and scale, but **cannot translate** (move) a point. By going up to
4 dimensions with that extra `w` component (called **homogeneous coordinates**) a single
4×4 matrix can encode rotation, scaling, *and* translation all at once. That uniformity is
why all of 3D graphics is built on 4×4 matrices.

The struct provides:
- A constructor `Mat4(n)` that builds a diagonal matrix; `Mat4(1.0f)` is the **identity**
  matrix (the "do nothing" transform, the matrix equivalent of multiplying by 1).
- `operator*(Vec4)`: transforms a vector by the matrix.
- `operator*(Mat4)`: composes two transforms into one. **Order matters**: `A * B` means
  "apply B first, then A."
- `data()`: returns a raw `const float*` pointer so the matrix can be handed straight to
  OpenGL via `glUniformMatrix4fv`.

### `Math` (`include/math/Math.hpp`)

A class of static functions plus the helpers `toRadians`/`toDegrees` and the constant
`PI`. (Trig functions work in radians, but humans think in degrees, hence the converters.)

- **`translate(m, v)`**: adds a movement to a matrix by updating its last column.
- **`rotate(m, angle, axis)`**: rotates around an arbitrary axis using the standard
  axis-angle rotation formula (Rodrigues' formula expanded into matrix form).
- **`scale(m, v)`**: multiplies the matrix's basis columns to resize.
- **`perspective(fov, aspect, near, far)`**: builds the **projection matrix** (next
  section).
- **`lookAt(eye, center, up)`**: builds the **view matrix** by constructing an
  orthonormal basis (right/up/forward) from where the camera is and what it looks at.

---

## 9. The MVP pipeline (the heart of 3D)

This is the single most important concept in the whole project. To get a 3D point onto
your 2D screen, you pass it through **three matrices**, conventionally called
**M**, **V**, and **P**, multiplied together into one **MVP** matrix.

Think of each matrix as a change of "frame of reference", a different coordinate space:

| Matrix | Name | What it does | "Space" you end up in |
|--------|------|--------------|------------------------|
| **M** | Model | Positions/orients/sizes the model in the world | **World space** |
| **V** | View | Moves the world so the camera is at the origin | **View (camera) space** |
| **P** | Projection | Applies perspective; far things shrink | **Clip space** |

In code (`main.cpp`):

```cpp
Mat4 mvp = projection * camera.getViewMatrix() * model.getModelMatrix(meshCenter, scaleFactor);
```

Because matrices apply right-to-left, a vertex experiences: **Model → View →
Projection**, exactly the table order. The combined `mvp` is then sent to the vertex
shader as a uniform, and the shader transforms every vertex with one multiplication:

```glsl
// shaders/mvp.vert.glsl
gl_Position = mvp * vec4(aPos, 1.0);
```

A few notes on the pieces:

- **Why the model is centered and scaled.** `getModelMatrix` recenters using
  `mesh.center()` and scales by `1 / mesh.extent()` so that *any* model, regardless of
  its original units or position, ends up roughly unit-sized and centered; so it's
  always nicely framed when the program starts.
- **The projection matrix** is built once with a 45° field of view, the window's aspect
  ratio (800/600), and near/far clipping planes at 0.1 and 100. The clipping planes
  define the range of distances that are actually drawn. Perspective is what makes
  distant parts of the model look smaller; it's encoded in how this matrix sets up the
  `w` component, which the GPU later divides by ("perspective divide").
- **The view matrix** comes from the camera each frame, since the camera can move.

### What the shaders do

```glsl
// mvp.vert.glsl — runs per vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;
uniform mat4 mvp;
out vec3 vColor;
out vec2 vUV;
void main() {
    vColor = aColor;
    vUV = aUV;
    gl_Position = mvp * vec4(aPos, 1.0);
}
```

```glsl
// basic.frag.glsl — runs per pixel
in vec3 vColor;
in vec2 vUV;
uniform sampler2D tex;     // texture, read from unit 0
uniform float useTexture;  // 0.0 = grey, 1.0 = texture
out vec4 FragColor;
void main() {
    vec3 texColor = texture(tex, vUV).rgb;
    FragColor = vec4(mix(vColor, texColor, useTexture), 1.0);
}
```

Each face of the model carries its own grey, so its surfaces read even before you rotate it.

`mix(a, b, t)` returns `a` when `t = 0`, `b` when `t = 1`, and a linear blend in between.
`useTexture` is driven from C++ by `texMix`, a value that eases smoothly through the whole
`[0, 1]` range when you press T (see §10). So this one line gives both the per-face grey,
the full texture, and every fade between them. (Holding `useTexture` at exactly `0` or `1`
would make it a hard switch; animating it is what makes the toggle smooth.)

### Per-face coloring

Giving each face its own shade is what lets you perceive the model's surfaces without a
lighting model. It rests on three ideas that work together.

**Color is a per-vertex attribute, not a per-face one.** A GPU can attach data to each
*vertex* (like position) or send one value for the whole *draw call* (a uniform). "Per
face" is neither. And because the original mesh uses indexed drawing, a single vertex is
*shared* by every triangle that touches it, so it has only one color slot and can't be
one shade for one face and a different shade for its neighbor.

**De-indexing resolves the conflict.** `Mesh::expandedColoredVertices()` removes the
sharing: it walks the triangles and writes out a fresh, private copy of each triangle's
three vertices, so every triangle owns its corners outright. Now those three vertices can
all be given the *same* color and there's nothing to fight over. The price is memory
(roughly three vertices per triangle, since nothing is reused).

**The color flows down the pipeline as a "varying."** It starts as the `aColor` attribute
(location 1), the vertex shader copies it into `out vec3 vColor`, and the fragment shader
reads it as `in vec3 vColor`. A value passed this way from the vertex stage to the
fragment stage is called a **varying**. Normally the GPU interpolates a varying across the
triangle, blending the three corners, but since all three corners hold the *same* color
here, there's nothing to blend and the face comes out perfectly flat.

**Choosing the shades.** The grey for triangle *i* is `0.25 + 0.60 * frac(i * φ)`, where
`φ ≈ 0.618` is the golden-ratio conjugate. Multiplying the index by an irrational number
and keeping only the fractional part scatters *neighboring* triangles to very different
shades, instead of a near-invisible gradient, while the `0.25`–`0.85` range keeps them
out of pure black and white. It's fully deterministic, so a given triangle gets the same
shade on every run. (To switch from greys to full color later, you'd feed that same
scrambled value into a hue instead of into all three channels equally.)

One subtlety: this colors per *triangle*, so a face that was a quad in the `.obj` shows up
as two slightly different shades split along its triangulation diagonal. Coloring per
*original face* instead would require the parser to remember which triangles came from the
same `f` line.

---

## 10. The render loop, line by line

`main.cpp` is the conductor. After parsing and uploading the mesh and creating the
shader, camera, and transform, it enters the loop that runs until you close the window:

```cpp
shader.use();
shader.setInt("tex", 0);   // sampler reads from texture unit 0 (set once)

bool  showTexture = false; // TARGET mode, flipped by T
float texMix = 0.0f;       // ANIMATED value actually sent to the shader

while (!renderer.shouldClose()) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;   // seconds since last frame
    lastFrame = currentFrame;

    if (ESC pressed) close the window;

    if (T just pressed) showTexture = !showTexture;   // flip the TARGET

    // Ease the displayed value toward the target at a constant rate, so a full
    // fade takes a fixed time (~0.3s) on any machine; clamp so it never overshoots.
    float target = showTexture ? 1.0f : 0.0f;
    float step = deltaTime / fadeDuration;
    texMix = move texMix toward target by step;

    camera.processInput(window, deltaTime);        // move camera from WASD/QE
    model.processInput(window, deltaTime);          // rotate model from arrows/Z/X

    Mat4 mvp = projection * camera.getViewMatrix()
                          * model.getModelMatrix(meshCenter, scaleFactor);

    renderer.beginFrame();      // clear screen + depth
    shader.setMat4("mvp", mvp);
    shader.setFloat("useTexture", texMix);   // a value in [0,1], not just 0 or 1
    texture.bind(0);
    renderer.draw();            // issue the draw call
    renderer.endFrame();        // swap buffers + poll input
}
```

**Why `deltaTime`?** Different computers run the loop at different speeds. If movement
were a fixed amount per frame, the model would spin faster on a fast machine. By
multiplying movement by `deltaTime` (the real seconds elapsed since the last frame),
motion is measured in "units per second" and feels identical everywhere. This is called
**frame-rate-independent movement**.

**Why edge-detect the T key?** `glfwGetKey` reports whether a key is *currently* down, and
the loop runs ~60 times a second, so one human press spans many frames. The toggle
compares the key's current state to its state last frame and flips only on the
up-to-down *transition*, otherwise the texture would strobe on and off while T is held.

**Why two variables for one toggle?** A `bool` has no in-between, so it can only *snap*.
The fade therefore splits the state in two: `showTexture` is the **target** (where we want
to be, flipped by T), and `texMix` is the **animated current value** in `[0, 1]` that is
actually sent to the shader. Each frame `texMix` takes one small step *toward* the target
rather than jumping to it, and the fragment shader's `mix(vColor, texColor, texMix)` turns
that number into a blend. The step is `deltaTime / fadeDuration`, so, exactly like camera
movement, the fade lasts the same wall-clock time (~0.3 s) regardless of frame rate, and
the value is clamped so the last partial step lands exactly on `0` or `1` instead of
overshooting. A nice side effect: because `texMix` always chases whatever the target
currently is, pressing T mid-fade simply reverses the motion smoothly, with no
"animation in progress" state to track.

---

## 11. Controls

| Key(s) | Action |
|--------|--------|
| **W / S** | Move model forward / backward |
| **A / D** | Move model left / right |
| **E / Q** | Move model up / down |
| **← / →** | Rotate model (yaw, around vertical axis) |
| **↑ / ↓** | Rotate model (pitch, around horizontal axis) |
| **Z / X** | Roll model (around the front-facing axis) |
| **T** | Smoothly fade texture on / off (per-face color ↔ texture) |
| **Esc** | Quit |

---

## 12. Glossary

- **Vertex**: a single point in 3D space (`x, y, z`).
- **Index / Element**: an integer referencing a vertex, used to build triangles without
  duplicating vertex data.
- **Mesh**: a model as vertices + indices.
- **Shader**: a small GPU program (vertex shader = position, fragment shader = color).
- **GLSL**: the C-like language shaders are written in.
- **VAO / VBO / EBO**: GPU objects: VAO records *how* to read vertex data; VBO holds the
  vertex floats; EBO holds the indices. Since the mesh is de-indexed for per-face color,
  the current build uses only a VAO and VBO.
- **Uniform**: a variable sent from CPU to a shader, constant for one draw call (e.g.
  the MVP matrix).
- **Attribute**: per-vertex input to the vertex shader (e.g. position at location 0,
  per-face color at location 1).
- **Varying**: a value the vertex shader outputs and the fragment shader receives,
  interpolated across the triangle by default (e.g. the per-face color `vColor`).
- **De-indexing (unwelding)**: expanding indexed geometry so each triangle owns a private
  copy of its three vertices instead of sharing them; needed so each face can carry its
  own color.
- **MVP**: Model × View × Projection, the combined transform from 3D model space to 2D
  screen space.
- **Model / World / View / Clip space**: successive coordinate systems a vertex passes
  through.
- **Homogeneous coordinates**: using a 4th component `w` so that a single 4×4 matrix can
  represent translation as well as rotation/scaling.
- **Projection**: the step that creates the perspective effect (distant = smaller).
- **Rasterization**: converting a triangle into the pixels it covers.
- **Depth test (Z-buffer)**: per-pixel distance tracking so nearer surfaces correctly
  hide farther ones.
- **Double buffering**: drawing to a hidden buffer and swapping it in, to avoid showing
  half-drawn frames.
- **Delta time**: real seconds elapsed since the previous frame, used to keep motion
  speed consistent across machines.
- **GLFW**: library for windowing and input.
- **GLAD**: library that loads OpenGL function pointers at runtime.
- **Wavefront OBJ**: a plain-text 3D model file format.
- **Column-major**: the matrix storage order OpenGL expects.
- **Texture**: a 2D image stored in GPU memory and sampled onto a surface.
- **Texel**: a single cell of a texture image (the texture analogue of a pixel).
- **Texture coordinate (UV)**: a 2D coordinate in `[0, 1]` naming a point in the texture
  image; `(0,0)` and `(1,1)` are opposite corners. Attached per vertex, interpolated across
  each triangle.
- **Sampler**: the fragment-shader object (`sampler2D`) that reads a texture at a given UV.
- **Wrapping**: how UVs outside `[0, 1]` are handled (e.g. `GL_REPEAT` tiles the image).
- **Filtering**: how texels are blended when the texture is scaled on screen
  (`GL_NEAREST` = blocky, `GL_LINEAR` = smooth).
- **Mipmap**: a chain of pre-shrunk copies of a texture, sampled when a surface is small
  on screen to avoid aliasing.
- **Planar projection**: generating UVs by projecting vertex positions onto a plane;
  simple but stretches surfaces that face away from the projection axis.
- **BMP**: a simple, uncompressed raster image format; the one this viewer can load.
- **Easing toward a target**: animating a value by moving it a fraction of the remaining
  distance toward a goal each frame (scaled by delta time), rather than snapping. Used for
  the smooth texture fade: the target is a boolean mode, the eased value is the `[0, 1]`
  blend factor sent to the shader.
