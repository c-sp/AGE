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
#ifndef AGE_UI_QT_RENDERER_HPP
#define AGE_UI_QT_RENDERER_HPP

//!
//! \file
//!

#include <memory> // std::shared_ptr

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QSize>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>

#include "age_ui_qt.hpp"



namespace age
{

struct qt_vertex_data
{
    QVector3D position;
    QVector2D texCoord;
};

QString qt_load_shader(const QString &file_name);



class qt_video_renderer : private QOpenGLFunctions
{
public:

    qt_video_renderer();
    ~qt_video_renderer();

    void set_matrix(const QSize &emulator_screen, const QSize &viewport);
    void render(const std::vector<std::shared_ptr<QOpenGLTexture>> &textures_to_render);

private:

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_indices;
    QMatrix4x4 m_projection;
};



class qt_video_output : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:

    qt_video_output(QWidget *parent = nullptr);
    ~qt_video_output() override;

    uint get_fps() const;

public slots:

    void set_emulator_screen_size(uint width, uint height);
    void new_frame(std::shared_ptr<const age::pixel_vector> new_frame);

    void set_blend_frames(uint num_frames_to_blend);
    void set_filter_chain(qt_filter_vector filter_chain);
    void set_bilinear_filter(bool bilinear_filter);



protected:

    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:

    void update_if_initialized(std::function<void()> update_func);

    std::unique_ptr<qt_video_renderer> m_renderer = nullptr;

    QSize m_emulator_screen = {1, 1};
    uint m_num_frames_to_blend = 1;



    // frame event handling

private:

    void new_frame_slot(std::shared_ptr<const pixel_vector> new_frame);
    Q_INVOKABLE void process_new_frame();

    void allocate_textures();
    void set_texture_filter();

    std::shared_ptr<const pixel_vector> m_new_frame = nullptr;
    uint m_frames_discarded = 0;

    std::vector<std::shared_ptr<QOpenGLTexture>> m_frame_texture;
    bool m_bilinear_filter = false;
};

} // namespace age



#endif // AGE_UI_QT_RENDERER_HPP
