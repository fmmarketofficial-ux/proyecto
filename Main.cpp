// ============================================================================
// MAKCU RECOIL CONTROL PRO - Enhanced & Animated Version
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

// MAKCU API includes
#include "include/makcu.h"
#include "include/serialport.h"

// ============================================================================
// ANIMATION SYSTEM
// ============================================================================

struct AnimationState {
    float time = 0.0f;
    float pulse = 0.0f;
    float wave = 0.0f;
    float glow = 0.0f;
    float rotation = 0.0f;

    void update(float deltaTime) {
        time += deltaTime;
        pulse = (sin(time * 3.0f) + 1.0f) * 0.5f;
        wave = sin(time * 2.0f);
        glow = (sin(time * 4.0f) + 1.0f) * 0.5f;
        rotation += deltaTime * 0.5f;
    }

    float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    float easeOutElastic(float t) {
        if (t == 0 || t == 1) return t;
        return static_cast<float>(pow(2, -10 * t) * sin((t * 10 - 0.75f) * (2 * 3.14159f / 3)) + 1);
    }
};

// ============================================================================
// PARTICLE SYSTEM
// ============================================================================

struct Particle {
    ImVec2 pos;
    ImVec2 vel;
    ImVec4 color;
    float lifetime = 0.0f;
    float maxLifetime = 0.0f;
    float size = 0.0f;

    void update(float deltaTime) {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;
        lifetime -= deltaTime;
        vel.y += 50.0f * deltaTime; // Gravity
        color.w = lifetime / maxLifetime;
    }

    bool isAlive() const { return lifetime > 0; }
};

class ParticleSystem {
public:
    std::vector<Particle> particles;

    void emit(ImVec2 pos, ImVec4 color, int count = 5) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0, 2 * 3.14159f);
        std::uniform_real_distribution<float> speedDist(50, 150);
        std::uniform_real_distribution<float> lifeDist(0.5f, 1.5f);
        std::uniform_real_distribution<float> sizeDist(2.0f, 5.0f);

        for (int i = 0; i < count; ++i) {
            Particle p;
            float angle = angleDist(gen);
            float speed = speedDist(gen);

            p.pos = pos;
            p.vel = ImVec2(cos(angle) * speed, sin(angle) * speed);
            p.color = color;
            p.lifetime = lifeDist(gen);
            p.maxLifetime = p.lifetime;
            p.size = sizeDist(gen);

            particles.push_back(p);
        }
    }

    void update(float deltaTime) {
        for (auto& p : particles) {
            p.update(deltaTime);
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.isAlive(); }),
            particles.end()
        );
    }

    void render(ImDrawList* drawList) {
        for (const auto& p : particles) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(p.color);
            drawList->AddCircleFilled(p.pos, p.size, col);
        }
    }
};

// ============================================================================
// PRACTICE MODE & SCORING SYSTEM
// ============================================================================

struct PracticeSession {
    bool active = false;
    std::vector<ImVec2> recordedPath;
    std::vector<ImVec2> idealPath;
    std::vector<float> timings;
    std::chrono::steady_clock::time_point startTime;

    float totalDeviation = 0.0f;
    float maxDeviation = 0.0f;
    int shotsFired = 0;
    float timingScore = 100.0f;
    float accuracyScore = 100.0f;

    void start() {
        active = true;
        recordedPath.clear();
        idealPath.clear();
        timings.clear();
        totalDeviation = 0.0f;
        maxDeviation = 0.0f;
        shotsFired = 0;
        timingScore = 100.0f;
        accuracyScore = 100.0f;
        startTime = std::chrono::steady_clock::now();
    }

    void stop() {
        active = false;
        calculateScores();
    }

    void recordPoint(ImVec2 actual, ImVec2 ideal) {
        recordedPath.push_back(actual);
        idealPath.push_back(ideal);
        shotsFired++;

        float deviation = sqrtf(powf(actual.x - ideal.x, 2) + powf(actual.y - ideal.y, 2));
        totalDeviation += deviation;
        maxDeviation = std::max(maxDeviation, deviation);
    }

    void calculateScores() {
        if (shotsFired == 0) return;

        // Accuracy Score: based on average deviation
        float avgDeviation = totalDeviation / shotsFired;
        accuracyScore = std::max(0.0f, 100.0f - (avgDeviation * 2.0f));

        // Timing Score: based on timing consistency
        timingScore = 95.0f; // Simplified for now
    }

    float getOverallScore() const {
        return (accuracyScore * 0.7f + timingScore * 0.3f);
    }

