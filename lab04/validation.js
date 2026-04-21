// MongoDB Validation Script for MaDisk
// Этот скрипт создает валидацию схемы для коллекции files

// Подключение к MongoDB
// mongosh "mongodb://mongo:mongo@localhost:27017/admin"

// Переключение на базу данных
use admin;

// ============================================================================
// 1. Создание коллекции с валидацией схемы
// ============================================================================

db.createCollection("files", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: [
        "id",
        "name",
        "size",
        "mime_type",
        "owner_id",
        "created_at",
        "updated_at",
        "status",
        "content"
      ],
      additionalProperties: true,
      properties: {
        id: {
          bsonType: "string",
          description: "UUID файла (бизнес-ключ)",
          minLength: 1,
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"
        },
        name: {
          bsonType: "string",
          description: "Имя файла",
          minLength: 1,
          maxLength: 255,
          pattern: "^[^\\\\/:*?\"<>|]+$"
        },
        size: {
          bsonType: ["long", "int", "number"],
          description: "Размер файла в байтах",
          minimum: 0,
          maximum: 10737418240  // 10GB
        },
        mime_type: {
          bsonType: "string",
          description: "MIME-тип файла",
          minLength: 1,
          pattern: "^[a-z]+/[a-z0-9.+_-]+$"
        },
        directory_id: {
          bsonType: ["string", "null"],
          description: "Ссылка на родительскую директорию (UUID)"
        },
        owner_id: {
          bsonType: "string",
          description: "Владелец файла (UUID пользователя)",
          minLength: 1
        },
        created_at: {
          bsonType: "date",
          description: "Дата создания файла"
        },
        updated_at: {
          bsonType: "date",
          description: "Дата последнего обновления"
        },
        status: {
          bsonType: "string",
          description: "Статус файла",
          enum: ["pending", "scanning", "available", "infected"]
        },
        content: {
          bsonType: "string",
          description: "Содержимое файла"
        },
        tags: {
          bsonType: "array",
          description: "Теги для категоризации",
          items: {
            bsonType: "string",
            minLength: 1,
            maxLength: 50
          },
          uniqueItems: true
        },
        metadata: {
          bsonType: "object",
          description: "Дополнительные метаданные",
          properties: {
            author: {
              bsonType: "string",
              description: "Автор файла"
            },
            version: {
              bsonType: ["int", "number"],
              description: "Версия файла",
              minimum: 1
            }
          }
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("✓ Коллекция 'files' создана с валидацией схемы");

// ============================================================================
// 2. Создание коллекции directories с валидацией
// ============================================================================

db.createCollection("directories", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: [
        "id",
        "name",
        "owner_id",
        "created_at",
        "updated_at",
        "is_root"
      ],
      additionalProperties: true,
      properties: {
        id: {
          bsonType: "string",
          description: "UUID директории",
          minLength: 1,
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"
        },
        name: {
          bsonType: "string",
          description: "Имя директории",
          minLength: 1,
          maxLength: 255,
          pattern: "^[^\\\\/:*?\"<>|]+$"
        },
        parent_id: {
          bsonType: ["string", "null"],
          description: "Ссылка на родительскую директорию"
        },
        owner_id: {
          bsonType: "string",
          description: "Владелец директории",
          minLength: 1
        },
        created_at: {
          bsonType: "date",
          description: "Дата создания"
        },
        updated_at: {
          bsonType: "date",
          description: "Дата обновления"
        },
        is_root: {
          bsonType: "bool",
          description: "Флаг корневой директории"
        },
        path: {
          bsonType: "string",
          description: "Полный путь к директории"
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("✓ Коллекция 'directories' создана с валидацией схемы");

// ============================================================================
// 3. Создание индексов
// ============================================================================

// Индексы для коллекции files
db.files.createIndex({ id: 1 }, { unique: true });
db.files.createIndex({ owner_id: 1 });
db.files.createIndex({ directory_id: 1, owner_id: 1 });
db.files.createIndex({ name: 1 });
db.files.createIndex({ created_at: -1 });
db.files.createIndex({ status: 1 });
db.files.createIndex({ mime_type: 1 });
db.files.createIndex({ tags: 1 });
db.files.createIndex({ "metadata.author": 1 });

print("✓ Индексы для коллекции 'files' созданы");

// Индексы для коллекции directories
db.directories.createIndex({ id: 1 }, { unique: true });
db.directories.createIndex({ owner_id: 1 });
db.directories.createIndex({ parent_id: 1 });
db.directories.createIndex({ owner_id: 1, parent_id: 1 });

print("✓ Индексы для коллекции 'directories' созданы");

// ============================================================================
// 4. Тестирование валидации - Корректные данные
// ============================================================================

print("\n=== Тестирование корректных данных ===\n");

// Вставка корректного файла
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440100",
    name: "test.txt",
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    directory_id: "dir-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Test content",
    tags: ["test", "sample"],
    metadata: { author: "Test User", version: 1 }
  });
  print("✓ Успешная вставка корректного файла");
} catch (e) {
  print("✗ Ошибка вставки корректного файла: " + e.message);
}

// Вставка файла без directory_id (корневой файл)
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440101",
    name: "root_file.txt",
    size: 512,
    mime_type: "text/plain",
    owner_id: "user-001",
    directory_id: null,
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Root content"
  });
  print("✓ Успешная вставка корневого файла (directory_id: null)");
} catch (e) {
  print("✗ Ошибка вставки корневого файла: " + e.message);
}

