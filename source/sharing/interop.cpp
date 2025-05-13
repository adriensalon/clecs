#include <clcompute/clcompute.hpp>

namespace compute {

clCreateFromD3D11Texture2DKHR_fn clCreateFromD3D11Texture2D = nullptr;
clEnqueueAcquireD3D11ObjectsKHR_fn clEnqueueAcquireD3D11Objects = nullptr;
clEnqueueReleaseD3D11ObjectsKHR_fn clEnqueueReleaseD3D11Objects = nullptr;

namespace {

    bool _has_extension(const char* extensions, const std::string& target)
    {
        std::string _extensions_list(extensions ? extensions : "");
        return _extensions_list.find(target) != std::string::npos;
    }

    void queryD3D11DeviceInfo(ID3D11Device* d3d11Device, UINT& vendorId, UINT& deviceId)
    {
        IDXGIDevice* dxgiDevice = nullptr;
        d3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

        IDXGIAdapter* dxgiAdapter = nullptr;
        dxgiDevice->GetAdapter(&dxgiAdapter);

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        vendorId = adapterDesc.VendorId;
        deviceId = adapterDesc.DeviceId;

        dxgiAdapter->Release();
        dxgiDevice->Release();
    }

    // Callback function to print the build log
    void CL_CALLBACK buildNotifyCallback(cl_program program, void* user_data)
    {
        cl_int err;

        // Get the device information (using the device ID from user_data)
        cl_device_id device = *static_cast<cl_device_id*>(user_data);

        // Get the build log length
        size_t logSize;
        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        if (err != CL_SUCCESS) {
            std::cerr << "Error getting build log size: " << err << std::endl;
            return;
        }

        // Allocate memory for the log
        char* buildLog = new char[logSize];

        // Retrieve the build log
        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Error getting build log: " << err << std::endl;
            delete[] buildLog;
            return;
        }

        // Print the build log
        std::cerr << "Build Log:\n"
                  << buildLog << std::endl;

        // Clean up
        delete[] buildLog;
    }

}

context::context(const cl_device_id device, const std::vector<cl_context_properties>& properties)
{
    cl_int _status;
    _device = device;
    _context = clCreateContext(properties.data(), 1, &device, nullptr, nullptr, &_status);
    if (_status != 0) {
        throw std::runtime_error("Successfully created OpenCL context with DirectX 11 interop.");
    }
    _command_queue = clCreateCommandQueue(_context, _device, 0, &_status);
    if (_status != 0) {
        throw std::runtime_error("Successfully created OpenCL context with DirectX 11 interop.");
    }
}

