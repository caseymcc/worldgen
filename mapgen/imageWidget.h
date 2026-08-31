#ifndef _mapgen_imageWidget_h_
#define _mapgen_imageWidget_h_

#include "imguiWidget.h"

#include <imglib/simpleImage.h>

#include <glbinding/gl/gl.h>
using namespace gl;

#include <imgui.h>

namespace mapgen
{

class ImageWidget:public ImguiWidget
{
public:
    ImageWidget();

    void onInitialize() override;

    void draw() override;
    void onDraw() override;

    //takes only rgba image,
    void updateImage(imglib::SimpleImage *image);

    //--- view ---
    //The visible part of the image, as a centre in 0 to 1 texture coordinates and a magnification.
    //A zoom of 1 fits the whole image; 4 shows a quarter of it across.
    void setView(const ImVec2 &center, float zoom);
    void resetView();
    void setWrapX(bool wrap) { m_wrapX=wrap; }

    float getZoom() const { return m_zoom; }
    float getImageWidth() const { return m_imageSize.x; }
    float getImageHeight() const { return m_imageSize.y; }
    const ImVec2 &getCenter() const { return m_center; }

    //Where a point on the screen falls on the image. Both account for the letterboxing that keeps
    //the image square inside a wider widget, and for the current view, so a pixel picked under the
    //cursor is the pixel the user is pointing at.
    bool screenToUnit(const ImVec2 &screenPos, ImVec2 &unit) const;
    bool screenToTexel(const ImVec2 &screenPos, ImVec2 &texel) const;

    //true while the pointer is over the drawn image rather than the margin around it
    bool isHovered(const ImVec2 &screenPos) const;

    ImVec2 getImagePosition(ImVec2 &widgetPos);

private:
    void viewBounds(ImVec2 &uvMin, ImVec2 &uvMax) const;

    bool m_textureValid;
    GLuint m_textureId;
    int m_textureWidth;
    int m_textureHeight;

    ImVec2 m_center;
    float m_zoom;
    bool m_wrapX;

    //screen rectangle the image was last drawn into
    ImVec2 m_imagePos;
    ImVec2 m_imageSize;

    ImVec2 lastDrawPos;
    ImVec2 lastDrawSize;
};

}//namespace mapgen

#endif//_mapgen_imageWidget_h_
