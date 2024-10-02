#pragma once

#include <userver/storages/postgres/query.hpp>
#include <userver/storages/query.hpp>
namespace sql {

const userver::storages::postgres::Query kInsertProject{
    R"~(
    INSERT INTO project_schema.projects(name,description)
    VALUES($1,$2)
)~",
    userver::storages::postgres::Query::Name{"insert_project"}};

const userver::storages::postgres::Query kSelectProjectsByUserId{
    R"~(
    SELECT p.id, p.created_at, p.name, p.description, p.status
    FROM project_schema.projects p
    JOIN project_schema.project_users pu ON pu.project_id = p.id
    WHERE pu.user_id=$1
)~",
    userver::storages::postgres::Query::Name{"select_projects_by_user_id"}};
} // namespace sql