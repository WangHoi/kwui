#include "TriangleGL.h"
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#ifdef _WIN32
#include <GL/gl.h>
#endif
#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#endif

void TriangleGL::draw(ID3D11Device1* device,
                      kwui::CustomElementPaintContextInterface& painter,
                      const kwui::CustomElementPaintOption& po)
{
	if (nullptr == wglGetCurrentContext()) {
		return;
	}

	struct FreeModule { void operator()(HMODULE m) { (void)FreeLibrary(m); } };
	std::unique_ptr<typename std::remove_pointer<HMODULE>::type, FreeModule> module(
			LoadLibraryA("opengl32.dll"));
	if (!module) {
		return;
	}
	GLADuserptrloadfunc win_get_gl_proc = [](void* ctx, const char* name) -> GLADapiproc {
		SkASSERT(wglGetCurrentContext());
		if (GLADapiproc p = (GLADapiproc)GetProcAddress((HMODULE)ctx, name)) {
			return p;
		}
		if (GLADapiproc p = (GLADapiproc)wglGetProcAddress(name)) {
			return p;
		}
		return nullptr;
	};
	gladLoadGLUserPtr(win_get_gl_proc, (void*)module.get());

    auto dpi_scale = painter.getDpiScale();
    auto pixel_width = po.width * dpi_scale;
    auto pixel_height = po.height * dpi_scale;

	struct Vertex {
		float position[3];
		float color[4];
	};

	GLuint old_vao, old_fbo, old_tex2d;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&old_vao);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&old_fbo);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&old_tex2d);

	GLuint fbo = 0;
	GLuint texture;
	{
		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_width, pixel_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLuint vao = 0;
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		Vertex vertices[] = {
			{{+0.0f, +0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
			{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
			{{+0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		};

		GLuint vbo = 0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, color));
	}

	GLuint program = glCreateProgram();
	{
		const char* vert_glsl = R"(
      #version 330 core

      layout(location=0) in vec3 a_position;
      layout(location=1) in vec4 a_color;

      out vec4 v_color;

      void main() {
        gl_Position = vec4(a_position, 1.0);
        v_color = a_color;
      }
    )";

		const char* frag_glsl = R"(
      #version 330 core

      in vec4 v_color;
      out vec4 f_color;

      void main() {
        f_color = v_color;
      }
    )";

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vert_glsl, 0);
		glCompileShader(vs);
		glAttachShader(program, vs);

		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &frag_glsl, 0);
		glCompileShader(fs);
		glAttachShader(program, fs);

		glLinkProgram(program);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

    // Render
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, pixel_width, pixel_height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fbo);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);

	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	glBindVertexArray(old_vao);
	glBindTexture(GL_TEXTURE_2D, old_tex2d);

    painter.setFillBitmap((void*)(uintptr_t)texture, dpi_scale);
    painter.drawRoundedRect(po.left, po.top, po.width, po.height, 8);
}
