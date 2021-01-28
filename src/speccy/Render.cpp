#include "Render.hpp"

namespace Speccy
{
    static std::string vertex_shader_string =
        R"(#version 100
        #ifdef GL_ES
        precision mediump float;
        #endif
        uniform mat4 projection;
        uniform mat4 view;
        attribute vec3 position;
        attribute vec2 texcoord;
        varying vec2 v_texcoord;
        void main()
        {
            v_texcoord = texcoord;
            gl_Position = projection * view * vec4(position, 1.0);
        })";

    static std::string fragment_shader_string =
        R"(#version 100
        #ifdef GL_ES
        precision mediump float;
        #endif
        varying vec2 v_texcoord;
        uniform sampler2D tex;
        vec3 to_linear_approx(vec3 v) { return pow(v, vec3(2.2)); }
        vec3 to_gamma_approx(vec3 v) { return pow(v, vec3(1.0 / 2.2)); }
        void main()
        {
            vec3 c = texture2D(tex, v_texcoord).xyz;
            gl_FragColor = vec4(c, 1.0);
        })";

    static const std::vector<float> quad_vertices_data
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f
    };

    static const std::vector<uint32_t> quad_indices_data
    {
         0, 1, 2, 2, 3, 0
    };

    Render::Render()
    {
    }

    void Render::Init(
        std::vector<uint32_t>& display_pixels)
    {
        display_texture_data = reinterpret_cast<uint8_t*>(
            &display_pixels[0]);

        display_texture = OpenGL::GenTextureRGBA8(
            320,
            240,
            display_texture_data);

        gl_shader_program = OpenGL::LinkShader(
            vertex_shader_string,
            fragment_shader_string);

        OpenGL::GLCheckError();

        position_attribute_location = glGetAttribLocation(
            gl_shader_program,
            "position");

        texcoord_attribute_location = glGetAttribLocation(
            gl_shader_program,
            "texcoord");

        projection_uniform_location = glGetUniformLocation(
            gl_shader_program,
            "projection");

        view_uniform_location = glGetUniformLocation(
            gl_shader_program,
            "view");

        texture_uniform_location = glGetUniformLocation(
            gl_shader_program,
            "tex");

        vertex_buffer = OpenGL::GenBuffer(
            quad_vertices_data);

        index_buffer = OpenGL::GenBufferIndex(
            quad_indices_data);

        OpenGL::GLCheckError();
    }

    void Render::Deinit()
    {
        glDeleteProgram(
            gl_shader_program);

        glDeleteBuffers(
            1, &vertex_buffer);

        glDeleteBuffers(
            1, &index_buffer);
    }

    void Render::Draw()
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        const uint32_t width = 320 * 4;
        const uint32_t height = 256 * 4;

        glViewport(
            0,
            0,
            width,
            height);

        glClearColor(
            1, 1, 1, 1);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

        proj = glm::ortho<float>(
            0,
            static_cast<float>(1),
            static_cast<float>(1),
            0,
            -1.0f,
            1.0f);

        view = glm::mat4();

        glActiveTexture(
            GL_TEXTURE0);

        glBindTexture(
            GL_TEXTURE_2D,
            display_texture);

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            gl_internal_format,
            320,
            256,
            0,
            gl_format,
            gl_type,
            (GLvoid*)display_texture_data);

        glGenerateMipmap(
            GL_TEXTURE_2D);

        glUseProgram(
            gl_shader_program);

        glUniformMatrix4fv(
            projection_uniform_location,
            1,
            false,
            &proj[0][0]);

        glUniformMatrix4fv(
            view_uniform_location,
            1,
            false,
            &view[0][0]);

        glActiveTexture(
            GL_TEXTURE0);

        glBindTexture(
            GL_TEXTURE_2D,
            display_texture);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);

        glUniform1i(
            texture_uniform_location,
            0);

        glBindBuffer(
            GL_ARRAY_BUFFER,
            vertex_buffer);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            position_attribute_location,
            3,
            GL_FLOAT,
            GL_FALSE,
            5 * sizeof(GLfloat),
            (GLvoid*)0);

        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            texcoord_attribute_location,
            2,
            GL_FLOAT,
            GL_FALSE,
            5 * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            index_buffer);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(quad_indices_data.size()),
            GL_UNSIGNED_INT,
            static_cast<char const*>(0));

        glBindTexture(
            GL_TEXTURE_2D,
            NULL);

        glUseProgram(
            NULL);
    }
}