#if CLG_USE_DIRECTX
context context::create_d3d11(ID3D11Device* device, ID3D11DeviceContext* device_context)
{

    unsigned int _vendor_id, _device_id;
    queryD3D11DeviceInfo(device, _vendor_id, _device_id);
    std::cout << "vendor id = " << _vendor_id << std::endl;
    std::cout << "device id = " << _device_id << std::endl;

    // Query platforms
    cl_uint platformCount = 0;
    clGetPlatformIDs(0, nullptr, &platformCount);
    if (platformCount == 0) {
        std::cerr << "No OpenCL platforms found." << std::endl;
        // return;
    }

    std::vector<cl_platform_id> platforms(platformCount);
    clGetPlatformIDs(platformCount, platforms.data(), nullptr);

    // Look for a platform with DirectX 11 sharing support
    for (auto platform : platforms) {
        char extensions[1024];
        clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, sizeof(extensions), extensions, nullptr);
        // std::cout << extensions << std::endl;
        if (_has_extension(extensions, "cl_khr_d3d11_sharing") || _has_extension(extensions, "cl_nv_d3d11_sharing")) {
            std::cout << "Platform supports DirectX 11 interop." << std::endl;

            // Query devices for this platform
            cl_uint deviceCount = 0;
            clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);

            if (deviceCount > 0) {
                std::vector<cl_device_id> devices(deviceCount);
                clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, deviceCount, devices.data(), nullptr);

                for (cl_device_id _dev : devices) {
                    char deviceExtensions[1024];
                    clGetDeviceInfo(_dev, CL_DEVICE_EXTENSIONS, sizeof(deviceExtensions), deviceExtensions, nullptr);

                    if (_has_extension(deviceExtensions, "cl_khr_d3d11_sharing") || _has_extension(deviceExtensions, "cl_nv_d3d11_sharing")) {
                        std::cout << "Device supports DirectX 11 interop." << std::endl;

                        cl_uint vendorId = 0;
                        cl_uint deviceId = 0;

                        // Vendor ID (as string, compare to known vendors if necessary)
                        char vendorName[128];
                        clGetDeviceInfo(_dev, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, nullptr);

                        // Device-specific information
                        clGetDeviceInfo(_dev, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &vendorId, nullptr);
                        clGetDeviceInfo(_dev, CL_DEVICE_NAME, sizeof(cl_uint), &deviceId, nullptr);

                        std::cout << "CL vendor id = " << vendorId << std::endl;
                        std::cout << "CL device id = " << deviceId << std::endl;

                        // Attempt to create an OpenCL context with DirectX 11 interop
                        // ID3D11Device* d3d11Device = nullptr; // You need to initialize this with your D3D11 device

                        std::vector<cl_context_properties> properties = {
                            CL_CONTEXT_D3D11_DEVICE_KHR, (cl_context_properties)device,
                            CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
                            // CL_CONTEXT_INTEROP_USER_SYNC, CL_FALSE,
                            0
                        };

                        context _ctx(_dev, properties);
                        if (!clCreateFromD3D11Texture2D) {
                            clCreateFromD3D11Texture2D = reinterpret_cast<clCreateFromD3D11Texture2DKHR_fn>(clGetExtensionFunctionAddressForPlatform(platform, "clCreateFromD3D11Texture2DNV"));
                        }
                        if (!clCreateFromD3D11Texture2D) {
                            throw std::runtime_error("Failed to load clCreateFromD3D11Texture2D.");
                        }
                        if (!clEnqueueAcquireD3D11Objects) {
                            clEnqueueAcquireD3D11Objects = reinterpret_cast<clEnqueueAcquireD3D11ObjectsKHR_fn>(clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueAcquireD3D11ObjectsNV"));
                        }
                        if (!clEnqueueAcquireD3D11Objects) {
                            throw std::runtime_error("Failed to load clEnqueueAcquireD3D11Objects.");
                        }
                        if (!clEnqueueReleaseD3D11Objects) {
                            clEnqueueReleaseD3D11Objects = reinterpret_cast<clEnqueueReleaseD3D11ObjectsKHR_fn>(clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueReleaseD3D11ObjectsNV"));
                        }
                        if (!clEnqueueReleaseD3D11Objects) {
                            throw std::runtime_error("Failed to load clEnqueueReleaseD3D11Objects.");
                        }
                        _ctx._d3d11_device = device;
                        _ctx._d3d11_device_context = device_context;
                        return _ctx;
                    }
                }
            }
        }
    }
}
#endif

texture_2d::texture_2d(const context& ctx, const std::size_t width, const std::size_t height, const std::vector<unsigned char>& pixels)
{
    cl_int _status = 0;
    _command_queue = ctx._command_queue;
#if CLG_USE_DIRECTX
    if (ctx._d3d11_device) {
        D3D11_TEXTURE2D_DESC _texture_desc;
        _texture_desc.Width = static_cast<UINT>(width);
        _texture_desc.Height = static_cast<UINT>(height);
        _texture_desc.MipLevels = 1; // no mipmaps
        _texture_desc.ArraySize = 1; // single texture
        _texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // assuming 4 channels (RGBA)
        _texture_desc.SampleDesc.Count = 1; // no MSAA
        _texture_desc.SampleDesc.Quality = 0;
        _texture_desc.Usage = D3D11_USAGE_DEFAULT;
        _texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        _texture_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // here
        _texture_desc.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA _initial_data;
        _initial_data.pSysMem = pixels.data();
        _initial_data.SysMemPitch = static_cast<UINT>(width * 4); // row size in bytes (RGBA)
        _initial_data.SysMemSlicePitch = 0;
        HRESULT _d3d11_status = ctx._d3d11_device->CreateTexture2D(&_texture_desc, &_initial_data, &_d3d11_texture);
        if (FAILED(_d3d11_status)) {
            throw std::runtime_error("[d3d11] call to ID3D11Device::CreateTexture2D failed");
        }
        _mem = clCreateFromD3D11Texture2D(ctx._context, CL_MEM_READ_WRITE, _d3d11_texture, 0, &_status);
        if (_status != 0) {
            throw std::runtime_error("[d3d11] call to clCreateFromD3D11Texture2DKHR failed");
        }
        return;
    }
#endif
}

