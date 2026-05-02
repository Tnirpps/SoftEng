# MaDisk - Лабораторная работа 05

В этой лабораторной работе для `lab05` реализован rate limiting средствами `userver` и первый Redis-кеш в `directory_service`. На текущем этапе Redis используется только для кеширования запроса содержимого папки.

## Что сделано

- Для `user_service` добавлены лимиты на:
  - `POST /v1/users` - `2 req/s`
  - `POST /v1/users/login` - `5 req/s`
- Для `directory_service` добавлены лимиты на mutating endpoints:
  - `POST /v1/directories` - `10 req/s`
  - `PUT /v1/directories/{directory_id}` - `10 req/s`
  - `DELETE /v1/directories/{directory_id}` - `10 req/s`
  - `POST /v1/directories/{directory_id}/move` - `10 req/s`
- Для `file_service` добавлены лимиты на mutating endpoints:
  - `POST /v1/files` - `10 req/s`
  - `PUT /v1/files/{file_id}` - `10 req/s`
  - `DELETE /v1/files/{file_id}` - `10 req/s`
- Read-endpoints не ограничивались, чтобы не ухудшать обычные сценарии чтения.
- В `directory_service` добавлен Redis и включен `Cache-Aside` для:
  - `GET /v1/directories/{directory_id}/files`
- Кешируются успешные ответы, включая пустой список файлов.
- TTL кеша списка файлов директории: `30s`.

## Почему выбраны именно эти endpoints

Rate limiting полезнее всего там, где запрос:

- создает нагрузку на базу данных или хранилище;
- изменяет состояние системы;
- может использоваться для brute force или flood-атак.

Поэтому в `user_service` ограничения наложены на регистрацию и логин, а в `directory_service` и `file_service` только на операции создания, обновления, удаления и перемещения.

Для Redis первым выбран именно endpoint содержимого папки, потому что это частый read-запрос, который напрямую нагружает PostgreSQL и хорошо подходит для простого начального кеширования без изменений HTTP API.

## Как это работает в userver

Используется встроенный механизм `userver` на уровне конфигурации handler'ов. Для нужных обработчиков в `static_config.yaml` задан параметр `max_requests_per_second`.

Если входящий поток запросов превышает допустимый RPS для конкретного handler'а, сервис начинает возвращать HTTP `429 Too Many Requests`. Дополнительный middleware или внешний limiter для этой лабораторной не использовались.

Сами значения лимитов вынесены в `config_vars`. Для тестового окружения заданы очень большие значения, чтобы limiter не ломал обычные интеграционные тесты. Реальные лимиты используются в docker-конфигах.

## Как работает Redis-кеш списка файлов

Для `GET /v1/directories/{directory_id}/files` используется `Cache-Aside`:

1. `directory_service` собирает ключ вида `dir-files:{owner_id}:{directory_id}:{limit}:{offset}`.
2. Сначала сервис пытается найти готовый JSON-ответ в Redis.
3. При cache hit ответ сразу возвращается клиенту.
4. При cache miss сервис читает данные из PostgreSQL, формирует обычный JSON-ответ и сохраняет его в Redis на `30s`.

На этом этапе межсервисной инвалидации нет. Если список файлов изменился через `file_service`, `directory_service` может отдавать устаревшие данные не дольше TTL.

## Запуск проекта

### Через Docker Compose

```bash
cd lab05
docker-compose up --build
```

Сервисы будут доступны на портах:

- `user_service` - `http://localhost:8081`
- `directory_service` - `http://localhost:8082`
- `file_service` - `http://localhost:8083`
- `redis` - `localhost:6379`

### Локальный запуск

```bash
cd lab05
python build.py configure
python build.py build
python build.py start user_service
python build.py start directory_service
python build.py start file_service
```

Для локального запуска Redis должен быть доступен на `localhost:6379`, так как `directory_service` читает Redis-конфиг из `directory_service/configs/secure_data.json`.

## Как проверить rate limiting

### Пример: регистрация пользователей

Ниже пример, который быстро отправляет 3 запроса в endpoint с лимитом `2 req/s`:

```bash
for i in 1 2 3; do
  curl -s -o /dev/null -w "%{http_code}\n" \
    -X POST http://localhost:8081/v1/users \
    -H "Content-Type: application/json" \
    -d "{\"login\":\"user$i\",\"password\":\"123456\",\"first_name\":\"Test\",\"last_name\":\"User\"}"
done
```

Ожидаемое поведение: первые запросы проходят с `201`, один из последующих получает `429`.

### Пример: логин

```bash
for i in 1 2 3 4 5 6; do
  curl -s -o /dev/null -w "%{http_code}\n" \
    -X POST http://localhost:8081/v1/users/login \
    -H "Content-Type: application/json" \
    -d '{"login":"testuser","password":"123456"}'
done
```

Для `login` установлен лимит `5 req/s`, поэтому при burst-нагрузке должен появиться `429`.

### Пример: создание файлов

```bash
for i in $(seq 1 11); do
  curl -s -o /dev/null -w "%{http_code}\n" \
    -X POST http://localhost:8083/v1/files \
    -H "Authorization: Bearer <JWT_TOKEN>" \
    -H "Content-Type: application/json" \
    -d "{\"name\":\"file_$i.txt\",\"content\":\"hello\"}"
done
```

Для `POST /v1/files` установлен лимит `10 req/s`, поэтому один из запросов при быстрой серии должен получить `429`.

### Пример: cold miss и hot hit для содержимого папки

Повторите один и тот же запрос дважды:

```bash
curl -s \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  "http://localhost:8082/v1/directories/<DIRECTORY_ID>/files?limit=20&offset=0"
```

Первый запрос заполнит Redis, второй будет обслужен из кеша. На этом этапе вручную удобнее всего проверять сам факт появления ключа в Redis:

```bash
redis-cli KEYS 'dir-files:*'
```

## Тесты

В testsuite rate limiting специально не проверяется: для тестового окружения выставлены очень большие значения `max_requests_per_second`, чтобы не получать ложные падения из-за накопленного состояния limiter'а между тестами.

Проверку `429` предполагается выполнять вручную через `curl` или в docker-стенде.

Для Redis добавлены functional tests в `directory_service`, которые проверяют:

- создание ключа после успешного `GET /v1/directories/{directory_id}/files`;
- раздельные ключи для разных `limit/offset`;
- отсутствие кеширования для `401` и `404`.

## Файлы лабораторной

- `performance_design.md` - анализ hot paths, rate limiting и первого Redis-кеша;
- исходный код сервисов с обновленными `static_config.yaml`;
- `Dockerfile` и `docker-compose.yml` для запуска проекта.
