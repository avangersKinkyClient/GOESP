#include "GUI.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

#include "Config.h"
#include "Hooks.h"
#include "ImGuiCustom.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <Windows.h>

GUI::GUI() noexcept
{
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(FindWindowW(L"Valve001", nullptr));

    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}

void GUI::render() noexcept
{
    const auto time = std::time(nullptr);
    const auto localTime = std::localtime(&time);

    const auto windowTitle = std::ostringstream{ } << "GOESP [" << std::setw(2) << std::setfill('0') << localTime->tm_hour << ':' << std::setw(2) << std::setfill('0') << localTime->tm_min << ':' << std::setw(2) << std::setfill('0') << localTime->tm_sec << "]###window";

    ImGui::SetNextWindowCollapsed(true, ImGui::GetIO().KeysDown[VK_SUBTRACT] ? ImGuiCond_Always : ImGuiCond_FirstUseEver);

    blockInput = ImGui::Begin(windowTitle.str().c_str());

    static int currentCategory = 0;
    static int currentItem = 0;

    if (ImGui::ListBoxHeader("##list", { 125.0f, 300.0f })) {
        static constexpr std::array categories{ "Allies", "Enemies" };

        for (size_t i = 0; i < categories.size(); i++) {
            if (ImGui::Selectable(categories[i], currentCategory == i && currentItem == 0)) {
                currentCategory = i;
                currentItem = 0;
            }

            constexpr auto getConfig = [](int category, int item) constexpr noexcept -> const Config::Shared& {
                switch (category) {
                case 0:
                    return config.players[item];
                case 1:
                    return config.players[item + 3];
                default:
                    return config.players[0];
                }
            };

            if (getConfig(i, 0).enabled)
                continue;

            constexpr auto getItems = [](int index) noexcept -> std::vector<const char*> {
                switch (index) {
                case 0:
                case 1:
                    return { "Visible", "Occluded" };
                case 2:
                    // return { "AK-47", "Tec-9" };
                case 3:
                    // return { "Smoke", "HE Grenade", "Flashbang" };
                default:
                    return { };
                }
            };

            ImGui::PushID(i);
            ImGui::Indent();

            auto items = getItems(i);
            for (size_t j = 0; j < items.size(); j++) {
                if (ImGui::Selectable(items[j], currentCategory == i && currentItem == j + 1)) {
                    currentCategory = i;
                    currentItem = j + 1;
                }
            }

            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::ListBoxFooter();
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        switch (currentCategory) {
        case 0:
        case 1: {
            auto& playerConfig = config.players[currentCategory * 3 + currentItem];

            ImGui::Checkbox("Enabled", &playerConfig.enabled);
            ImGui::Separator();

            constexpr auto spacing{ 200.0f };
            ImGuiCustom::colorPicker("Snaplines", playerConfig.snaplines);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Box", playerConfig.box);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("", &playerConfig.boxType, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
        }
        }
        ImGui::EndChild();
    }

    ImGui::Separator();

    ImGui::TextUnformatted("Build date: " __DATE__ " " __TIME__);
    ImGui::SameLine(0.0f, 30.0f);

    if (ImGui::Button("Unload"))
        hooks.restore();

    ImGui::End();
}