#if CLG_USE_DIRECTX
texture_2d::texture_2d(const context& ctx, ID3D11Texture2D* texture)
{
    cl_int _status = 0;
    _command_queue = ctx._command_queue;
    _d3d11_texture = texture;
    _mem = clCreateFromD3D11Texture2D(ctx._context, CL_MEM_READ_WRITE, _d3d11_texture, 0, &_status);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clCreateFromD3D11Texture2DKHR failed");
    }
}

ID3D11Texture2D* texture_2d::get_d3d11() const
{
    if (!_d3d11_texture) {
        throw std::runtime_error("[d3d11] call to texture_2d::get_d3d11 failed because texture was not created with d3d11");
    }
    return _d3d11_texture;
}
#endif

void texture_2d::acquire()
{
    cl_int _status = 0;
    if (_d3d11_texture) {
        _status = clEnqueueAcquireD3D11Objects(_command_queue, 1, &_mem, 0, nullptr, nullptr);
    }
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clEnqueueAcquireD3D11ObjectsKHR failed");
    }
}

void texture_2d::release()
{
    cl_int _status = 0;
    if (_d3d11_texture) {
        _status = clEnqueueReleaseD3D11Objects(_command_queue, 1, &_mem, 0, nullptr, nullptr);
    }
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clEnqueueReleaseD3D11ObjectsKHR failed");
    }
}

kernel::kernel(const context& ctx, const std::string& code, const std::string& name)
{
    cl_int _status = 0;
    std::size_t _kernel_code_length = code.length();
    const char* _kernel_code = code.c_str();
    const char* _kernel_name = name.c_str();
    _device = ctx._device;
    _context = ctx._context;
    _command_queue = ctx._command_queue;
#if CLG_USE_DIRECTX
    _d3d11_device_context = ctx._d3d11_device_context;
#endif
    _program = clCreateProgramWithSource(_context, 1, &_kernel_code, &_kernel_code_length, &_status);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clCreateProgramWithSource failed");
    }
    _status = clBuildProgram(_program, 1, &_device, nullptr, buildNotifyCallback, &_device);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clBuildProgram failed");
    }
    _kernel = clCreateKernel(_program, _kernel_name, &_status);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clCreateKernel failed");
    }
}

void kernel::set_arg(const std::size_t index, texture_2d& texture)
{
    cl_int _status = 0;
    _status = clSetKernelArg(_kernel, static_cast<cl_uint>(index), static_cast<cl_uint>(sizeof(cl_mem)), &texture._mem);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clSetKernelArg failed");
    }
}

void kernel::set_arg(const std::size_t index, const std::size_t size, void* value)
{
    cl_int _status = 0;
    _status = clSetKernelArg(_kernel, static_cast<cl_uint>(index), static_cast<cl_uint>(size), value);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clSetKernelArg failed");
    }
}

void kernel::run(const std::vector<std::size_t>& global_work_size)
{
    cl_int _status = 0;
    _status = clEnqueueNDRangeKernel(_command_queue, _kernel, static_cast<cl_uint>(global_work_size.size()), nullptr, global_work_size.data(), nullptr, 0, nullptr, nullptr);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clSetKernelArg failed");
    }
    _status = clFinish(_command_queue);
    if (_status != 0) {
        throw std::runtime_error("[d3d11] call to clFinish failed");
    }
    if (_d3d11_device_context) {
        _d3d11_device_context->Flush();
    }
}
}