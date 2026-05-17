SELECT uuid, name, parent_uuid, owner_uuid, created_at, updated_at, is_root
FROM directories
WHERE owner_uuid = $1
  AND (
    ($2::uuid IS NULL AND parent_uuid IS NULL)
    OR ($2::uuid IS NOT NULL AND parent_uuid = $2::uuid)
  )
ORDER BY created_at
LIMIT $3 OFFSET $4
