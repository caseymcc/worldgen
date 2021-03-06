if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
hunter_config(glm VERSION ${HUNTER_glm_VERSION}
    CMAKE_ARGS 
        GLM_FORCE_CTOR_INIT=ON
)
endif()

hunter_config(imgui VERSION ${HUNTER_imgui_VERSION}
    CMAKE_ARGS 
        IMGUI_BUILD_EXAMPLES=OFF
        IMGUI_ENABLE_TEST_ENGINE=OFF
        IMGUI_IMPL_OPENGL3=ON
        IMGUI_IMPL_GLFW=ON
        IMGUI_IMPL_OPENGL_LOADER=GLEW
)
