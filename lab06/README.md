# MaDisk, ЛР6

В этой версии проекта добавлена Event-Driven архитектура на RabbitMQ. Основной реализованный сценарий: `file_service` публикует события об изменении файлов, а `directory_service` потребляет эти события и обновляет read-модель списка файлов в директории через инвалидацию Redis-кэша.

## Что добавлено

- RabbitMQ в `docker-compose.yml`.
- Producer `file-event-publisher` в `file_service`.
- Consumer `file-events-consumer` в `directory_service`.
- Direct exchange `madisk.events`.
- Очередь `directory_service.file_events`.
- События `FileCreated`, `FileUpdated`, `FileDeleted`.
- Документация Event-Driven архитектуры и каталог событий.

## Поток событий

1. Клиент вызывает write-команду в `file_service`: создать, обновить или удалить файл.
2. `file_service` успешно меняет состояние файла в MongoDB.
3. `file_service` публикует событие в RabbitMQ exchange `madisk.events`.
4. RabbitMQ доставляет событие в очередь `directory_service.file_events`.
5. `directory_service` получает событие и инвалидирует Redis read-модель `GET /v1/directories/{directory_id}/files`.
6. Следующий read-запрос пересобирает актуальный список файлов.

## RabbitMQ

RabbitMQ Management UI доступен на:

- `http://localhost:15672`
- login: `guest`
- password: `guest`

AMQP endpoint:

- `localhost:5672`

Routing keys:

- `file.created`
- `file.updated`
- `file.deleted`

## Запуск

```bash
cd lab06
docker compose up --build
```

Ожидаемые HTTP-порты:

- `user_service` - `localhost:8081`
- `directory_service` - `localhost:8082`
- `file_service` - `localhost:8083`
- RabbitMQ Management - `localhost:15672`

## Ручная проверка

1. Зарегистрировать пользователя через `POST /v1/users`.
2. Выполнить login через `POST /v1/users/login` и получить JWT.
3. Создать директорию через `POST /v1/directories`.
4. Создать файл в этой директории через `POST /v1/files`.
5. Проверить в логах `file_service`, что событие опубликовано.
6. Проверить в логах `directory_service`, что событие получено и read-модель списка файлов инвалидирована.

## Документация

- [event_driven_design.md](/home/egortest/Desktop/MyProject/SoftEng/lab06/event_driven_design.md:1) - описание Event-Driven архитектуры, RabbitMQ и CQRS.
- [event_catalog.md](/home/egortest/Desktop/MyProject/SoftEng/lab06/event_catalog.md:1) - каталог событий системы.
