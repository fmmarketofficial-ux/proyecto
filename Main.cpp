// ============================================================================
// MAKCU RECOIL CONTROL PRO - ENHANCED EDITION 2025
// Nueva Interfaz con Animaciones Avanzadas y Loading Screen
// ============================================================================

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <map>
#include <queue>
#include <chrono>
#include <cmath>
#include <algorithm>

#include "ImGui/ImGui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

#include "include/makcu.h"
#include "include/serialport.h"

// ============================================================================
// LOADING SCREEN SYSTEM
// ============================================================================

struct LoadingScreen {
    bool isActive = true;
    float progress = 0.0f;
    float animTime = 0.0f;
    std::string currentTask = "Iniciando...";
    std::vector<std::string> loadingPhrases = {
        "Inicializando sistema MAKCU...",
        "Cargando patrones de retroceso...",
        "Conectando dispositivos...",
        "Optimizando rendimiento...",
        "Calibrando sensores...",
        "Preparando interfaz...",
        "Listo para dominar!"
    };

    void update(float deltaTime) {
        animTime += deltaTime;

        // Simular progreso de carga
        if (progress < 100.0f) {
            progress += deltaTime * 35.0f; // Carga en ~3 segundos

            // Actualizar frase según progreso
            int phaseIndex = static_cast<int>((progress / 100.0f) * loadingPhrases.size());
            if (phaseIndex < loadingPhrases.size()) {
                currentTask = loadingPhrases[phaseIndex];
            }

            if (progress >= 100.0f) {
                progress = 100.0f;
                isActive = false;
            }
        }
    }

    void render() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin("Loading", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground);

        auto drawList = ImGui::GetWindowDrawList();

        // Fondo con gradiente animado
        float wave = sin(animTime * 2.0f) * 0.1f;
        drawList->AddRectFilledMultiColor(
            ImVec2(0, 0),
            io.DisplaySize,
            IM_COL32(10 + wave * 20, 10 + wave * 20, 25 + wave * 30, 255),
            IM_COL32(20 + wave * 20, 15 + wave * 20, 40 + wave * 30, 255),
            IM_COL32(30 + wave * 20, 20 + wave * 20, 50 + wave * 30, 255),
            IM_COL32(15 + wave * 20, 15 + wave * 20, 35 + wave * 30, 255)
        );

        // Logo / Título animado
        float titlePulse = (sin(animTime * 3.0f) + 1.0f) * 0.5f;
        float titleGlow = 30.0f + titlePulse * 20.0f;

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

        // Título con efecto de brillo
        const char* title = "MAKCU RECOIL CONTROL PRO";
        float titleWidth = ImGui::CalcTextSize(title).x * 2.0f;
        ImVec2 titlePos(center.x - titleWidth * 0.5f, center.y - 150.0f);

        // Sombra del título
        for (int i = 0; i < 3; i++) {
            drawList->AddText(
                ImGui::GetFont(), 48.0f,
                ImVec2(titlePos.x + i * 2, titlePos.y + i * 2),
                IM_COL32(0, 0, 0, 80 - i * 20),
                title
            );
        }

        // Título principal con brillo
        drawList->AddText(
            ImGui::GetFont(), 48.0f,
            titlePos,
            IM_COL32(100 + titleGlow, 200 + titleGlow, 255, 255),
            title
        );

        // Círculo de carga giratorio
        float radius = 50.0f;
        int segments = 64;
        float rotation = animTime * 2.0f;

        for (int i = 0; i < segments; i++) {
            float angle1 = rotation + (i / (float)segments) * 2.0f * 3.14159f;
            float angle2 = rotation + ((i + 1) / (float)segments) * 2.0f * 3.14159f;

            float alpha = (sin(animTime * 3.0f + i * 0.1f) + 1.0f) * 0.5f;
            ImU32 color = IM_COL32(100, 200, 255, 100 + alpha * 155);

            ImVec2 p1(center.x + cos(angle1) * radius, center.y + sin(angle1) * radius);
            ImVec2 p2(center.x + cos(angle2) * radius, center.y + sin(angle2) * radius);

            drawList->AddLine(p1, p2, color, 4.0f);
        }

        // Partículas orbitando
        for (int i = 0; i < 8; i++) {
            float angle = rotation * 1.5f + (i / 8.0f) * 2.0f * 3.14159f;
            float orbitRadius = radius + 30.0f + sin(animTime * 2.0f + i) * 10.0f;
            ImVec2 particlePos(
                center.x + cos(angle) * orbitRadius,
                center.y + sin(angle) * orbitRadius
            );

            float particleAlpha = (sin(animTime * 4.0f + i * 0.5f) + 1.0f) * 0.5f;
            drawList->AddCircleFilled(
                particlePos,
                4.0f,
                IM_COL32(150, 220, 255, 100 + particleAlpha * 155)
            );
        }

        // Barra de progreso con efecto
        float barWidth = 400.0f;
        float barHeight = 8.0f;
        ImVec2 barPos(center.x - barWidth * 0.5f, center.y + 80.0f);

        // Fondo de la barra
        drawList->AddRectFilled(
            barPos,
            ImVec2(barPos.x + barWidth, barPos.y + barHeight),
            IM_COL32(30, 30, 40, 200),
            4.0f
        );

        // Progreso con gradiente
        float progressWidth = (progress / 100.0f) * barWidth;
        if (progressWidth > 0) {
            drawList->AddRectFilledMultiColor(
                barPos,
                ImVec2(barPos.x + progressWidth, barPos.y + barHeight),
                IM_COL32(50, 150, 255, 255),
                IM_COL32(100, 200, 255, 255),
                IM_COL32(150, 220, 255, 255),
                IM_COL32(100, 180, 255, 255)
            );

            // Brillo en el progreso
            float glowSize = 20.0f;
            drawList->AddRectFilled(
                ImVec2(barPos.x + progressWidth - glowSize, barPos.y),
                ImVec2(barPos.x + progressWidth, barPos.y + barHeight),
                IM_COL32(200, 230, 255, 150)
            );
        }

        // Texto de progreso
        char progressText[32];
        snprintf(progressText, sizeof(progressText), "%.0f%%", progress);
        ImVec2 progressTextSize = ImGui::CalcTextSize(progressText);
        drawList->AddText(
            ImVec2(center.x - progressTextSize.x * 0.5f, barPos.y + 20.0f),
            IM_COL32(200, 220, 255, 255),
            progressText
        );

        // Tarea actual con animación de escritura
        float textAlpha = (sin(animTime * 5.0f) + 1.0f) * 0.5f;
        ImVec2 taskSize = ImGui::CalcTextSize(currentTask.c_str());
        drawList->AddText(
            ImVec2(center.x - taskSize.x * 0.5f, barPos.y + 50.0f),
            IM_COL32(150, 180, 255, 150 + textAlpha * 105),
            currentTask.c_str()
        );

        // Puntos animados después del texto
        int dotCount = static_cast<int>(animTime * 3.0f) % 4;
        std::string dots(dotCount, '.');
        ImVec2 dotsPos(center.x - taskSize.x * 0.5f + taskSize.x, barPos.y + 50.0f);
        drawList->AddText(dotsPos, IM_COL32(150, 180, 255, 200), dots.c_str());

        ImGui::End();
    }
};

// ============================================================================
// ENHANCED ANIMATION SYSTEM
// ============================================================================

struct EnhancedAnimationState {
    float time = 0.0f;
    float pulse = 0.0f;
    float wave = 0.0f;
    float glow = 0.0f;
    float rotation = 0.0f;
    float breathe = 0.0f;
    float bounce = 0.0f;
    float sparkle = 0.0f;

    void update(float deltaTime) {
        time += deltaTime;
        pulse = (sin(time * 3.0f) + 1.0f) * 0.5f;
        wave = sin(time * 2.0f);
        glow = (sin(time * 4.0f) + 1.0f) * 0.5f;
        rotation += deltaTime * 0.5f;
        breathe = (sin(time * 1.5f) + 1.0f) * 0.5f;
        bounce = abs(sin(time * 2.5f));
        sparkle = (sin(time * 8.0f) + 1.0f) * 0.5f;
    }

