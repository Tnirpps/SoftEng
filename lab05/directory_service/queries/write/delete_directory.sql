DELETE FROM directories
WHERE uuid = $1 AND owner_uuid = $2
