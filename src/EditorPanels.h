#ifndef EDITORPANELS_H
#define EDITORPANELS_H

#include "imgui.h"
#include "imgui_internal.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Constants.h"
#include "Game.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ColliderComponent.h"
#include "../Components/LabelComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "EditorStyle.h"
#include <vector>
#include <string>
#include <deque>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>

extern EntityManager manager;
extern Map* map;
extern Entity* player;
extern Game* gameInstance;

// Log buffer and Level definitions
struct LogEntry {
    enum Level { INFO, LUA, WARN, ERR };
    Level level;
    std::string message;
};

inline std::deque<LogEntry>& GetLogBuffer() {
    static std::deque<LogEntry> buffer;
    return buffer;
}

inline void AddLogEntry(LogEntry::Level level, const std::string& msg) {
    auto& buffer = GetLogBuffer();
    buffer.push_back({level, msg});
    if (buffer.size() > 500) {
        buffer.pop_front();
    }
}

// Editor state helpers
inline Entity*& GetSelectedEntity() {
    static Entity* selectedEntity = nullptr;
    return selectedEntity;
}

inline bool& GetIsPlaying() {
    static bool isPlaying = true;
    return isPlaying;
}

inline int& GetCurrentTool() {
    static int currentTool = 0; // 0=select, 1=move, 2=rotate, 3=scale
    return currentTool;
}

inline bool& GetGridSnap() {
    static bool gridSnap = false;
    return gridSnap;
}

inline int& GetSelectedTileX() {
    static int selectedTileX = 0;
    return selectedTileX;
}

inline int& GetSelectedTileY() {
    static int selectedTileY = 0;
    return selectedTileY;
}

inline bool& GetHierarchyOpen() { static bool open = true; return open; }
inline bool& GetPropertiesOpen() { static bool open = true; return open; }
inline bool& GetViewportOpen() { static bool open = true; return open; }
inline bool& GetConsoleOpen() { static bool open = true; return open; }
inline bool& GetAssetBrowserOpen() { static bool open = true; return open; }
inline bool& GetToolbarOpen() { static bool open = true; return open; }
inline bool& GetTilemapEditorOpen() { static bool open = true; return open; }
inline bool& GetScriptPanelOpen() { static bool open = true; return open; }

// ---------------------------------------------------------
// Custom key-value readout (black/gray text, blue values)
// ---------------------------------------------------------
inline void DrawStatusReadout(const char* label, const char* value) {
    ImGui::BeginGroup();

    // Label (uppercase, dimmed gray)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();

    ImGui::SameLine(80);  // align values

    // Value (blue)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.00f, 0.50f, 1.0f));
    ImGui::TextUnformatted(value);
    ImGui::PopStyleColor();

    ImGui::EndGroup();
}

// ---------------------------------------------------------
// Segmented Status Bar Helper (white background segments)
// ---------------------------------------------------------
inline void DrawStatusSegment(ImDrawList* dl, float x, float y, float w, float h,
                             const char* label, const char* value) {
    ImVec2 min = ImVec2(x, y);
    ImVec2 max = ImVec2(x + w, y + h);
    dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, 255)); // white bg
    DrawBevelRect(dl, min, max, false, 1.0f); // sunken

    // Draw label
    ImVec2 textPos = ImVec2(x + 4, y + 2);
    dl->AddText(textPos, IM_COL32(80, 80, 80, 255), label);  // dark gray
    float labelWidth = ImGui::CalcTextSize(label).x;
    // Draw value
    dl->AddText(ImVec2(x + 4 + labelWidth + 4, y + 2),
                IM_COL32(0, 0, 128, 255), value);  // dark blue
}

