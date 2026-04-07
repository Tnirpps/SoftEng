SELECT uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
FROM directories
WHERE uuid = $1 AND owner_uuid = $2
