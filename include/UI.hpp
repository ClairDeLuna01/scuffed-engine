#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utils.hpp"
#include "transform3D.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct UIWindow
{
    std::string name;
    bool open;
    std::vector<std::function<void()>> render_callbacks;
    void render_wrapper()
    {
        if (open)
        {
            ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize);
            for (auto &callback : render_callbacks)
            {
                callback();
            }
            ImGui::End();
        }
    }

    enum WatcherMode
    {
        INPUT = 0,
        SLIDER = 1,
        DRAG = 2
    };
    void add_watcher(const std::string &name, i32 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, f32 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, f64 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, bool *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, std::string *value);
    void add_watcher(const std::string &name, vec2 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, vec3 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, vec4 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, ivec2 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, ivec3 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, ivec4 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, quat *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, mat2 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, mat3 *value, WatcherMode mode = WatcherMode::INPUT);
    void add_watcher(const std::string &name, mat4 *value, WatcherMode mode = WatcherMode::INPUT);
    enum Transform3DWatchFlags
    {
        POSITION = 0b0001,
        ROTATION = 0b0010,
        SCALE = 0b0100,
        ALL = 0b0111
    };
    void add_watcher(const std::string &name, Transform3D *value, Transform3DWatchFlags flags = Transform3DWatchFlags::ALL, WatcherMode mode = WatcherMode::INPUT);
};

typedef std::shared_ptr<UIWindow> UIWindowPtr;

class UI
{
private:
    std::vector<UIWindowPtr> windows;

public:
    UI();
    ~UI();
    UIWindowPtr add_window(const std::string &name, std::vector<std::function<void()>> render_callbacks);
    void remove_window(const std::string &name);
    void remove_window(UIWindowPtr window);
    void render();
};