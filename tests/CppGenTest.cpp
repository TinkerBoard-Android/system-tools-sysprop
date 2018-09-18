/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <string>

#include <android-base/file.h>
#include <android-base/scopeguard.h>
#include <android-base/test_utils.h>
#include <gtest/gtest.h>

#include "CppGen.h"

namespace {

constexpr const char* kTestSyspropFile =
    R"(owner: Platform
module: "android.os.PlatformProperties"
prefix: "android.os"

prop {
    name: "test_double"
    type: Double
    scope: Internal
    readonly: false
}
prop {
    name: "test_int"
    type: Integer
    scope: Public
    readonly: false
}
prop {
    name: "test.string"
    type: String
    scope: System
    readonly: false
}

prop {
    name: "test.enum"
    type: Enum
    enum_values: "a|b|c|D|e|f|G"
    scope: Internal
    readonly: false
}
prop {
    name: "test_BOOLeaN"
    type: Boolean
    scope: Public
    readonly: false
}
prop {
    name: "longlonglongLONGLONGlongLONGlongLONG"
    type: Long
    scope: System
    readonly: false
}

prop {
    name: "test_double_list"
    type: DoubleList
    scope: Internal
    readonly: false
}
prop {
    name: "test_list_int"
    type: IntegerList
    scope: Public
    readonly: false
}
prop {
    name: "test.strlist"
    type: StringList
    scope: System
    readonly: false
}

prop {
    name: "el"
    type: EnumList
    enum_values: "enu|mva|lue"
    scope: Internal
    readonly: false
}
)";

constexpr const char* kExpectedHeaderOutput =
    R"(// Generated by the sysprop generator. DO NOT EDIT!

#ifndef SYSPROPGEN_android_os_PlatformProperties_H_
#define SYSPROPGEN_android_os_PlatformProperties_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace android::os::PlatformProperties {

std::optional<double> test_double();
bool test_double(const double& value);

std::optional<std::int32_t> test_int();
bool test_int(const std::int32_t& value);

std::optional<std::string> test_string();
bool test_string(const std::string& value);

enum class test_enum_values {
    a,
    b,
    c,
    D,
    e,
    f,
    G,
};

std::optional<test_enum_values> test_enum();
bool test_enum(const test_enum_values& value);

std::optional<bool> test_BOOLeaN();
bool test_BOOLeaN(const bool& value);

std::optional<std::int64_t> longlonglongLONGLONGlongLONGlongLONG();
bool longlonglongLONGLONGlongLONGlongLONG(const std::int64_t& value);

std::optional<std::vector<double>> test_double_list();
bool test_double_list(const std::vector<double>& value);

std::optional<std::vector<std::int32_t>> test_list_int();
bool test_list_int(const std::vector<std::int32_t>& value);

std::optional<std::vector<std::string>> test_strlist();
bool test_strlist(const std::vector<std::string>& value);

enum class el_values {
    enu,
    mva,
    lue,
};

std::optional<std::vector<el_values>> el();
bool el(const std::vector<el_values>& value);

}  // namespace android::os::PlatformProperties

#endif  // SYSPROPGEN_android_os_PlatformProperties_H_
)";

constexpr const char* kExpectedSourceOutput =
    R"(// Generated by the sysprop generator. DO NOT EDIT!

#include <properties/PlatformProperties.sysprop.h>

#include <cstring>
#include <iterator>
#include <type_traits>
#include <utility>

#include <strings.h>
#include <sys/system_properties.h>

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>

