#include <imgui.h> 
#include "engine/imgui.hpp" 
#include "engine/ui-imgui.hpp" 

int UI::Start(void *e, void *r) {
    this->e = (EngineApi *) e; 
    this->r = (RendererApi *) r; 
    return 0; 
}

void UI::NewUpdate(float dt) {
    imgui_events(dt);
    ImGui::NewFrame();
}

void UI::Update(float dt) {
    ImGui::ShowDemoWindow();
    this->UpdateObjectsWindow(dt); 
    this->UpdatePropertiesWindow(dt); 
}

void UI::UpdateObjectsWindow(float dt) {
    ImGui::Begin("Objects window", NULL, ImGuiWindowFlags_MenuBar);
    
    // general properties
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug")) { 
            ImGui::MenuItem("Toggle debug wireframe", NULL, &r->debug_wireframe);
            ImGui::MenuItem("Toggle debug stats", NULL, &r->debug_stats);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Text("Object tree structure:");

    if (ImGui::Button("Add object")) {
        EngineObject obj; 
        obj.name = "Empty object";
        obj.model = new ModelComponent(); 
        e->objs.push_back(obj); 
    }

    // object tree structure 
    if (ImGui::BeginTable("3ways", 2, ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoHide);
        ImGui::TableHeadersRow(); 

        size_t n = e->objs.size(); 
        for (size_t i = 0; i < n; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            EngineObject *obj = &e->objs[i];
            ImGui::PushID(obj);

            size_t flags = ImGuiTreeNodeFlags_SpanAllColumns | 
                ImGuiTreeNodeFlags_Leaf | 
                ImGuiTreeNodeFlags_NoTreePushOnOpen;
            flags |= ImGuiTreeNodeFlags_Selected ? e->obj_selected == obj : 0; 

            if (ImGui::TreeNodeEx(obj->name.c_str(), flags)) {
                e->obj_selected  = ImGui::IsItemClicked() ? obj : e->obj_selected;  
            }
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::Text("Engine::EngineObject");
        } 

        ImGui::EndTable();
    }

    ImGui::End(); 
}

void UI::UpdatePropertiesWindow(float dt) {
    ImGui::Begin("Object properties window", NULL, ImGuiWindowFlags_MenuBar); 

    if (e->obj_selected == NULL) {
        ImGui::Text("Object properties...");
        ImGui::End(); 
        return; 
    }
    char name_buf[256] = { 0 }; 
    EngineObject *obj = e->obj_selected; 

    memset(name_buf, 0x00, sizeof(name_buf));
    memcpy(name_buf, obj->name.c_str(), obj->name.length()); // assuming (length < sizeof(buff))
    
    if (ImGui::InputText("Object Name", name_buf, sizeof(name_buf))) {
        obj->name = std::string(name_buf);
    }

    if (ImGui::Button("Select model..")) {
        ImGui::OpenPopup("File picker dialog"); 
    }

    if (ImGui::BeginPopupModal("File picker dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("hello there this is abstracted away");
        ImGui::EndPopup(); 
    }

    ImGui::SeparatorText("Position");
    { 
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position x", &obj->position.x, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position y", &obj->position.y, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("position z", &obj->position.z, 0.01f, -30.0f, 30.0f);
    }

    ImGui::SeparatorText("Rotation");
    { 
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Rotation x", &obj->rotation.x, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Rotation y", &obj->rotation.y, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Rotation z", &obj->rotation.z, 0.01f, -30.0f, 30.0f);
    }

    ImGui::SeparatorText("Scale");
    { 
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Scale x", &obj->scale.x, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Scale y", &obj->scale.y, 0.01f, -30.0f, 30.0f);

        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("Scale z", &obj->scale.z, 0.01f, -30.0f, 30.0f);
    }

    ImGui::End(); 
}

void UI::Reset(int width, int height) {
    imgui_reset(width, height);
}
