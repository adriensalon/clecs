#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>


#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/istreamwrapper.h>

namespace fs = std::filesystem;

std::string generate_host_code(const std::string& name, const rapidjson::Value& fields)
{
    std::ostringstream oss;

    oss << "#pragma once\n\n";
    oss << "// generated component for host code\n";
    oss << "struct " << name << " {\n";
    for (auto itr = fields.MemberBegin(); itr != fields.MemberEnd(); ++itr) {
        oss << "    " << itr->value.GetString() << " " << itr->name.GetString() << ";\n";
    }
    oss << "\n    template <typename archive_t>\n";
    oss << "    void serialize(archive_t& archive)\n";
    oss << "    {\n";
    for (auto itr = fields.MemberBegin(); itr != fields.MemberEnd(); ++itr) {
        oss << "        archive(" << itr->name.GetString() << ");\n";
    }
    oss << "    }\n};\n";

    return oss.str();
}

std::string generate_device_code(const std::string& name, const rapidjson::Value& fields)
{
    std::ostringstream oss;
    oss << "typedef struct {\n";
    for (auto itr = fields.MemberBegin(); itr != fields.MemberEnd(); ++itr) {
        oss << "    " << itr->value.GetString() << " " << itr->name.GetString() << ";\n";
    }
    oss << "} " << name << ";\n";
    return oss.str();
}

void process_file(const fs::path& input_path, const fs::path& out_host_dir, const fs::path& out_device_dir)
{
    std::ifstream ifs(input_path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open file: " + input_path.string());
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (!doc.HasMember("name") || !doc.HasMember("fields")) {
        throw std::runtime_error("Invalid component schema in: " + input_path.string());
    }

    std::string name = doc["name"].GetString();
    const auto& fields = doc["fields"];

    auto host_code = generate_host_code(name, fields);
    auto device_code = generate_device_code(name, fields);

    fs::create_directories(out_host_dir);
    fs::create_directories(out_device_dir);

    std::ofstream host_file(out_host_dir / (name + ".hpp"));
    host_file << host_code;

    std::ofstream device_file(out_device_dir / (name + ".cl"));
    device_file << device_code;

    std::cout << "Generated component: " << name << "\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input_dir> <host_output_dir> <device_output_dir>\n";
        return 1;
    }

    fs::path input_dir = argv[1];
    fs::path out_host_dir = argv[2];

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cout << "Error: Input directory does not exist or is not a directory: " << input_dir << "\n";
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".json") {
            try {
                process_file(entry.path(), out_host_dir, out_host_dir);
            } catch (const std::exception& ex) {
                std::cout << "Error processing " << entry.path() << ": " << ex.what() << "\n";
            }
        }
    }

    std::cout << "Done.\n";
    return 0;
}