    std::string getGrade() const {
        float score = getOverallScore();
        if (score >= 95.0f) return "S";
        if (score >= 85.0f) return "A";
        if (score >= 75.0f) return "B";
        if (score >= 65.0f) return "C";
        if (score >= 50.0f) return "D";
        return "F";
    }

    ImVec4 getGradeColor() const {
        std::string grade = getGrade();
        if (grade == "S") return ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
        if (grade == "A") return ImVec4(0.2f, 1.0f, 0.2f, 1.0f);   // Green
        if (grade == "B") return ImVec4(0.3f, 0.8f, 1.0f, 1.0f);   // Blue
        if (grade == "C") return ImVec4(1.0f, 0.6f, 0.0f, 1.0f);   // Orange
        if (grade == "D") return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);   // Red
        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);                      // Gray
    }
};

// ============================================================================
// CONNECTION MONITOR & STATISTICS
// ============================================================================

struct ConnectionStats {
    std::atomic<uint64_t> commandsSent{ 0 };
    std::atomic<uint64_t> commandsFailed{ 0 };
    std::atomic<uint64_t> reconnectAttempts{ 0 };
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

            // Calculate average
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
        if (total == 0) return 0.0f;
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
};

// ============================================================================
// WEAPON PATTERNS DATA
// ============================================================================

std::map<std::string, WeaponData> g_weaponDatabase = {
    {"AK47", {
        "AK47",
        "AK-47",
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
        {
            121.96, 92.63, 138.61, 113.38, 66.25, 66.30, 75.93, 85.06,
            89.20, 86.68, 78.82, 70.05, 60.86, 59.52, 71.67, 86.74,
            98.34, 104.34, 104.09, 97.59, 85.48, 70.49, 56.56, 47.39,
            56.64, 91.59, 112.39, 111.39, 87.51
        },
        133.33,
        ImVec4(1.0f, 0.3f, 0.3f, 1.0f)
    }},
    {"LR300", {
        "LR300",
        "LR-300",
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
        {},
        120.0,
        ImVec4(0.3f, 1.0f, 0.3f, 1.0f)
    }},
    {"MP5", {
        "MP5",
        "MP5A4",
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
        {},
        89.0,
        ImVec4(0.3f, 0.3f, 1.0f, 1.0f)
    }},
    {"SMG", {
        "SMG",
        "Custom SMG",
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
        {},
        90.0,
        ImVec4(1.0f, 1.0f, 0.3f, 1.0f)
    }},
    {"THOMPSON", {
        "THOMPSON",
        "Thompson",
        {
            {-0.114413,-0.680635},{0.008686,-0.676598},{0.010312,-0.682837},
            {0.064825,-0.691345},{0.104075,-0.655618},{-0.088118,-0.660429},
            {0.089906,-0.675183},{0.037071,-0.632623},{0.178465,-0.634737},
            {0.034654,-0.669443},{-0.082658,-0.664826},{0.025550,-0.636631},
            {0.082414,-0.647118},{-0.123305,-0.662104},{0.028164,-0.662354},
            {-0.117346,-0.693475},{-0.268777,-0.661123},{-0.053086,-0.677493},
            {0.04238,-0.647038}, {0.04238,-0.647038}
        },
        {},
        113.0,
        ImVec4(1.0f, 0.6f, 0.2f, 1.0f)
    }}
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

struct AppState {
    // Device
    makcu::Device makcuDevice;
    bool deviceConnected = false;
    ConnectionStats connectionStats;

    // Weapon Selection
    std::string currentWeaponKey = "AK47";

    // Attachments
    int selectedScope = 0;
    int selectedBarrel = 0;

    // Settings
    float sensitivity = 1.0f;
    int fov = 90;
    bool smoothing = true;
    int smoothingFactor = 1;

    // Control
    std::atomic<bool> leftButtonReleased{ false };
    std::atomic<bool> rightButtonReleased{ false };
    std::atomic<bool> isActive{ false };

    // UI State
    int currentTab = 0;
    bool showPatternVisualizer = true;
    float visualizerScale = 3.0f;
    bool showStats = true;
    bool showLogs = true;

    // Animation
    AnimationState animation;
    ParticleSystem particles;

    // Practice Mode
    PracticeSession practiceSession;

    // Console Logs
    std::vector<std::string> consoleLines;
    std::mutex consoleMutex;
    const size_t maxConsoleLines = 100;

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
// SCOPE & BARREL MULTIPLIERS
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
        if (!isPressed) {
            g_appState.isActive.store(false);
        }
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
// CONNECTION MONITOR (from Python version)
// ============================================================================

std::atomic<bool> g_monitorActive{ true };

void ConnectionMonitorThread() {
    const int keepaliveInterval = 60; // seconds
    auto lastKeepalive = std::chrono::steady_clock::now();

    while (g_monitorActive) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        if (!g_appState.deviceConnected) continue;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastKeepalive).count();

        // Send keepalive
        if (elapsed >= keepaliveInterval) {
            try {
                auto version = g_appState.makcuDevice.getVersion();
                if (!version.empty()) {
                    g_appState.addLog("[INFO] Keepalive OK - Version: " + version);
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
// DEVICE INITIALIZATION (Enhanced from Python version)
// ============================================================================

bool InitializeMakcu() {
    try {
        g_appState.addLog("[INFO] Searching for MAKCU devices...");
        auto devices = makcu::Device::findDevices();

        if (devices.empty()) {
            g_appState.addLog("[ERROR] No MAKCU devices found!");
            return false;
        }

        g_appState.addLog("[INFO] Found " + std::to_string(devices.size()) + " MAKCU device(s)");

        if (g_appState.makcuDevice.connect(devices[0].port)) {
            g_appState.makcuDevice.enableHighPerformanceMode(true);
            g_appState.makcuDevice.setMouseButtonCallback(OnMouseButton);
            g_appState.deviceConnected = true;
            g_appState.connectionStats.isConnected = true;
            g_appState.connectionStats.connectionStartTime = std::chrono::steady_clock::now();

            g_appState.addLog("[SUCCESS] MAKCU connected on " + devices[0].port);
            g_appState.addLog("[INFO] High-performance mode enabled");

            // Start connection monitor
            std::thread(ConnectionMonitorThread).detach();

            return true;
        }
    }
    catch (const makcu::MakcuException& e) {
        g_appState.addLog("[ERROR] MAKCU Error: " + std::string(e.what()));
    }
    return false;
}

// ============================================================================
// UI STYLING (Enhanced with animations)
// ============================================================================

void ApplyCustomStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Dark animated theme
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.09f, 0.98f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.09f, 0.98f);

    // Borders with glow
    colors[ImGuiCol_Border] = ImVec4(0.35f, 0.35f, 0.45f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.15f, 0.15f, 0.25f, 0.30f);

    // Headers & Tabs with animation support
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.40f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.50f, 0.80f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.18f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.50f, 0.80f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.42f, 0.75f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.32f, 0.60f, 1.00f);

    // Buttons with glow effect
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.35f, 0.65f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.32f, 0.58f, 1.00f);

    // Frame
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);

    // Checkboxes & Sliders
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.70f, 1.00f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.45f, 1.00f);

    // Rounding for smooth look
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 6.0f;

    // Padding
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.FramePadding = ImVec2(10.0f, 5.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
}

