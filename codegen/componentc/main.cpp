#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

std::string generate_host_code(const std::string& name, const rapidjson::Value& fields)
{
    std::ostringstream _oss;
    _oss << "#pragma once\n\n";
    _oss << "// generated component for host code\n";
    _oss << "struct " << name << " {\n";
    for (auto _it = fields.MemberBegin(); _it != fields.MemberEnd(); ++_it) {
        _oss << "    " << _it->value.GetString() << " " << _it->name.GetString() << ";\n";
    }
    _oss << "\n    template <typename archive_t>\n";
    _oss << "    void serialize(archive_t& archive)\n";
    _oss << "    {\n";
    for (auto itr = fields.MemberBegin(); itr != fields.MemberEnd(); ++itr) {
        _oss << "        archive(" << itr->name.GetString() << ");\n";
    }
    _oss << "    }\n};\n";
    return _oss.str();
}

std::string generate_device_code(const std::string& name, const rapidjson::Value& fields)
{
    std::ostringstream _oss;
    _oss << "typedef struct {\n";
    for (auto _it = fields.MemberBegin(); _it != fields.MemberEnd(); ++_it) {
        _oss << "    " << _it->value.GetString() << " " << _it->name.GetString() << ";\n";
    }
    _oss << "} " << name << ";\n";
    return _oss.str();
}

void process_file(const std::filesystem::path& input_path, const std::filesystem::path& out_host_dir, const std::filesystem::path& out_device_dir)
{
    std::ifstream _ifs(input_path);
    if (!_ifs.is_open()) {
        throw std::runtime_error("Failed to open file: " + input_path.string());
    }
    rapidjson::IStreamWrapper _isw(_ifs);
    rapidjson::Document _doc;
    _doc.ParseStream(_isw);
    if (!_doc.HasMember("name") || !_doc.HasMember("fields")) {
        throw std::runtime_error("Invalid component schema in: " + input_path.string());
    }
    std::string _name = _doc["name"].GetString();
    const auto& _fields = _doc["fields"];
    auto _host_code = generate_host_code(_name, _fields);
    auto _device_code = generate_device_code(_name, _fields);
    std::filesystem::create_directories(out_host_dir);
    std::filesystem::create_directories(out_device_dir);
    std::filesystem::path _output_path = out_host_dir / (_name + ".hpp");
    std::ofstream _host_file(_output_path);
    _host_file << _host_code;
    std::ofstream _device_file(out_device_dir / (_name + ".cl"));
    _device_file << _device_code;
    std::cout << "Generated component: " << _output_path << "\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input_dir> <host_output_dir> <device_output_dir>\n";
        return 1;
    }
    std::filesystem::path _input_dir = argv[1];
    std::filesystem::path _out_host_dir = argv[2];
    if (!std::filesystem::exists(_input_dir) || !std::filesystem::is_directory(_input_dir)) {
        std::cout << "Error: Input directory does not exist or is not a directory: " << _input_dir << "\n";
        return 1;
    }
    for (const auto& _entry : std::filesystem::directory_iterator(_input_dir)) {
        if (_entry.path().extension() == ".json") {
            try {
                process_file(_entry.path(), _out_host_dir, _out_host_dir);
            } catch (const std::exception& ex) {
                std::cout << "Error processing " << _entry.path() << ": " << ex.what() << "\n";
            }
        }
    }
    return 0;
}