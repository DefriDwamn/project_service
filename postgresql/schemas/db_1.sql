-- DROP SCHEMA IF EXISTS hello_schema CASCADE;
-- CREATE SCHEMA IF NOT EXISTS hello_schema;
-- CREATE TABLE IF NOT EXISTS hello_schema.users (
--     name TEXT PRIMARY KEY,
--     count INTEGER DEFAULT(1)
-- );
--- 
DROP SCHEMA IF EXISTS project_schema;

CREATE SCHEMA IF NOT EXISTS project_schema;

DROP TYPE IF EXISTS project_schema.status;

CREATE TYPE project_schema.status AS ENUM (
    'not_started',
    'in_progress',
    'on_hold',
    'completed',
    'cancelled',
    'archived'
);

CREATE TABLE
    IF NOT EXISTS project_schema.projects (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid (),
        created_at TIMESTAMPTZ NOT NULL DEFAULT NOW (),
        name VARCHAR(15) NOT NULL UNIQUE,
        description TEXT,
        status project_schema.status NOT NULL DEFAULT 'not_started'
    );

CREATE TABLE
    IF NOT EXISTS project_schema.tasks (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid (),
        created_at TIMESTAMPTZ NOT NULL DEFAULT NOW (),
        name TEXT NOT NULL,
        summary TEXT NOT NULL,
        description TEXT,
        project_id UUID REFERENCES project_schema.projects (id) ON DELETE CASCADE,
        status project_schema.status NOT NULL DEFAULT 'not_started',
        user_id UUID,
        UNIQUE (name, user_id)
    );

CREATE TABLE
    IF NOT EXISTS project_schema.project_users (
        user_id UUID NOT NULL,
        project_id UUID NOT NULL,
        PRIMARY KEY (user_id, project_id),
        FOREIGN KEY (project_id) REFERENCES project_schema.projects (id) ON DELETE CASCADE
    );