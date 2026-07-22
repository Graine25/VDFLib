/**
 * @file    src/vdf_value.h
 * @brief   Declares VDF values and objects. Represents Steam binary VDF data in memory.
 *
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace vdflib {

class VdfTypeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class VdfValue;

class VdfObject {
public:
    using Entry = std::pair<std::string, VdfValue>;
    using iterator = std::vector<Entry>::iterator;
    using const_iterator = std::vector<Entry>::const_iterator;

    VdfValue& operator[](const std::string& key);
    const VdfValue* find(const std::string& key) const;
    VdfValue* find(const std::string& key);
    bool contains(const std::string& key) const { return find(key) != nullptr; }
    bool erase(const std::string& key);

    iterator begin() { return entries_.begin(); }
    iterator end() { return entries_.end(); }
    const_iterator begin() const { return entries_.begin(); }
    const_iterator end() const { return entries_.end(); }

    size_t size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }

    bool operator==(const VdfObject& other) const;
    bool operator!=(const VdfObject& other) const { return !(*this == other); }

private:
    std::vector<Entry> entries_;
};

class VdfValue {
public:
    using Storage = std::variant<VdfObject, std::string, uint32_t, uint64_t, float>;

    VdfValue() : storage_(VdfObject{}) {}
    VdfValue(VdfObject value) : storage_(std::move(value)) {}
    VdfValue(std::string value) : storage_(std::move(value)) {}
    VdfValue(const char* value) : storage_(std::string(value)) {}
    VdfValue(uint32_t value) : storage_(value) {}
    VdfValue(uint64_t value) : storage_(value) {}
    VdfValue(float value) : storage_(value) {}

    bool isObject() const { return std::holds_alternative<VdfObject>(storage_); }
    bool isString() const { return std::holds_alternative<std::string>(storage_); }
    bool isUInt32() const { return std::holds_alternative<uint32_t>(storage_); }
    bool isUInt64() const { return std::holds_alternative<uint64_t>(storage_); }
    bool isFloat() const { return std::holds_alternative<float>(storage_); }

    const VdfObject& asObject() const { return get<VdfObject>("object"); }
    VdfObject& asObject() { return get<VdfObject>("object"); }
    const std::string& asString() const { return get<std::string>("string"); }
    uint32_t asUInt32() const { return get<uint32_t>("uint32"); }
    uint64_t asUInt64() const { return get<uint64_t>("uint64"); }
    float asFloat() const { return get<float>("float"); }

    std::string getString(const std::string& key, const std::string& fallback = "") const;
    uint32_t getUInt32(const std::string& key, uint32_t fallback = 0) const;
    uint64_t getUInt64(const std::string& key, uint64_t fallback = 0) const;

    bool operator==(const VdfValue& other) const { return storage_ == other.storage_; }
    bool operator!=(const VdfValue& other) const { return !(*this == other); }

private:
    template <typename T>
    const T& get(const char* typeName) const {
        if (const T* value = std::get_if<T>(&storage_)) {
            return *value;
        }
        throw VdfTypeError(std::string("VdfValue does not hold a ") + typeName);
    }
    template <typename T>
    T& get(const char* typeName) {
        if (T* value = std::get_if<T>(&storage_)) {
            return *value;
        }
        throw VdfTypeError(std::string("VdfValue does not hold a ") + typeName);
    }

    Storage storage_;
};

}
