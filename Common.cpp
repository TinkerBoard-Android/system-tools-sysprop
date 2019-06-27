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

#include "Common.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <google/protobuf/text_format.h>

#include "sysprop.pb.h"

using android::base::ErrnoErrorf;
using android::base::Errorf;
using android::base::Result;

namespace {

std::string GenerateDefaultPropName(const sysprop::Properties& props,
                                    const sysprop::Property& prop);
bool IsCorrectIdentifier(const std::string& name);
Result<void> ValidateProp(const sysprop::Properties& props,
                          const sysprop::Property& prop);
Result<void> ValidateProps(const sysprop::Properties& props);

std::string GenerateDefaultPropName(const sysprop::Properties& props,
                                    const sysprop::Property& prop) {
  std::string ret;

  if (prop.access() != sysprop::ReadWrite) ret = "ro.";

  switch (props.owner()) {
    case sysprop::Vendor:
      ret += "vendor.";
      break;
    case sysprop::Odm:
      ret += "odm.";
      break;
    default:
      break;
  }

  ret += prop.api_name();

  return ret;
}

bool IsCorrectIdentifier(const std::string& name) {
  if (name.empty()) return false;
  if (std::isalpha(name[0]) == 0 && name[0] != '_') return false;

  return std::all_of(name.begin() + 1, name.end(), [](char ch) {
    return std::isalnum(ch) != 0 || ch == '_';
  });
}

bool IsCorrectPropertyOrApiName(const std::string& name) {
  if (name.empty()) return false;

  static std::unordered_set<char> allowed{'_', '-', '.'};

  return std::all_of(name.begin(), name.end(), [](char ch) {
    return std::isalnum(ch) != 0 || allowed.count(ch) != 0;
  });
}

Result<void> ValidateProp(const sysprop::Properties& props,
                          const sysprop::Property& prop) {
  if (!IsCorrectPropertyOrApiName(prop.api_name())) {
    return Errorf("Invalid API name \"{}\"", prop.api_name());
  }

  if (prop.type() == sysprop::Enum || prop.type() == sysprop::EnumList) {
    std::vector<std::string> names =
        android::base::Split(prop.enum_values(), "|");
    if (names.empty()) {
      return Errorf("Enum values are empty for API \"{}\"", prop.api_name());
    }

    for (const std::string& name : names) {
      if (!IsCorrectIdentifier(name)) {
        return Errorf("Invalid enum value \"{}\" for API \"{}\"", name,
                      prop.api_name());
      }
    }

    std::unordered_set<std::string> name_set;
    for (const std::string& name : names) {
      if (!name_set.insert(ToUpper(name)).second) {
        return Errorf("Duplicated enum value \"{}\" for API \"{}\"", name,
                      prop.api_name());
      }
    }
  }

  std::string prop_name = prop.prop_name();
  if (prop_name.empty()) prop_name = GenerateDefaultPropName(props, prop);

  if (!IsCorrectPropertyOrApiName(prop_name)) {
    return Errorf("Invalid prop name \"{}\"", prop.prop_name());
  }

  static const std::regex vendor_regex(
      "(init\\.svc\\.|ro\\.|persist\\.)?vendor\\..+|ro\\.hardware\\..+");
  static const std::regex odm_regex(
      "(init\\.svc\\.|ro\\.|persist\\.)?odm\\..+|ro\\.hardware\\..+");

  switch (props.owner()) {
    case sysprop::Platform:
      if (std::regex_match(prop_name, vendor_regex) ||
          std::regex_match(prop_name, odm_regex)) {
        return Errorf(
            "Prop \"{}\" owned by platform cannot have vendor. or odm. "
            "namespace",
            prop_name);
      }
      break;
    case sysprop::Vendor:
      if (!std::regex_match(prop_name, vendor_regex)) {
        return Errorf(
            "Prop \"{}\" owned by vendor should have vendor. namespace",
            prop_name);
      }
      break;
    case sysprop::Odm:
      if (!std::regex_match(prop_name, odm_regex)) {
        return Errorf("Prop \"{}\" owned by odm should have odm. namespace",
                      prop_name);
      }
      break;
    default:
      break;
  }

  switch (prop.access()) {
    case sysprop::ReadWrite:
      if (android::base::StartsWith(prop_name, "ro.")) {
        return Errorf("Prop \"{}\" is ReadWrite and also have prefix \"ro.\"",
                      prop_name);
      }
      break;
    default:
      /*
       * TODO: Some properties don't have prefix "ro." but not written in any
       * Java or C++ codes. They might be misnamed and should be readonly. Will
       * uncomment this check after fixing them all / or making a whitelist for
       * them
      if (!android::base::StartsWith(prop_name, "ro.")) {
        return Errorf("Prop \"{}\" isn't ReadWrite, but don't have prefix
      \"ro.\"", prop_name);
      }
      */
      break;
  }

  if (prop.integer_as_bool() && !(prop.type() == sysprop::Boolean ||
                                  prop.type() == sysprop::BooleanList)) {
    return Errorf("Prop \"{}\" has integer_as_bool: true, but not a boolean",
                  prop_name);
  }

  return {};
}

Result<void> ValidateProps(const sysprop::Properties& props) {
  std::vector<std::string> names = android::base::Split(props.module(), ".");
  if (names.size() <= 1) {
    return Errorf("Invalid module name \"{}\"", props.module());
  }

  for (const auto& name : names) {
    if (!IsCorrectIdentifier(name)) {
      return Errorf("Invalid name \"{}\" in module", name);
    }
  }

  if (props.prop_size() == 0) {
    return Errorf("There is no defined property");
  }

  for (int i = 0; i < props.prop_size(); ++i) {
    const auto& prop = props.prop(i);
    if (auto res = ValidateProp(props, prop); !res) return res;
  }

  std::unordered_set<std::string> prop_names;

  for (int i = 0; i < props.prop_size(); ++i) {
    const auto& prop = props.prop(i);
    auto res = prop_names.insert(ApiNameToIdentifier(prop.api_name()));

    if (!res.second) {
      return Errorf("Duplicated API name \"{}\"", prop.api_name());
    }
  }

  return {};
}

}  // namespace

