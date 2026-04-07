SELECT COUNT(*)::int as count
FROM directories
WHERE owner_uuid = $1
  AND (
    ($2::uuid IS NULL AND parent_uuid IS NULL)
    OR ($2::uuid IS NOT NULL AND parent_uuid = $2::uuid)
  )
