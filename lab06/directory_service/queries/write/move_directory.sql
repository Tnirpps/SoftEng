UPDATE directories
SET parent_uuid = $1, is_root = $2, updated_at = NOW()
WHERE uuid = $3 AND owner_uuid = $4
RETURNING uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
