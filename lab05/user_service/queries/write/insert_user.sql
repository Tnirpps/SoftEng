INSERT INTO users (login, password, first_name, last_name)
VALUES ($1, $2, $3, $4)
RETURNING uuid, login, first_name, last_name, created_at