// Вставка файла без tags и metadata (необязательные поля)
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440102",
    name: "minimal.txt",
    size: 256,
    mime_type: "text/plain",
    owner_id: "user-002",
    created_at: new Date(),
    updated_at: new Date(),
    status: "pending",
    content: "Minimal file"
  });
  print("✓ Успешная вставка файла без необязательных полей");
} catch (e) {
  print("✗ Ошибка вставки файла без необязательных полей: " + e.message);
}

// ============================================================================
// 5. Тестирование валидации - Некорректные данные
// ============================================================================

print("\n=== Тестирование некорректных данных (ожидаются ошибки) ===\n");

// Попытка вставки без обязательного поля 'name'
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440200",
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "No name"
  });
  print("✗ ОШИБКА: Вставка без 'name' должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: отсутствие обязательного поля 'name'");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с некорректным статусом
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440201",
    name: "bad_status.txt",
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "unknown_status",  // Недопустимое значение
    content: "Bad status"
  });
  print("✗ ОШИБКА: Вставка с некорректным статусом должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: некорректное значение status");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с отрицательным размером
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440202",
    name: "negative_size.txt",
    size: -100,  // Отрицательный размер
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Negative size"
  });
  print("✗ ОШИБКА: Вставка с отрицательным размером должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: отрицательный размер файла");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с некорректным UUID
try {
  db.files.insertOne({
    id: "invalid-uuid",  // Некорректный UUID
    name: "bad_uuid.txt",
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Bad UUID"
  });
  print("✗ ОШИБКА: Вставка с некорректным UUID должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: некорректный формат UUID");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с пустым именем
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440203",
    name: "",  // Пустое имя
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Empty name"
  });
  print("✗ ОШИБКА: Вставка с пустым именем должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: пустое имя файла");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с некорректным MIME-типом
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440204",
    name: "bad_mime.txt",
    size: 1024,
    mime_type: "invalid-mime-type",  // Некорректный MIME
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Bad MIME"
  });
  print("✗ ОШИБКА: Вставка с некорректным MIME-типом должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: некорректный MIME-тип");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с некорректным типом для created_at
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440205",
    name: "bad_date.txt",
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: "2024-01-01",  // Строка вместо Date
    updated_at: new Date(),
    status: "available",
    content: "Bad date"
  });
  print("✗ ОШИБКА: Вставка с некорректным типом даты должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: некорректный тип поля created_at");
  print("  Сообщение: " + e.message);
}

// Попытка вставки с недопустимыми символами в имени
try {
  db.files.insertOne({
    id: "550e8400-e29b-41d4-a716-446655440206",
    name: "file:name.txt",  // Недопустимый символ ':'
    size: 1024,
    mime_type: "text/plain",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "Invalid chars"
  });
  print("✗ ОШИБКА: Вставка с недопустимыми символами в имени должна была быть отклонена!");
} catch (e) {
  print("✓ Правильно отклонено: недопустимые символы в имени файла");
  print("  Сообщение: " + e.message);
}

// ============================================================================
// 6. Проверка созданных коллекций и индексов
// ============================================================================

print("\n=== Информация о коллекциях ===\n");

// Показать информацию о коллекции files
var filesInfo = db.getCollectionInfos({ name: "files" })[0];
print("Коллекция 'files':");
printjson(filesInfo.options.validator);

// Показать индексы files
print("\nИндексы коллекции 'files':");
db.files.getIndexes().forEach(function(idx) {
  print("  - " + idx.name + ": " + JSON.stringify(idx.key));
});

// Показать индексы directories
print("\nИндексы коллекции 'directories':");
db.directories.getIndexes().forEach(function(idx) {
  print("  - " + idx.name + ": " + JSON.stringify(idx.key));
});

// ============================================================================
// 7. Итоговая статистика
// ============================================================================

print("\n=== Итоговая статистика ===\n");
print("Файлов в коллекции 'files': " + db.files.countDocuments());
print("Директорий в коллекции 'directories': " + db.directories.countDocuments());

print("\n✓ Валидация схем успешно настроена и протестирована!");
