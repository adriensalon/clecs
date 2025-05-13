#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

std::string load_file(const fs::path& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }

    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

std::string resolve_includes(const std::string& source, const fs::path& include_base, std::unordered_set<std::string>& visited) {
    std::istringstream iss(source);
    std::ostringstream resolved;
    std::string line;

    // std::regex include_pattern(R"(^\s*#include\s+<([^>]+)>\s*$)");
    std::regex include_pattern(R"(^\s*#include\s+\"([^\"]+)\"\s*$)");

    while (std::getline(iss, line)) {
        std::smatch match;
        if (std::regex_match(line, match, include_pattern)) {
            std::string include_file = match[1];
            fs::path full_path = include_base / include_file;

            if (visited.count(full_path.string()) == 0) {
                visited.insert(full_path.string());
                std::string included_code = load_file(full_path);
                resolved << resolve_includes(included_code, include_base, visited) << "\n";
            }
        } else {
            resolved << line << "\n";
        }
    }

    return resolved.str();
}

void generate_kernel_struct(const std::string& kernel_name, const std::string& resolved_code, const fs::path& output_path) {
    std::ofstream ofs(output_path);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_path.string());
    }

    ofs << "#pragma once\n\n";
    ofs << "struct " << kernel_name << " {\n";
    ofs << "    inline static const std::string kernel_source = R\"(\n";
    ofs << resolved_code;
    ofs << ")\";\n";
    ofs << "};\n\n";
}

std::string get_struct_name(const fs::path& kernel_path) {
    return kernel_path.stem().string(); // e.g., "calculate_speed"
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <kernel_input_dir> <include_base_dir> <output_dir>\n";
        return 1;
    }

    fs::path input_dir = argv[1];
    fs::path include_base = argv[2];
    fs::path output_dir = include_base;

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Invalid input directory.\n";
        return 1;
    }

    fs::create_directories(output_dir);

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".cl") {
            try {
                std::string kernel_source = load_file(entry.path());
                std::unordered_set<std::string> visited;

                std::string resolved = resolve_includes(kernel_source, include_base, visited);
                std::string kernel_name = get_struct_name(entry.path());

                fs::path output_file = output_dir / (kernel_name + ".hpp");
                generate_kernel_struct(kernel_name, resolved, output_file);

                std::cout << "Generated: " << output_file << "\n";
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << "\n";
            }
        }
    }

    return 0;
}
