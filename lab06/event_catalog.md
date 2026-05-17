# Event Catalog

## FileCreated

- Routing key: `file.created`
- Producer: `file_service`
- Consumers: `directory_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `CreateFile`
- Payload:

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

Consumer behavior: `directory_service` инвалидирует read-модель списка файлов директории.

## FileUpdated

- Routing key: `file.updated`
- Producer: `file_service`
- Consumers: `directory_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `UpdateFile`
- Payload:

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

Consumer behavior: `directory_service` инвалидирует read-модель списка файлов директории.

## FileDeleted

- Routing key: `file.deleted`
- Producer: `file_service`
- Consumers: `directory_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `DeleteFile`
- Payload:

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

Consumer behavior: `directory_service` инвалидирует read-модель списка файлов директории.

## DirectoryCreated

- Routing key: `directory.created`
- Producer: `directory_service`
- Consumers: future `audit_service`, `search_service`, notification handlers
- Delivery guarantee: `at-least-once`
- Trigger command: `CreateDirectory`
- Payload:

```json
{
  "event_id": "uuid",
  "event_type": "DirectoryCreated",
  "occurred_at": "datetime",
  "directory_id": "uuid",
  "owner_id": "uuid",
  "parent_id": "uuid|null",
  "name": "string"
}
```

## DirectoryUpdated

- Routing key: `directory.updated`
- Producer: `directory_service`
- Consumers: future `audit_service`, `search_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `UpdateDirectory`
- Payload:

```json
{
  "event_id": "uuid",
  "event_type": "DirectoryUpdated",
  "occurred_at": "datetime",
  "directory_id": "uuid",
  "owner_id": "uuid",
  "parent_id": "uuid|null",
  "name": "string"
}
```

## DirectoryMoved

- Routing key: `directory.moved`
- Producer: `directory_service`
- Consumers: future `audit_service`, `search_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `MoveDirectory`
- Payload:

```json
{
  "event_id": "uuid",
  "event_type": "DirectoryMoved",
  "occurred_at": "datetime",
  "directory_id": "uuid",
  "owner_id": "uuid",
  "old_parent_id": "uuid|null",
  "new_parent_id": "uuid|null"
}
```

## DirectoryDeleted

- Routing key: `directory.deleted`
- Producer: `directory_service`
- Consumers: future `audit_service`, `search_service`
- Delivery guarantee: `at-least-once`
- Trigger command: `DeleteDirectory`
- Payload:

```json
{
  "event_id": "uuid",
  "event_type": "DirectoryDeleted",
  "occurred_at": "datetime",
  "directory_id": "uuid",
  "owner_id": "uuid",
  "parent_id": "uuid|null",
  "recursive": "boolean"
}
```

## UserRegistered

- Routing key: `user.registered`
- Producer: `user_service`
- Consumers: future `audit_service`, notification handlers
- Delivery guarantee: `at-least-once`
- Trigger command: `RegisterUser`
- Payload:

```json
{
  "event_id": "uuid",
  "event_type": "UserRegistered",
  "occurred_at": "datetime",
  "user_id": "uuid",
  "login": "string"
}
```
