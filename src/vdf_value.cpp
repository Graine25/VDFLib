/**
 * @file    src/vdf_value.cpp
 * @copyright Copyright (c) 2026 Rien Gupta <rgupta9@scu.edu>
 *            All rights reserved.
 * @license   MIT License
 *            See LICENSE file in the project root for full license text.
 */

#include "vdf_value.h"

#include <algorithm>

namespace vdflib {

VdfObject::iterator VdfObject::begin() { return entries_.begin(); }
VdfObject::iterator VdfObject::end() { return entries_.end(); }
VdfObject::const_iterator VdfObject::begin() const { return entries_.begin(); }
VdfObject::const_iterator VdfObject::end() const { return entries_.end(); }

size_t VdfObject::size() const { return entries_.size(); }
bool VdfObject::empty() const { return entries_.empty(); }

VdfValue& VdfObject::operator[](const std::string& key) {
    if (VdfValue* existing = find(key)) {
        return *existing;
    }
    entries_.emplace_back(key, VdfValue{});
    return entries_.back().second;
}

const VdfValue* VdfObject::find(const std::string& key) const {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                            [&](const Entry& entry) { return entry.first == key; });
    return it == entries_.end() ? nullptr : &it->second;
}

VdfValue* VdfObject::find(const std::string& key) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                            [&](const Entry& entry) { return entry.first == key; });
    return it == entries_.end() ? nullptr : &it->second;
}

bool VdfObject::erase(const std::string& key) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                            [&](const Entry& entry) { return entry.first == key; });
    if (it == entries_.end()) {
        return false;
    }
    entries_.erase(it);
    return true;
}

bool VdfObject::operator==(const VdfObject& other) const {
    if (entries_.size() != other.entries_.size()) {
        return false;
    }
    for (const auto& [key, value] : entries_) {
        const VdfValue* otherValue = other.find(key);
        if (!otherValue || *otherValue != value) {
            return false;
        }
    }
    return true;
}

std::string VdfValue::getString(const std::string& key, const std::string& fallback) const {
    if (!isObject()) return fallback;
    const VdfValue* value = asObject().find(key);
    return (value && value->isString()) ? value->asString() : fallback;
}

uint32_t VdfValue::getUInt32(const std::string& key, uint32_t fallback) const {
    if (!isObject()) return fallback;
    const VdfValue* value = asObject().find(key);
    return (value && value->isUInt32()) ? value->asUInt32() : fallback;
}

uint64_t VdfValue::getUInt64(const std::string& key, uint64_t fallback) const {
    if (!isObject()) return fallback;
    const VdfValue* value = asObject().find(key);
    return (value && value->isUInt64()) ? value->asUInt64() : fallback;
}

}
