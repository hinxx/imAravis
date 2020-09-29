# imAravis

Dear ImGui aravis interface

## Using fake camera

With debug output:

	$ ./aravis/bin/arv-fake-gv-camera-0.8 -s GV02 -d all -g aravis/share/aravis-0.8/arv-fake-camera.xml 

Normal:
	
	$ ./aravis/bin/arv-fake-gv-camera-0.8 -s GV02 -g aravis/share/aravis-0.8/arv-fake-camera.xml 


## How to do own GL drawing?

To draw color gradient for greyscale images from the detector I have a custom shader
for openGL 3.0 that renders to a framebufer object (FBO) texture. This texture is then
used as a textureID with the ImGui::Image() call.

Shader has access to an indexed image and a colormap. Colomap can be any colormap; see
https://github.com/kbinani/colormap-shaders for example colormap.


## Resources

	* http://docs.gl/gl3/glTexImage2D
	* https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
	* https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
	* https://www.khronos.org/opengl/wiki/Texture
	* https://www.khronos.org/opengl/wiki/Uniform_(GLSL)
	* https://www.khronos.org/opengl/wiki/Common_Mistakes
	* https://www.khronos.org/opengl/wiki/GLSL_:_common_mistakes
	* https://www.reddit.com/r/gamedev/comments/a9tnkk/anyone_made_an_enginegame_app_with_imgui_here/
	* https://github.com/ocornut/imgui/issues/984
	* https://github.com/ocornut/imgui/pull/1639
	* https://gamedev.stackexchange.com/questions/150214/render-in-a-imgui-window
	* https://learnopengl.com/Advanced-OpenGL/Framebuffers
	* https://gamedev.stackexchange.com/questions/43294/creating-a-retro-style-palette-swapping-effect-in-opengl
	* https://github.com/kbinani/colormap-shaders

