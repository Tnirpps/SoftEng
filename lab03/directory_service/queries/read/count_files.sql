SELECT COUNT(*)::int as count
FROM files
WHERE directory_uuid = $1 AND owner_uuid = $2
