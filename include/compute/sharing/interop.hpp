#pragma once

#define CLG_USE_DIRECTX 1
#define CLG_USE_OPENGL 1
#define CLG_USE_VULKAN 0

#define CLG_USER_OPENGL 0
#define CLG_USER_VULKAN 0

// cl
#if defined(__APPLE__)
#else // if windows
#include <CL/cl_d3d11.h>
#endif

// d3d11
#if CLG_USE_DIRECTX
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#endif

// gl2
#if CLG_USE_OPENGL && !CLG_USER_OPENGL
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenCL/cl_gl.h>
#else
#include <GL/gl.h>
#include <CL/cl_gl.h>
#endif
#endif

// vk
#if CLG_USE_VULKAN && !CLG_USER_VULKAN
#endif

#include <iostream>
#include <string>
#include <vector>

namespace compute {

struct device {
};

const std::vector<cl_device_id>& get_devices();

/// @brief
struct context {

    /// @brief
    /// @param device
    context(const cl_device_id device, const std::vector<cl_context_properties>& properties);

#if CLG_USE_DIRECTX
    /// @brief
    /// @param device
    static context create_d3d11(ID3D11Device* device, ID3D11DeviceContext* device_context);
#endif

#if CLG_USE_OPENGL
    /// @brief
    static context create_gl2();
#endif

private:
#if CLG_USE_DIRECTX
    ID3D11Device* _d3d11_device;
    ID3D11DeviceContext* _d3d11_device_context;
#endif
    cl_device_id _device;
    cl_context _context;
    cl_command_queue _command_queue;
    friend struct kernel;
    friend struct texture_2d;
};

/// @brief
struct texture_2d {

    /// @brief
    /// @param ctx
    /// @param width
    /// @param height
    texture_2d(const context& ctx, const std::size_t width, const std::size_t height, const std::vector<unsigned char>& pixels);

#if CLG_USE_DIRECTX
    /// @brief
    /// @param ctx
    /// @param texture
    texture_2d(const context& ctx, ID3D11Texture2D* texture);

    /// @brief 
    /// @return 
    [[nodiscard]] ID3D11Texture2D* get_d3d11() const;
#endif


    // /// @brief
    // /// @param ctx
    // /// @param texture
    // texture_2d(const context& ctx, const gluint texture);

    /// @brief
    void acquire();

    /// @brief
    void release();


private:
#if CLG_USE_DIRECTX
    ID3D11Texture2D* _d3d11_texture;
#endif
    cl_mem _mem;
    cl_command_queue _command_queue;
    friend struct kernel;
};

/// @brief
struct kernel {

    /// @brief
    /// @param ctx
    /// @param code
    kernel(const context& ctx, const std::string& code, const std::string& name);

    /// @brief
    /// @param index
    /// @param texture
    void set_arg(const std::size_t index, texture_2d& texture);

    /// @brief
    /// @param index
    /// @param size
    /// @param value
    void set_arg(const std::size_t index, const std::size_t size, void* value);

    /// @brief
    void run(const std::vector<std::size_t>& global_work_size);

private:
#if CLG_USE_DIRECTX
    ID3D11DeviceContext* _d3d11_device_context;
#endif
    cl_device_id _device;
    cl_context _context;
    cl_command_queue _command_queue;
    cl_program _program;
    cl_kernel _kernel;
};

}