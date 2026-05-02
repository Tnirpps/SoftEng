# MaDisk - Лабораторная работа 05

В этой лабораторной работе для `lab05` реализован только rate limiting средствами `userver`. Кеширование сознательно отложено на следующую итерацию, чтобы сначала закрыть задачу защиты API от всплесков запросов.

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

## Почему выбраны именно эти endpoints

Rate limiting полезнее всего там, где запрос:

- создает нагрузку на базу данных или хранилище;
- изменяет состояние системы;
- может использоваться для brute force или flood-атак.

Поэтому в `user_service` ограничения наложены на регистрацию и логин, а в `directory_service` и `file_service` только на операции создания, обновления, удаления и перемещения.

## Как это работает в userver

Используется встроенный механизм `userver` на уровне конфигурации handler'ов. Для нужных обработчиков в `static_config.yaml` задан параметр `max_requests_per_second`.

Если входящий поток запросов превышает допустимый RPS для конкретного handler'а, сервис начинает возвращать HTTP `429 Too Many Requests`. Дополнительный middleware или внешний limiter для этой лабораторной не использовались.

Сами значения лимитов вынесены в `config_vars`. Для тестового окружения заданы очень большие значения, чтобы limiter не ломал обычные интеграционные тесты. Реальные лимиты используются в docker-конфигах.

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

### Локальный запуск

```bash
cd lab05
python build.py configure
python build.py build
python build.py start user_service
python build.py start directory_service
python build.py start file_service
```

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

## Тесты

В testsuite rate limiting специально не проверяется: для тестового окружения выставлены очень большие значения `max_requests_per_second`, чтобы не получать ложные падения из-за накопленного состояния limiter'а между тестами.

Проверку `429` предполагается выполнять вручную через `curl` или в docker-стенде.

## Файлы лабораторной

- `performance_design.md` - анализ hot paths и выбранной стратегии rate limiting;
- исходный код сервисов с обновленными `static_config.yaml`;
- `Dockerfile` и `docker-compose.yml` для запуска проекта.