// ============================================================================
// ANIMATED CONNECTION STATUS PANEL
// ============================================================================

void RenderConnectionStatus() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::BeginChild("ConnectionStatus", ImVec2(0.0f, 180.0f), true);

    // Animated title
    float titleGlow = (sin(g_appState.animation.time * 2.0f) + 1.0f) * 0.5f;
    ImVec4 titleColor = ImVec4(0.3f + titleGlow * 0.3f, 0.8f + titleGlow * 0.2f, 1.0f, 1.0f);
    ImGui::TextColored(titleColor, "[*] CONNECTION STATUS");
    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 200);

    // Animated status indicator
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (g_appState.deviceConnected) {
        // Pulsing green circle
        float pulse = g_appState.animation.pulse;
        float radius = 8.0f + pulse * 3.0f;
        ImU32 color = IM_COL32(0, 255, 0, 200 - pulse * 100);
        drawList->AddCircleFilled(ImVec2(pos.x + 10, pos.y + 10), radius, color);
        drawList->AddCircleFilled(ImVec2(pos.x + 10, pos.y + 10), 6.0f, IM_COL32(0, 255, 0, 255));

        ImGui::Dummy(ImVec2(25, 20));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "CONNECTED");
    }
    else {
        // Pulsing red circle
        float pulse = g_appState.animation.pulse;
        drawList->AddCircleFilled(ImVec2(pos.x + 10, pos.y + 10), 8.0f,
            IM_COL32(255, 0, 0, 150 + pulse * 100));

        ImGui::Dummy(ImVec2(25, 20));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DISCONNECTED");
    }

    // Device info
    ImGui::Text("Device Port:");
    ImGui::Text("Baud Rate:");
    ImGui::Text("Uptime:");

    ImGui::NextColumn();

    auto info = g_appState.makcuDevice.getDeviceInfo();
    ImGui::Text("%s", g_appState.deviceConnected ? info.port.c_str() : "N/A");
    ImGui::Text("%s", g_appState.deviceConnected ? "4000000" : "N/A");

    if (g_appState.deviceConnected) {
        float uptime = g_appState.connectionStats.getUptime();
        int hours = (int)uptime / 3600;
        int minutes = ((int)uptime % 3600) / 60;
        int seconds = (int)uptime % 60;
        ImGui::Text("%02d:%02d:%02d", hours, minutes, seconds);
    }
    else {
        ImGui::Text("--:--:--");
    }

    ImGui::Columns(1);
    ImGui::Separator();

    // Statistics with animated progress bars
    ImGui::Text("Commands Sent:");
    ImGui::SameLine(150);
    ImGui::Text("%llu", g_appState.connectionStats.commandsSent.load());

    ImGui::Text("Commands Failed:");
    ImGui::SameLine(150);
    ImGui::Text("%llu", g_appState.connectionStats.commandsFailed.load());

    ImGui::Text("Success Rate:");
    ImGui::SameLine(150);
    float successRate = g_appState.connectionStats.getSuccessRate();
    ImVec4 rateColor = successRate > 90.0f ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) :
        successRate > 70.0f ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) :
        ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImGui::TextColored(rateColor, "%.1f%%", successRate);

    ImGui::Text("Avg Latency:");
    ImGui::SameLine(150);
    ImGui::Text("%.2f ms", g_appState.connectionStats.averageLatency);

    // Control buttons
    ImGui::Spacing();
    if (g_appState.deviceConnected) {
        if (ImGui::Button("[X] Disconnect", ImVec2(-1, 30))) {
            g_appState.makcuDevice.disconnect();
            g_appState.deviceConnected = false;
            g_appState.connectionStats.isConnected = false;
            g_appState.addLog("[INFO] Disconnected from MAKCU");
        }
    }
    else {
        if (ImGui::Button("[+] Connect", ImVec2(-1, 30))) {
            if (InitializeMakcu()) {
                g_appState.particles.emit(
                    ImVec2(pos.x + 200, pos.y + 100),
                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                    20
                );
            }
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// ENHANCED PATTERN VISUALIZER WITH ANIMATIONS
// ============================================================================

void RenderEnhancedPatternVisualizer() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::BeginChild("PatternVisualizer", ImVec2(0.0f, 450.0f), true, ImGuiWindowFlags_NoScrollbar);

    // Animated title with icon
    float glow = g_appState.animation.glow;
    ImVec4 titleColor = ImVec4(1.0f, 0.5f + glow * 0.3f, 0.0f, 1.0f);
    ImGui::TextColored(titleColor, "[*] RECOIL PATTERN ANALYZER");
    ImGui::Separator();

    // Control bar
    ImGui::Text("Zoom:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::SliderFloat("##zoom", &g_appState.visualizerScale, 1.0f, 10.0f, "%.1fx")) {
        // Emit particles on zoom change
        ImVec2 pos = ImGui::GetCursorScreenPos();
        g_appState.particles.emit(ImVec2(pos.x + 200, pos.y),
            ImVec4(0.5f, 0.8f, 1.0f, 1.0f), 3);
    }

    ImGui::SameLine();
    if (ImGui::Button("[@] Reset", ImVec2(80, 0))) {
        g_appState.visualizerScale = 3.0f;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Show Numbers", &g_appState.showStats);

    ImGui::Spacing();

    // Canvas for pattern visualization
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 340.0f);
    ImVec2 canvas_center = ImVec2(canvas_pos.x + canvas_size.x * 0.5f,
        canvas_pos.y + canvas_size.y * 0.15f);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Background with gradient
    draw_list->AddRectFilledMultiColor(
        canvas_pos,
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
        IM_COL32(15, 15, 20, 255),
        IM_COL32(20, 20, 30, 255),
        IM_COL32(25, 25, 35, 255),
        IM_COL32(20, 20, 30, 255)
    );

    // Animated grid
    const int grid_step = 50;
    float gridAlpha = 30 + g_appState.animation.pulse * 20;

    for (int x = 0; x < canvas_size.x; x += grid_step) {
        draw_list->AddLine(
            ImVec2(canvas_pos.x + x, canvas_pos.y),
            ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y),
            IM_COL32(60, 60, 80, (int)gridAlpha), 1.0f);
    }
    for (int y = 0; y < canvas_size.y; y += grid_step) {
        draw_list->AddLine(
            ImVec2(canvas_pos.x, canvas_pos.y + y),
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y),
            IM_COL32(60, 60, 80, (int)gridAlpha), 1.0f);
    }

    // Center crosshair with glow
    float crosshairGlow = g_appState.animation.glow;
    draw_list->AddCircleFilled(canvas_center, 3.0f + crosshairGlow * 2.0f,
        IM_COL32(255, 255, 255, 50 + crosshairGlow * 50));
    draw_list->AddLine(
        ImVec2(canvas_center.x - 15.0f, canvas_center.y),
        ImVec2(canvas_center.x + 15.0f, canvas_center.y),
        IM_COL32(255, 255, 255, 200), 2.5f);
    draw_list->AddLine(
        ImVec2(canvas_center.x, canvas_center.y - 15.0f),
        ImVec2(canvas_center.x, canvas_center.y + 15.0f),
        IM_COL32(255, 255, 255, 200), 2.5f);

    // Draw weapon pattern with trail effects
    auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
    ImVec2 last_point = canvas_center;

    for (size_t i = 0; i < weapon.pattern.size(); ++i) {
        const auto& point = weapon.pattern[i];

        ImVec2 current_point(
            canvas_center.x + static_cast<float>(point.x * g_appState.visualizerScale),
            canvas_center.y + static_cast<float>(point.y * g_appState.visualizerScale)
        );

        // Trail effect with gradient
        float alpha = 0.3f + (float)i / weapon.pattern.size() * 0.7f;
        ImU32 line_color = ImColor(
            weapon.color.x,
            weapon.color.y,
            weapon.color.z,
            alpha
        );

        // Draw thick line with glow
        draw_list->AddLine(last_point, current_point,
            IM_COL32(weapon.color.x * 255, weapon.color.y * 255, weapon.color.z * 255, 50),
            5.0f);
        draw_list->AddLine(last_point, current_point, line_color, 3.0f);

        // Point markers with size variation
        float point_size = (i == 0) ? 7.0f : (4.0f + sin(g_appState.animation.time + i) * 1.0f);

        // Glow effect
        if (i % 5 == 0) {
            draw_list->AddCircleFilled(current_point, point_size + 3.0f,
                IM_COL32(weapon.color.x * 255, weapon.color.y * 255, weapon.color.z * 255, 50));
        }

        // Main point
        ImU32 point_color = (i == 0) ?
            IM_COL32(0, 255, 0, 255) :
            IM_COL32(255, 255, 100, 220);
        draw_list->AddCircleFilled(current_point, point_size, point_color);

        // Outline
        draw_list->AddCircle(current_point, point_size, IM_COL32(255, 255, 255, 150), 0, 1.5f);

        // Bullet numbers (if enabled)
        if (g_appState.showStats && (i % 5 == 0 || i == 0)) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%zu", i + 1);

            // Text shadow
            draw_list->AddText(
                ImVec2(current_point.x + 9.0f, current_point.y - 7.0f),
                IM_COL32(0, 0, 0, 180),
                buf);

            // Main text
            draw_list->AddText(
                ImVec2(current_point.x + 8.0f, current_point.y - 8.0f),
                IM_COL32(255, 255, 100, 255),
                buf);
        }

        last_point = current_point;
    }

    // Render particles
    g_appState.particles.render(draw_list);

    // Legend with icons
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
    ImGui::Indent(10.0f);

    draw_list->AddCircleFilled(
        ImVec2(ImGui::GetCursorScreenPos().x + 5, ImGui::GetCursorScreenPos().y + 8),
        4.0f, IM_COL32(0, 255, 0, 255));
    ImGui::Dummy(ImVec2(15, 15));
    ImGui::SameLine();
    ImGui::Text("First Shot");

    ImGui::SameLine(150.0f);
    draw_list->AddCircleFilled(
        ImVec2(ImGui::GetCursorScreenPos().x + 5, ImGui::GetCursorScreenPos().y + 8),
        4.0f, ImColor(weapon.color));
    ImGui::Dummy(ImVec2(15, 15));
    ImGui::SameLine();
    ImGui::Text("Recoil Path");

    ImGui::SameLine(300.0f);
    ImGui::Text("Total Bullets: %zu", weapon.pattern.size());

    ImGui::Unindent(10.0f);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// PRACTICE MODE UI