// ---------------------------------------------------------
// Scene Hierarchy Panel
// ---------------------------------------------------------
inline void DrawSceneHierarchy() {
    bool& open = GetHierarchyOpen();
    if (!ImGui::Begin("SCENE HIERARCHY", &open)) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::Text("ENTITIES: %d", manager.GetEntityCount());
    ImGui::PopStyleColor();
    ImGui::Separator();

    const char* layerNames[] = {
        "TILEMAP", "VEGETATION", "ENEMY", "OBSTACLE",
        "PLAYER", "PROJECTILE", "GUI"
    };

    Entity*& selectedEntity = GetSelectedEntity();

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        auto entities = manager.GetEntitiesByLayer(static_cast<LayerType>(layer));
        if (entities.empty()) continue;

        char headerLabel[64];
        snprintf(headerLabel, sizeof(headerLabel), "[%s] (%zu)",
                 layerNames[layer], entities.size());

        if (ImGui::CollapsingHeader(headerLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto* entity : entities) {
                ImGuiTreeNodeFlags nodeFlags =
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_SpanAvailWidth;

                bool isSelected = (entity == selectedEntity);
                if (isSelected) {
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;
                    // White text to contrast with dark blue selected header bg
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }

                const char* icon = "  ";
                if (entity->HasComponent<SpriteComponent>())       icon = "# ";
                if (entity->HasComponent<ColliderComponent>())     icon = "@ ";
                if (entity->name == "chopper")                     icon = "> ";

                char label[128];
                snprintf(label, sizeof(label), "%s%s##%p",
                         icon, entity->name.c_str(), (void*)entity);

                if (ImGui::TreeNodeEx(label, nodeFlags)) {
                    ImGui::TreePop();
                }

                if (isSelected) {
                    ImGui::PopStyleColor();
                }

                if (ImGui::IsItemClicked()) {
                    selectedEntity = entity;
                }
            }
        }
    }

    ImGui::End();
}

