#pragma once
#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/formats/parse/boost_uuid.hpp>
#include <userver/formats/serialize/boost_uuid.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/formats/serialize/to.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/enum_types.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>
#include <userver/utils/trivial_map.hpp>

namespace models {

enum class ProjectStatus {
  kNotStarted,
  kInProgress,
  kOnHold,
  kCompleted,
  kCancelled,
  kArchived
};

struct Project {
  boost::uuids::uuid id;
  userver::storages::postgres::TimePointTz created_at;
  std::string name;
  std::string description;
  ProjectStatus status;
};

template <class Value>
Value Serialize(const Project& data, userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder["id"] = data.id;
  builder["created_at"] = data.created_at;
  builder["name"] = data.name;
  builder["description"] = data.description;
  builder["status"] = static_cast<int>(data.status);
  return builder.ExtractValue();
}

}  // namespace models

template <>
struct userver::storages::postgres::io::CppToUserPg<models::ProjectStatus> {
  static constexpr DBTypeName postgres_name = "project_schema.status";
  static constexpr USERVER_NAMESPACE::utils::TrivialBiMap enumerators =
      [](auto selector) {
        return selector()
            .Case("not_started", models::ProjectStatus::kNotStarted)
            .Case("in_progress", models::ProjectStatus::kInProgress)
            .Case("on_hold", models::ProjectStatus::kOnHold)
            .Case("completed", models::ProjectStatus::kCompleted)
            .Case("cancelled", models::ProjectStatus::kCancelled)
            .Case("archived", models::ProjectStatus::kArchived);
      };
};
