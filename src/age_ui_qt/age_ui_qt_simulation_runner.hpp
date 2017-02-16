//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_UI_QT_SIMULATION_RUNNER_HPP
#define AGE_UI_QT_SIMULATION_RUNNER_HPP

//!
//! \file
//!

#include "age_ui_qt_gl_renderer.hpp"
#include "age_ui_qt_audio.hpp"
#include "age_ui_qt_simulator.hpp"



namespace age
{

//!
//! \brief The qt_simulation_runner runs a simulation and handles audio output.
//!
//! Can be passed to another thread for async simulation (slots triggered automatically
//! as events).
//!
class qt_simulation_runner : public QObject, non_copyable
{
    Q_OBJECT
public:

    qt_simulation_runner(qt_gl_renderer &renderer, int simulation_interval_milliseconds);
    virtual ~qt_simulation_runner();

    uint get_speed_percent() const;
    uint64 get_simulated_milliseconds() const;

signals:

    void audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size);

public slots:

    // interesting read about threaded signals & slots:
    // https://woboq.com/blog/how-qt-signals-slots-work-part3-queuedconnection.html

    void initialize();

    void set_simulator(std::shared_ptr<age::qt_simulator> new_simulator);
    void set_simulator_buttons_down(uint buttons);
    void set_simulator_buttons_up(uint buttons);

    void set_simulation_synchronize(bool synchronize);
    void set_simulation_paused(bool paused);

    void set_audio_output(QAudioDeviceInfo device, QAudioFormat format);
    void set_audio_volume(int volume_percent);
    void set_audio_latency(int latency_milliseconds);
    void set_audio_downsampler_quality(age::qt_downsampler_quality quality);



private slots:

    void timer_event();

private:

    void simulate(std::shared_ptr<simulator> sim);
    void set_simulation_timer_interval();
    void emit_audio_output_activated();

    // usable by multiple threads

    atomic_uint m_speed_percent = {0};
    atomic_uint64 m_simulation_timer_ticks = {0};

    // used only by event handling thread

    const int m_simulation_interval_milliseconds;
    const uint64 m_simulation_interval_ticks;
    QElapsedTimer m_timer;
    QTimer *m_simulation_event_trigger = nullptr;

    qt_gl_renderer &m_renderer;
    qt_audio_output m_audio_output;
    int m_audio_latency_milliseconds = qt_audio_latency_milliseconds_min;
    bool m_audio_latency_changed = false;

    bool m_synchronize = true;
    bool m_paused = false;
    uint64 m_last_timer_ticks = 0;
    speed_calculator<50> m_speed_calculator;

    std::shared_ptr<qt_simulator> m_simulator;
    uint m_buttons_down = 0;
    uint m_buttons_up = 0;
};

} // namespace age



#endif // AGE_UI_QT_SIMULATION_RUNNER_HPP