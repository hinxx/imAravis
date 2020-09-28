# imAravis
Dear ImGui aravis interface

## How to do own GL drawing?

To draw color gradient for greyscale images from the detector I would need a custom shader under openGL 3.0 (that is what I think I need).

How to get this done in Imgui?

1. Ine `imgui_impl_opengl3.cpp` the `ImGui_ImplOpenGL3_RenderDrawData()` it has ability to call `pcmd->UserCallback`, instead of
using default `ImGui_ImplOpenGL3_SetupRenderState()` to setup GL context. Need to figure out if this is what I want and how to use it!

Some resources.

https://github.com/ocornut/imgui/releases

	ImDrawList: Added ImDrawCallback_ResetRenderState = -1, a special ImDrawList::AddCallback() value to request the renderer back-end
	to reset its render state. (#2037, #1639, #2452). Added support for ImDrawCallback_ResetRenderState in all renderer back-ends.
	Each renderer code setting up initial render state has been moved to a function so it could be called at the start of rendering 
	and when a ResetRenderState is requested. [@ocornut, @bear24rw]


2. https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/

See chapter "The shader", "Additional buffers & uniforms" that describes binding texture to a texture units.

3. https://www.khronos.org/opengl/wiki/Sampler_(GLSL)

Chapter "Binding textures to samplers" talking about the same topic a # 2.

The `glBindTextureUnit()` found in core GL 4.5 or ARB_direct_state_access on 3.x!

4. https://www.khronos.org/opengl/wiki/Texture

See chapters "Sampling parameters", "Texture image units" and "GLSL binding". 

5. https://www.khronos.org/opengl/wiki/Uniform_(GLSL)

Describes how the GLSL and user (c++) work with uniforms; they store data for GLSL sampling.
ImGui in `ImGui_ImplOpenGL3_CreateDeviceObjects()` defines one program with `glCreateProgram()` that has one shader,
composed of 1 vertex and 1 fragment shader.
In `ImGui_ImplOpenGL3_RenderDrawData()` is saved before actual drawing starts, and then restored at the end.

What if we had a custom program to work with our greyscale images? In it we would have as special shader that
would also be capable of using user defined gradient; image data and gradient data would both come from an uniform.
Would probably need to make sure that the rendered image window location is proper (still within ImGui window)!

6. https://www.khronos.org/opengl/wiki/Common_Mistakes

See chapter on "Paletted textures". This is what we need!!!

Shader example:

	//Fragment shader
	#version 110
	uniform sampler2D ColorTable;     //256 x 1 pixels
	uniform sampler2D MyIndexTexture;
	varying vec2 TexCoord0;
	
	void main()
	{
	  //What color do we want to index?
	  vec4 myindex = texture2D(MyIndexTexture, TexCoord0);
	  //Do a dependency texture read
	  vec4 texel = texture2D(ColorTable, myindex.xy);
	  gl_FragColor = texel;   //Output the color
	}

`ColorTable` might be in a format of your choice such as `GL_RGBA8`. `ColorTable` could be a texture of 256 x 1 pixels in size.

`MyIndexTexture` can be in any format, though `GL_R8` is quite appropriate (`GL_R8` is available in GL 3.0). `MyIndexTexture`
could be of any dimension such as 64 x 32.

We read `MyIndexTexture` and we use this result as a texcoord to read `ColorTable`. If you wish to perform palette animation, or
simply update the colors in the color table, you can submit new values to `ColorTable` with `glTexSubImage2D`.

Assuming that the color table is in `GL_RGBA` format:

	glBindTexture(GL_TEXTURE_2D, myColorTableID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_BGRA, GL_UNSIGNED_BYTE, mypixels);



7. https://www.khronos.org/opengl/wiki/GLSL_:_common_mistakes

8. https://www.reddit.com/r/gamedev/comments/a9tnkk/anyone_made_an_enginegame_app_with_imgui_here/

Points to # 9.

9. https://github.com/ocornut/imgui/issues/984

How to draw on an ImGui window? #984

@ocornut commented on Jan 17, 2017

	You can:
	
	1- Render your scene in a texture and then render the texture in imgui. For the second part read about
	comments related to ImTextureId and check the code in your imgui_impl_xxxx.cpp renderer.
	Check the Image() functions, and ImDrawList::AddImage() etc.
	
	2- Add a Callback to the current window ImDrawList and perform your full rendering within this callback
	at the right point during rendering.
	
	The first one is simple and convenient and this is what most people would do.
	The second one can be useful if you need to save 1 render pass but it is rarely a real problem.


@citruslee commented on Jan 18, 2017

	@morizotter in my engine I create a plus one more buffer for my engine and I let ImGui draw on backbuffer. 
	So this one more buffer is actually the "backbuffer" for my engine, since then I do postprocessing over the 
	image, so it makes sense to make it like this. So just make your engine backbuffer a RenderTargetView or FBO 
	(or whatever you fancy) and pass it to ImGui control as image. Easy, no hassle and quite clean solution

morizotter commented on Jan 18, 2017 • 

	I'm also new to C++... I's a bit difficult for me..
	
	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window) {
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
	
		ImGui::GetWindowDrawList()->AddCallback(draw_callback, NULL);
	
		ImGui::End();
	}

	and,

	static void draw_callback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
	{
		printf("hello!\n");
		{
			static GLuint programID = LoadShaders( "/Users/moritanaoki/Desktop/rendering-in-imgui/SimpleVertexShader.vert", "/Users/moritanaoki/Desktop/rendering-in-imgui/SimpleFragmentShader.frag" );
			glViewport(0, 0, 300, 300);
	
			glUseProgram(programID);
	
			GLuint VertexArrayID;
			glGenVertexArrays(1, &VertexArrayID);
			glBindVertexArray(VertexArrayID);
	
			static const GLfloat g_vertex_buffer_data[] = {
					-1.0f, -1.0f, 0.0f,
					1.0f, -1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
			};
	
			GLuint vertexbuffer;
			glGenBuffers(1, &vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void *) 0            // array buffer offset
			);
			glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
			glDisableVertexAttribArray(0);
		}
	}

	produces:
	
	https://cloud.githubusercontent.com/assets/536954/22075502/8b4160d6-ddef-11e6-803d-2158484f6df4.png
	
	[on th epic there is a red triangle bottom left, not in a ImGui window]
	
	I want to a draw red triangle in the window..
	
	Sorry for easy question..

ocornut commented on Jan 18, 2017

	The callback won't know anything about windows. If you want to draw at a certain screen position you may use eg
	GetCursorScreenPos() and transfer that information into your callback.


ocornut commented on Jan 18, 2017

	Note you might use the ImDrawList api as a primitive renderer if you can rely on using the ImDrawVert type
	for all your vertices. You can then change shaders/uniform/constantbuffers using callback. Of course this is only a
	solution if your rendering needs are simple and you can't/don't want to create a full fledge renderer.


ocornut commented on May 1, 2017

	@morizotter Have you solved this problem?
	
	I strongly suggest you should aim for the first solution that I posted in my initial answer, aka render to a
	different render target and render this render target as a texture in imgui.
	
	Using callbacks and rendering without an intermediary target is harder to get right.


Mentions # 10 ..

10. https://github.com/ocornut/imgui/pull/1639

11. https://gamedev.stackexchange.com/questions/150214/render-in-a-imgui-window

12. https://learnopengl.com/Advanced-OpenGL/Framebuffers

