# Event-Driven архитектура MaDisk

## Контекст

MaDisk состоит из трех сервисов:

- `user_service` управляет пользователями и аутентификацией.
- `directory_service` управляет деревом директорий и read-моделью списка файлов в директории.
- `file_service` управляет файлами и хранит их данные в MongoDB.

В ЛР6 в систему добавлен брокер RabbitMQ. Основной реализованный поток: изменения файлов публикуются как события, а `directory_service` получает эти события и синхронизирует read-модель, которая используется для быстрых read-запросов.

## Команды и события

Команды - это write-операции, которые меняют состояние системы:

- `CreateFile` - HTTP `POST /v1/files`
- `UpdateFile` - HTTP `PUT /v1/files/{file_id}`
- `DeleteFile` - HTTP `DELETE /v1/files/{file_id}`
- `CreateDirectory` - HTTP `POST /v1/directories`
- `UpdateDirectory` - HTTP `PUT /v1/directories/{directory_id}`
- `MoveDirectory` - HTTP `POST /v1/directories/{directory_id}/move`
- `DeleteDirectory` - HTTP `DELETE /v1/directories/{directory_id}`
- `RegisterUser` - HTTP `POST /v1/users`

События - это уже произошедшие факты:

- `FileCreated`
- `FileUpdated`
- `FileDeleted`
- `DirectoryCreated`
- `DirectoryUpdated`
- `DirectoryMoved`
- `DirectoryDeleted`
- `UserRegistered`

В коде ЛР6 реализованы file-события. Directory/user-события описаны в каталоге как часть целевой событийной модели проекта.

## RabbitMQ

Для обмена событиями используется RabbitMQ.

- Exchange: `madisk.events`
- Тип exchange: `direct`
- Очередь consumer: `directory_service.file_events`
- Routing keys:
  - `file.created`
  - `file.updated`
  - `file.deleted`

`file_service` является producer для file-событий. После успешной записи в MongoDB он публикует событие в `madisk.events`.

`directory_service` является consumer. Он слушает очередь `directory_service.file_events`, привязанную к file routing keys, и при получении события инвалидирует read-модель списка файлов директории.

## Формат сообщения

Сообщения передаются в JSON.

```json
{
  "event_id": "uuid",
  "event_type": "FileCreated",
  "occurred_at": "2026-05-17T12:00:00+0000",
  "file_id": "uuid",
  "owner_id": "uuid",
  "directory_id": "uuid",
  "name": "report.pdf"
}
```

Для `FileDeleted` поле `name` не обязательно. Если файл находится вне директории, `directory_id` передается как `null`, и consumer не инвалидирует список файлов директории.

## Гарантии доставки

Выбрана гарантия `at-least-once`.

- Producer использует reliable publish.
- Consumer должен быть готов к повторной доставке сообщения.
- Обработка file-событий идемпотентна: повторная инвалидация Redis read-модели не меняет бизнес-состояние и безопасна.

Exactly-once не используется, потому что для текущей задачи она избыточна: событие не создает новые бизнес-сущности в consumer, а только сбрасывает кэш.

## CQRS

CQRS применим к MaDisk, потому что write-операции и read-операции имеют разный профиль нагрузки.

Write-модель:

- `file_service` пишет файлы в MongoDB.
- `directory_service` пишет директории в PostgreSQL.
- `user_service` пишет пользователей в PostgreSQL.

Read-модель:

- `directory_service` хранит кэшированные ответы списков директорий и файлов в Redis.
- `file_service` хранит кэшированные ответы файлов в Redis.

Синхронизация:

- После изменения файла `file_service` публикует событие.
- `directory_service` получает событие и инвалидирует read-модель `GET /v1/directories/{directory_id}/files`.
- Следующий read-запрос пересобирает актуальный ответ из базы и снова кладет его в Redis.

Такой подход отделяет запись в бизнес-хранилище от обновления read-модели и снижает связанность сервисов.
