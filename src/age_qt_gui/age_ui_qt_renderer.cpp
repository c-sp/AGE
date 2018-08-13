//
// Copyright 2018 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <algorithm> // std::min, std::max
#include <utility> // std::move

#include <QRectF>
#include <QSurfaceFormat>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <age_debug.hpp>

#include "age_ui_qt_renderer.hpp"

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

// we don't really use the alpha channel but OpenGL ES would not work without it
constexpr QOpenGLTexture::TextureFormat tx_format = QOpenGLTexture::RGBAFormat;
constexpr QOpenGLTexture::PixelFormat tx_pixel_format = QOpenGLTexture::RGBA;
constexpr QOpenGLTexture::PixelType tx_pixel_type = QOpenGLTexture::UInt8;



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_renderer::qt_renderer(QWidget *parent)
    : QOpenGLWidget(parent),
      m_indices(QOpenGLBuffer::IndexBuffer)
{
    // Which OpenGL Version is being used?
    // https://stackoverflow.com/questions/41021681/qt-how-to-detect-which-version-of-opengl-is-being-used
    LOG("OpenGL Module Type: " << QOpenGLContext::openGLModuleType()
        << " (LibGL " << QOpenGLContext::LibGL << ", LibGLES " << QOpenGLContext::LibGLES << ")");

    LOG("default format version: " << format().majorVersion() << "." << format().minorVersion());
    LOG("default format options: " << format().options());
}

age::qt_renderer::~qt_renderer()
{
    makeCurrent();

    m_vertices.destroy();
    m_indices.destroy();

    // explicitly delete textures since the OpenGL context is required
    std::for_each(begin(m_frame_texture), end(m_frame_texture), [](auto &texture) {
        texture = nullptr;
    });

    doneCurrent();
}





//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

age::uint age::qt_renderer::get_fps() const
{
    //! \todo implement age::qt_renderer::get_fps
    return 0;
}



void age::qt_renderer::set_emulator_screen_size(uint width, uint height)
{
    LOG(width << ", " << height);
    m_emulator_screen = QSize(width, height);

    // allocate texture only after initializeGL() has been called
    if (textures_initialized())
    {
        makeCurrent();
        allocate_textures();
        doneCurrent();
    }
    update_projection_matrix();

    update(); // trigger paintGL()
}

void age::qt_renderer::new_frame(std::shared_ptr<const age::pixel_vector> new_frame)
{
    new_frame_slot(new_frame);
}



void age::qt_renderer::set_blend_frames(uint num_frames_to_blend)
{
    LOG(num_frames_to_blend);

    num_frames_to_blend = std::max(num_frames_to_blend, 1u);
    num_frames_to_blend = std::min(num_frames_to_blend, m_frame_texture.size());
    m_num_frames_to_blend = num_frames_to_blend;

    update(); // trigger paintGL()
}

void age::qt_renderer::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
    //! \todo implement age::qt_renderer::set_filter_chain
}

void age::qt_renderer::set_bilinear_filter(bool bilinear_filter)
{
    m_bilinear_filter = bilinear_filter;

    // update textures only after initializeGL() has been called
    if (textures_initialized())
    {
        makeCurrent();
        set_texture_filter();
        doneCurrent();

        update(); // trigger paintGL()
    }
}





//---------------------------------------------------------
//
//   OpenGL rendering
//
//---------------------------------------------------------

void age::qt_renderer::initializeGL()
{
    LOG("format version: " << format().majorVersion() << "." << format().minorVersion());

    initializeOpenGLFunctions();

    // Log OpenGL Version information.
    // (OpenGL functions must be initialized)
    LOG("GL_VERSION: " << glGetString(GL_VERSION));
    LOG("GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION));

    // OpenGL configuration
    glClearColor(0, 0, 0, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // shader program (failures are logged by Qt)
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/age_ui_qt_render_vshader.glsl");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/age_ui_qt_render_fshader.glsl");
    m_program.link();

    // vertex buffer
    VertexData vertices[] = {
        {QVector3D(0, 0, 0), QVector2D(0, 0)},
        {QVector3D(0, 1, 0), QVector2D(0, 1)},
        {QVector3D(1, 0, 0), QVector2D(1, 0)},
        {QVector3D(1, 1, 0), QVector2D(1, 1)},
    };

    m_vertices.create();
    m_vertices.bind();
    m_vertices.allocate(vertices, 4 * sizeof(VertexData));

    // index buffer
    GLushort indices[] = {0, 1, 2, 3};

    m_indices.create();
    m_indices.bind();
    m_indices.allocate(indices, 4 * sizeof(GLushort));

    // create textures
    allocate_textures();
}



void age::qt_renderer::resizeGL(int width, int height)
{
    LOG("viewport size: " << width << " x " << height);
    m_current_viewport = QSize(width, height);

    update_projection_matrix();
}



void age::qt_renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    m_program.bind();
    m_program.setUniformValue("u_projection", m_projection);

    m_vertices.bind();
    m_indices.bind();

    int vertexLocation = m_program.attributeLocation("a_vertex");
    m_program.enableAttributeArray(vertexLocation);
    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    int texcoordLocation  = m_program.attributeLocation("a_texcoord");
    m_program.enableAttributeArray(texcoordLocation);
    m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, sizeof(QVector3D), 2, sizeof(VertexData));

    for (size_t i = 0; i < m_num_frames_to_blend; ++i)
    {
        m_program.setUniformValue("u_color", QVector4D(1, 1, 1, 1.f / (i + 1)));
        m_frame_texture[i]->bind();
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);
    }
}



