namespace compute {

template <typename value_t>
void kernel::set_arg(const std::size_t idx, buffer<value_t>& buf)
{
    auto _err = clSetKernelArg(_kernel, static_cast<cl_uint>(idx), sizeof(cl_mem), &(buf._mem));
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to set buffer kernel argument at index " + std::to_string(idx));
    }
}

template <typename value_t>
void kernel::set_arg(const std::size_t idx, array_buffer<value_t>& buf)
{
    auto _err = clSetKernelArg(_kernel, static_cast<cl_uint>(idx), sizeof(cl_mem), &(buf._mem));
    if (_err != CL_SUCCESS) {
        throw std::runtime_error("Failed to set array_buffer kernel argument at index " + std::to_string(idx));
    }
}

}