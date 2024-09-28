#include "component.hpp"
#include <boost/uuid/uuid.hpp>
#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/enum_types.hpp>
#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/result_set.hpp>
#include "dto/project.hpp"
#include "exceptions/not_found.hpp"
#include "models/project.hpp"
#include "sql/queries.hpp"

namespace components {
Project::Project(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : userver::components::LoggableComponentBase(config, context),
      _pg_cluster(
          context.FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()) {}

void Project::create(const dto::Project& project) {
  _pg_cluster->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                       sql::kInsertProject, project.name, project.description);
}

std::vector<models::Project> Project::take(const boost::uuids::uuid& user_id) {
  userver::storages::postgres::ResultSet res = _pg_cluster->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      sql::kSelectProjectsByUserId, user_id);
  if (res.IsEmpty()) {
    throw exceptions::NotFound();
  }
  return res.AsContainer<std::vector<models::Project>>();
}

}  // namespace components