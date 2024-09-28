#pragma once

#include <string_view>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_fwd.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include "dto/project.hpp"
#include "models/project.hpp"

namespace components {
class Project final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "component-project";
  Project(const userver::components::ComponentConfig& config,
          const userver::components::ComponentContext& context);

  void create(const dto::Project& project);
  std::vector<models::Project> take(const boost::uuids::uuid& user_id);

 private:
  userver::storages::postgres::ClusterPtr _pg_cluster;
};
}  // namespace components
