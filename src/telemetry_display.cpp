#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "telemetry_display.h"

TelemetryDisplay::TelemetryDisplay(TextRenderer& textRenderer, Shader& shader, float scale)
    : m_textRenderer { textRenderer }, m_shader { shader }, m_scale { scale }
{
}

void TelemetryDisplay::addEntry(std::string_view label, TelemetryPosition pos) {
    m_entries.emplace_back(TelemetryEntry{std::string(label), "", pos});
}

void TelemetryDisplay::updateEntry(std::string_view label, std::string_view value) {
    for (auto& e : m_entries) {
        if (e.label == label) {
            e.value = value;
            return;
        }
    }
}

void TelemetryDisplay::collateEntries()
{
    addEntry("RW Torque (Nm):", TelemetryPosition::BottomLeft);
    addEntry("RW AngVel (rad/s):", TelemetryPosition::BottomLeft);
    addEntry("Altitude (m):", TelemetryPosition::TopLeft);
    addEntry("Time Elapsed:", TelemetryPosition::TopRight);
}

float TelemetryDisplay::calcTextWidth(std::string_view text, float scale) const {
    float width {};
    for (char c : text) {
        auto ch = m_textRenderer.Characters.at(c);
        width += (ch.Advance >> 6) * scale;
    }

    return width;
}

void TelemetryDisplay::render(int windowWidth, int windowHeight) {
    renderSection(TelemetryPosition::BottomLeft, windowWidth, windowHeight);
    renderSection(TelemetryPosition::TopLeft, windowWidth, windowHeight);
    renderSection(TelemetryPosition::TopRight, windowWidth, windowHeight);
}

void TelemetryDisplay::renderSection(TelemetryPosition pos, int windowWidth, int windowHeight) {
    float x { 10.0f };
    float y { 10.0f };
    float lineSpacing { 20.0f };
 
    if (pos == TelemetryPosition::BottomLeft) {
        y = 30.0f;
    } else if (pos == TelemetryPosition::TopLeft) {
        y = windowHeight - 30.0f;
    } else if (pos == TelemetryPosition::TopRight) {
        y = windowHeight - 30.0f;
    }

    int line {};
    for (const auto& e : m_entries) {
        if (e.position != pos) continue;

        std::string text = e.label + " " + e.value;
        float drawX = x;
        if (pos == TelemetryPosition::TopRight) {
            drawX = windowWidth - calcTextWidth(text, m_scale) - 10.0f;
        }

        m_textRenderer.RenderText(m_shader, text, drawX, y - line * lineSpacing, m_scale, glm::vec3(1.0f));
        line++;
    }
}

void TelemetryDisplay::updateAndRender(const glm::vec3& torque, float angVelX, float angVelY, float angVelZ,
                                       float altitude, float elapsedTime, float winWidth, float winHeight) 
{ 
    auto addSpace = [](float v) {

        if (std::abs(v) < 1e-7f) { v = 0.0f; } // use epislon value to treat small negative nums as +0

        std::ostringstream ss;
        ss << std::scientific << std::setprecision(2);
        if (v >= 0.0f) ss << " "; 
        ss << v;
        return ss.str();
    };

    std::ostringstream torqueStream, angVelStream, altitudeStream, timeStream;

    torqueStream << "   X=" << addSpace(torque.x)
                 << "  Y=" << addSpace(torque.y)
                 << "  Z=" << addSpace(torque.z);

    angVelStream << "X=" << addSpace(angVelX)
                 << "  Y=" << addSpace(angVelY)
                 << "  Z=" << addSpace(angVelZ);

    altitudeStream << std::fixed << std::setprecision(0) << altitude;
    timeStream << std::fixed << std::setprecision(2) << elapsedTime << "s";

    updateEntry("RW Torque (Nm):", torqueStream.str());
    updateEntry("RW AngVel (rad/s):", angVelStream.str());
    updateEntry("Altitude (m):", altitudeStream.str());
    updateEntry("Time Elapsed:", timeStream.str());

    render(winWidth, winHeight);
}
