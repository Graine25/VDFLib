/**
 * @file    src/vdf_io.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "vdf_io.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <system_error>
#include <vector>

namespace vdflib {

namespace {

constexpr uint8_t kTypeObject = 0x00;
constexpr uint8_t kTypeString = 0x01;
constexpr uint8_t kTypeUInt32 = 0x02;
constexpr uint8_t kTypeFloat32 = 0x03;
constexpr uint8_t kTypeUInt64 = 0x07;
constexpr uint8_t kTypeEnd = 0x08;

class BinaryReader {
public:
    explicit BinaryReader(std::vector<uint8_t> data) : data_(std::move(data)) {}

    bool atEnd() const { return offset_ >= data_.size(); }

    bool readByte(uint8_t& out) {
        if (offset_ >= data_.size()) return false;
        out = data_[offset_++];
        return true;
    }

    std::string readCString() {
        size_t start = offset_;
        while (offset_ < data_.size() && data_[offset_] != 0) {
            ++offset_;
        }
        if (offset_ >= data_.size()) {
            return "";
        }
        std::string result(reinterpret_cast<const char*>(&data_[start]), offset_ - start);
        ++offset_;
        return result;
    }

    uint32_t readUInt32() {
        if (offset_ + 4 > data_.size()) return 0;
        uint32_t value;
        std::memcpy(&value, &data_[offset_], 4);
        offset_ += 4;
        return value;
    }

    uint64_t readUInt64() {
        if (offset_ + 8 > data_.size()) return 0;
        uint64_t value;
        std::memcpy(&value, &data_[offset_], 8);
        offset_ += 8;
        return value;
    }

    float readFloat32() {
        if (offset_ + 4 > data_.size()) return 0.0f;
        float value;
        std::memcpy(&value, &data_[offset_], 4);
        offset_ += 4;
        return value;
    }

private:
    std::vector<uint8_t> data_;
    size_t offset_ = 0;
};

VdfObject parseSection(BinaryReader& reader) {
    VdfObject result;
    size_t unnamedSubsections = 0;

    uint8_t typeTag;
    while (reader.readByte(typeTag)) {
        if (typeTag == kTypeEnd) {
            break;
        }

        std::string key = reader.readCString();

        switch (typeTag) {
            case kTypeObject: {
                if (key.empty()) {
                    key = "_subsection_" + std::to_string(unnamedSubsections);
                }
                ++unnamedSubsections;
                result[key] = VdfValue(parseSection(reader));
                break;
            }
            case kTypeString:
                result[key] = VdfValue(reader.readCString());
                break;
            case kTypeUInt32:
                result[key] = VdfValue(reader.readUInt32());
                break;
            case kTypeFloat32:
                result[key] = VdfValue(reader.readFloat32());
                break;
            case kTypeUInt64:
                result[key] = VdfValue(reader.readUInt64());
                break;
            default:
                if (!key.empty()) {
                    result[key] = VdfValue(reader.readCString());
                }
                break;
        }
    }

    return result;
}

void writeCString(std::ofstream& out, const std::string& value) {
    out.write(value.data(), static_cast<std::streamsize>(value.size()));
    out.put('\0');
}

void encodeValue(std::ofstream& out, const std::string& key, const VdfValue& value) {
    if (value.isObject()) {
        out.put(static_cast<char>(kTypeObject));
        writeCString(out, key);
        for (const auto& [childKey, childValue] : value.asObject()) {
            encodeValue(out, childKey, childValue);
        }
        out.put(static_cast<char>(kTypeEnd));
    } else if (value.isString()) {
        out.put(static_cast<char>(kTypeString));
        writeCString(out, key);
        writeCString(out, value.asString());
    } else if (value.isUInt32()) {
        out.put(static_cast<char>(kTypeUInt32));
        writeCString(out, key);
        uint32_t raw = value.asUInt32();
        out.write(reinterpret_cast<const char*>(&raw), sizeof(raw));
    } else if (value.isUInt64()) {
        out.put(static_cast<char>(kTypeUInt64));
        writeCString(out, key);
        uint64_t raw = value.asUInt64();
        out.write(reinterpret_cast<const char*>(&raw), sizeof(raw));
    } else if (value.isFloat()) {
        out.put(static_cast<char>(kTypeFloat32));
        writeCString(out, key);
        float raw = value.asFloat();
        out.write(reinterpret_cast<const char*>(&raw), sizeof(raw));
    }
}

}

VdfValue parseVdf(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw VdfIoError("Could not open VDF file for reading: " + filePath.string());
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    BinaryReader reader(std::move(data));
    return VdfValue(parseSection(reader));
}

void writeVdf(const VdfValue& value, const std::filesystem::path& filePath,
              bool createBackup) {
    if (filePath.has_parent_path()) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::filesystem::path temporary = filePath;
    temporary += ".tmp";
    std::filesystem::path backup = filePath;
    backup += ".bak";

    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw VdfIoError("Could not open temporary VDF file for writing: " +
                             temporary.string());
        }

        for (const auto& [key, childValue] : value.asObject()) {
            encodeValue(out, key, childValue);
        }
        out.put(static_cast<char>(kTypeEnd));
        out.flush();
        if (!out) {
            out.close();
            std::filesystem::remove(temporary);
            throw VdfIoError("Could not finish writing VDF file: " + temporary.string());
        }
    }

    std::error_code error;
    const bool hadOriginal = std::filesystem::exists(filePath);
    if (hadOriginal) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(filePath, backup, error);
        if (error) {
            std::filesystem::remove(temporary);
            throw VdfIoError("Could not prepare VDF file for replacement: " +
                             error.message());
        }
    }

    std::filesystem::rename(temporary, filePath, error);
    if (error) {
        if (hadOriginal) {
            std::error_code restoreError;
            std::filesystem::rename(backup, filePath, restoreError);
        }
        std::filesystem::remove(temporary);
        throw VdfIoError("Could not replace VDF file: " + error.message());
    }

    if (hadOriginal && !createBackup) {
        std::filesystem::remove(backup, error);
    }
}

}
