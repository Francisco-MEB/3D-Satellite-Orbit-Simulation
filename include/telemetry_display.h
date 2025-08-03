#ifndef TELEMETRY_DISPLAY_H
#define TELEMETRY_DISPLAY_H

#include <string_view>
#include <vector>

#include "text_renderer.h"
#include "shader_s.h"

enum class TelemetryPosition 
{ 
    BottomLeft, 
    TopLeft, 
    TopRight,
};

struct TelemetryEntry {
    std::string label;
    std::string value;
    TelemetryPosition position;
};

class TelemetryDisplay {
public:
    TelemetryDisplay(TextRenderer& textRenderer, Shader& shader, float scale = 0.8f);

    void addEntry(std::string_view label, TelemetryPosition pos);
    void collateEntries();
    void updateEntry(std::string_view label, std::string_view value);

    void render(int windowWidth, int windowHeight);
 
    void updateAndRender(const glm::vec3& torque, float angVelX, float angVelY, float angVelZ,
                         float altitude, float elapsedTime, float winWidth, float winHeight);

private:
    TextRenderer& m_textRenderer;
    Shader& m_shader;
    float m_scale;
    std::vector<TelemetryEntry> m_entries;

    float calcTextWidth(std::string_view text, float scale) const;
    void renderSection(TelemetryPosition pos, int windowWidth, int windowHeight);
};

#endif

