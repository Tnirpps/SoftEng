INSERT INTO directories (uuid, name, parent_uuid, owner_uuid, is_root)
VALUES (gen_random_uuid(), $1, $2::uuid, $3::uuid, $4)
RETURNING uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
