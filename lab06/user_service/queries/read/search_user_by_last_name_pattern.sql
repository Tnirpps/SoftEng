SELECT uuid, login, first_name, last_name, created_at
FROM users
WHERE last_name ILIKE $1
LIMIT 1
