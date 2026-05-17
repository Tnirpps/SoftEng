# Каталог событий

Здесь описаны события, которые есть или планируются в MaDisk. В коде ЛР6 реализованы file-события, а directory/user-события оставлены как проектная часть каталога.

## FileCreated

Происходит после успешного создания файла.

- Producer: `file_service`
- Consumer: `directory_service`
- Routing key: `file.created`
- Гарантия доставки: `at-least-once`

Payload:

```json
{
  "event_id": "uuid",
  "event_type": "FileCreated",
  "occurred_at": "datetime",
  "file_id": "uuid",
  "owner_id": "uuid",
  "directory_id": "uuid|null",
  "name": "string"
}
```

## FileUpdated

Происходит после успешного изменения файла.

- Producer: `file_service`
- Consumer: `directory_service`
- Routing key: `file.updated`
- Гарантия доставки: `at-least-once`

Payload:

```json
{
  "event_id": "uuid",
  "event_type": "FileUpdated",
  "occurred_at": "datetime",
  "file_id": "uuid",
  "owner_id": "uuid",
  "directory_id": "uuid|null",
  "name": "string"
}
```

## FileDeleted

Происходит после успешного удаления файла.

- Producer: `file_service`
- Consumer: `directory_service`
- Routing key: `file.deleted`
- Гарантия доставки: `at-least-once`

Payload:

```json
{
  "event_id": "uuid",
  "event_type": "FileDeleted",
  "occurred_at": "datetime",
  "file_id": "uuid",
  "owner_id": "uuid",
  "directory_id": "uuid|null"
}
```

## DirectoryCreated

Происходит после создания директории.

- Producer: `directory_service`
- Routing key: `directory.created`
- Возможные consumers: audit/search-сервисы

Payload: `event_id`, `event_type`, `occurred_at`, `directory_id`, `owner_id`, `parent_id`, `name`.

## DirectoryUpdated

Происходит после переименования директории.

- Producer: `directory_service`
- Routing key: `directory.updated`
- Возможные consumers: audit/search-сервисы

Payload: `event_id`, `event_type`, `occurred_at`, `directory_id`, `owner_id`, `parent_id`, `name`.

## DirectoryMoved

Происходит после перемещения директории.

- Producer: `directory_service`
- Routing key: `directory.moved`
- Возможные consumers: audit/search-сервисы

Payload: `event_id`, `event_type`, `occurred_at`, `directory_id`, `owner_id`, `old_parent_id`, `new_parent_id`.

## DirectoryDeleted

Происходит после удаления директории.

- Producer: `directory_service`
- Routing key: `directory.deleted`
- Возможные consumers: audit/search-сервисы

Payload: `event_id`, `event_type`, `occurred_at`, `directory_id`, `owner_id`, `parent_id`, `recursive`.

## UserRegistered

Происходит после регистрации пользователя.

- Producer: `user_service`
- Routing key: `user.registered`
- Возможные consumers: audit/notification-сервисы

Payload: `event_id`, `event_type`, `occurred_at`, `user_id`, `login`.
