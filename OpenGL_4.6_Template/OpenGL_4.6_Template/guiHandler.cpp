#include "guiHandler.h"

extern bool lineMode;
extern bool mouseLocked;

void initializeGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void renderGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    float currentFPS = ImGui::GetIO().Framerate;

    ImGui::SetNextWindowSize(ImVec2(400, 200));

    ImGui::Begin("Performance");

    ImGui::Text("Modalita' camera: %s", mouseLocked ? "ATTIVA" : "DISATTIVA");
    ImGui::Text("Premi tasto destro mouse per %s il mouse.", mouseLocked ? "sbloccare" : "bloccare");
    ImGui::Text("Premi tasto sinistro mouse per rallentare.");
    ImGui::Text("Premi \"L\" per entrare in modalita' LINE.");
    ImGui::Text("Premi \"F\" per entrare in modalita' FILL.");

    ImGui::Text("FPS: %.1f", currentFPS);

    ImGui::Text("Current mode: %s", lineMode ? "LINE" : "FILL");

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void destroyGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}