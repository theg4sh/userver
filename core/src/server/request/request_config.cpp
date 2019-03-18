#include "request_config.hpp"

#include <stdexcept>

#include <yaml_config/value.hpp>

namespace server {
namespace request {
namespace {

const std::string kHttp = "http";

RequestConfig::Type StringToType(const std::string& str) {
  if (str == kHttp) return RequestConfig::Type::kHttp;
  throw std::runtime_error("unknown RequestConfig Type: '" + str + '\'');
}

}  // namespace

RequestConfig::RequestConfig(formats::yaml::Value yaml, std::string full_path,
                             yaml_config::VariableMapPtr config_vars_ptr)
    : yaml_config::YamlConfig(std::move(yaml), std::move(full_path),
                              std::move(config_vars_ptr)),
      type_(StringToType(Parse<std::string>("type", kHttp))) {}

const RequestConfig::Type& RequestConfig::GetType() const { return type_; }

RequestConfig RequestConfig::ParseFromYaml(
    const formats::yaml::Value& yaml, const std::string& full_path,
    const yaml_config::VariableMapPtr& config_vars_ptr) {
  return {yaml, full_path, config_vars_ptr};
}

const std::string& RequestConfig::TypeToString(Type type) {
  switch (type) {
    case Type::kHttp:
      return kHttp;
  }
  throw std::runtime_error(
      "can't convert to string unknown RequestConfig::Type (" +
      std::to_string(static_cast<int>(type)) + ')');
}

}  // namespace request
}  // namespace server
