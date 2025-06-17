#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>

std::string load_file(const std::filesystem::path& path)
{
    std::ifstream _ifs(path);
    if (!_ifs.is_open()) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }
    std::ostringstream _oss;
    _oss << _ifs.rdbuf();
    return _oss.str();
}

std::string resolve_includes(const std::string& source, const std::filesystem::path& include_base, std::unordered_set<std::string>& visited)
{
    std::istringstream _iss(source);
    std::ostringstream _resolved;
    std::string _line;
    // std::regex _include_pattern(R"(^\s*#include\s+<([^>]+)>\s*$)");
    std::regex _include_pattern(R"(^\s*#include\s+\"([^\"]+)\"\s*$)");
    while (std::getline(_iss, _line)) {
        std::smatch _match;
        if (std::regex_match(_line, _match, _include_pattern)) {
            std::string _include_file = _match[1];
            std::filesystem::path _full_path = include_base / _include_file;
            if (visited.count(_full_path.string()) == 0) {
                visited.insert(_full_path.string());
                std::string _included_code = load_file(_full_path);
                _resolved << resolve_includes(_included_code, include_base, visited) << "\n";
            }
        } else {
            _resolved << _line << "\n";
        }
    }
    return _resolved.str();
}

void generate_kernel_struct(const std::string& kernel_name, const std::string& resolved_code, const std::filesystem::path& output_path)
{
    std::ofstream _ofs(output_path);
    if (!_ofs.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_path.string());
    }
    _ofs << "#pragma once\n\n";
    _ofs << "struct " << kernel_name << " {\n";
    _ofs << "    inline static const std::string kernel_source = R\"(\n";
    _ofs << resolved_code;
    _ofs << ")\";\n";
    _ofs << "};\n\n";
}

std::string get_struct_name(const std::filesystem::path& kernel_path)
{
    return kernel_path.stem().string();
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <kernel_input_dir> <include_base_dir> <output_dir>\n";
        return 1;
    }
    std::filesystem::path _input_dir = argv[1];
    std::filesystem::path _include_base = argv[2];
    std::filesystem::path _output_dir = _include_base;
    if (!std::filesystem::exists(_input_dir) || !std::filesystem::is_directory(_input_dir)) {
        std::cout << "Error: Input directory does not exist or is not a directory: " << _input_dir << "\n";
        return 1;
    }
    std::filesystem::create_directories(_output_dir);
    for (const auto& _entry : std::filesystem::directory_iterator(_input_dir)) {
        if (_entry.path().extension() == ".cl") {
            try {
                std::string _kernel_source = load_file(_entry.path());
                std::unordered_set<std::string> _visited;
                std::string _resolved = resolve_includes(_kernel_source, _include_base, _visited);
                std::string _kernel_name = get_struct_name(_entry.path());
                std::filesystem::path _output_file = _output_dir / (_kernel_name + ".hpp");
                generate_kernel_struct(_kernel_name, _resolved, _output_file);
                std::cout << "Generated system: " << _output_file << "\n";
            } catch (const std::exception& ex) {
                std::cout << "Error processing " << _entry.path() << ": " << ex.what() << "\n";
            }
        }
    }

    return 0;
}
