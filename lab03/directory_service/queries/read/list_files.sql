SELECT uuid, name, size, mime_type, directory_uuid, owner_uuid, created_at, updated_at, status
FROM files
WHERE directory_uuid = $1 AND owner_uuid = $2
ORDER BY created_at
LIMIT $3 OFFSET $4
