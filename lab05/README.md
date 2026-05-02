# MaDisk - MongoDB Lab (Лабораторная работа №4)

Система управления файлами и директориями с использованием MongoDB.

## Файлы работы

| Файл | Описание |
|------|----------|
| [`schema_design.md`](./schema_design.md) | Проектирование документной модели с обоснованием embedded/references |
| [`data.json`](./data.json) | Тестовые данные (12 файлов, 5 директорий) |
| [`queries.md`](./queries.md) | MongoDB запросы для CRUD операций |
| [`validation.js`](./validation.js) | Валидация схем и тестирование |

## Быстрый старт

### 1. Запуск MongoDB

```bash
cd lab04
docker-compose up -d mongo
```

### 2. Подключение к MongoDB

```bash
mongosh "mongodb://mongo:mongo@localhost:27017/admin"
```

### 3. Создание коллекции с валидацией

```bash
mongosh "mongodb://mongo:mongo@localhost:27017/admin" validation.js
```

### 4. Загрузка тестовых данных

```javascript
// В mongosh
use admin

// Загрузка файлов
db.files.insertMany([... данные из data.json ...])
```

## Структура коллекций

### files

```json
{
  "id": "uuid-string",
  "name": "file.txt",
  "size": 1024,
  "mime_type": "text/plain",
  "directory_id": "uuid-or-null",
  "owner_id": "uuid-string",
  "created_at": ISODate(),
  "updated_at": ISODate(),
  "status": "available",
  "content": "string",
  "tags": ["array"],
  "metadata": { "object" }
}
```

### directories

```json
{
  "id": "uuid-string",
  "name": "Documents",
  "parent_id": "uuid-or-null",
  "owner_id": "uuid-string",
  "created_at": ISODate(),
  "updated_at": ISODate(),
  "is_root": true,
  "path": "/Documents"
}
```

## Валидация схем

Валидация включает:
- Обязательные поля: id, name, size, mime_type, owner_id, status, content, created_at, updated_at
- Типы данных: string, number, date, array, object
- Ограничения: size >= 0, status enum, UUID pattern
- Паттерны для имен файлов

## CRUD операции

Все операции описаны в [`queries.md`](./queries.md):

- **Create**: `insertOne()`, `insertMany()`
- **Read**: `find()`, `findOne()` с операторами `$eq`, `$ne`, `$gt`, `$lt`, `$in`, `$and`, `$or`
- **Update**: `updateOne()`, `updateMany()` с `$set`, `$inc`, `$push`, `$pull`, `$addToSet`
- **Delete**: `deleteOne()`, `deleteMany()`

## Агрегации

Примеры aggregation pipeline:
- `$match` - фильтрация
- `$group` - группировка
- `$project` - проекция
- `$sort` - сортировка
- `$lookup` - соединение коллекций

## Индексы

```javascript
db.files.createIndex({ owner_id: 1 });
db.files.createIndex({ directory_id: 1, owner_id: 1 });
db.files.createIndex({ created_at: -1 });
db.files.createIndex({ name: "text", content: "text" });
```

## Docker Compose

MongoDB настроен в `docker-compose.yml`:

```yaml
mongo:
  image: mongo:7
  environment:
    - MONGO_INITDB_ROOT_USERNAME=mongo
    - MONGO_INITDB_ROOT_PASSWORD=mongo
  ports:
    - "27017:27017"
```
