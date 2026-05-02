-- Users table schema optimized for high-load
CREATE TABLE IF NOT EXISTS users (
    uuid UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    login VARCHAR(20) NOT NULL,
    password VARCHAR(40) NOT NULL,
    first_name VARCHAR(20) NOT NULL,
    last_name VARCHAR(20) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT users_login_unique UNIQUE (login)
);

-- Index for login lookups (used in authentication)
CREATE INDEX IF NOT EXISTS idx_users_login ON users USING btree (login);

-- Index for last_name search (used in search functionality)
CREATE INDEX IF NOT EXISTS idx_users_last_name ON users USING btree (last_name);
