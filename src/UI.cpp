#include "UI.hpp"
#include "globals.hpp"

void ImGuiInit(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

UI::UI()
{
    ImGuiInit(EngineGlobals::window);
}
UI::~UI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

UIWindowPtr UI::add_window(const std::string &name, std::vector<std::function<void()>> render_callbacks)
{
    auto window = std::make_shared<UIWindow>();
    window->name = name;
    window->open = true;
    window->render_callbacks = render_callbacks;
    windows.push_back(window);
    return window;
}

void UI::remove_window(const std::string &name)
{
    for (auto it = windows.begin(); it != windows.end();)
    {
        if ((*it)->name == name)
        {
            it = windows.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void UI::remove_window(UIWindowPtr window)
{
    for (auto it = windows.begin(); it != windows.end();)
    {
        if (*it == window)
        {
            it = windows.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void UI::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto &window : windows)
    {
        window->render_wrapper();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIWindow::add_watcher(const std::string &name, i32 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputInt(name.c_str(), value); });
}

void UIWindow::add_watcher(const std::string &name, f32 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputFloat(name.c_str(), value); });
}

void UIWindow::add_watcher(const std::string &name, f64 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputDouble(name.c_str(), value); });
}

void UIWindow::add_watcher(const std::string &name, bool *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::Checkbox(name.c_str(), value); });
}

void UIWindow::add_watcher(const std::string &name, std::string *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputText(name.c_str(), value->data(), value->size()); });
}

void UIWindow::add_watcher(const std::string &name, vec2 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputFloat2(name.c_str(), (float *)&value); });
}

void UIWindow::add_watcher(const std::string &name, vec3 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputFloat3(name.c_str(), (float *)&value); });
}

void UIWindow::add_watcher(const std::string &name, vec4 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputFloat4(name.c_str(), (float *)&value); });
}

void UIWindow::add_watcher(const std::string &name, ivec2 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputInt2(name.c_str(), (int *)&value); });
}

void UIWindow::add_watcher(const std::string &name, ivec3 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputInt3(name.c_str(), (int *)&value); });
}

void UIWindow::add_watcher(const std::string &name, ivec4 *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputInt4(name.c_str(), (int *)&value); });
}

void UIWindow::add_watcher(const std::string &name, quat *value)
{
    render_callbacks.push_back([=]()
                               { ImGui::InputFloat4(name.c_str(), (float *)&value); });
}

void UIWindow::add_watcher(const std::string &name, mat2 *value)
{
    render_callbacks.push_back([=]()
                               {
                                   ImGui::InputFloat2(name.c_str(), (float *)&value);
                                   ImGui::InputFloat2(name.c_str(), (float *)&value + 2); });
}

void UIWindow::add_watcher(const std::string &name, mat3 *value)
{
    render_callbacks.push_back([=]()
                               {
                                   ImGui::InputFloat3(name.c_str(), (float *)&value);
                                   ImGui::InputFloat3(name.c_str(), (float *)&value + 3);
                                   ImGui::InputFloat3(name.c_str(), (float *)&value + 6); });
}

void UIWindow::add_watcher(const std::string &name, mat4 *value)
{
    render_callbacks.push_back([=]()
                               {
                                   ImGui::InputFloat4(name.c_str(), (float *)&value);
                                   ImGui::InputFloat4(name.c_str(), (float *)&value + 4);
                                   ImGui::InputFloat4(name.c_str(), (float *)&value + 8);
                                   ImGui::InputFloat4(name.c_str(), (float *)&value + 12); });
}

void UIWindow::add_watcher(const std::string &name, Transform3D *value, Transform3DWatchFlags flags)
{
    render_callbacks.push_back([=]()
                               {
                                   if (flags & Transform3DWatchFlags::POSITION)
                                   {
                                    vec3 position = value->getPosition();
                                       bool i = ImGui::InputFloat3((name + " Position").c_str(), (float *)&position, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
                                       if (i)
                                       {
                                           value->setPosition(position);
                                       }

                                   }
                                   if (flags & Transform3DWatchFlags::ROTATION)
                                   {
                                    vec3 rotation = value->getEulerRotation();
                                       bool i = ImGui::InputFloat3((name + " Rotation").c_str(), (float *)&rotation, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
                                       if (i)
                                       {
                                           value->setRotation(rotation);
                                       }
                                   }
                                   if (flags & Transform3DWatchFlags::SCALE)
                                   {
                                    vec3 scale = value->getScale();
                                       bool i = ImGui::InputFloat3((name + " Scale").c_str(), (float *)&scale, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
                                       if (i)
                                       {
                                           value->setScale(scale);
                                       }
                                   } });
}