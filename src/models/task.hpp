#pragma once
#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/storages/postgres/io/>
#include <userver/storages/postgres/io/enum_types.hpp>
#include <userver/utils/trivial_map.hpp>

namespace models {

enum class TaskStatus {
  kNotStarted,
  kInProgress,
  kOnHold,
  kCompleted,
  kCancelled,
  kArchived
};

struct Task {
  boost::uuids::uuid id;
  userver::storages::postgres::TimePointTz created_at;
  std::string name;
  std::string summary;
  std::string description;
  boost::uuids::uuid project_id;
  boost::uuids::uuid user_id;
  TaskStatus status;
};

template <class Value>
Value Serialize(const Task& data, userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder["id"] = data.id;
  builder["created_at"] = data.created_at;
  builder["name"] = data.name;
  builder["summary"] = data.summary;
  builder["description"] = data.description;
  builder["project_id"] = data.project_id;
  builder["user_id"] = data.user_id;
  builder["status"] = data.status;
  return builder.ExtractValue();
}

}  // namespace models

template <>
struct userver::storages::postgres::io::CppToUserPg<models::TaskStatus> {
  static constexpr DBTypeName postgres_name = "project_schema.status";
  static constexpr USERVER_NAMESPACE::utils::TrivialBiMap enumerators =
      [](auto selector) {
        return selector()
            .Case("not_started", models::TaskStatus::kNotStarted)
            .Case("in_progress", models::TaskStatus::kInProgress)
            .Case("on_hold", models::TaskStatus::kOnHold)
            .Case("completed", models::TaskStatus::kCompleted)
            .Case("cancelled", models::TaskStatus::kCancelled)
            .Case("archived", models::TaskStatus::kArchived);
      };
};
