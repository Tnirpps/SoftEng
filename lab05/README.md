# MaDisk, ЛР5

В этой версии проекта мы занялись не новыми HTTP-методами, а поведением системы под нагрузкой. Для записи остался rate limiting, а для чтения добавили Redis-кэш. Идея простая: не ходить в базу каждый раз за одними и теми же данными, если их можно быстро отдать из памяти.

## Что теперь кешируется

- `GET /v1/directories/{directory_id}`
- `GET /v1/directories?parent_id=&limit=&offset=`
- `GET /v1/directories/{directory_id}/files`
- `GET /v1/files/{file_id}`

Кэш работает по схеме `Cache-Aside`: сначала сервис пробует взять готовый ответ из Redis, а если записи нет, читает данные из базы, формирует обычный ответ и кладет его в Redis на некоторое время.

Используем такие TTL:

- директория по id: `300` секунд
- список директорий: `60` секунд
- список файлов в папке: `30` секунд
- файл по id: `300` секунд

## Где и как инвалидируется кэш

С директориями все довольно прямолинейно:

- после `POST /v1/directories` сбрасывается кэш списка у родительской папки;
- после `PUT /v1/directories/{directory_id}` сбрасывается кэш самой директории и списка, в котором она находится;
- после `DELETE /v1/directories/{directory_id}` сбрасывается кэш директории, список родителя и кэш содержимого удаленной папки;
- после `POST /v1/directories/{directory_id}/move` сбрасывается кэш самой директории и списки у старого и нового родителя.

С файлами:

- после `PUT /v1/files/{file_id}` сбрасывается кэш файла и кэш списка файлов той папки, где он лежит;
- после `DELETE /v1/files/{file_id}` делается то же самое;
- после `POST /v1/files` сбрасывается кэш списка файлов родительской папки.

Для списков мы не удаляем все ключи вручную. Вместо этого используется version-key в Redis: когда данные меняются, версия увеличивается, и следующие запросы автоматически начинают писать и читать уже новые cache key. Это удобнее, чем перебирать все варианты `limit/offset`.

## Rate limiting

Rate limiting остался встроенным, средствами `userver`, без дополнительного middleware.

Лимиты такие:

- `POST /v1/users` — `2 req/s`
- `POST /v1/users/login` — `5 req/s`
- `POST /v1/directories` — `10 req/s`
- `PUT /v1/directories/{directory_id}` — `10 req/s`
- `DELETE /v1/directories/{directory_id}` — `10 req/s`
- `POST /v1/directories/{directory_id}/move` — `10 req/s`
- `POST /v1/files` — `10 req/s`
- `PUT /v1/files/{file_id}` — `10 req/s`
- `DELETE /v1/files/{file_id}` — `10 req/s`

Read-endpoints специально не ограничивались, потому что именно их мы сейчас стараемся ускорять через кэш.

## Что поменялось по инфраструктуре

- Redis подключен к `directory_service`;
- Redis подключен к `file_service`;
- оба сервиса используют один и тот же Redis, поэтому `file_service` может инвалидировать кэш списка файлов, который хранит `directory_service`;
- в конфиги добавлены `secdist` и TTL-параметры;
- в `docker-compose.yml` добавлен `redis` и зависимости сервисов от него.

## Запуск

Если стенд поднимается через Docker Compose, сервисы ожидаются такие:

- `user_service` — `localhost:8081`
- `directory_service` — `localhost:8082`
- `file_service` — `localhost:8083`
- `redis` — `localhost:6379`

Локально для Redis используются:

- [directory_service/configs/secure_data.json](/home/egortest/Desktop/MyProject/SoftEng/lab05/directory_service/configs/secure_data.json:1)
- [file_service/configs/secure_data.json](/home/egortest/Desktop/MyProject/SoftEng/lab05/file_service/configs/secure_data.json:1)

Для Docker:

- [directory_service/configs/secure_data.docker.json](/home/egortest/Desktop/MyProject/SoftEng/lab05/directory_service/configs/secure_data.docker.json:1)
- [file_service/configs/secure_data.docker.json](/home/egortest/Desktop/MyProject/SoftEng/lab05/file_service/configs/secure_data.docker.json:1)

## Что важно понимать

Это не финальная кэш-подсистема на весь проект, а уже рабочая база для нее. Redis теперь встроен сразу в оба сервиса, основные read-endpoints уже кешируются, а инвалидация сделана там, где она реально нужна для текущего набора операций.

Если потом захотим расширять это дальше, логика уже лежит в отдельных cache-компонентах, а не размазана по хендлерам.