void age::qt_renderer::update_projection_matrix()
{
    double viewport_ratio = 1. * m_current_viewport.width() / m_current_viewport.height();
    double screen_ratio = 1. * m_emulator_screen.width() / m_emulator_screen.height();

    QRectF proj;
    if (viewport_ratio > screen_ratio)
    {
        double diff = viewport_ratio - screen_ratio;
        AGE_ASSERT(diff > 0);
        proj = QRectF(-.5 * diff, 0, 1 + diff, 1); // x, y, width, height
    }
    else
    {
        double diff = (1 / viewport_ratio) - (1 / screen_ratio);
        AGE_ASSERT(diff >= 0);
        proj = QRectF(0, -.5 * diff, 1, 1 + diff); // x, y, width, height
    }

    m_projection.setToIdentity();
    m_projection.ortho(proj);
}





//---------------------------------------------------------
//
//   new frame event handling
//
//---------------------------------------------------------

void age::qt_renderer::new_frame_slot(std::shared_ptr<const pixel_vector> new_frame)
{
    if (new_frame == nullptr)
    {
        return;
    }

    // if multiple frame are queued,
    // we handle only the last one and discard all others
    // (this should not happen if the machine can handle the load)
    if (m_new_frame == nullptr)
    {
        // call process_new_frame() after all events scheduled so far (with the same priority) have been processed
        //  -> this requires an event loop
        QMetaObject::invokeMethod(this, "process_new_frame", Qt::QueuedConnection);
    }
    else
    {
        ++m_frames_discarded;
        LOG(m_frames_discarded << " frame(s) discarded (total)");
    }
    m_new_frame = new_frame;
}

void age::qt_renderer::process_new_frame()
{
    makeCurrent();
    {
        // the oldest frame-texture will store the new frame
        std::unique_ptr<QOpenGLTexture> tmp = std::move(m_frame_texture[m_frame_texture.size() - 1]);
        tmp->bind();
        tmp->setData(tx_pixel_format, tx_pixel_type, m_new_frame->data());

        // move all other frames so that the new frame can be placed at the front
        for (size_t i = m_frame_texture.size() - 1; i > 0; --i)
        {
            m_frame_texture[i] = std::move(m_frame_texture[i - 1]);
        }
        m_frame_texture[0] = std::move(tmp);
    }
    doneCurrent();

    m_new_frame = nullptr; // allow next frame to be processed
    update(); // trigger paintGL
}



bool age::qt_renderer::textures_initialized() const
{
    return m_frame_texture[0] != nullptr;
}

void age::qt_renderer::allocate_textures()
{
    LOG("");

    std::for_each(begin(m_frame_texture), end(m_frame_texture), [&](auto &texture) {
        texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->setFormat(tx_format);
        texture->setSize(m_emulator_screen.width(), m_emulator_screen.height());
        texture->allocateStorage(tx_pixel_format, tx_pixel_type);
    });

    set_texture_filter();
}

void age::qt_renderer::set_texture_filter()
{
    LOG(m_bilinear_filter);

    auto min_filter = QOpenGLTexture::Linear; // always use linear filter for rendering downscaled texture
    auto mag_filter = m_bilinear_filter ? QOpenGLTexture::Linear : QOpenGLTexture::Nearest;

    std::for_each(begin(m_frame_texture), end(m_frame_texture), [&](auto &texture) {
        texture->setMinMagFilters(min_filter, mag_filter);
    });
}