namespace {

using namespace android::os::PlatformProperties;

template <typename T> std::optional<T> DoParse(const char* str);

constexpr const std::pair<const char*, test_enum_values> test_enum_list[] = {
    {"a", test_enum_values::a},
    {"b", test_enum_values::b},
    {"c", test_enum_values::c},
    {"D", test_enum_values::D},
    {"e", test_enum_values::e},
    {"f", test_enum_values::f},
    {"G", test_enum_values::G},
};

template <>
std::optional<test_enum_values> DoParse(const char* str) {
    for (auto [name, val] : test_enum_list) {
        if (strcmp(str, name) == 0) {
            return val;
        }
    }
    return std::nullopt;
}

std::string FormatValue(test_enum_values value) {
    for (auto [name, val] : test_enum_list) {
        if (val == value) {
            return name;
        }
    }
    LOG(FATAL) << "Invalid value " << static_cast<std::int32_t>(value) << " for property " << "android.os.test_enum";
    __builtin_unreachable();
}

constexpr const std::pair<const char*, el_values> el_list[] = {
    {"enu", el_values::enu},
    {"mva", el_values::mva},
    {"lue", el_values::lue},
};

template <>
std::optional<el_values> DoParse(const char* str) {
    for (auto [name, val] : el_list) {
        if (strcmp(str, name) == 0) {
            return val;
        }
    }
    return std::nullopt;
}

std::string FormatValue(el_values value) {
    for (auto [name, val] : el_list) {
        if (val == value) {
            return name;
        }
    }
    LOG(FATAL) << "Invalid value " << static_cast<std::int32_t>(value) << " for property " << "android.os.el";
    __builtin_unreachable();
}

template <typename T> constexpr bool is_vector = false;

template <typename T> constexpr bool is_vector<std::vector<T>> = true;

template <> [[maybe_unused]] std::optional<bool> DoParse(const char* str) {
    static constexpr const char* kYes[] = {"1", "true"};
    static constexpr const char* kNo[] = {"0", "false"};

    for (const char* yes : kYes) {
        if (strcasecmp(yes, str) == 0) return std::make_optional(true);
    }

    for (const char* no : kNo) {
        if (strcasecmp(no, str) == 0) return std::make_optional(false);
    }

    return std::nullopt;
}

template <> [[maybe_unused]] std::optional<std::int32_t> DoParse(const char* str) {
    std::int32_t ret;
    bool success = android::base::ParseInt(str, &ret);
    return success ? std::make_optional(ret) : std::nullopt;
}

template <> [[maybe_unused]] std::optional<std::int64_t> DoParse(const char* str) {
    std::int64_t ret;
    bool success = android::base::ParseInt(str, &ret);
    return success ? std::make_optional(ret) : std::nullopt;
}

template <> [[maybe_unused]] std::optional<double> DoParse(const char* str) {
    int old_errno = errno;
    errno = 0;
    char* end;
    double ret = std::strtod(str, &end);
    if (errno != 0) {
        return std::nullopt;
    }
    if (str == end || *end != '\0') {
        errno = old_errno;
        return std::nullopt;
    }
    errno = old_errno;
    return std::make_optional(ret);
}

template <> [[maybe_unused]] std::optional<std::string> DoParse(const char* str) {
    return std::make_optional(str);
}

template <typename Vec> [[maybe_unused]] std::optional<Vec> DoParseList(const char* str) {
    Vec ret;
    for (auto&& element : android::base::Split(str, ",")) {
        auto parsed = DoParse<typename Vec::value_type>(element.c_str());
        if (!parsed) {
            return std::nullopt;
        }
        ret.emplace_back(std::move(*parsed));
    }
    return std::make_optional(std::move(ret));
}

template <typename T> inline std::optional<T> TryParse(const char* str) {
    if constexpr(is_vector<T>) {
        return DoParseList<T>(str);
    } else {
        return DoParse<T>(str);
    }
}

[[maybe_unused]] std::string FormatValue(std::int32_t value) {
    return std::to_string(value);
}

[[maybe_unused]] std::string FormatValue(std::int64_t value) {
    return std::to_string(value);
}

[[maybe_unused]] std::string FormatValue(double value) {
    return android::base::StringPrintf("%.*g", std::numeric_limits<double>::max_digits10, value);
}

[[maybe_unused]] std::string FormatValue(bool value) {
    return value ? "true" : "false";
}

template <typename T>
[[maybe_unused]] std::string FormatValue(const std::vector<T>& value) {
    if (value.empty()) return "";

    std::string ret;

    for (auto&& element : value) {
        if (ret.empty()) ret.push_back(',');
        if constexpr(std::is_same_v<T, std::string>) {
            ret += element;
        } else {
            ret += FormatValue(element);
        }
    }

    return ret;
}

template <typename T>
std::optional<T> GetProp(const char* key) {
    auto pi = __system_property_find(key);
    if (pi == nullptr) return std::nullopt;
    std::optional<T> ret;
    __system_property_read_callback(pi, [](void* cookie, const char*, const char* value, std::uint32_t) {
        *static_cast<std::optional<T>*>(cookie) = TryParse<T>(value);
    }, &ret);
    return ret;
}

}  // namespace