    float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    float easeOutElastic(float t) {
        if (t == 0 || t == 1) return t;
        return static_cast<float>(pow(2, -10 * t) * sin((t * 10 - 0.75f) * (2 * 3.14159f / 3)) + 1);
    }

    float easeInCubic(float t) {
        return t * t * t;
    }

    float easeOutBounce(float t) {
        if (t < 1.0f / 2.75f) {
            return 7.5625f * t * t;
        }
        else if (t < 2.0f / 2.75f) {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        }
        else if (t < 2.5f / 2.75f) {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        }
        else {
            t -= 2.625f / 2.75f;
            return 7.5625f * t * t + 0.984375f;
        }
    }
};

// ============================================================================
// ADVANCED PARTICLE SYSTEM
// ============================================================================

struct EnhancedParticle {
    ImVec2 pos;
    ImVec2 vel;
    ImVec4 color;
    float lifetime = 0.0f;
    float maxLifetime = 0.0f;
    float size = 0.0f;
    float rotation = 0.0f;
    float rotationSpeed = 0.0f;
    int type = 0; // 0=circle, 1=star, 2=spark

    void update(float deltaTime) {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;
        lifetime -= deltaTime;
        vel.y += 50.0f * deltaTime; // Gravity
        rotation += rotationSpeed * deltaTime;
        color.w = (lifetime / maxLifetime) * 0.8f;
        size = size * (lifetime / maxLifetime);
    }

    bool isAlive() const { return lifetime > 0; }
};

class EnhancedParticleSystem {
public:
    std::vector<EnhancedParticle> particles;

    void emit(ImVec2 pos, ImVec4 color, int count = 5, int type = 0) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0, 2 * 3.14159f);
        std::uniform_real_distribution<float> speedDist(50, 200);
        std::uniform_real_distribution<float> lifeDist(0.5f, 2.0f);
        std::uniform_real_distribution<float> sizeDist(2.0f, 8.0f);
        std::uniform_real_distribution<float> rotDist(-5.0f, 5.0f);

        for (int i = 0; i < count; ++i) {
            EnhancedParticle p;
            float angle = angleDist(gen);
            float speed = speedDist(gen);

            p.pos = pos;
            p.vel = ImVec2(cos(angle) * speed, sin(angle) * speed - 100.0f);
            p.color = color;
            p.lifetime = lifeDist(gen);
            p.maxLifetime = p.lifetime;
            p.size = sizeDist(gen);
            p.rotation = angleDist(gen);
            p.rotationSpeed = rotDist(gen);
            p.type = type;

            particles.push_back(p);
        }
    }

    void emitTrail(ImVec2 start, ImVec2 end, ImVec4 color, int count = 10) {
        for (int i = 0; i < count; i++) {
            float t = i / (float)count;
            ImVec2 pos(
                start.x + (end.x - start.x) * t,
                start.y + (end.y - start.y) * t
            );
            emit(pos, color, 2, 2);
        }
    }

    void update(float deltaTime) {
        for (auto& p : particles) {
            p.update(deltaTime);
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const EnhancedParticle& p) { return !p.isAlive(); }),
            particles.end()
        );
    }

    void render(ImDrawList* drawList) {
        for (const auto& p : particles) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(p.color);

            if (p.type == 0) {
                // Círculo normal
                drawList->AddCircleFilled(p.pos, p.size, col);
            }
            else if (p.type == 1) {
                // Estrella
                renderStar(drawList, p.pos, p.size, p.rotation, col);
            }
            else if (p.type == 2) {
                // Chispa (línea)
                ImVec2 end(p.pos.x - p.vel.x * 0.05f, p.pos.y - p.vel.y * 0.05f);
                drawList->AddLine(p.pos, end, col, p.size * 0.5f);
            }
        }
    }

private:
    void renderStar(ImDrawList* drawList, ImVec2 center, float size, float rotation, ImU32 color) {
        const int points = 5;
        std::vector<ImVec2> vertices;

        for (int i = 0; i < points * 2; i++) {
            float angle = rotation + (i * 3.14159f / points);
            float radius = (i % 2 == 0) ? size : size * 0.5f;
            vertices.push_back(ImVec2(
                center.x + cos(angle) * radius,
                center.y + sin(angle) * radius
            ));
        }

        drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), color);
    }
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct Vector1 {
    double x;
    double y;
};

struct WeaponData {
    std::string name;
    std::string displayName;
    std::vector<Vector1> pattern;
    std::vector<double> timings;
    double baseWaitTime = 0.0;
    ImVec4 color;
    std::string category; // "AR", "SMG", "PISTOL", "LMG"
};

// ============================================================================
// WEAPON DATABASE - SOLO ARMAS CON PATRONES COMPLEJOS
// ============================================================================

std::map<std::string, WeaponData> g_weaponDatabase = {
    // ASSAULT RIFLES
    {"AK47", {
        "AK47", "AK-47",
        {
            {0.000000,-2.257792},{0.323242,-2.300758},{0.649593,-2.299759},
            {0.848786,-2.259034},{1.075408,-2.323947},{1.268491,-2.215956},
            {1.330963,-2.236556},{1.336833,-2.218203},{1.505516,-2.143454},
            {1.504423,-2.233091},{1.442116,-2.270194},{1.478543,-2.204318},
            {1.392874,-2.165817},{1.480824,-2.177887},{1.597069,-2.270915},
            {1.449996,-2.145893},{1.369179,-2.270450},{1.582363,-2.298334},
            {1.516872,-2.235066},{1.498249,-2.238401},{1.465769,-2.331642},
            {1.564812,-2.242621},{1.517519,-2.303052},{1.422433,-2.211946},
            {1.553195,-2.248043},{1.510463,-2.285327},{1.553878,-2.240047},
            {1.520380,-2.221839},{1.553878,-2.240047},{1.553195,-2.248043}
        },
        {121.96,92.63,138.61,113.38,66.25,66.30,75.93,85.06,89.20,86.68,78.82,
         70.05,60.86,59.52,71.67,86.74,98.34,104.34,104.09,97.59,85.48,70.49,
         56.56,47.39,56.64,91.59,112.39,111.39,87.51},
        133.33,
        ImVec4(1.0f, 0.15f, 0.15f, 1.0f),
        "AR"
    }},
    {"LR300", {
        "LR300", "LR-300",
        {
            {0.000000,-2.052616},{0.055584,-1.897695},{-0.247226,-1.863222},
            {-0.243871,-1.940010},{0.095727,-1.966751},{0.107707,-1.885520},
            {0.324888,-1.946722},{-0.181137,-1.880342},{0.162399,-1.820107},
            {-0.292076,-1.994940},{0.064575,-1.837156},{-0.126699,-1.887880},
            {-0.090568,-1.832799},{0.065338,-1.807480},{-0.197343,-1.705888},
            {-0.216561,-1.785949},{0.042567,-1.806371},{-0.065534,-1.757623},
            {0.086380,-1.904010},{-0.097326,-1.969296},{-0.213034,-1.850288},
            {-0.017790,-1.730867},{-0.045577,-1.783686},{-0.053309,-1.886260},
            {0.055072,-1.793076},{-0.091874,-1.921906},{-0.033719,-1.796160},
            {0.266464,-1.993952},{0.079090,-1.921165}
        },
        {}, 120.0,
        ImVec4(0.15f, 1.0f, 0.35f, 1.0f),
        "AR"
    }},

    // SUBMACHINE GUNS
    {"MP5", {
        "MP5", "MP5A4",
        {
            {0.125361,-1.052446},{-0.099548,-0.931548},{0.027825,-0.954094},
            {-0.013715,-0.851504},{-0.007947,-1.070579},{0.096096,-1.018017},
            {-0.045937,-0.794216},{0.034316,-1.112618},{-0.003968,-0.930040},
            {-0.009403,-0.888503},{0.140813,-0.970807},{-0.015052,-1.046551},
            {0.095699,-0.860475},{-0.269643,-1.038896},{0.000285,-0.840478},
            {0.018413,-1.038126},{0.099191,-0.851701},{0.199659,-0.893041},
            {-0.082660,-1.069278},{0.006826,-0.881493},{0.091709,-1.150956},
            {-0.108677,-0.965513},{0.169612,-1.099499},{-0.038244,-1.120084},
            {-0.085513,-0.876956},{0.136279,-1.047589},{0.196392,-1.039977},
            {-0.152513,-1.209291},{-0.214510,-0.956648},{0.034276,-0.095177}
        },
        {}, 89.0,
        ImVec4(0.25f, 0.45f, 1.0f, 1.0f),
        "SMG"
    }},
    {"SMG", {
        "SMG", "Custom SMG",
        {
            {-0.114414,-0.680635},{0.008685,-0.676597},{0.010312,-0.682837},
            {0.064825,-0.691344},{0.104075,-0.655617},{-0.088118,-0.660429},
            {0.089906,-0.675183},{0.037071,-0.632623},{0.178466,-0.634737},
            {0.034653,-0.669444},{-0.082658,-0.664827},{0.025551,-0.636631},
            {0.082413,-0.647118},{-0.123305,-0.662104},{0.028164,-0.662354},
            {-0.117345,-0.693474},{-0.268777,-0.661122},{-0.053086,-0.677493},
            {0.004238,-0.647037},{0.014169,-0.551440},{-0.009907,-0.552079},
            {0.044076,-0.577694},{-0.043187,-0.549581}
        },
        {}, 90.0,
        ImVec4(1.0f, 0.85f, 0.15f, 1.0f),
        "SMG"
    }},
    {"THOMPSON", {
        "THOMPSON", "Thompson",
        {
            {-0.114413,-0.680635},{0.008686,-0.676598},{0.010312,-0.682837},
            {0.064825,-0.691345},{0.104075,-0.655618},{-0.088118,-0.660429},
            {0.089906,-0.675183},{0.037071,-0.632623},{0.178465,-0.634737},
            {0.034654,-0.669443},{-0.082658,-0.664826},{0.025550,-0.636631},
            {0.082414,-0.647118},{-0.123305,-0.662104},{0.028164,-0.662354},
            {-0.117346,-0.693475},{-0.268777,-0.661123},{-0.053086,-0.677493},
            {0.04238,-0.647038},{0.04238,-0.647038}
        },
        {}, 113.0,
        ImVec4(1.0f, 0.45f, 0.1f, 1.0f),
        "SMG"
    }},

    // LIGHT MACHINE GUNS
    {"M249", {
        "M249", "M249",
        {
            {0.0,-1.49},{0.39,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},
            {0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},
            {0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},
            {0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49},{0.72,-1.49}
        },
        {}, 100.0,
        ImVec4(1.0f, 0.35f, 0.0f, 1.0f),
        "LMG"
    }},

    // SPECIAL
    {"SKS", {
        "SKS", "SKS",
        {
            {0.0, -1.966075}, {0.0, -2.455723}, {0.320768, -2.075308},
            {0.565088, -2.073886}, {0.911924, -2.322124}, {1.081, -2.622581},
            {1.431705, -2.164627}, {1.500359, -2.165166}, {1.450049, -2.298362},
            {1.369828, -2.133563}, {1.476682, -2.002966}, {1.82793, -2.159637},
            {1.661027, -2.234897}, {1.214138, -2.018232}, {1.852968, -2.239446}
        },
        {}, 150.0,
        ImVec4(0.75f, 0.55f, 0.35f, 1.0f),
        "SPECIAL"
    }}
};