bool IsListProp(const sysprop::Property& prop) {
  switch (prop.type()) {
    case sysprop::BooleanList:
    case sysprop::IntegerList:
    case sysprop::LongList:
    case sysprop::DoubleList:
    case sysprop::StringList:
    case sysprop::EnumList:
      return true;
    default:
      return false;
  }
}

std::string GetModuleName(const sysprop::Properties& props) {
  const std::string& module = props.module();
  return module.substr(module.rfind('.') + 1);
}

Result<sysprop::Properties> ParseProps(const std::string& input_file_path) {
  sysprop::Properties ret;
  std::string file_contents;

  if (!android::base::ReadFileToString(input_file_path, &file_contents, true)) {
    return ErrnoErrorf("Error reading file {}", input_file_path);
  }

  if (!google::protobuf::TextFormat::ParseFromString(file_contents, &ret)) {
    return Errorf("Error parsing file {}", input_file_path);
  }

  if (auto res = ValidateProps(ret); !res) {
    return res.error();
  }

  for (int i = 0; i < ret.prop_size(); ++i) {
    // set each optional field to its default value
    sysprop::Property& prop = *ret.mutable_prop(i);
    if (prop.prop_name().empty())
      prop.set_prop_name(GenerateDefaultPropName(ret, prop));
    if (prop.scope() == sysprop::Scope::System) {
      LOG(WARNING) << "Sysprop API " << prop.api_name()
                   << ": System scope is deprecated."
                   << " Please use Public scope instead.";
      prop.set_scope(sysprop::Scope::Public);
    }
  }

  return ret;
}

std::string ToUpper(std::string str) {
  for (char& ch : str) {
    ch = toupper(ch);
  }
  return str;
}

std::string ApiNameToIdentifier(const std::string& name) {
  static const std::regex kRegexAllowed{"-|\\."};
  return (isdigit(name[0]) ? "_" : "") +
         std::regex_replace(name, kRegexAllowed, "_");
}
