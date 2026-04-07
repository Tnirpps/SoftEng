SELECT uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
FROM directories
WHERE owner_uuid = $1
  AND name = $2
  AND (
    ($3::uuid IS NULL AND parent_uuid IS NULL)
    OR ($3::uuid IS NOT NULL AND parent_uuid = $3::uuid)
  )