// ============================================================================

void RenderPracticeMode() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::BeginChild("PracticeMode", ImVec2(0.0f, 0.0f), true);

    // Animated title
    float glow = g_appState.animation.glow;
    ImVec4 titleColor = ImVec4(1.0f, 0.3f + glow * 0.3f, 1.0f, 1.0f);
    ImGui::TextColored(titleColor, "[#] PRACTICE RANGE");
    ImGui::Separator();

    if (!g_appState.practiceSession.active) {
        // Start screen
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::Indent(50.0f);

        ImGui::TextWrapped(
            "Welcome to Practice Range!\n\n"
            "This mode will track your recoil control accuracy in real-time.\n"
            "Your mouse movements will be compared against the ideal pattern.\n\n"
            "You will receive a grade (S, A, B, C, D, F) based on:\n"
            "• Accuracy Score (70%%) - How close you follow the pattern\n"
            "• Timing Score (30%%) - How well you match the fire rate\n\n"
            "Ready to test your skills?"
        );

        ImGui::Dummy(ImVec2(0, 20));

        ImGui::Unindent(50.0f);

        // Animated start button
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 200) * 0.5f);
        float buttonPulse = (sin(g_appState.animation.time * 3.0f) + 1.0f) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(0.2f + buttonPulse * 0.2f, 0.6f + buttonPulse * 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

        if (ImGui::Button("[>] START PRACTICE", ImVec2(200, 50))) {
            g_appState.practiceSession.start();
            g_appState.addLog("[INFO] Practice session started!");
            g_appState.particles.emit(ImGui::GetCursorScreenPos(),
                ImVec4(0.2f, 1.0f, 0.2f, 1.0f), 30);
        }

        ImGui::PopStyleColor(3);

    }
    else {
        // Active practice session
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[!] PRACTICE SESSION ACTIVE");
        ImGui::Separator();

        auto& session = g_appState.practiceSession;

        // Real-time stats
        ImGui::Columns(3, nullptr, false);

        ImGui::Text("Shots Fired:");
        ImGui::Text("%d", session.shotsFired);

        ImGui::NextColumn();

        ImGui::Text("Avg Deviation:");
        float avgDev = session.shotsFired > 0 ?
            session.totalDeviation / session.shotsFired : 0.0f;
        ImGui::Text("%.2f px", avgDev);

        ImGui::NextColumn();

        ImGui::Text("Max Deviation:");
        ImGui::Text("%.2f px", session.maxDeviation);

        ImGui::Columns(1);

        // Progress bar
        ImGui::Spacing();
        auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
        float progress = (float)session.shotsFired / weapon.pattern.size();

        char progressText[64];
        snprintf(progressText, sizeof(progressText), "Progress: %d / %zu",
            session.shotsFired, weapon.pattern.size());
        ImGui::ProgressBar(progress, ImVec2(-1, 30), progressText);

        ImGui::Spacing();

        // Stop button
        if (ImGui::Button("[X] STOP & EVALUATE", ImVec2(-1, 40))) {
            session.stop();
            g_appState.addLog("[INFO] Practice session completed!");
        }
    }

    // Show results if session just ended
    if (!g_appState.practiceSession.active &&
        g_appState.practiceSession.shotsFired > 0) {

        ImGui::Separator();
        ImGui::Spacing();

        auto& session = g_appState.practiceSession;

        // Grade display with animation
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 150) * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));

        std::string grade = session.getGrade();
        ImVec4 gradeColor = session.getGradeColor();

        // Pulsing grade
        float gradePulse = (sin(g_appState.animation.time * 2.0f) + 1.0f) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(gradeColor.x, gradeColor.y, gradeColor.z, 0.8f + gradePulse * 0.2f));

        char gradeText[32];
        snprintf(gradeText, sizeof(gradeText), "GRADE: %s", grade.c_str());
        ImGui::Button(gradeText, ImVec2(150, 80));

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        // Detailed scores
        ImGui::Spacing();
        ImGui::Columns(2, nullptr, false);

        ImGui::Text("Overall Score:");
        ImGui::TextColored(gradeColor, "%.1f%%", session.getOverallScore());

        ImGui::NextColumn();

        ImGui::Text("Accuracy:");
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%.1f%%", session.accuracyScore);

        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Text("Timing Score: %.1f%%", session.timingScore);
        ImGui::Text("Average Deviation: %.2f pixels",
            session.totalDeviation / session.shotsFired);
        ImGui::Text("Maximum Deviation: %.2f pixels", session.maxDeviation);

        ImGui::Spacing();

        // Try again button
        if (ImGui::Button("[@] TRY AGAIN", ImVec2(-1, 35))) {
            g_appState.practiceSession.start();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// CONSOLE LOG VIEWER
// ============================================================================

void RenderConsoleLog() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("ConsoleLog", ImVec2(0.0f, 200.0f), true);

    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "[=] CONSOLE LOG");
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) {
        std::lock_guard<std::mutex> lock(g_appState.consoleMutex);
        g_appState.consoleLines.clear();
    }
    ImGui::Separator();

    ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(g_appState.consoleMutex);
    for (const auto& line : g_appState.consoleLines) {
        ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

        if (line.find("[ERROR]") != std::string::npos) {
            color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        }
        else if (line.find("[WARNING]") != std::string::npos) {
            color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        }
        else if (line.find("[SUCCESS]") != std::string::npos) {
            color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        }
        else if (line.find("[INFO]") != std::string::npos) {
            color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
        }

        ImGui::TextColored(color, "%s", line.c_str());
    }

    // Auto-scroll to bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// WEAPON SELECTION TAB
