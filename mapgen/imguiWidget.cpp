#include "imguiWidget.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace mapgen
{

ImguiWidget::ImguiWidget():
    m_show(true),
    m_x(0),
    m_y(0),
    m_width(0),
    m_height(0)
{}

void ImguiWidget::initialize()
{
    onInitialize();
}

void ImguiWidget::onInitialize()
{

}

void ImguiWidget::setPosition(int x, int y)
{
    m_x=x;
    m_y=y; 

    onPosition();
}

void ImguiWidget::onPosition()
{
}

void ImguiWidget::setSize(int width, int height)
{ 
    m_width=width; 
    m_height=height; 

    onSize();
}

void ImguiWidget::onSize()
{
}

void ImguiWidget::draw()
{
    ImGui::SetNextWindowPos({(float)m_x, (float)m_y});
    ImGui::SetNextWindowSize({(float)m_width, (float)m_height});

    ImGui::Begin(m_name.c_str(), &m_show, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);

    onDraw();

    ImGui::End();
}

void ImguiWidget::onDraw()
{

}

}//namespace mapgen
