#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "transform3D.hpp"
#include "utils.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

using namespace glm;

class UIWindow
{
  private:
    std::string name;
    bool open = true;
    std::vector<std::function<void()>> render_callbacks;

  public:
    UIWindow(const std::string &name, std::vector<std::function<void()>> render_callbacks)
        : name(name), render_callbacks(render_callbacks)
    {
    }
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

    std::string getName()
    {
        return name;
    }

    bool getIsOpen()
    {
        return open;
    }

    enum WatcherMode
    {
        INPUT = 0,
        SLIDER = 1,
        DRAG = 2
    };
    void add_watcher(const std::string &name, i32 *value, WatcherMode mode = WatcherMode::INPUT, i32 min = 0,
                     i32 max = 100, i32 step = 1);
    void add_watcher(const std::string &name, f32 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, bool *value);
    void add_watcher(const std::string &name, std::string *value);
    void add_watcher(const std::string &name, vec2 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, vec3 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, vec4 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, ivec2 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, ivec3 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, ivec4 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, quat *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, mat2 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, mat3 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    void add_watcher(const std::string &name, mat4 *value, WatcherMode mode = WatcherMode::INPUT, f32 min = 0,
                     f32 max = 100, f32 step = 0.1f);
    enum Transform3DWatchFlags
    {
        POSITION = 0b0001,
        ROTATION = 0b0010,
        SCALE = 0b0100,
        ALL = 0b0111
    };
    void add_watcher(const std::string &name, Transform3D *value,
                     Transform3DWatchFlags flags = Transform3DWatchFlags::ALL, WatcherMode mode = WatcherMode::INPUT,
                     f32 min = 0, f32 max = 100, f32 step = 0.1f);
};

typedef std::shared_ptr<UIWindow> UIWindowPtr;

class UI
{
  private:
    std::vector<UIWindowPtr> windows;
    bool isAnyWindowHovered = false;

  public:
    UI();
    ~UI();

    void shutdown();

    UIWindowPtr add_window(const std::string &name, std::vector<std::function<void()>> render_callbacks);
    void remove_window(const std::string &name);
    void remove_window(UIWindowPtr window);
    void render();

    bool getAnyWindowHovered()
    {
        return isAnyWindowHovered;
    }
};

UI &getUI();