// ============================================================================
// PRACTICE MODE & SCORING
// ============================================================================

struct PracticeSession {
    bool active = false;
    std::vector<ImVec2> recordedPath;
    std::vector<ImVec2> idealPath;
    std::chrono::steady_clock::time_point startTime;
    float totalDeviation = 0.0f;
    float maxDeviation = 0.0f;
    int shotsFired = 0;
    float accuracyScore = 100.0f;

    void start() {
        active = true;
        recordedPath.clear();
        idealPath.clear();
        totalDeviation = 0.0f;
        maxDeviation = 0.0f;
        shotsFired = 0;
        accuracyScore = 100.0f;
        startTime = std::chrono::steady_clock::now();
    }

    void stop() {
        active = false;
        if (shotsFired > 0) {
            float avgDeviation = totalDeviation / shotsFired;
            accuracyScore = std::max(0.0f, 100.0f - (avgDeviation * 2.0f));
        }
    }

    void recordPoint(ImVec2 actual, ImVec2 ideal) {
        recordedPath.push_back(actual);
        idealPath.push_back(ideal);
        shotsFired++;
        float deviation = sqrtf(powf(actual.x - ideal.x, 2) + powf(actual.y - ideal.y, 2));
        totalDeviation += deviation;
        maxDeviation = std::max(maxDeviation, deviation);
    }

    std::string getGrade() const {
        if (accuracyScore >= 95.0f) return "S+";
        if (accuracyScore >= 90.0f) return "S";
        if (accuracyScore >= 85.0f) return "A+";
        if (accuracyScore >= 80.0f) return "A";
        if (accuracyScore >= 75.0f) return "B+";
        if (accuracyScore >= 70.0f) return "B";
        if (accuracyScore >= 65.0f) return "C";
        return "D";
    }

    ImVec4 getGradeColor() const {
        std::string grade = getGrade();
        if (grade == "S+" || grade == "S") return ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
        if (grade == "A+" || grade == "A") return ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        if (grade == "B+" || grade == "B") return ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
        if (grade == "C") return ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
        return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    }
};

// ============================================================================
// CONNECTION STATS
// ============================================================================

struct ConnectionStats {
    std::atomic<uint64_t> commandsSent{ 0 };
    std::atomic<uint64_t> commandsFailed{ 0 };
    std::chrono::steady_clock::time_point lastCommandTime;
    std::chrono::steady_clock::time_point connectionStartTime;
    std::queue<float> latencyHistory;
    const size_t maxHistorySize = 100;
    float averageLatency = 0.0f;
    bool isConnected = false;

    void recordCommand(bool success, float latency = 0.0f) {
        if (success) {
            commandsSent++;
            lastCommandTime = std::chrono::steady_clock::now();
            latencyHistory.push(latency);
            if (latencyHistory.size() > maxHistorySize) {
                latencyHistory.pop();
            }
            float sum = 0.0f;
            auto temp = latencyHistory;
            while (!temp.empty()) {
                sum += temp.front();
                temp.pop();
            }
            averageLatency = sum / latencyHistory.size();
        }
        else {
            commandsFailed++;
        }
    }

    float getSuccessRate() const {
        uint64_t total = commandsSent + commandsFailed;
        if (total == 0) return 100.0f;
        return (float)commandsSent * 100.0f / total;
    }

    float getUptime() const {
        if (!isConnected) return 0.0f;
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - connectionStartTime);
        return static_cast<float>(duration.count());
    }
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

struct AppState {
    makcu::Device makcuDevice;
    bool deviceConnected = false;
    ConnectionStats connectionStats;

    std::string currentWeaponKey = "AK47";
    int selectedScope = 0;
    int selectedBarrel = 0;
    float sensitivity = 1.0f;
    int fov = 90;
    bool smoothing = true;
    int smoothingFactor = 1;

    std::atomic<bool> leftButtonReleased{ false };
    std::atomic<bool> rightButtonReleased{ false };
    std::atomic<bool> isActive{ false };

    int currentTab = 0;
    bool showPatternVisualizer = true;
    float visualizerScale = 15.0f;  // Zoom ajustable con slider
    bool showStats = true;
    bool showLogs = true;

    EnhancedAnimationState animation;
    EnhancedParticleSystem particles;
    PracticeSession practiceSession;
    LoadingScreen loadingScreen;

    std::vector<std::string> consoleLines;
    std::mutex consoleMutex;
    const size_t maxConsoleLines = 100;

    std::string selectedCategory = "ALL";

    void addLog(const std::string& message) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "[%H:%M:%S] ", localtime(&time));
        consoleLines.push_back(std::string(timeStr) + message);
        if (consoleLines.size() > maxConsoleLines) {
            consoleLines.erase(consoleLines.begin());
        }
    }
};

AppState g_appState;

// ============================================================================
// SCOPE & BARREL DATA
// ============================================================================

struct ScopeData {
    const char* name;
    double multiplier;
    double extraMultiplier;
};

