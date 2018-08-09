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



constexpr const char *vshader =
        R"(
        // Set default precision to medium
        #ifdef GL_ES
        precision mediump int;
        precision mediump float;
        #endif

        uniform mat4 u_projection;
        uniform vec4 u_color;

        attribute vec4 a_vertex;
        attribute vec2 a_texcoord;

        varying vec2 v_texcoord;
        varying vec4 v_color;

        void main()
        {
            gl_Position = u_projection * a_vertex;
            v_texcoord = a_texcoord;
            v_color = u_color;
        }
        )";

constexpr const char *fshader =
        R"(
        // Set default precision to medium
        #ifdef GL_ES
        precision mediump int;
        precision mediump float;
        #endif

        // uniform sampler2D texture;

        varying vec2 v_texcoord;
        varying vec4 v_color;

        void main()
        {
            // gl_FragColor = texture2D(texture, v_texcoord);
            gl_FragColor = vec4(1, 1, 1, 1); // v_color;
        }
        )";



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_renderer::qt_renderer(QWidget *parent)
    : QOpenGLWidget(parent),
      m_indices(QOpenGLBuffer::IndexBuffer)
{
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
#ifdef AGE_DEBUG
    fmt.setOption(QSurfaceFormat::DebugContext);
#endif

    setFormat(fmt);
    LOG("format options: " << format().options());
}

age::qt_renderer::~qt_renderer()
{
    makeCurrent();

    m_vertices.destroy();
    m_indices.destroy();

#ifdef AGE_DEBUG
    if (m_logger)
    {
        m_logger->stopLogging();
    }
#endif

    doneCurrent();
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint age::qt_renderer::get_fps() const
{
    //! \todo implement age::qt_renderer::get_fps
    return 0;
}



//---------------------------------------------------------
//
//   public slots
//
//---------------------------------------------------------

void age::qt_renderer::set_emulator_screen_size(uint width, uint height)
{
    LOG(width << ", " << height);
    m_emulator_screen = QSize(width, height);

    update_projection(); // calculate new projection matrix
    update(); // trigger paintGL()
}

void age::qt_renderer::new_video_frame(pixel_vector new_video_frame)
{
    LOG("#pixel: " << new_video_frame.size());
    //! \todo implement age::qt_renderer::new_video_frame
}



void age::qt_renderer::set_blend_video_frames(uint num_video_frames_to_blend)
{
    LOG(num_video_frames_to_blend);
    //! \todo implement age::qt_renderer::set_blend_video_frames
}

void age::qt_renderer::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
    //! \todo implement age::qt_renderer::set_filter_chain
}

void age::qt_renderer::set_bilinear_filter(bool set_bilinear_filter)
{
    LOG(set_bilinear_filter);
    //! \todo implement age::qt_renderer::set_bilinear_filter
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

void age::qt_renderer::initializeGL()
{
    initializeOpenGLFunctions();

    // init logger
#ifdef AGE_DEBUG
    init_logger();
#endif

    // clear color
    glClearColor(0, 0, 0, 1);

    // shader program (failures are logged by Qt)
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vshader);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fshader);
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

    glDisable(GL_TEXTURE_2D);
}



void age::qt_renderer::resizeGL(int width, int height)
{
    LOG("viewport size: " << width << " x " << height);
    m_current_viewport = QSize(width, height);

    update_projection();
}



void age::qt_renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    m_program.bind();
    m_program.setUniformValue("u_projection", m_projection);
    m_program.setUniformValue("u_color", QVector4D(.5, .5, .5, 1));

    m_vertices.bind();
    m_indices.bind();

    int vertexLocation = m_program.attributeLocation("a_vertex");
    m_program.enableAttributeArray(vertexLocation);
    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    int texcoordLocation  = m_program.attributeLocation("a_texcoord");
    m_program.enableAttributeArray(texcoordLocation);
    m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, 3, 2, sizeof(VertexData));

    glDrawElements(GL_TRIANGLE_STRIP, 2, GL_UNSIGNED_SHORT, nullptr);
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_renderer::update_projection()
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
        AGE_ASSERT(diff > 0);
        proj = QRectF(0, -.5 * diff, 1, 1 + diff); // x, y, width, height
    }

    m_projection.setToIdentity();
    m_projection.ortho(proj);
}



#ifdef AGE_DEBUG

void age::qt_renderer::init_logger()
{
    QOpenGLContext *ctx = context();
    if (!ctx->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
    {
        LOG("GL_KHR_debug extension not available");
        return;
    }

    auto options = ctx->format().options();
    if ((options & QSurfaceFormat::DebugContext) == 0)
    {
        LOG("this is no DebugContext (format options: " << options << ")");
        return;
    }

    m_logger = std::make_unique<QOpenGLDebugLogger>(this);
    if (!m_logger->initialize())
    {
        LOG("QOpenGLDebugLogger not available");
        return;
    }
    connect(&*m_logger, &QOpenGLDebugLogger::messageLogged, this, &qt_renderer::log_message);
    m_logger->startLogging();
}

void age::qt_renderer::log_message(const QOpenGLDebugMessage &debugMessage)
{
    qDebug() << debugMessage;
}

#endif
