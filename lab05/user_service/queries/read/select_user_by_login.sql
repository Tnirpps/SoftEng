SELECT uuid, login, first_name, last_name, created_at
FROM users
WHERE login = $1 AND password = $2
