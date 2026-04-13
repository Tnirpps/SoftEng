UPDATE directories
SET name = $1, updated_at = NOW()
WHERE uuid = $2 AND owner_uuid = $3
RETURNING uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