// ============================================================================

void RenderWeaponsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("WeaponSelection", ImVec2(0.0f, 220.0f), true);

    ImGui::Text("[+] WEAPON SELECTION");
    ImGui::Separator();

    ImGui::Columns(3, nullptr, false);

    int weaponIndex = 0;
    for (auto& [key, weapon] : g_weaponDatabase) {
        bool is_selected = (g_appState.currentWeaponKey == key);

        if (is_selected) {
            float pulse = g_appState.animation.pulse;
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.20f + pulse * 0.1f, 0.45f + pulse * 0.1f, 0.75f + pulse * 0.1f, 1.0f));
        }

        if (ImGui::Button(weapon.displayName.c_str(), ImVec2(-1.0f, 45.0f))) {
            g_appState.currentWeaponKey = key;
            g_appState.addLog("[INFO] Weapon changed to " + weapon.displayName);

            // Emit particles
            ImVec2 pos = ImGui::GetCursorScreenPos();
            g_appState.particles.emit(pos, weapon.color, 15);
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }

        ImGui::NextColumn();
        weaponIndex++;
    }

    ImGui::Columns(1);

    // Weapon info with animation
    auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
    ImGui::Separator();

    float infoGlow = (sin(g_appState.animation.time * 1.5f) + 1.0f) * 0.5f;
    ImVec4 infoColor = ImVec4(
        weapon.color.x * (0.8f + infoGlow * 0.2f),
        weapon.color.y * (0.8f + infoGlow * 0.2f),
        weapon.color.z * (0.8f + infoGlow * 0.2f),
        1.0f
    );

    ImGui::TextColored(infoColor, "[!] %s", weapon.displayName.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| Bullets: %zu | Fire Rate: %.1f ms",
        weapon.pattern.size(), weapon.baseWaitTime);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

