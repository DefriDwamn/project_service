#pragma once

#include <boost/uuid/uuid.hpp>
#include <string_view>
#include <userver/components/component_context.hpp>
#include <userver/components/component_fwd.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include "components/project/component.hpp"

namespace handlers::project_v1_project {
class Handler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-project";
  Handler(const userver::components::ComponentConfig& config,
          const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  components::Project& _project;
  userver::formats::json::Value createProject(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json) const;
  userver::formats::json::Value takeProject(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json) const;
  userver::formats::json::Value updateProject(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json) const;
  userver::formats::json::Value deleteProject(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json) const;
};
}  // namespace handlers::project_v1_project