-- Directories table schema
CREATE TABLE IF NOT EXISTS directories (
    uuid UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL,
    parent_uuid UUID REFERENCES directories(uuid) ON DELETE CASCADE,
    owner_uuid UUID NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    is_root BOOLEAN NOT NULL DEFAULT false,
    CONSTRAINT directories_owner_parent_name_unique UNIQUE (owner_uuid, parent_uuid, name)
);

-- Index for parent lookups (used in list operations)
CREATE INDEX IF NOT EXISTS idx_directories_parent ON directories USING btree (parent_uuid);

-- Index for owner lookups (used in filtering by owner)
CREATE INDEX IF NOT EXISTS idx_directories_owner ON directories USING btree (owner_uuid);

-- Index for owner and parent combination (used in list operations)
CREATE INDEX IF NOT EXISTS idx_directories_owner_parent ON directories USING btree (owner_uuid, parent_uuid);

-- Files table schema
CREATE TABLE IF NOT EXISTS files (
    uuid UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL,
    size BIGINT NOT NULL,
    mime_type VARCHAR(255) NOT NULL,
    directory_uuid UUID NOT NULL REFERENCES directories(uuid) ON DELETE CASCADE,
    owner_uuid UUID NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    status VARCHAR(50) NOT NULL DEFAULT 'pending',
    CONSTRAINT files_status_check CHECK (status IN ('pending', 'scanning', 'available', 'infected'))
);

-- Index for directory lookups (used in list files operations)
CREATE INDEX IF NOT EXISTS idx_files_directory ON files USING btree (directory_uuid);

-- Index for owner lookups (used in filtering by owner)
CREATE INDEX IF NOT EXISTS idx_files_owner ON files USING btree (owner_uuid);

-- Index for directory and owner combination
CREATE INDEX IF NOT EXISTS idx_files_directory_owner ON files USING btree (directory_uuid, owner_uuid);
