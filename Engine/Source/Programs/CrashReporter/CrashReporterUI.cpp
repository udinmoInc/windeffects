#include "CrashReporterUI.hpp"
#include "Widgets/Panel.hpp"
#include "Layout/Box.hpp"
#include "Layout/Splitter.hpp"
#include "Layout/ScrollLayout.hpp"
#include "Layout/Spacer.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/Button.hpp"
#include "Widgets/TextBox.hpp"
#include "Core/Logger.hpp"
#include "ConfigManager.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <windows.h>
#include <cstdlib>

using namespace we::UI;

namespace we::programs::crashreporter {

CrashReporterUI::CrashReporterUI() {
    m_CrashDir = "Saved/Logs/Crashes/Latest";
}

void CrashReporterUI::LoadCrashData() {
    auto read_file = [](const std::string& path) -> std::string {
        std::ifstream f(path);
        if (!f.is_open()) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    };

    auto read_json = [&](const std::string& name) -> nlohmann::json {
        std::string content = read_file(m_CrashDir + "/" + name);
        if (content.empty()) return nlohmann::json::object();
        try {
            return nlohmann::json::parse(content);
        } catch(...) {
            return nlohmann::json::object();
        }
    };

    m_CrashData = read_json("Crash.json");
    m_ExceptionData = read_json("Exception.json");
    m_SystemData = read_json("System.json");
    m_ModulesData = read_json("Modules.json");
    m_MemoryData = read_json("Memory.json");

    m_StackTrace = read_file(m_CrashDir + "/StackTrace.txt");
    m_EngineLog = read_file(m_CrashDir + "/Engine.log");
}

void CrashReporterUI::Construct() {
    ConfigManager::Get().Load();
    LoadCrashData();

    auto rootBox = std::make_shared<VerticalBox>();
    rootBox->SetPadding(Margin{16.0f, 16.0f, 16.0f, 16.0f});
    rootBox->SetSpacing(16.0f);
    
    const auto& cfg = ConfigManager::Get().GetConfig();

    // Top Header
    auto headerBox = std::make_shared<HorizontalBox>();
    headerBox->SetSpacing(16.0f);
    
    auto headerTitles = std::make_shared<VerticalBox>();
    headerTitles->AddChild(std::make_shared<Label>(cfg.engineName + " - Crash Reporter", Color::White(), 24.0f));
    headerTitles->AddChild(std::make_shared<Label>("Project: " + m_CrashData.value("Project", "Unknown") + " | Build: " + m_CrashData.value("BuildConfiguration", "Unknown"), Color{0.7f, 0.7f, 0.7f, 1.0f}, 14.0f));
    
    headerBox->AddChild(headerTitles);
    rootBox->AddChild(headerBox);

    // Main Splitter
    auto mainSplitter = std::make_shared<Splitter>(Orientation::Horizontal, 0.65f);
    
    // LEFT PANEL (65%)
    auto leftScroll = std::make_shared<ScrollLayout>();
    auto leftPanel = std::make_shared<VerticalBox>();
    leftPanel->SetSpacing(24.0f);
    leftPanel->SetPadding(Margin{0.0f, 0.0f, 16.0f, 0.0f});

    // Crash Summary Card
    auto summaryCard = std::make_shared<VerticalBox>();
    summaryCard->SetSpacing(8.0f);
    summaryCard->AddChild(std::make_shared<Label>("WindEffects Engine has crashed", Color{1.0f, 0.3f, 0.3f, 1.0f}, 20.0f));
    summaryCard->AddChild(std::make_shared<Label>("A crash report has been generated automatically. The information below will help diagnose the problem.", Color::White(), 14.0f));
    leftPanel->AddChild(summaryCard);

    // Exception Details
    auto exceptionCard = std::make_shared<VerticalBox>();
    exceptionCard->SetSpacing(4.0f);
    
    auto exceptionContent = std::make_shared<VerticalBox>();
    exceptionContent->SetSpacing(4.0f);
    exceptionContent->SetVisible(false); // Collapsed by default
    exceptionContent->AddChild(std::make_shared<Label>("Message: " + m_ExceptionData.value("Message", "Unknown"), Color{1.0f, 0.5f, 0.5f, 1.0f}, 14.0f));
    exceptionContent->AddChild(std::make_shared<Label>("Module: " + m_ExceptionData.value("Module", "Unknown") + " | Code: " + m_ExceptionData.value("Code", "N/A")));
    exceptionContent->AddChild(std::make_shared<Label>("Address: " + m_ExceptionData.value("Address", "N/A")));
    
    auto toggleExceptionBtn = std::make_shared<Button>("Exception Details (Toggle)", [exceptionContent]() {
        exceptionContent->SetVisible(!exceptionContent->IsVisible());
    });
    
    exceptionCard->AddChild(toggleExceptionBtn);
    exceptionCard->AddChild(exceptionContent);
    leftPanel->AddChild(exceptionCard);

    // Call Stack
    auto callStackCard = std::make_shared<VerticalBox>();
    callStackCard->SetSpacing(4.0f);
    callStackCard->AddChild(std::make_shared<Label>("Call Stack", Color{0.9f, 0.9f, 0.9f, 1.0f}, 18.0f));
    auto stackScroll = std::make_shared<ScrollLayout>();
    stackScroll->SetContent(std::make_shared<Label>(m_StackTrace, Color{0.7f, 0.7f, 0.7f, 1.0f}, 12.0f));
    callStackCard->AddChild(stackScroll);
    leftPanel->AddChild(callStackCard);

    // Logs
    auto logsCard = std::make_shared<VerticalBox>();
    logsCard->SetSpacing(4.0f);
    logsCard->AddChild(std::make_shared<Label>("Recent Engine Logs", Color{0.9f, 0.9f, 0.9f, 1.0f}, 18.0f));
    auto logScroll = std::make_shared<ScrollLayout>();
    logScroll->SetContent(std::make_shared<Label>(m_EngineLog, Color{0.6f, 0.8f, 0.6f, 1.0f}, 12.0f));
    logsCard->AddChild(logScroll);
    leftPanel->AddChild(logsCard);

    leftScroll->SetContent(leftPanel);
    mainSplitter->SetFirstChild(leftScroll);

    // RIGHT PANEL (35%)
    auto rightScroll = std::make_shared<ScrollLayout>();
    auto rightPanel = std::make_shared<VerticalBox>();
    rightPanel->SetSpacing(24.0f);
    rightPanel->SetPadding(Margin{16.0f, 0.0f, 0.0f, 0.0f});

    // Quick Actions
    auto actionsCard = std::make_shared<VerticalBox>();
    actionsCard->SetSpacing(8.0f);
    actionsCard->AddChild(std::make_shared<Label>("Quick Actions", Color::White(), 16.0f));
    
    if (cfg.allowRestart) {
        actionsCard->AddChild(std::make_shared<Button>("Restart Editor", [this]{ OnRestartEditor(); }));
    }
    actionsCard->AddChild(std::make_shared<Button>("Open Crash Folder", [this]{ OnOpenCrashFolder(); }));
    if (cfg.allowZipExport) {
        actionsCard->AddChild(std::make_shared<Button>("Export ZIP", [this]{ OnExportZip(); }));
    }
    rightPanel->AddChild(actionsCard);

    // User Comments
    auto commentsCard = std::make_shared<VerticalBox>();
    commentsCard->SetSpacing(8.0f);
    commentsCard->AddChild(std::make_shared<Label>("User Comments", Color::White(), 16.0f));
    // Simulated textbox placeholder for now
    commentsCard->AddChild(std::make_shared<Label>("What were you doing before the crash?", Color{0.5f, 0.5f, 0.5f, 1.0f}, 12.0f));
    rightPanel->AddChild(commentsCard);

    // System Info
    auto sysCard = std::make_shared<VerticalBox>();
    sysCard->SetSpacing(4.0f);
    sysCard->AddChild(std::make_shared<Label>("System Information", Color::White(), 16.0f));
    sysCard->AddChild(std::make_shared<Label>("CPU: " + m_SystemData.value("CPU", "Unknown")));
    sysCard->AddChild(std::make_shared<Label>("GPU: " + m_SystemData.value("GPU", "Unknown")));
    sysCard->AddChild(std::make_shared<Label>("RAM: " + m_SystemData.value("RAM", "Unknown")));
    sysCard->AddChild(std::make_shared<Label>("OS: " + m_SystemData.value("OS", "Unknown")));
    rightPanel->AddChild(sysCard);

    // Support
    auto supportCard = std::make_shared<VerticalBox>();
    supportCard->SetSpacing(4.0f);
    supportCard->AddChild(std::make_shared<Label>("Support", Color::White(), 16.0f));
    supportCard->AddChild(std::make_shared<Label>("Email: " + cfg.supportEmail));
    supportCard->AddChild(std::make_shared<Label>("Website: " + cfg.supportWebsite));
    rightPanel->AddChild(supportCard);

    rightScroll->SetContent(rightPanel);
    mainSplitter->SetSecondChild(rightScroll);

    // Fill remaining space with splitter
    rootBox->AddChild(mainSplitter);

    // Status bar (bottom)
    auto statusBar = std::make_shared<Panel>("");
    statusBar->SetHeaderHeight(30.0f);
    rootBox->AddChild(statusBar);

    m_RootLayout = rootBox;
}

we::UI::Size CrashReporterUI::Measure(const Size& availableSize) {
    if (m_RootLayout) m_RootLayout->Measure(availableSize);
    return availableSize;
}

void CrashReporterUI::Arrange(const Rect& allottedRect) {
    if (m_RootLayout) {
        m_RootLayout->Arrange(allottedRect);
    }
}

void CrashReporterUI::Paint(PaintContext& context) {
    if (m_RootLayout) {
        m_RootLayout->Paint(context);
    }
}

void CrashReporterUI::OnExportZip() {
    std::string cmd = "powershell.exe -c \"Compress-Archive -Path '" + m_CrashDir + "/All*' -DestinationPath 'CrashReport.zip' -Force\"";
    system(cmd.c_str());
}

void CrashReporterUI::OnRestartEditor() {
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::string editorPath = "WindEffectsEditor.exe";
    CreateProcessA(nullptr, (LPSTR)editorPath.c_str(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, nullptr, &si, &pi);
    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    exit(0);
}

void CrashReporterUI::OnOpenCrashFolder() {
    std::string cmd = "explorer.exe \"" + m_CrashDir + "\"";
    system(cmd.c_str());
}

} // namespace we::programs::crashreporter