ScopeData g_scopes[] = {
    {"None", 1.0, 0.0},
    {"Handmade", 0.8, 0.1},
    {"Holographic", 1.2, 0.3},
    {"8x Scope", 4.76, 0.75}
};

struct BarrelData {
    const char* name;
    double multiplier;
};

BarrelData g_barrels[] = {
    {"None", 1.0},
    {"Silencer", 0.8}
};

// ============================================================================
// CALCULATION FUNCTIONS
// ============================================================================

double GetScopeMultiplier() {
    double mult = g_scopes[g_appState.selectedScope].multiplier;
    if (g_appState.currentWeaponKey == "SMG" || g_appState.currentWeaponKey == "THOMPSON") {
        mult += g_scopes[g_appState.selectedScope].extraMultiplier;
    }
    if (g_appState.selectedScope == 3 && g_appState.selectedBarrel == 1) {
        return 1.46;
    }
    return mult;
}

double GetBarrelMultiplier() {
    double mult = g_barrels[g_appState.selectedBarrel].multiplier;
    if (g_appState.currentWeaponKey == "SMG" && g_appState.selectedBarrel == 1) {
        mult = 0.85;
    }
    if (g_appState.selectedBarrel == 1 &&
        (g_appState.selectedScope == 1 || g_appState.selectedScope == 2)) {
        return 0.75;
    }
    return mult;
}

double CalculateMovement(double coordinate, bool isCrouching) {
    double scopeMult = GetScopeMultiplier();
    double barrelMult = GetBarrelMultiplier();
    double sensMultiplier = isCrouching ? (g_appState.sensitivity * 2.0) : g_appState.sensitivity;
    return ((coordinate * scopeMult * barrelMult) /
        (-0.03 * sensMultiplier * 3.0 * (g_appState.fov / 100.0)));
}

// ============================================================================
// RECOIL CONTROL
// ============================================================================

void ApplyRecoilMovement(int x, int y, int waitTime) {
    if (!g_appState.deviceConnected) return;
    auto startTime = std::chrono::high_resolution_clock::now();
    try {
        if (g_appState.smoothing && (abs(x) > 2 || abs(y) > 2)) {
            g_appState.makcuDevice.mouseMoveSmooth(x, y, g_appState.smoothingFactor);
        }
        else {
            g_appState.makcuDevice.mouseMove(x, y);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        float latency = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        g_appState.connectionStats.recordCommand(true, latency);
        Sleep(waitTime);
    }
    catch (const makcu::MakcuException& e) {
        g_appState.connectionStats.recordCommand(false);
        g_appState.addLog("[ERROR] MAKCU Error: " + std::string(e.what()));
    }
}

bool RecoilControlLoop() {
    auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
    bool isCrouching = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    for (size_t i = 0; i < weapon.pattern.size() && !g_appState.leftButtonReleased.load(); i++) {
        int x = static_cast<int>(CalculateMovement(weapon.pattern[i].x, isCrouching));
        int y = static_cast<int>(CalculateMovement(weapon.pattern[i].y, isCrouching));
        double timing = weapon.timings.empty() ? weapon.baseWaitTime : weapon.timings[i];
        int waitTime = static_cast<int>(timing);
        ApplyRecoilMovement(x, y, waitTime);
    }
    return true;
}

void OnMouseButton(makcu::MouseButton button, bool isPressed) {
    if (button == makcu::MouseButton::RIGHT) {
        g_appState.rightButtonReleased.store(!isPressed);
        if (!isPressed) g_appState.isActive.store(false);
        return;
    }
    if (button == makcu::MouseButton::LEFT) {
        if (!isPressed) {
            g_appState.leftButtonReleased.store(true);
            g_appState.isActive.store(false);
        }
        else if (!g_appState.rightButtonReleased.load()) {
            g_appState.leftButtonReleased.store(false);
            g_appState.isActive.store(true);
            std::thread(RecoilControlLoop).detach();
        }
    }
}

// ============================================================================
// CONNECTION MONITOR
// ============================================================================

std::atomic<bool> g_monitorActive{ true };

void ConnectionMonitorThread() {
    const int keepaliveInterval = 60;
    auto lastKeepalive = std::chrono::steady_clock::now();
    while (g_monitorActive) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (!g_appState.deviceConnected) continue;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastKeepalive).count();
        if (elapsed >= keepaliveInterval) {
            try {
                auto version = g_appState.makcuDevice.getVersion();
                if (!version.empty()) {
                    g_appState.addLog("[INFO] Keepalive OK");
                }
                lastKeepalive = now;
            }
            catch (...) {
                g_appState.addLog("[WARNING] Keepalive failed");
            }
        }
    }
}

// ============================================================================
// DEVICE INITIALIZATION
// ============================================================================

bool InitializeMakcu() {
    try {
        g_appState.addLog("[INFO] Buscando dispositivos MAKCU...");
        auto devices = makcu::Device::findDevices();
        if (devices.empty()) {
            g_appState.addLog("[ERROR] No se encontraron dispositivos MAKCU!");
            return false;
        }
        g_appState.addLog("[INFO] Encontrado " + std::to_string(devices.size()) + " dispositivo(s)");
        if (g_appState.makcuDevice.connect(devices[0].port)) {
            g_appState.makcuDevice.enableHighPerformanceMode(true);
            g_appState.makcuDevice.setMouseButtonCallback(OnMouseButton);
            g_appState.deviceConnected = true;
            g_appState.connectionStats.isConnected = true;
            g_appState.connectionStats.connectionStartTime = std::chrono::steady_clock::now();
            g_appState.addLog("[SUCCESS] MAKCU conectado en " + devices[0].port);
            std::thread(ConnectionMonitorThread).detach();
            return true;
        }
    }
    catch (const makcu::MakcuException& e) {
        g_appState.addLog("[ERROR] Error MAKCU: " + std::string(e.what()));
    }
    return false;
}

// ============================================================================
// MODERN UI STYLING
// ============================================================================

void ApplyModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Tema oscuro moderno con acentos neón
    colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.08f, 0.98f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.08f, 0.98f);

    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.45f, 0.75f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.10f, 0.20f, 0.40f, 0.30f);

    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.45f, 0.75f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.55f, 0.90f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.15f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.45f, 0.80f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.12f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.35f, 0.65f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.38f, 0.68f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.65f, 1.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.30f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.60f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.75f, 1.00f, 1.00f);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.42f, 0.50f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.22f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.28f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.48f, 1.00f);

    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 8.0f;

    style.WindowPadding = ImVec2(16.0f, 16.0f);
    style.FramePadding = ImVec2(12.0f, 6.0f);
    style.ItemSpacing = ImVec2(12.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
}

// ============================================================================
// ANIMATED CONNECTION STATUS
// ============================================================================

