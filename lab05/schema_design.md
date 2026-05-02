# Проектирование документной модели MongoDB для MaDisk

## Обзор системы

MaDisk - система управления файлами и директориями. В данной работе реализуется хранение метаданных файлов в MongoDB.

## Коллекции MongoDB

### 1. files

Основная коллекция для хранения метаданных файлов.

#### Структура документа:

```json
{
  "_id": ObjectId("..."),
  "id": "uuid-string",
  "name": "document.txt",
  "size": 1024,
  "mime_type": "text/plain",
  "directory_id": "uuid-string",
  "owner_id": "uuid-string",
  "created_at": ISODate("2024-01-01T00:00:00Z"),
  "updated_at": ISODate("2024-01-01T00:00:00Z"),
  "status": "available",
  "content": "file content here",
  "tags": ["important", "work"],
  "metadata": {
    "author": "John Doe",
    "version": 1
  }
}
```

#### Поля:

| Поле | Тип | Обязательное | Описание |
|------|-----|--------------|----------|
| _id | ObjectId | Да | Автоматический первичный ключ MongoDB |
| id | String | Да | UUID файла (бизнес-ключ) |
| name | String | Да | Имя файла |
| size | Number (Long) | Да | Размер в байтах |
| mime_type | String | Да | MIME-тип файла |
| directory_id | String | Нет | Ссылка на родительскую директорию |
| owner_id | String | Да | Владелец файла |
| created_at | Date | Да | Дата создания |
| updated_at | Date | Да | Дата обновления |
| status | String | Да | Статус (pending/scanning/available/infected) |
| content | String | Да | Содержимое файла |
| tags | Array | Нет | Теги для категоризации |
| metadata | Object | Нет | Дополнительные метаданные |

### 2. directories (опционально, для полноты модели)

Коллекция для хранения директорий.

#### Структура документа:

```json
{
  "_id": ObjectId("..."),
  "id": "uuid-string",
  "name": "Documents",
  "parent_id": "uuid-string",
  "owner_id": "uuid-string",
  "created_at": ISODate("2024-01-01T00:00:00Z"),
  "updated_at": ISODate("2024-01-01T00:00:00Z"),
  "is_root": true,
  "path": "/root/Documents"
}
```

## Выбор между Embedded Documents и References

### Решение: Использовать References для связей

#### Обоснование:

| Связь | Выбор | Обоснование |
|-------|-------|-------------|
| Files ↔ Directories | **Reference** | - Директории и файлы имеют независимый жизненный цикл<br>- Один файл может быть перемещен между директориями<br>- Избегание дублирования данных при обновлении директории<br>- Поддержка иерархической структуры директорий |
| Files ↔ Owners (Users) | **Reference** | - Пользователи хранятся в отдельном сервисе (PostgreSQL)<br>- Межсервисное взаимодействие через owner_id<br>- Нормализация данных между микросервисами |
| File metadata | **Embedded** | - Метаданные всегда запрашиваются вместе с файлом<br>- Небольшой размер<br>- Нет необходимости в отдельном поиске по метаданным |
| File tags | **Embedded Array** | - Теги - простая коллекция значений<br>- Частые операции добавления/удаления ($push, $pull)<br>- Нет необходимости в нормализации |

### Преимущества выбранного подхода:

1. **Гибкость**: Файлы можно легко перемещать между директориями
2. **Масштабируемость**: Коллекции можно шардировать независимо
3. **Целостность**: Ссылки обеспечивают консистентность при изменениях
4. **Производительность**: Индексы на directory_id и owner_id ускоряют поиск

### Индексы:

```javascript
// Индекс для поиска файлов в директории
db.files.createIndex({ directory_id: 1, owner_id: 1 })

// Индекс для поиска файлов владельца
db.files.createIndex({ owner_id: 1 })

// Индекс для поиска по имени файла
db.files.createIndex({ name: 1 })

// Индекс для полнотекстового поиска
db.files.createIndex({ name: "text", content: "text" })

// Индекс для сортировки по дате создания
db.files.createIndex({ created_at: -1 })
```

## Схема валидации

Для коллекции `files` создана валидация через `$jsonSchema`:

- Обязательные поля: id, name, size, mime_type, owner_id, status, content, created_at, updated_at
- Типы данных проверены для всех полей
- Ограничения: size >= 0, status из enum, name не пустой
- Паттерны для UUID полей

## Агрегации

Для сложных запросов используется aggregation pipeline:
- `$match` - фильтрация по условиям
- `$group` - группировка по владельцу/директории
- `$project` - проекция нужных полей
- `$sort` - сортировка результатов
- `$lookup` - соединение с коллекцией directories (при необходимости)