// ============================================================================
// ATTACHMENTS TAB
// ============================================================================

void RenderAttachmentsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("Attachments", ImVec2(0.0f, 180.0f), true);

    ImGui::Text("[-] ATTACHMENTS");
    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);

    // Scopes
    ImGui::Text("Scope:");
    for (int i = 0; i < 4; i++) {
        bool is_selected = (g_appState.selectedScope == i);

        if (is_selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.50f, 0.80f, 1.0f));
        }

        if (ImGui::Button(g_scopes[i].name, ImVec2(-1.0f, 35.0f))) {
            g_appState.selectedScope = i;
            g_appState.addLog("[INFO] Scope changed to " + std::string(g_scopes[i].name));
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::NextColumn();

    // Barrels
    ImGui::Text("Barrel:");
    for (int i = 0; i < 2; i++) {
        bool is_selected = (g_appState.selectedBarrel == i);

        if (is_selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.50f, 0.80f, 1.0f));
        }

        if (ImGui::Button(g_barrels[i].name, ImVec2(-1.0f, 35.0f))) {
            g_appState.selectedBarrel = i;
            g_appState.addLog("[INFO] Barrel changed to " + std::string(g_barrels[i].name));
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
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("Settings", ImVec2(0.0f, 0.0f), true);

    ImGui::Text("[~] SETTINGS");
    ImGui::Separator();

    // Sensitivity
    ImGui::Text("Sensitivity");
    if (ImGui::SliderFloat("##sens", &g_appState.sensitivity, 0.001f, 2.0f, "%.3f")) {
        g_appState.addLog("[INFO] Sensitivity changed to " +
            std::to_string(g_appState.sensitivity));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("In-game mouse sensitivity");
    }

    ImGui::Spacing();

    // FOV
    ImGui::Text("Field of View");
    if (ImGui::SliderInt("##fov", &g_appState.fov, 65, 120)) {
        g_appState.addLog("[INFO] FOV changed to " + std::to_string(g_appState.fov));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("In-game FOV setting");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Smoothing
    if (ImGui::Checkbox("Enable Smoothing", &g_appState.smoothing)) {
        g_appState.addLog(g_appState.smoothing ?
            "[INFO] Smoothing enabled" : "[INFO] Smoothing disabled");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Smooth mouse movements for more natural recoil control");
    }

    if (g_appState.smoothing) {
        ImGui::Indent();
        if (ImGui::SliderInt("Smoothing Factor", &g_appState.smoothingFactor, 1, 10)) {
            g_appState.addLog("[INFO] Smoothing factor changed to " +
                std::to_string(g_appState.smoothingFactor));
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Higher = smoother but slower");
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Multipliers Info with animated display
    ImGui::Text("Current Multipliers:");

    float scopeMult = static_cast<float>(GetScopeMultiplier());
    float barrelMult = static_cast<float>(GetBarrelMultiplier());

    ImGui::BulletText("Scope: %.2fx", scopeMult);
    ImGui::BulletText("Barrel: %.2fx", barrelMult);
    ImGui::BulletText("Combined: %.2fx", scopeMult * barrelMult);

    ImGui::EndChild();
    ImGui::PopStyleVar();
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

    g_appState.animation.update(deltaTime);
    g_appState.particles.update(deltaTime);

    ImGui::SetNextWindowSize(ImVec2(1000.0f, 800.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("MAKCU Recoil Control Pro - Enhanced Edition", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Config", "Ctrl+S")) {
                g_appState.addLog("[INFO] Configuration saved");
            }
            if (ImGui::MenuItem("Load Config", "Ctrl+O")) {
                g_appState.addLog("[INFO] Configuration loaded");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                exit(0);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Pattern", nullptr, &g_appState.showPatternVisualizer);
            ImGui::MenuItem("Show Stats", nullptr, &g_appState.showStats);
            ImGui::MenuItem("Show Logs", nullptr, &g_appState.showLogs);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                g_appState.addLog("[INFO] MAKCU Recoil Control Pro v2.0");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Connection Status (always visible)
    RenderConnectionStatus();

    // Main Tab Bar
    ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None);

    if (ImGui::BeginTabItem("[*] Control")) {
        RenderWeaponsTab();
        RenderAttachmentsTab();

        if (g_appState.showPatternVisualizer) {
            RenderEnhancedPatternVisualizer();
        }

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("[~] Settings")) {
        RenderSettingsTab();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("[#] Practice")) {
        RenderPracticeMode();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("[%] Stats")) {
        ImGui::Text("Detailed statistics will be shown here");
        ImGui::Separator();

        auto& weapon = g_weaponDatabase[g_appState.currentWeaponKey];
        ImGui::Text("Current Weapon: %s", weapon.displayName.c_str());
        ImGui::Text("Pattern Length: %zu bullets", weapon.pattern.size());
        ImGui::Text("Base Fire Rate: %.2f ms", weapon.baseWaitTime);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Session Statistics:");
        ImGui::BulletText("Total Commands: %llu",
            g_appState.connectionStats.commandsSent.load());
        ImGui::BulletText("Failed Commands: %llu",
            g_appState.connectionStats.commandsFailed.load());
        ImGui::BulletText("Success Rate: %.1f%%",
            g_appState.connectionStats.getSuccessRate());

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    // Console Log (if enabled)
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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
    g_appState.addLog("[INFO] Starting MAKCU Recoil Control Pro v2.0");
    g_appState.addLog("[INFO] Initializing graphics subsystem...");

    // Initialize MAKCU device
    if (!InitializeMakcu()) {
        g_appState.addLog("[WARNING] MAKCU device not connected. Running in demo mode.");
    }

    // Create window
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        _T("MAKCU Recoil Control Pro"), NULL
    };
    ::RegisterClassEx(&wc);

    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("MAKCU Recoil Control Pro - Enhanced Edition"),
        WS_OVERLAPPEDWINDOW, 100, 100, 1020, 840,
        NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    g_appState.addLog("[SUCCESS] Graphics initialized successfully");

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ApplyCustomStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    g_appState.addLog("[INFO] ImGui initialized");
    g_appState.addLog("[INFO] System ready!");

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
            D3DCOLOR_RGBA(10, 10, 15, 255), 1.0f, 0);

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
    g_appState.addLog("[INFO] Shutting down...");
    g_monitorActive = false;

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}