void RenderConnectionStatus() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("ConnectionStatus", ImVec2(0.0f, 200.0f), true);

    auto drawList = ImGui::GetWindowDrawList();

    // Título con efecto de neón
    float titleGlow = g_appState.animation.glow;
    ImVec4 titleColor = ImVec4(0.2f + titleGlow * 0.3f, 0.7f + titleGlow * 0.3f, 1.0f, 1.0f);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(titleColor, "ESTADO DE CONEXION");
    ImGui::PopFont();
    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 200);

    // Indicador animado de estado
    ImVec2 pos = ImGui::GetCursorScreenPos();

    if (g_appState.deviceConnected) {
        // Anillo pulsante verde
        float pulse = g_appState.animation.pulse;
        float breathe = g_appState.animation.breathe;

        // Anillo exterior pulsante
        for (int i = 0; i < 3; i++) {
            float radius = 10.0f + (2.0f - i) * 3.0f + pulse * 5.0f;
            drawList->AddCircle(
                ImVec2(pos.x + 15, pos.y + 15),
                radius,
                IM_COL32(0, 255 - i * 60, 100, 80 - i * 25),
                32, 2.0f
            );
        }

        // Núcleo brillante
        drawList->AddCircleFilled(
            ImVec2(pos.x + 15, pos.y + 15),
            7.0f + breathe * 2.0f,
            IM_COL32(100, 255, 150, 255)
        );
        drawList->AddCircleFilled(
            ImVec2(pos.x + 15, pos.y + 15),
            5.0f,
            IM_COL32(200, 255, 200, 255)
        );

        ImGui::Dummy(ImVec2(35, 30));
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));
        ImGui::Text("CONECTADO");
        ImGui::PopStyleColor();

        // Partículas de conexión exitosa
        if (g_appState.animation.sparkle > 0.95f) {
            g_appState.particles.emit(
                ImVec2(pos.x + 15, pos.y + 15),
                ImVec4(0.3f, 1.0f, 0.5f, 1.0f),
                2, 1
            );
        }
    }
    else {
        // Anillo pulsante rojo
        float pulse = g_appState.animation.pulse;
        drawList->AddCircle(
            ImVec2(pos.x + 15, pos.y + 15),
            10.0f + pulse * 4.0f,
            IM_COL32(255, 0, 0, 100 + pulse * 100),
            32, 2.5f
        );
        drawList->AddCircleFilled(
            ImVec2(pos.x + 15, pos.y + 15),
            8.0f,
            IM_COL32(255, 50, 50, 200)
        );

        ImGui::Dummy(ImVec2(35, 30));
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("DESCONECTADO");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Text("Puerto:");
    ImGui::Text("Baudios:");
    ImGui::Text("Uptime:");

    ImGui::NextColumn();

    auto info = g_appState.makcuDevice.getDeviceInfo();
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
        "%s", g_appState.deviceConnected ? info.port.c_str() : "N/A");
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
        "%s", g_appState.deviceConnected ? "4000000" : "N/A");

    if (g_appState.deviceConnected) {
        float uptime = g_appState.connectionStats.getUptime();
        int hours = (int)uptime / 3600;
        int minutes = ((int)uptime % 3600) / 60;
        int seconds = (int)uptime % 60;
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.7f, 1.0f),
            "%02d:%02d:%02d", hours, minutes, seconds);
    }
    else {
        ImGui::Text("--:--:--");
    }

    ImGui::Columns(1);
    ImGui::Separator();

    // Estadísticas con barras animadas
    ImGui::Text("Comandos enviados:");
    ImGui::SameLine(180);
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f),
        "%llu", g_appState.connectionStats.commandsSent.load());

    ImGui::Text("Tasa de exito:");
    ImGui::SameLine(180);
    float successRate = g_appState.connectionStats.getSuccessRate();
    ImVec4 rateColor = successRate > 95.0f ? ImVec4(0.3f, 1.0f, 0.5f, 1.0f) :
        successRate > 85.0f ? ImVec4(1.0f, 1.0f, 0.3f, 1.0f) :
        ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    ImGui::TextColored(rateColor, "%.1f%%", successRate);

    ImGui::Text("Latencia promedio:");
    ImGui::SameLine(180);
    float latency = g_appState.connectionStats.averageLatency;
    ImVec4 latencyColor = latency < 5.0f ? ImVec4(0.3f, 1.0f, 0.5f, 1.0f) :
        latency < 10.0f ? ImVec4(1.0f, 1.0f, 0.3f, 1.0f) :
        ImVec4(1.0f, 0.5f, 0.3f, 1.0f);
    ImGui::TextColored(latencyColor, "%.2f ms", latency);

    ImGui::Spacing();

    // Botón de conexión con animación
    float buttonPulse = g_appState.animation.pulse;
    if (g_appState.deviceConnected) {
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(0.8f + buttonPulse * 0.1f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("DESCONECTAR", ImVec2(-1, 35))) {
            g_appState.makcuDevice.disconnect();
            g_appState.deviceConnected = false;
            g_appState.connectionStats.isConnected = false;
            g_appState.addLog("[INFO] Desconectado de MAKCU");
        }
        ImGui::PopStyleColor(2);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(0.2f, 0.6f + buttonPulse * 0.2f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
        if (ImGui::Button("CONECTAR", ImVec2(-1, 35))) {
            if (InitializeMakcu()) {
                ImVec2 btnPos = ImGui::GetItemRectMin();
                g_appState.particles.emit(
                    ImVec2(btnPos.x + 100, btnPos.y + 20),
                    ImVec4(0.3f, 1.0f, 0.5f, 1.0f),
                    25, 1
                );
            }
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// ENHANCED PATTERN VISUALIZER - ZOOM FIJO ALTO
// ============================================================================

void RenderEnhancedPatternVisualizer() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("PatternVisualizer", ImVec2(0.0f, 650.0f), true,
        ImGuiWindowFlags_NoScrollbar);

    // Título animado
    float glow = g_appState.animation.glow;
    ImVec4 titleColor = ImVec4(1.0f, 0.4f + glow * 0.3f, 0.2f, 1.0f);
    ImGui::TextColored(titleColor, "VISUALIZADOR DE PATRON DE RETROCESO");
    ImGui::Separator();

    // Control de zoom con slider AMPLIO
    ImGui::Text("Zoom:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300.0f);

    // Slider con rango MUY AMPLIO (1x a 100x)
    if (ImGui::SliderFloat("##zoom", &g_appState.visualizerScale, 1.0f, 100.0f, "%.1fx", ImGuiSliderFlags_Logarithmic)) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        g_appState.particles.emit(
            ImVec2(pos.x + 200, pos.y),
            ImVec4(0.4f, 0.8f, 1.0f, 1.0f), 3, 2
        );
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset (15x)", ImVec2(100, 0))) {
        g_appState.visualizerScale = 15.0f;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Stats", &g_appState.showStats);

    // Mostrar valor actual grande
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "Actual: %.1fx", g_appState.visualizerScale);

    ImGui::Spacing();

    // Canvas para visualización - CENTRADO PARA VER PATRÓN COMPLETO
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 530.0f);
    ImVec2 canvas_center = ImVec2(
        canvas_pos.x + canvas_size.x * 0.25f,  // Centrar más a la izquierda
        canvas_pos.y + canvas_size.y * 0.15f  // Centrar arriba para dar espacio vertical
    );

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Fondo con gradiente animado
    float wave = g_appState.animation.wave * 0.05f;
    draw_list->AddRectFilledMultiColor(
        canvas_pos,
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
        IM_COL32(12 + wave * 50, 12 + wave * 50, 20 + wave * 50, 255),
        IM_COL32(18 + wave * 50, 15 + wave * 50, 28 + wave * 50, 255),
        IM_COL32(25 + wave * 50, 20 + wave * 50, 38 + wave * 50, 255),
        IM_COL32(15 + wave * 50, 18 + wave * 50, 30 + wave * 50, 255)
    );

    // Grid animado para zoom 8x
    const int grid_step = 50;  // Grid espaciado apropiado
    float gridPulse = g_appState.animation.pulse;
    float gridAlpha = 35 + gridPulse * 20;

    for (int x = 0; x < canvas_size.x; x += grid_step) {
        float lineAlpha = gridAlpha + sin(g_appState.animation.time + x * 0.01f) * 10;
        draw_list->AddLine(
            ImVec2(canvas_pos.x + x, canvas_pos.y),
            ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
            IM_COL32(50, 100, 150, (int)lineAlpha), 1.0f
        );
    }
    for (int y = 0; y < canvas_size.y; y += grid_step) {
        float lineAlpha = gridAlpha + sin(g_appState.animation.time + y * 0.01f) * 10;
        draw_list->AddLine(
            ImVec2(canvas_pos.x, canvas_pos.y + y),
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
            IM_COL32(50, 100, 150, (int)lineAlpha), 1.0f
        );
    }

    // Crosshair central con efecto de brillo - ESCALABLE
    float crosshairGlow = g_appState.animation.glow;
    float zoomScaleCrosshair = std::min(1.0f, 15.0f / g_appState.visualizerScale);
    float crosshairSize = (20.0f + crosshairGlow * 5.0f) * zoomScaleCrosshair;

    // Anillos de brillo
    for (int i = 0; i < 3; i++) {
        float ring_radius = (8.0f + i * 5.0f + crosshairGlow * 4.0f) * zoomScaleCrosshair;
        draw_list->AddCircle(
            canvas_center,
            std::max(3.0f, ring_radius),
            IM_COL32(100, 200, 255, 60 - i * 15),
            32, std::max(1.0f, 2.0f * zoomScaleCrosshair)
        );
    }

    // Cruz central VISIBLE
    draw_list->AddLine(
        ImVec2(canvas_center.x - crosshairSize, canvas_center.y),
        ImVec2(canvas_center.x + crosshairSize, canvas_center.y),
        IM_COL32(150 + crosshairGlow * 105, 220 + crosshairGlow * 35, 255, 255),
        std::max(2.0f, 4.0f * zoomScaleCrosshair)
    );
    draw_list->AddLine(
        ImVec2(canvas_center.x, canvas_center.y - crosshairSize),
        ImVec2(canvas_center.x, canvas_center.y + crosshairSize),
        IM_COL32(150 + crosshairGlow * 105, 220 + crosshairGlow * 35, 255, 255),
        std::max(2.0f, 4.0f * zoomScaleCrosshair)
    );

    // Núcleo brillante
    draw_list->AddCircleFilled(
        canvas_center,
        std::max(3.0f, (7.0f + crosshairGlow * 3.0f) * zoomScaleCrosshair),
        IM_COL32(200, 240, 255, 230)
    );

    // Dibujar patrón del arma con efectos avanzados
    auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
    ImVec2 last_point = canvas_center;

    for (size_t i = 0; i < weapon.pattern.size(); ++i) {
        const auto& point = weapon.pattern[i];

        ImVec2 current_point(
            canvas_center.x + static_cast<float>(point.x * g_appState.visualizerScale),
            canvas_center.y + static_cast<float>(point.y * g_appState.visualizerScale)
        );

        // Trail con gradiente y brillo - ESCALABLE CON ZOOM
        float progress = (float)i / weapon.pattern.size();
        float alpha = 0.6f + progress * 0.4f;

        // Calcular escala dinámica basada en zoom
        float zoomScale = std::min(1.0f, 15.0f / g_appState.visualizerScale);
        float baseLineThickness = 4.0f * zoomScale;
        float baseGlowThickness = 8.0f * zoomScale;

        // Glow del trail
        draw_list->AddLine(
            last_point, current_point,
            IM_COL32(
                weapon.color.x * 255 * 0.7f,
                weapon.color.y * 255 * 0.7f,
                weapon.color.z * 255 * 0.7f,
                100
            ),
            std::max(2.0f, baseGlowThickness)
        );

        // Trail principal
        ImU32 line_color = ImColor(
            weapon.color.x,
            weapon.color.y,
            weapon.color.z,
            alpha
        );
        draw_list->AddLine(last_point, current_point, line_color, std::max(1.5f, baseLineThickness));

        // Puntos de bala - ESCALABLES
        float basePointSize = (i == 0) ? 12.0f : 8.0f;
        float point_size = basePointSize * zoomScale + sin(g_appState.animation.time * 2.0f + i * 0.2f) * 2.0f * zoomScale;
        float point_glow = (sin(g_appState.animation.time * 3.0f + i * 0.3f) + 1.0f) * 0.5f;

        // Halo exterior
        draw_list->AddCircleFilled(
            current_point,
            point_size + 5.0f * zoomScale,
            IM_COL32(
                weapon.color.x * 255 * 0.6f,
                weapon.color.y * 255 * 0.6f,
                weapon.color.z * 255 * 0.6f,
                80 + point_glow * 80
            )
        );

        // Punto principal - ESCALABLE
        ImU32 point_color = (i == 0) ?
            IM_COL32(100 + point_glow * 155, 255, 150, 255) :
            IM_COL32(255, 255 - progress * 155, 100 + point_glow * 155, 255);

        draw_list->AddCircleFilled(current_point, std::max(2.0f, point_size), point_color);

        // Borde brillante - ESCALABLE
        draw_list->AddCircle(
            current_point,
            std::max(2.0f, point_size),
            IM_COL32(255, 255, 255, 220),
            0, std::max(1.5f, 2.5f * zoomScale)
        );

        // Números de bala visibles - solo mostrar si hay espacio
        if (g_appState.showStats && zoomScale > 0.3f && (i % 3 == 0 || i == 0 || i == weapon.pattern.size() - 1)) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%zu", i + 1);

            float textScale = std::max(0.5f, zoomScale);
            float fontSize = 15.0f * textScale;
            float offsetX = 12.0f * textScale;
            float offsetY = 10.0f * textScale;

            // Sombra
            for (int j = 4; j > 0; j--) {
                draw_list->AddText(
                    ImVec2(current_point.x + offsetX + j * textScale, current_point.y - offsetY + j * textScale),
                    IM_COL32(0, 0, 0, 150 - j * 30),
                    buf
                );
            }

            // Texto principal
            ImFont* font = ImGui::GetFont();
            draw_list->AddText(
                font, fontSize,
                ImVec2(current_point.x + offsetX, current_point.y - offsetY),
                IM_COL32(
                    255,
                    255 - progress * 100,
                    100 + point_glow * 155,
                    255
                ),
                buf
            );
        }

        last_point = current_point;
    }

    // Renderizar partículas
    g_appState.particles.render(draw_list);

    // Leyenda con iconos animados
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::Indent(15.0f);

    float legendPulse = g_appState.animation.pulse;

    // Primera bala
    draw_list->AddCircleFilled(
        ImVec2(ImGui::GetCursorScreenPos().x + 7, ImGui::GetCursorScreenPos().y + 11),
        7.0f + legendPulse * 1.0f,
        IM_COL32(100, 255, 150, 255)
    );
    draw_list->AddCircle(
        ImVec2(ImGui::GetCursorScreenPos().x + 7, ImGui::GetCursorScreenPos().y + 11),
        7.0f + legendPulse * 1.0f,
        IM_COL32(255, 255, 255, 220), 0, 2.5f
    );
    ImGui::Dummy(ImVec2(20, 22));
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.7f, 1.0f), "Primera bala");

    ImGui::SameLine(200.0f);

    // Trayectoria
    draw_list->AddCircleFilled(
        ImVec2(ImGui::GetCursorScreenPos().x + 7, ImGui::GetCursorScreenPos().y + 11),
        7.0f,
        ImColor(weapon.color)
    );
    ImGui::Dummy(ImVec2(20, 22));
    ImGui::SameLine();
    ImGui::TextColored(weapon.color, "Trayectoria");

    ImGui::SameLine(400.0f);
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
        "Total: %zu balas", weapon.pattern.size());

    ImGui::Unindent(15.0f);

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// WEAPON SELECTION WITH CATEGORIES
// ============================================================================

void RenderWeaponsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);

    // Category selector
    ImGui::BeginChild("CategorySelector", ImVec2(0.0f, 60.0f), true);
    ImGui::Text("CATEGORIA:");
    ImGui::SameLine();

    const char* categories[] = { "ALL", "AR", "SMG", "LMG", "SPECIAL" };
    float buttonWidth = (ImGui::GetContentRegionAvail().x - 60.0f) / 5.0f;

    for (int i = 0; i < 5; i++) {
        if (i > 0) ImGui::SameLine();

        bool isSelected = (g_appState.selectedCategory == categories[i]);
        if (isSelected) {
            float pulse = g_appState.animation.pulse;
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.25f + pulse * 0.15f, 0.55f + pulse * 0.15f, 0.85f + pulse * 0.15f, 1.0f));
        }

        if (ImGui::Button(categories[i], ImVec2(buttonWidth, 30))) {
            g_appState.selectedCategory = categories[i];
        }

        if (isSelected) {
            ImGui::PopStyleColor();
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();

    // Weapon grid
    ImGui::BeginChild("WeaponSelection", ImVec2(0.0f, 250.0f), true);
    ImGui::Text("ARMAS DISPONIBLES");
    ImGui::Separator();

    ImGui::Columns(4, nullptr, false);

    int weaponIndex = 0;
    for (auto& [key, weapon] : g_weaponDatabase) {
        // Filter by category
        if (g_appState.selectedCategory != "ALL" &&
            weapon.category != g_appState.selectedCategory) {
            continue;
        }

        bool is_selected = (g_appState.currentWeaponKey == key);

        if (is_selected) {
            float pulse = g_appState.animation.pulse;
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(
                    weapon.color.x * (0.7f + pulse * 0.3f),
                    weapon.color.y * (0.7f + pulse * 0.3f),
                    weapon.color.z * (0.7f + pulse * 0.3f),
                    1.0f
                ));
        }

        if (ImGui::Button(weapon.displayName.c_str(), ImVec2(-1.0f, 50.0f))) {
            g_appState.currentWeaponKey = key;
            g_appState.addLog("[INFO] Arma cambiada a " + weapon.displayName);

            ImVec2 btnPos = ImGui::GetItemRectMin();
            ImVec2 btnSize = ImGui::GetItemRectSize();
            g_appState.particles.emit(
                ImVec2(btnPos.x + btnSize.x * 0.5f, btnPos.y + btnSize.y * 0.5f),
                weapon.color, 20, 1
            );
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }

        ImGui::NextColumn();
        weaponIndex++;
    }

    ImGui::Columns(1);

    // Weapon info con animación
    auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
    ImGui::Separator();

    float infoGlow = g_appState.animation.glow;
    ImVec4 infoColor = ImVec4(
        weapon.color.x * (0.7f + infoGlow * 0.3f),
        weapon.color.y * (0.7f + infoGlow * 0.3f),
        weapon.color.z * (0.7f + infoGlow * 0.3f),
        1.0f
    );

    ImGui::TextColored(infoColor, "%s", weapon.displayName.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| Balas: %zu | Cadencia: %.1f ms | Categoria: %s",
        weapon.pattern.size(), weapon.baseWaitTime, weapon.category.c_str());

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// ATTACHMENTS TAB
// ============================================================================

void RenderAttachmentsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::BeginChild("Attachments", ImVec2(0.0f, 220.0f), true);

    ImGui::Text("ACCESORIOS");
    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);

    // Scopes
    ImGui::Text("Mira:");
    for (int i = 0; i < 4; i++) {
        bool is_selected = (g_appState.selectedScope == i);

        if (is_selected) {
            float pulse = g_appState.animation.pulse;
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.25f + pulse * 0.2f, 0.55f + pulse * 0.2f, 0.85f + pulse * 0.2f, 1.0f));
        }

        if (ImGui::Button(g_scopes[i].name, ImVec2(-1.0f, 38.0f))) {
            g_appState.selectedScope = i;
            g_appState.addLog("[INFO] Mira cambiada a " + std::string(g_scopes[i].name));
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::NextColumn();

    // Barrels
    ImGui::Text("Canon:");
    for (int i = 0; i < 2; i++) {
        bool is_selected = (g_appState.selectedBarrel == i);

        if (is_selected) {
            float pulse = g_appState.animation.pulse;
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.25f + pulse * 0.2f, 0.55f + pulse * 0.2f, 0.85f + pulse * 0.2f, 1.0f));
        }

        if (ImGui::Button(g_barrels[i].name, ImVec2(-1.0f, 38.0f))) {
            g_appState.selectedBarrel = i;
            g_appState.addLog("[INFO] Canon cambiado a " + std::string(g_barrels[i].name));
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// SETTINGS TAB
// ============================================================================

void RenderSettingsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("Settings", ImVec2(0.0f, 0.0f), true);

    ImGui::Text("CONFIGURACION");
    ImGui::Separator();
    ImGui::Spacing();

    // Sensitivity con barra animada
    ImGui::Text("Sensibilidad");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat("##sens", &g_appState.sensitivity, 0.001f, 2.0f, "%.3f")) {
        g_appState.addLog("[INFO] Sensibilidad: " + std::to_string(g_appState.sensitivity));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Sensibilidad del raton en el juego");
    }

    ImGui::Spacing();

    // FOV
    ImGui::Text("Campo de Vision (FOV)");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##fov", &g_appState.fov, 65, 120)) {
        g_appState.addLog("[INFO] FOV: " + std::to_string(g_appState.fov));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Campo de vision en el juego");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Smoothing con toggle animado
    bool smoothingChanged = ImGui::Checkbox("Suavizado de Movimiento", &g_appState.smoothing);
    if (smoothingChanged) {
        g_appState.addLog(g_appState.smoothing ?
            "[INFO] Suavizado activado" : "[INFO] Suavizado desactivado");

        if (g_appState.smoothing) {
            ImVec2 pos = ImGui::GetItemRectMin();
            g_appState.particles.emit(
                ImVec2(pos.x - 20, pos.y + 10),
                ImVec4(0.3f, 0.8f, 1.0f, 1.0f), 15, 1
            );
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Suaviza los movimientos del raton para un control mas natural");
    }

    if (g_appState.smoothing) {
        ImGui::Indent();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::SliderInt("Factor de Suavizado", &g_appState.smoothingFactor, 1, 10)) {
            g_appState.addLog("[INFO] Factor: " + std::to_string(g_appState.smoothingFactor));
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Mayor = mas suave pero mas lento");
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Multiplicadores con visualización animada
    ImGui::Text("MULTIPLICADORES ACTUALES:");
    ImGui::Spacing();

    float scopeMult = static_cast<float>(GetScopeMultiplier());
    float barrelMult = static_cast<float>(GetBarrelMultiplier());
    float combinedMult = scopeMult * barrelMult;

    // Progress bar para cada multiplicador
    float pulse = g_appState.animation.pulse;

    ImGui::Text("Mira:");
    ImGui::SameLine(150);
    ImGui::ProgressBar(scopeMult / 5.0f, ImVec2(-1, 25),
        (std::to_string(scopeMult) + "x").c_str());

    ImGui::Text("Canon:");
    ImGui::SameLine(150);
    ImGui::ProgressBar(barrelMult / 1.5f, ImVec2(-1, 25),
        (std::to_string(barrelMult) + "x").c_str());

    ImGui::Text("Combinado:");
    ImGui::SameLine(150);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
        ImVec4(0.3f + pulse * 0.2f, 0.7f + pulse * 0.2f, 1.0f, 1.0f));
    ImGui::ProgressBar(combinedMult / 6.0f, ImVec2(-1, 25),
        (std::to_string(combinedMult) + "x").c_str());
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// CONSOLE LOG
// ============================================================================

void RenderConsoleLog() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
    ImGui::BeginChild("ConsoleLog", ImVec2(0.0f, 220.0f), true);

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "CONSOLA DE EVENTOS");
    ImGui::SameLine();
    float pulse = g_appState.animation.pulse;
    ImGui::PushStyleColor(ImGuiCol_Button,
        ImVec4(0.6f + pulse * 0.2f, 0.2f, 0.2f, 1.0f));
    if (ImGui::SmallButton("Limpiar")) {
        std::lock_guard<std::mutex> lock(g_appState.consoleMutex);
        g_appState.consoleLines.clear();
        g_appState.addLog("[INFO] Consola limpiada");
    }
    ImGui::PopStyleColor();

    ImGui::Separator();

    ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(g_appState.consoleMutex);
    for (const auto& line : g_appState.consoleLines) {
        ImVec4 color = ImVec4(0.85f, 0.85f, 0.9f, 1.0f);

        if (line.find("[ERROR]") != std::string::npos) {
            color = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
        }
        else if (line.find("[WARNING]") != std::string::npos) {
            color = ImVec4(1.0f, 0.75f, 0.15f, 1.0f);
        }
        else if (line.find("[SUCCESS]") != std::string::npos) {
            color = ImVec4(0.25f, 1.0f, 0.35f, 1.0f);
        }
        else if (line.find("[INFO]") != std::string::npos) {
            color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        }

        ImGui::TextColored(color, "%s", line.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// PRACTICE MODE
// ============================================================================

void RenderPracticeMode() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("PracticeMode", ImVec2(0.0f, 0.0f), true);

    float glow = g_appState.animation.glow;
    ImVec4 titleColor = ImVec4(1.0f, 0.3f + glow * 0.4f, 1.0f, 1.0f);
    ImGui::TextColored(titleColor, "MODO DE PRACTICA");
    ImGui::Separator();

    if (!g_appState.practiceSession.active) {
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::Indent(40.0f);

        ImGui::TextWrapped(
            "Bienvenido al Modo de Practica!\n\n"
            "Este modo rastreara tu precision en el control de retroceso en tiempo real.\n"
            "Tus movimientos del raton se compararan con el patron ideal.\n\n"
            "Recibiras una calificacion (S+, S, A, B, C, D) basada en:\n"
            "- Precision - Que tan cerca sigues el patron\n\n"
            "Listo para probar tus habilidades?"
        );

        ImGui::Dummy(ImVec2(0, 30));
        ImGui::Unindent(40.0f);

        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 250) * 0.5f);
        float buttonPulse = g_appState.animation.pulse;
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(0.2f + buttonPulse * 0.3f, 0.7f + buttonPulse * 0.2f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
            ImVec4(0.2f, 0.7f, 0.3f, 1.0f));

        if (ImGui::Button("COMENZAR PRACTICA", ImVec2(250, 55))) {
            g_appState.practiceSession.start();
            g_appState.addLog("[INFO] Sesion de practica iniciada!");
            ImVec2 btnPos = ImGui::GetItemRectMin();
            ImVec2 btnSize = ImGui::GetItemRectSize();
            g_appState.particles.emit(
                ImVec2(btnPos.x + btnSize.x * 0.5f, btnPos.y + btnSize.y * 0.5f),
                ImVec4(0.3f, 1.0f, 0.4f, 1.0f), 35, 1
            );
        }

        ImGui::PopStyleColor(3);

    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "SESION ACTIVA");
        ImGui::Separator();

        auto& session = g_appState.practiceSession;

        ImGui::Columns(3, nullptr, false);

        ImGui::Text("Disparos:");
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "%d", session.shotsFired);

        ImGui::NextColumn();

        ImGui::Text("Desv. Promedio:");
        float avgDev = session.shotsFired > 0 ? session.totalDeviation / session.shotsFired : 0.0f;
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%.2f px", avgDev);

        ImGui::NextColumn();

        ImGui::Text("Desv. Maxima:");
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.5f, 1.0f), "%.2f px", session.maxDeviation);

        ImGui::Columns(1);

        ImGui::Spacing();
        auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
        float progress = (float)session.shotsFired / weapon.pattern.size();

        char progressText[64];
        snprintf(progressText, sizeof(progressText), "Progreso: %d / %zu",
            session.shotsFired, weapon.pattern.size());

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
            ImVec4(0.3f, 0.7f + glow * 0.3f, 1.0f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(-1, 35), progressText);
        ImGui::PopStyleColor();

        ImGui::Spacing();

        if (ImGui::Button("DETENER Y EVALUAR", ImVec2(-1, 45))) {
            session.stop();
            g_appState.addLog("[INFO] Sesion completada!");
        }
    }

    // Mostrar resultados
    if (!g_appState.practiceSession.active && g_appState.practiceSession.shotsFired > 0) {
        ImGui::Separator();
        ImGui::Spacing();

        auto& session = g_appState.practiceSession;

        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 180) * 0.5f);

        std::string grade = session.getGrade();
        ImVec4 gradeColor = session.getGradeColor();

        float gradePulse = g_appState.animation.breathe;
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(
                gradeColor.x * (0.7f + gradePulse * 0.3f),
                gradeColor.y * (0.7f + gradePulse * 0.3f),
                gradeColor.z * (0.7f + gradePulse * 0.3f),
                1.0f
            ));

        char gradeText[32];
        snprintf(gradeText, sizeof(gradeText), "GRADO: %s", grade.c_str());
        ImGui::Button(gradeText, ImVec2(180, 80));
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Columns(2, nullptr, false);

        ImGui::Text("Puntuacion Total:");
        ImGui::TextColored(gradeColor, "%.1f%%", session.accuracyScore);

        ImGui::NextColumn();

        float avgDev = session.totalDeviation / session.shotsFired;
        ImGui::Text("Desv. Promedio:");
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%.2f px", avgDev);

        ImGui::Columns(1);

        ImGui::Spacing();
        if (ImGui::Button("INTENTAR DE NUEVO", ImVec2(-1, 40))) {
            g_appState.practiceSession.start();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// MAIN UI RENDER
// ============================================================================

void RenderMainUI() {
    // Update animations
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Update loading screen
    if (g_appState.loadingScreen.isActive) {
        g_appState.loadingScreen.update(deltaTime);
        g_appState.loadingScreen.render();
        return;
    }

    g_appState.animation.update(deltaTime);
    g_appState.particles.update(deltaTime);

    ImGui::SetNextWindowSize(ImVec2(1100.0f, 850.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100.0f, 50.0f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("MAKCU RECOIL CONTROL PRO - EDITION 2025", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Guardar Config", "Ctrl+S")) {
                g_appState.addLog("[INFO] Configuracion guardada");
            }
            if (ImGui::MenuItem("Cargar Config", "Ctrl+O")) {
                g_appState.addLog("[INFO] Configuracion cargada");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Salir", "Alt+F4")) {
                exit(0);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Vista")) {
            ImGui::MenuItem("Patron", nullptr, &g_appState.showPatternVisualizer);
            ImGui::MenuItem("Stats", nullptr, &g_appState.showStats);
            ImGui::MenuItem("Logs", nullptr, &g_appState.showLogs);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Ayuda")) {
            if (ImGui::MenuItem("Acerca de")) {
                g_appState.addLog("[INFO] MAKCU Recoil Control Pro 2025 v3.0");
            }
            if (ImGui::MenuItem("GitHub")) {
                g_appState.addLog("[INFO] Abriendo GitHub...");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Connection Status (siempre visible)
    RenderConnectionStatus();

    // Main Tab Bar
    ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None);

    if (ImGui::BeginTabItem("Control")) {
        RenderWeaponsTab();
        RenderAttachmentsTab();

        if (g_appState.showPatternVisualizer) {
            RenderEnhancedPatternVisualizer();
        }

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Ajustes")) {
        RenderSettingsTab();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Practica")) {
        RenderPracticeMode();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Estadisticas")) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::BeginChild("Stats", ImVec2(0, 0), true);

        ImGui::Text("ESTADISTICAS DETALLADAS");
        ImGui::Separator();

        auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
        ImGui::TextColored(weapon.color, "Arma Actual: %s", weapon.displayName.c_str());
        ImGui::Text("Longitud del Patron: %zu balas", weapon.pattern.size());
        ImGui::Text("Cadencia Base: %.2f ms", weapon.baseWaitTime);
        ImGui::Text("Categoria: %s", weapon.category.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("ESTADISTICAS DE SESION:");
        ImGui::BulletText("Comandos Totales: %llu",
            g_appState.connectionStats.commandsSent.load());
        ImGui::BulletText("Comandos Fallidos: %llu",
            g_appState.connectionStats.commandsFailed.load());
        ImGui::BulletText("Tasa de Exito: %.1f%%",
            g_appState.connectionStats.getSuccessRate());
        ImGui::BulletText("Latencia Media: %.2f ms",
            g_appState.connectionStats.averageLatency);

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    // Console Log (si está habilitado)
    if (g_appState.showLogs) {
        RenderConsoleLog();
    }

    ImGui::End();
}

// ============================================================================
// D3D9 SETUP
// ============================================================================

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    // Initialize logs
    g_appState.addLog("[INFO] Iniciando MAKCU Recoil Control Pro 2025 v3.0");
    g_appState.addLog("[INFO] Inicializando subsistemas...");

    // Create window
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        _T("MAKCU Recoil Control Pro"), NULL
    };
    ::RegisterClassEx(&wc);

    HWND hwnd = ::CreateWindow(
        wc.lpszClassName,
        _T("MAKCU Recoil Control Pro - Enhanced Edition 2025"),
        WS_OVERLAPPEDWINDOW, 100, 50, 1120, 890,
        NULL, NULL, wc.hInstance, NULL
    );

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    g_appState.addLog("[SUCCESS] Graficos inicializados");

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ApplyModernStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    g_appState.addLog("[INFO] ImGui inicializado");

    // Try to connect MAKCU after loading screen
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        if (InitializeMakcu()) {
            g_appState.addLog("[SUCCESS] Sistema listo!");
        }
        else {
            g_appState.addLog("[WARNING] MAKCU no conectado - Modo demo");
        }
        }).detach();

    g_appState.addLog("[INFO] Sistema iniciado!");

    // Main loop
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderMainUI();

        ImGui::EndFrame();

        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_RGBA(8, 8, 12, 255), 1.0f, 0);

        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST &&
            g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // Cleanup
    g_appState.addLog("[INFO] Cerrando aplicacion...");
    g_monitorActive = false;

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}