// ---------------------------------------------------------
// Properties Panel
// ---------------------------------------------------------
inline void DrawPropertiesPanel() {
    bool& open = GetPropertiesOpen();
    if (!ImGui::Begin("PROPERTIES", &open)) {
        ImGui::End();
        return;
    }

    Entity* selectedEntity = GetSelectedEntity();
    if (!selectedEntity) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.40f, 0.40f, 1.0f));
        ImGui::TextUnformatted("NO ENTITY SELECTED");
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.00f, 0.50f, 1.00f)); // blue title
    ImGui::Text("ENTITY: %s", selectedEntity->name.c_str());
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::Text("LAYER: %d  ACTIVE: %s",
                selectedEntity->layer,
                selectedEntity->IsActive() ? "YES" : "NO");
    ImGui::PopStyleColor();

    ImGui::Separator();

    // TransformComponent
    if (selectedEntity->HasComponent<TransformComponent>()) {
        if (ImGui::CollapsingHeader("TRANSFORM", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* t = selectedEntity->GetComponent<TransformComponent>();

            ImGui::DragFloat2("POSITION", &t->position.x, 1.0f);
            ImGui::DragFloat2("VELOCITY", &t->velocity.x, 0.5f);
            ImGui::DragInt("WIDTH",  &t->width, 1, 1, 1024);
            ImGui::DragInt("HEIGHT", &t->height, 1, 1, 1024);
            ImGui::DragInt("SCALE",  &t->scale, 1, 1, 10);
        }
    }

    // SpriteComponent
    if (selectedEntity->HasComponent<SpriteComponent>()) {
        if (ImGui::CollapsingHeader("SPRITE", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* s = selectedEntity->GetComponent<SpriteComponent>();

            ImGui::Checkbox("ANIMATED", &s->isAnimated);
            ImGui::DragInt("FRAMES", &s->numFrames, 1, 1, 64);
            ImGui::DragInt("SPEED", &s->animationSpeed, 1, 1, 1000);
            ImGui::Checkbox("FIXED", &s->isFixed);

            ImGui::Separator();
            if (ImGui::Button("PLAY DOWN"))  s->Play("DownAnimation");
            ImGui::SameLine();
            if (ImGui::Button("PLAY UP"))    s->Play("UpAnimation");
            if (ImGui::Button("PLAY LEFT"))  s->Play("LeftAnimation");
            ImGui::SameLine();
            if (ImGui::Button("PLAY RIGHT")) s->Play("RightAnimation");

            int flipVal = static_cast<int>(s->spriteFlip);
            const char* flipItems[] = { "NONE", "HORIZONTAL", "VERTICAL", "BOTH" };
            if (ImGui::Combo("FLIP", &flipVal, flipItems, IM_ARRAYSIZE(flipItems))) {
                if (flipVal == 0) s->spriteFlip = SDL_FLIP_NONE;
                else if (flipVal == 1) s->spriteFlip = SDL_FLIP_HORIZONTAL;
                else if (flipVal == 2) s->spriteFlip = SDL_FLIP_VERTICAL;
            }
        }
    }

    // ColliderComponent
    if (selectedEntity->HasComponent<ColliderComponent>()) {
        if (ImGui::CollapsingHeader("COLLIDER", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* c = selectedEntity->GetComponent<ColliderComponent>();

            char tagBuf[256];
            strncpy(tagBuf, c->colliderTag.c_str(), sizeof(tagBuf));
            if (ImGui::InputText("TAG", tagBuf, sizeof(tagBuf))) {
                c->colliderTag = tagBuf;
            }

            ImGui::DragInt("POS X", &c->collider.x, 1);
            ImGui::DragInt("POS Y", &c->collider.y, 1);
            ImGui::DragInt("WIDTH", &c->collider.w, 1);
            ImGui::DragInt("HEIGHT", &c->collider.h, 1);
        }
    }

    // ProjectileEmitterComponent
    if (selectedEntity->HasComponent<ProjectileEmitterComponent>()) {
        if (ImGui::CollapsingHeader("PROJECTILE EMITTER", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* pe = selectedEntity->GetComponent<ProjectileEmitterComponent>();
            ImGui::DragInt("SPEED", &pe->speed, 1, 0, 2000);
            ImGui::DragInt("RANGE", &pe->range, 1, 0, 5000);
            float angleDeg = glm::degrees(pe->angleRadians);
            if (ImGui::DragFloat("ANGLE (DEG)", &angleDeg, 1.0f, -360.0f, 360.0f)) {
                pe->angleRadians = glm::radians(angleDeg);
            }
            ImGui::Checkbox("LOOP", &pe->shouldLoop);
        }
    }

    // LabelComponent
    if (selectedEntity->HasComponent<LabelComponent>()) {
        if (ImGui::CollapsingHeader("LABEL", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto* l = selectedEntity->GetComponent<LabelComponent>();

            char textBuf[512];
            strncpy(textBuf, l->text.c_str(), sizeof(textBuf));
            if (ImGui::InputText("TEXT", textBuf, sizeof(textBuf))) {
                l->text = textBuf;
                l->SetLabelText(l->text, l->fontFamily);
            }

            char fontBuf[256];
            strncpy(fontBuf, l->fontFamily.c_str(), sizeof(fontBuf));
            if (ImGui::InputText("FONT", fontBuf, sizeof(fontBuf))) {
                l->fontFamily = fontBuf;
                l->SetLabelText(l->text, l->fontFamily);
            }

            float colorArr[4] = { l->color.r / 255.0f, l->color.g / 255.0f, l->color.b / 255.0f, l->color.a / 255.0f };
            if (ImGui::ColorEdit4("COLOR", colorArr)) {
                l->color.r = static_cast<Uint8>(colorArr[0] * 255.0f);
                l->color.g = static_cast<Uint8>(colorArr[1] * 255.0f);
                l->color.b = static_cast<Uint8>(colorArr[2] * 255.0f);
                l->color.a = static_cast<Uint8>(colorArr[3] * 255.0f);
                l->SetLabelText(l->text, l->fontFamily);
            }
        }
    }

    ImGui::Separator();

    // Add Component button
    if (ImGui::Button("+ ADD COMPONENT", ImVec2(-1, 0))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (!selectedEntity->HasComponent<TransformComponent>()) {
            if (ImGui::MenuItem("Transform")) {
                selectedEntity->AddComponent<TransformComponent>(0, 0, 0, 0, 32, 32, 1);
            }
        }
        if (!selectedEntity->HasComponent<ColliderComponent>()) {
            if (ImGui::MenuItem("Collider")) {
                auto* t = selectedEntity->GetComponent<TransformComponent>();
                if (t) {
                    selectedEntity->AddComponent<ColliderComponent>(
                        "DEFAULT",
                        static_cast<int>(t->position.x),
                        static_cast<int>(t->position.y),
                        t->width, t->height);
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

// ---------------------------------------------------------
// Asset Browser
// ---------------------------------------------------------
inline void DrawDirectoryTree(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return;

    struct dirent* entry;
    std::vector<std::string> dirs, files;

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == ".." || name == ".DS_Store" || name == ".git") continue;

        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            dirs.push_back(name);
        } else {
            files.push_back(name);
        }
    }
    closedir(dir);

    // Sort alphabetically
    std::sort(dirs.begin(), dirs.end());
    std::sort(files.begin(), files.end());

    // Draw directories first
    for (const auto& d : dirs) {
        if (ImGui::TreeNode(d.c_str())) {
            DrawDirectoryTree(path + "/" + d);
            ImGui::TreePop();
        }
    }

    // Draw files
    for (const auto& f : files) {
        // Icon based on extension
        const char* icon = "  ";
        if (f.find(".png") != std::string::npos)  icon = "# ";  // image
        if (f.find(".ttf") != std::string::npos)  icon = "F ";  // font
        if (f.find(".wav") != std::string::npos)  icon = "~ ";  // audio
        if (f.find(".lua") != std::string::npos)  icon = "> ";  // script
        if (f.find(".map") != std::string::npos)  icon = "M ";  // tilemap

        ImGui::TreeNodeEx(
            (std::string(icon) + f).c_str(),
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
        );

        if (ImGui::BeginDragDropSource()) {
            std::string fullPath = path + "/" + f;
            ImGui::SetDragDropPayload("ASSET_PATH", fullPath.c_str(), fullPath.size() + 1);
            ImGui::TextUnformatted(f.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

inline void DrawAssetBrowser() {
    bool& open = GetAssetBrowserOpen();
    if (!ImGui::Begin("ASSET BROWSER", &open)) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::TextUnformatted("PROJECT ASSETS");
    ImGui::PopStyleColor();
    ImGui::Separator();

    DrawDirectoryTree("assets");

    ImGui::End();
}

// ---------------------------------------------------------
// Console / Log Panel
// ---------------------------------------------------------
inline void DrawConsolePanel() {
    bool& open = GetConsoleOpen();
    if (!ImGui::Begin("CONSOLE", &open)) {
        ImGui::End();
        return;
    }

    // Filter buttons
    static bool showInfo = true, showLua = true, showWarn = true, showErr = true;
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));
    
    if (ImGui::SmallButton(showInfo ? "INFO [ON]" : "INFO [OFF]")) showInfo = !showInfo; ImGui::SameLine();
    if (ImGui::SmallButton(showLua ? "LUA [ON]" : "LUA [OFF]"))  showLua  = !showLua;  ImGui::SameLine();
    if (ImGui::SmallButton(showWarn ? "WARN [ON]" : "WARN [OFF]")) showWarn = !showWarn;  ImGui::SameLine();
    if (ImGui::SmallButton(showErr ? "ERR [ON]" : "ERR [OFF]"))  showErr  = !showErr;   ImGui::SameLine();
    if (ImGui::SmallButton("CLEAR")) GetLogBuffer().clear();

    ImGui::PopStyleVar();
    ImGui::Separator();

    // Log output area
    ImGui::BeginChild("LogScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

    auto& logBuffer = GetLogBuffer();
    for (const auto& entry : logBuffer) {
        if (entry.level == LogEntry::INFO && !showInfo) continue;
        if (entry.level == LogEntry::LUA  && !showLua)  continue;
        if (entry.level == LogEntry::WARN && !showWarn) continue;
        if (entry.level == LogEntry::ERR  && !showErr)  continue;

        ImVec4 color;
        const char* prefix;
        switch (entry.level) {
            case LogEntry::INFO: color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f); prefix = "[INFO] "; break; // black
            case LogEntry::LUA:  color = ImVec4(0.00f, 0.40f, 0.20f, 1.00f); prefix = "[LUA]  "; break; // green
            case LogEntry::WARN: color = ImVec4(0.70f, 0.40f, 0.00f, 1.00f); prefix = "[WARN] "; break; // brown
            case LogEntry::ERR:  color = ImVec4(0.80f, 0.00f, 0.00f, 1.00f); prefix = "[ERR]  "; break; // red
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted((std::string(prefix) + entry.message).c_str());
        ImGui::PopStyleColor();
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    // Lua REPL input line
    static char luaInputBuf[256] = "";
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##LuaInput", luaInputBuf, sizeof(luaInputBuf),
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string cmd(luaInputBuf);
        AddLogEntry(LogEntry::LUA, "> " + cmd);

        // Execute via ScriptManager
        sol::state& lua = Game::scriptManager->GetLuaState();
        try {
            auto result = lua.safe_script(cmd);
            if (result.valid()) {
                if (result.return_count() > 0) {
                    std::string resStr = result.get<std::string>();
                    AddLogEntry(LogEntry::LUA, resStr);
                }
            } else {
                sol::error err = result;
                AddLogEntry(LogEntry::ERR, err.what());
            }
        } catch (const std::exception& e) {
            AddLogEntry(LogEntry::ERR, e.what());
        }

        luaInputBuf[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1);
    }
    ImGui::PopItemWidth();

    ImGui::End();
}

// ---------------------------------------------------------
// Toolbar
// ---------------------------------------------------------
inline void DrawToolbar() {
    bool& open = GetToolbarOpen();
    if (!ImGui::Begin("TOOLBAR", &open,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    bool& isPlaying = GetIsPlaying();

    // Play controls (Win95 style - green if playing)
    ImGui::PushStyleColor(ImGuiCol_Button,
        isPlaying ? ImVec4(0.00f, 0.50f, 0.00f, 0.6f) : ImGui::GetStyle().Colors[ImGuiCol_Button]);
    if (ImGui::Button(isPlaying ? "||" : " >")) {
        isPlaying = !isPlaying;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.10f, 0.10f, 0.6f));
    if (ImGui::Button("[]")) {
        isPlaying = false;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Tool selection
    int& currentTool = GetCurrentTool();
    const char* toolLabels[] = { "SEL", "MOV", "ROT", "SCL" };
    for (int i = 0; i < 4; i++) {
        if (i > 0) ImGui::SameLine();
        bool selected = (currentTool == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.00f, 0.50f, 1.0f)); // active blue
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        if (ImGui::Button(toolLabels[i], ImVec2(32, 0))) {
            currentTool = i;
        }
        if (selected) {
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Grid snap
    bool& gridSnap = GetGridSnap();
    ImGui::Checkbox("GRID", &gridSnap);

    ImGui::End();
}

// ---------------------------------------------------------
// Tilemap Editor Panel
// ---------------------------------------------------------
inline void DrawTilemapEditor() {
    bool& open = GetTilemapEditorOpen();
    if (!ImGui::Begin("TILEMAP EDITOR", &open)) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("TILE PALETTE");
    ImGui::Separator();

    int& selectedTileX = GetSelectedTileX();
    int& selectedTileY = GetSelectedTileY();

    int tileCols = 8;
    int tileRows = 4;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));
    for (int r = 0; r < tileRows; r++) {
        for (int c = 0; c < tileCols; c++) {
            if (c > 0) ImGui::SameLine();

            char buf[16];
            snprintf(buf, sizeof(buf), "%d,%d", c, r);

            bool isSelected = (c == selectedTileX && r == selectedTileY);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.00f, 0.50f, 1.0f)); // selected active blue
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            if (ImGui::Button(buf, ImVec2(28, 24))) {
                selectedTileX = c;
                selectedTileY = r;
            }

            if (isSelected) {
                ImGui::PopStyleColor(2);
            }
        }
    }
    ImGui::PopStyleVar();

    ImGui::Separator();

    static int paintTool = 0;
    const char* tools[] = { "PAINT", "ERASE", "FILL", "PICK" };
    for (int i = 0; i < 4; i++) {
        if (i > 0) ImGui::SameLine();
        bool selected = (paintTool == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.00f, 0.50f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        if (ImGui::Button(tools[i])) paintTool = i;
        if (selected) ImGui::PopStyleColor(2);
    }

    ImGui::Separator();

    ImGui::TextUnformatted("LAYERS:");
    static bool layerVisible[4] = {true, true, true, true};
    for (int i = 0; i < 4; i++) {
        char layerLabel[16];
        snprintf(layerLabel, sizeof(layerLabel), "LAYER %d", i);
        ImGui::Checkbox(layerLabel, &layerVisible[i]);
    }

    ImGui::End();
}

// ---------------------------------------------------------
// Lua Script List Panel
// ---------------------------------------------------------
inline void DrawScriptPanel() {
    bool& open = GetScriptPanelOpen();
    if (!ImGui::Begin("LUA SCRIPTS", &open)) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("LOADED SCRIPTS:");
    ImGui::Separator();

    static const char* scripts[] = {
        "assets/scripts/level0.lua",
        "assets/scripts/enemy_patrol.lua"
    };

    static const char* selectedScript = nullptr;

    for (const auto& script : scripts) {
        bool isSelected = (selectedScript && strcmp(selectedScript, script) == 0);
        if (ImGui::Selectable(script, isSelected)) {
            selectedScript = script;
            AddLogEntry(LogEntry::INFO, "Selected script: " + std::string(script));
        }
    }

    ImGui::Separator();

    if (selectedScript) {
        if (strstr(selectedScript, "level") != nullptr) {
            if (ImGui::Button("LOAD LEVEL", ImVec2(-1, 0))) {
                AddLogEntry(LogEntry::INFO, "Loading level: " + std::string(selectedScript));
                int levelNum = 0;
                if (sscanf(selectedScript, "assets/scripts/level%d.lua", &levelNum) == 1) {
                    if (gameInstance) {
                        gameInstance->LoadLevel(levelNum);
                        AddLogEntry(LogEntry::INFO, "Level loaded successfully!");
                    } else {
                        AddLogEntry(LogEntry::ERR, "gameInstance is NULL!");
                    }
                } else {
                    AddLogEntry(LogEntry::ERR, "Failed to parse level number from " + std::string(selectedScript));
                }
            }
            ImGui::Separator();
        }
        if (ImGui::Button("OPEN IN EXTERNAL EDITOR", ImVec2(-1, 0))) {
            std::string cmd = "open " + std::string(selectedScript);
            system(cmd.c_str());
        }
        ImGui::Separator();
    }

    if (ImGui::Button("RELOAD ALL", ImVec2(-1, 0))) {
        AddLogEntry(LogEntry::INFO, "Forcing reload of all scripts...");
    }

    ImGui::End();
}

// ---------------------------------------------------------
// Taskbar Button helper for status bar
// ---------------------------------------------------------
inline void DrawTaskbarButton(const char* label, bool& isOpen, float x, float y, float w, float h) {
    ImGui::SetCursorScreenPos(ImVec2(x, y));
    
    if (isOpen) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.65f, 0.65f, 1.00f)); // sunken darker gray
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.60f, 0.60f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.70f, 0.70f, 1.00f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 1.00f)); // raised light gray
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.65f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.80f, 0.80f, 1.00f));
    }
    
    char idLabel[64];
    snprintf(idLabel, sizeof(idLabel), "%s##Taskbar", label);
    if (ImGui::Button(idLabel, ImVec2(w, h))) {
        isOpen = !isOpen;
    }
    
    // Draw 3D bevel effect
    ImDrawList* dl = ImGui::GetWindowDrawList();
    DrawBevelRect(dl, ImVec2(x, y), ImVec2(x + w, y + h), !isOpen, 1.5f);
    
    ImGui::PopStyleColor(3);
}

// ---------------------------------------------------------
// Segmented Status Bar
// ---------------------------------------------------------
inline void DrawRetroStatusBar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float barHeight = ImGui::GetFrameHeight() + 6;

    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - barHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, barHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.75f, 0.75f, 0.75f, 1.0f)); // Win95 light gray
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 2));

    ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wmin = ImGui::GetWindowPos();
    ImVec2 wmax = ImVec2(wmin.x + ImGui::GetWindowWidth(), wmin.y + barHeight);

    // Sunken border
    DrawBevelRect(dl, wmin, wmax, false);

    // Retrieve stats
    char fpsStr[32], entStr[32], mouseStr[64], camStr[64];
    snprintf(fpsStr, sizeof(fpsStr), "%.0f", ImGui::GetIO().Framerate);
    snprintf(entStr, sizeof(entStr), "%d", manager.GetEntityCount());
    snprintf(mouseStr, sizeof(mouseStr), "%.0f,%.0f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    snprintf(camStr, sizeof(camStr), "%d,%d", Game::camera.x, Game::camera.y);

    // Segment 1: FPS
    DrawStatusSegment(dl, wmin.x + 4, wmin.y + 2, 110, barHeight - 4, "FPS:", fpsStr);
    // Segment 2: Entities
    DrawStatusSegment(dl, wmin.x + 118, wmin.y + 2, 130, barHeight - 4, "ENTITIES:", entStr);
    // Segment 3: Mouse
    DrawStatusSegment(dl, wmin.x + 252, wmin.y + 2, 120, barHeight - 4, "MOUSE:", mouseStr);
    // Segment 4: Camera
    DrawStatusSegment(dl, wmin.x + 376, wmin.y + 2, 120, barHeight - 4, "CAM:", camStr);

    // ---------------------------------------------------------
    // Taskbar (Win95 style - lets user minimize/restore windows)
    // ---------------------------------------------------------
    // Vertical separator
    dl->AddLine(ImVec2(wmin.x + 504, wmin.y + 2), ImVec2(wmin.x + 504, wmin.y + barHeight - 2), IM_COL32(128, 128, 128, 255), 1.0f);
    dl->AddLine(ImVec2(wmin.x + 505, wmin.y + 2), ImVec2(wmin.x + 505, wmin.y + barHeight - 2), IM_COL32(255, 255, 255, 255), 1.0f);

    float startX = wmin.x + 512;
    float btnWidth = 90;
    float btnHeight = barHeight - 6;
    float btnY = wmin.y + 3;
    float spacing = 4;

    DrawTaskbarButton("VIEWPORT", GetViewportOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("HIERARCHY", GetHierarchyOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("PROPERTIES", GetPropertiesOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("CONSOLE", GetConsoleOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("ASSETS", GetAssetBrowserOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("SCRIPTS", GetScriptPanelOpen(), startX, btnY, btnWidth, btnHeight);
    startX += btnWidth + spacing;

    DrawTaskbarButton("TILEMAP", GetTilemapEditorOpen(), startX, btnY, btnWidth, btnHeight);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------
// Viewport panel with game viewport texture
// ---------------------------------------------------------
inline void DrawViewportPanel(SDL_Texture* gameViewport) {
    bool& open = GetViewportOpen();
    if (!ImGui::Begin("VIEWPORT", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }

    // A small toolbar at the top of the viewport content to allow minimizing it directly
    ImGui::BeginGroup();
    if (ImGui::Button("MINIMIZE VIEWPORT", ImVec2(130, 20))) {
        open = false;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(" | Use WASD to fly camera | Drag middle mouse button to pan");
    ImGui::EndGroup();
    ImGui::Separator();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (gameViewport) {
        ImGui::Image((ImTextureID)gameViewport, viewportSize);
    } else {
        ImGui::Text("Viewport texture is NULL - check console");
    }

    // Overlay editor gizmos on top:
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 viewportPos = ImGui::GetItemRectMin();

    // Draw selected entity's collider box:
    Entity* selectedEntity = GetSelectedEntity();
    if (selectedEntity && selectedEntity->HasComponent<ColliderComponent>()) {
        auto* col = selectedEntity->GetComponent<ColliderComponent>();
        ImVec2 colMin = ImVec2(
            viewportPos.x + col->collider.x - Game::camera.x,
            viewportPos.y + col->collider.y - Game::camera.y
        );
        ImVec2 colMax = ImVec2(
            colMin.x + col->collider.w,
            colMin.y + col->collider.h
        );
        dl->AddRect(colMin, colMax, IM_COL32(0, 0, 255, 255), 0.0f, 0, 2.0f); // blue gizmo border
    }

    ImGui::End();
}

// ---------------------------------------------------------
// Draw All Panels inside a Main Dockspace
// ---------------------------------------------------------
inline void DrawEditorUI(SDL_Texture* gameViewport) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Status bar height offset
    float statusBarHeight = ImGui::GetFrameHeight() + 6;
    ImVec2 dockPos = viewport->WorkPos;
    ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - statusBarHeight);

    ImGui::SetNextWindowPos(dockPos);
    ImGui::SetNextWindowSize(dockSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr, dockFlags);
    ImGui::PopStyleVar();

    ImGuiID dockspaceId = ImGui::GetID("AurumDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("FILE")) {
            ImGui::Separator();
            if (ImGui::MenuItem("EXIT")) {
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("VIEW")) {
            ImGui::MenuItem("SCENE HIERARCHY", nullptr, &GetHierarchyOpen());
            ImGui::MenuItem("PROPERTIES", nullptr, &GetPropertiesOpen());
            ImGui::MenuItem("VIEWPORT", nullptr, &GetViewportOpen());
            ImGui::MenuItem("CONSOLE", nullptr, &GetConsoleOpen());
            ImGui::MenuItem("ASSET BROWSER", nullptr, &GetAssetBrowserOpen());
            ImGui::MenuItem("TOOLBAR", nullptr, &GetToolbarOpen());
            ImGui::MenuItem("TILEMAP EDITOR", nullptr, &GetTilemapEditorOpen());
            ImGui::MenuItem("LUA SCRIPTS", nullptr, &GetScriptPanelOpen());
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

    // Draw the individual editor panels
    if (GetHierarchyOpen())     DrawSceneHierarchy();
    if (GetPropertiesOpen())    DrawPropertiesPanel();
    if (GetViewportOpen())      DrawViewportPanel(gameViewport);
    if (GetConsoleOpen())       DrawConsolePanel();
    if (GetAssetBrowserOpen())  DrawAssetBrowser();
    if (GetToolbarOpen())       DrawToolbar();
    if (GetTilemapEditorOpen()) DrawTilemapEditor();
    if (GetScriptPanelOpen())   DrawScriptPanel();
    DrawRetroStatusBar();
}

#endif