namespace android::os::PlatformProperties {

std::optional<double> test_double() {
    return GetProp<double>("android.os.test_double");
}

bool test_double(const double& value) {
    return __system_property_set("android.os.test_double", FormatValue(value).c_str()) == 0;
}

std::optional<std::int32_t> test_int() {
    return GetProp<std::int32_t>("android.os.test_int");
}

bool test_int(const std::int32_t& value) {
    return __system_property_set("android.os.test_int", FormatValue(value).c_str()) == 0;
}

std::optional<std::string> test_string() {
    return GetProp<std::string>("android.os.test.string");
}

bool test_string(const std::string& value) {
    return __system_property_set("android.os.test.string", value.c_str()) == 0;
}

std::optional<test_enum_values> test_enum() {
    return GetProp<test_enum_values>("android.os.test.enum");
}

bool test_enum(const test_enum_values& value) {
    return __system_property_set("android.os.test.enum", FormatValue(value).c_str()) == 0;
}

std::optional<bool> test_BOOLeaN() {
    return GetProp<bool>("android.os.test_BOOLeaN");
}

bool test_BOOLeaN(const bool& value) {
    return __system_property_set("android.os.test_BOOLeaN", FormatValue(value).c_str()) == 0;
}

std::optional<std::int64_t> longlonglongLONGLONGlongLONGlongLONG() {
    return GetProp<std::int64_t>("android.os.longlonglongLONGLONGlongLONGlongLONG");
}

bool longlonglongLONGLONGlongLONGlongLONG(const std::int64_t& value) {
    return __system_property_set("android.os.longlonglongLONGLONGlongLONGlongLONG", FormatValue(value).c_str()) == 0;
}

std::optional<std::vector<double>> test_double_list() {
    return GetProp<std::vector<double>>("android.os.test_double_list");
}

bool test_double_list(const std::vector<double>& value) {
    return __system_property_set("android.os.test_double_list", FormatValue(value).c_str()) == 0;
}

std::optional<std::vector<std::int32_t>> test_list_int() {
    return GetProp<std::vector<std::int32_t>>("android.os.test_list_int");
}

bool test_list_int(const std::vector<std::int32_t>& value) {
    return __system_property_set("android.os.test_list_int", FormatValue(value).c_str()) == 0;
}

std::optional<std::vector<std::string>> test_strlist() {
    return GetProp<std::vector<std::string>>("android.os.test.strlist");
}

bool test_strlist(const std::vector<std::string>& value) {
    return __system_property_set("android.os.test.strlist", FormatValue(value).c_str()) == 0;
}

std::optional<std::vector<el_values>> el() {
    return GetProp<std::vector<el_values>>("android.os.el");
}

bool el(const std::vector<el_values>& value) {
    return __system_property_set("android.os.el", FormatValue(value).c_str()) == 0;
}

}  // namespace android::os::PlatformProperties
)";

}  // namespace

using namespace std::string_literals;

TEST(SyspropTest, CppGenTest) {
  TemporaryDir temp_dir;

  std::string temp_sysprop_path = temp_dir.path + "/PlatformProperties.sysprop"s;
  ASSERT_TRUE(
      android::base::WriteStringToFile(kTestSyspropFile, temp_sysprop_path));

  auto sysprop_deleter = android::base::make_scope_guard(
      [&] { unlink(temp_sysprop_path.c_str()); });

  std::string err;
  ASSERT_TRUE(GenerateCppFiles(temp_sysprop_path, temp_dir.path, temp_dir.path,
                               "properties/PlatformProperties.sysprop.h", &err));
  ASSERT_TRUE(err.empty());

  std::string header_output_path =
      temp_dir.path + "/PlatformProperties.sysprop.h"s;
  std::string source_output_path =
      temp_dir.path + "/PlatformProperties.sysprop.cpp"s;

  auto generated_file_deleter = android::base::make_scope_guard([&] {
    unlink(header_output_path.c_str());
    unlink(source_output_path.c_str());
  });

  std::string header_output;
  ASSERT_TRUE(android::base::ReadFileToString(header_output_path,
                                              &header_output, true));
  ASSERT_EQ(header_output, kExpectedHeaderOutput);

  std::string source_output;
  ASSERT_TRUE(android::base::ReadFileToString(source_output_path,
                                              &source_output, true));
  ASSERT_EQ(source_output, kExpectedSourceOutput);
}
