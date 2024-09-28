#include "handler.hpp"
#include <boost/uuid/uuid.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/storages/postgres/sql_state.hpp>
#include "components/project/component.hpp"
#include "dto/project.hpp"
#include "exceptions/not_found.hpp"

namespace handlers::project_v1_project {

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context),
      _project(context.FindComponent<components::Project>()) {}

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext& context) const {
  switch (request.GetMethod()) {
    case userver::v2_5_rc::server::http::HttpMethod::kPost:
      createProject(request, request_json);
      break;
    case userver::v2_5_rc::server::http::HttpMethod::kGet:
      takeProject(request, request_json);
      break;
    case userver::v2_5_rc::server::http::HttpMethod::kPut:
      // updateProject(request, request_json);
    case userver::v2_5_rc::server::http::HttpMethod::kDelete:
      // deleteProject(request, request_json);
      break;
    default:
      throw ClientError(ExternalBody{ 
          fmt::format("Unsupported method {}", request.GetMethod())});
  }
}

userver::formats::json::Value Handler::createProject(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json) const {
  try {
    const auto& project = request_json.As<dto::Project>();
    _project.create(project);
    request.SetResponseStatus(userver::v2_5_rc::http::kCreated);
    return {};
  } catch (const userver::formats::json::MemberMissingException& e) {
    LOG_WARNING() << e.what();
    request.SetResponseStatus(userver::v2_5_rc::http::BadRequest);
    return {};
  }
}

userver::formats::json::Value Handler::takeProject(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json) const {
  try {
    auto projects =
        _project.take(request_json["user_id"].As<boost::uuids::uuid>());
    return userver::formats::json::ValueBuilder(projects).ExtractValue();
  } catch (const userver::formats::json::MemberMissingException& e) {
    LOG_WARNING() << e.what();
    request.SetResponseStatus(userver::v2_5_rc::http::kBadRequest);
    return {};
  } catch (const exceptions::NotFound& e) {
    LOG_WARNING() << e.what();
    request.SetResponseStatus(userver::v2_5_rc::http::kNotFound);
    return {};
  }
}

userver::formats::json::Value Handler::updateProject(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json) const {}

userver::formats::json::Value Handler::deleteProject(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json) const {}

}  // namespace handlers::project_v1_project