#ifndef _mapgen_imguiWidget_h_
#define _mapgen_imguiWidget_h_

#include <string_view>
#include <string>

#include <imgui.h>

namespace mapgen
{

class ImguiWidget
{
public:
    ImguiWidget();
    virtual ~ImguiWidget() {}

    void initialize();
    virtual void onInitialize();

    void setName(std::string_view name) { m_name=name; }
    void setShow(bool show) { m_show=show; }

    void setPosition(int x, int y);
    virtual void onPosition();

    void setSize(int width, int height);
    virtual void onSize();

    virtual void draw();
    virtual void onDraw();

protected:
    bool m_show;
    std::string m_name;

    int m_x, m_y;
    int m_width, m_height;
};

}//namespace mapgen

#endif//_mapgen_imguiWidget_h_
