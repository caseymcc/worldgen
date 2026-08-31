#include "imageWidget.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <imglib/simpleImage.h>
#include <imglib/draw.h>

#include <algorithm>
#include <cmath>

namespace mapgen
{

namespace
{

float clampf(float value, float low, float high)
{
    return (value<low)?low:((value>high)?high:value);
}

}//unnamed namespace

ImageWidget::ImageWidget():
    m_textureValid(false),
    m_textureWidth(0),
    m_textureHeight(0),
    m_center(0.5f, 0.5f),
    m_zoom(1.0f),
    m_wrapX(false),
    m_imagePos(0.0f, 0.0f),
    m_imageSize(0.0f, 0.0f)
{

}

void ImageWidget::onInitialize()
{
    glGenTextures(1, &m_textureId);
}

void ImageWidget::updateImage(imglib::SimpleImage *image)
{
    m_textureWidth=image->width;
    m_textureHeight=image->height;

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //An equirectangular map joins up in x, so panning past one edge should arrive at the other
    //rather than running into a wall. Repeating the texture is what lets the view straddle the seam.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapX?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_textureValid=true;
}

void ImageWidget::setView(const ImVec2 &center, float zoom)
{
    m_zoom=std::max(zoom, 1.0f);
    m_center.x=center.x;

    //Half a view fits either side of the centre, so past the poles there is nothing to show. In x
    //there always is, as long as the image wraps.
    float half=0.5f/m_zoom;

    m_center.y=clampf(center.y, half, 1.0f-half);

    if(!m_wrapX)
        m_center.x=clampf(center.x, half, 1.0f-half);
    else
        m_center.x=center.x-floorf(center.x);   //keep it in 0 to 1 so it cannot drift away
}

void ImageWidget::resetView()
{
    m_center=ImVec2(0.5f, 0.5f);
    m_zoom=1.0f;
}

void ImageWidget::viewBounds(ImVec2 &uvMin, ImVec2 &uvMax) const
{
    float half=0.5f/m_zoom;

    uvMin=ImVec2(m_center.x-half, m_center.y-half);
    uvMax=ImVec2(m_center.x+half, m_center.y+half);
}

bool ImageWidget::isHovered(const ImVec2 &screenPos) const
{
    ImVec2 unit;

    return screenToUnit(screenPos, unit);
}

bool ImageWidget::screenToUnit(const ImVec2 &screenPos, ImVec2 &unit) const
{
    if((m_imageSize.x<=0.0f)||(m_imageSize.y<=0.0f))
        return false;

    unit.x=(screenPos.x-m_imagePos.x)/m_imageSize.x;
    unit.y=(screenPos.y-m_imagePos.y)/m_imageSize.y;

    return (unit.x>=0.0f)&&(unit.x<=1.0f)&&(unit.y>=0.0f)&&(unit.y<=1.0f);
}

bool ImageWidget::screenToTexel(const ImVec2 &screenPos, ImVec2 &texel) const
{
    ImVec2 unit;
    bool inside=screenToUnit(screenPos, unit);
    ImVec2 uvMin;
    ImVec2 uvMax;

    viewBounds(uvMin, uvMax);

    float u=uvMin.x+(unit.x*(uvMax.x-uvMin.x));
    float v=uvMin.y+(unit.y*(uvMax.y-uvMin.y));

    texel.x=u*(float)m_textureWidth;
    texel.y=v*(float)m_textureHeight;

    if(m_wrapX&&(m_textureWidth>0))
    {
        texel.x=fmodf(texel.x, (float)m_textureWidth);

        if(texel.x<0.0f)
            texel.x+=(float)m_textureWidth;
    }
    else
        texel.x=clampf(texel.x, 0.0f, (float)std::max(m_textureWidth-1, 0));

    texel.y=clampf(texel.y, 0.0f, (float)std::max(m_textureHeight-1, 0));

    return inside;
}

//Kept for callers that still work in widget space. Now routed through the same letterbox and view
//maths as everything else rather than assuming the image fills the widget.
ImVec2 ImageWidget::getImagePosition(ImVec2 &widgetPos)
{
    ImVec2 texel;

    screenToTexel(ImVec2(widgetPos.x+(float)m_x, widgetPos.y+(float)m_y), texel);

    return texel;
}

void ImageWidget::draw()
{
    onDraw();
}

void ImageWidget::onDraw()
{
    if(!m_textureValid)
        return;

    lastDrawPos=ImGui::GetCursorPos();
    int imageWidth=m_width-(2*lastDrawPos.y)-2.0f;//margin, -2.0f border
    int imageHeight=m_height-(2*lastDrawPos.y)-2.0f;//margin, -2.0f border

    float ratioX=(float)imageWidth/m_textureWidth;
    float ratioY=(float)imageHeight/m_textureHeight;

    ImVec2 textureStart=lastDrawPos;
    ImVec2 textureSize(imageWidth, imageHeight);

    if(ratioX<ratioY)
    {
        textureSize.y=ratioX*m_textureHeight;
        textureStart.y=lastDrawPos.y+(imageHeight-textureSize.y)/2.0f;
    }
    else
    {
        textureSize.x=ratioY*m_textureWidth;
        textureStart.x=lastDrawPos.x+(imageWidth-textureSize.x)/2.0f;
    }
    lastDrawPos=textureStart;
    ImGui::SetCursorPos(textureStart);

    lastDrawSize.x=textureSize.x;
    lastDrawSize.y=textureSize.y;

    //The screen rectangle, taken here rather than derived from the widget position, so picking a
    //pixel under the cursor does not depend on working out where imgui actually put things.
    m_imagePos=ImGui::GetCursorScreenPos();
    m_imageSize=textureSize;

    ImVec2 uvMin;
    ImVec2 uvMax;

    viewBounds(uvMin, uvMax);

    ImGui::Image((ImTextureID)m_textureId, {(float)textureSize.x, (float)textureSize.y},
        uvMin, uvMax, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.5f});
}


}//namespace mapgen
