# MongoDB запросы для MaDisk

## Подключение к MongoDB

```bash
# Подключение через mongosh
mongosh "mongodb://mongo:mongo@localhost:27017/admin"
```

## 1. CREATE (Вставка документов)

### 1.1 Вставка одного документа

```javascript
// Вставка файла
db.files.insertOne({
  id: "550e8400-e29b-41d4-a716-446655440013",
  name: "document.txt",
  size: 2048,
  mime_type: "text/plain",
  directory_id: "dir-001",
  owner_id: "user-001",
  created_at: new Date(),
  updated_at: new Date(),
  status: "available",
  content: "File content here",
  tags: ["work", "document"],
  metadata: { author: "John Doe", version: 1 }
});
```

### 1.2 Вставка нескольких документов

```javascript
// Массовая вставка файлов
db.files.insertMany([
  {
    id: "550e8400-e29b-41d4-a716-446655440014",
    name: "image1.png",
    size: 1048576,
    mime_type: "image/png",
    directory_id: "dir-002",
    owner_id: "user-001",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "PNG data...",
    tags: ["personal", "images"]
  },
  {
    id: "550e8400-e29b-41d4-a716-446655440015",
    name: "data.csv",
    size: 51200,
    mime_type: "text/csv",
    directory_id: "dir-001",
    owner_id: "user-002",
    created_at: new Date(),
    updated_at: new Date(),
    status: "available",
    content: "col1,col2\nval1,val2",
    tags: ["work", "data"]
  }
]);
```

### 1.3 Вставка с использованием операторов массива

```javascript
// Вставка файла с тегами через $each
db.files.insertOne({
  id: "550e8400-e29b-41d4-a716-446655440016",
  name: "project.zip",
  size: 5242880,
  mime_type: "application/zip",
  directory_id: "dir-003",
  owner_id: "user-003",
  created_at: new Date(),
  updated_at: new Date(),
  status: "pending",
  content: "ZIP data...",
  tags: ["project", "archive", "important"]
});
```

---

## 2. READ (Поиск документов)

### 2.1 Поиск всех документов

```javascript
// Получить все файлы
db.files.find();

// Получить все файлы с проекцией полей
db.files.find({}, { _id: 0, id: 1, name: 1, size: 1 });
```

### 2.2 Поиск с точным совпадением ($eq)

```javascript
// Найти файл по ID
db.files.findOne({ id: "550e8400-e29b-41d4-a716-446655440001" });

// Найти файлы владельца
db.files.find({ owner_id: "user-001" });
```

### 2.3 Поиск с неравенством ($ne)

```javascript
// Найти все файлы кроме доступных
db.files.find({ status: { $ne: "available" } });

// Найти файлы без директории (корневые)
db.files.find({ directory_id: null });
```

### 2.4 Поиск с числовыми операторами ($gt, $lt, $gte, $lte)

```javascript
// Найти файлы больше 1MB
db.files.find({ size: { $gt: 1048576 } });

// Найти файлы меньше 10KB
db.files.find({ size: { $lt: 10240 } });

// Найти файлы от 1KB до 1MB
db.files.find({ 
  size: { $gte: 1024, $lte: 1048576 } 
});
```

### 2.5 Поиск с $in и $nin

```javascript
// Найти файлы с определенными статусами
db.files.find({ 
  status: { $in: ["available", "scanning"] } 
});

// Найти файлы с MIME-типами
db.files.find({ 
  mime_type: { $in: ["text/plain", "application/json", "text/markdown"] } 
});

// Найти файлы НЕ из списка типов
db.files.find({ 
  mime_type: { $nin: ["application/zip", "video/mp4"] } 
});
```

### 2.6 Поиск с логическими операторами ($and, $or, $not, $nor)

```javascript
// $and - файлы больше 1MB И владельца user-001
db.files.find({
  $and: [
    { size: { $gt: 1048576 } },
    { owner_id: "user-001" }
  ]
});

// $or - файлы PDF ИЛИ файлы больше 5MB
db.files.find({
  $or: [
    { mime_type: "application/pdf" },
    { size: { $gt: 5242880 } }
  ]
});

// Комбинированный запрос: (PDF ИЛИ Images) И владелец user-001
db.files.find({
  $and: [
    {
      $or: [
        { mime_type: "application/pdf" },
        { mime_type: { $in: ["image/jpeg", "image/png"] } }
      ]
    },
    { owner_id: "user-001" }
  ]
});

// $not - файлы где имя НЕ начинается с "test"
db.files.find({
  name: { $not: /^test/ }
});
```

### 2.7 Поиск по массивам

```javascript
// Найти файлы с тегом "work"
db.files.find({ tags: "work" });

// Найти файлы с несколькими тегами (все теги должны совпасть)
db.files.find({ tags: { $all: ["work", "important"] } });

// Найти файлы с любым из тегов
db.files.find({ tags: { $in: ["work", "personal"] } });

// Найти файлы с точным набором тегов
db.files.find({ tags: { $eq: ["code", "javascript"] } });
```

### 2.8 Поиск по вложенным полям

```javascript
// Найти файлы по метаданным
db.files.find({ "metadata.author": "John Doe" });

// Найти файлы с определенной версией
db.files.find({ "metadata.version": 1 });
```

### 2.9 Регулярные выражения

```javascript
// Найти файлы с именем, содержащим "report"
db.files.find({ name: { $regex: "report", $options: "i" } });

// Найти файлы с расширением .txt
db.files.find({ name: { $regex: /\.txt$/ } });

// Найти файлы, имя которых начинается с "data"
db.files.find({ name: { $regex: /^data/ } });
```

### 2.10 Сортировка, лимит и смещение

```javascript
// Сортировка по имени (возрастание)
db.files.find().sort({ name: 1 });

// Сортировка по размеру (убывание)
db.files.find().sort({ size: -1 });

// Сортировка по дате создания
db.files.find({ owner_id: "user-001" }).sort({ created_at: -1 });

// Лимит результатов
db.files.find().limit(5);

// Смещение (пагинация)
db.files.find().skip(10).limit(5);

// Комбинированный запрос с сортировкой и лимитом
db.files.find({ owner_id: "user-001" })
  .sort({ created_at: -1 })
  .limit(10)
  .skip(0);
```

---

## 3. UPDATE (Обновление документов)

### 3.1 Обновление одного документа ($set)

```javascript
// Обновить имя файла
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $set: { name: "new_name.txt", updated_at: new Date() } }
);
```

### 3.2 Обновление нескольких документов

```javascript
// Обновить статус всех файлов владельца
db.files.updateMany(
  { owner_id: "user-001", status: "pending" },
  { $set: { status: "available", updated_at: new Date() } }
);
```

### 3.3 Операции с числовыми полями ($inc, $mul, $min, $max)

```javascript
// Увеличить размер (например, после добавления данных)
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $inc: { size: 100 } }
);

// Умножить значение (редко используется для файлов)
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $mul: { size: 2 } }
);

// Установить минимальное значение
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $min: { size: 1024 } }
);

// Установить максимальное значение
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $max: { size: 10485760 } }
);
```

### 3.4 Операции с массивами ($push, $pull, $addToSet, $pop)

```javascript
// $push - добавить тег в массив
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $push: { tags: "urgent" } }
);

// $push с $each - добавить несколько тегов
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $push: { tags: { $each: ["important", "review"] } } }
);

// $push с $position - добавить тег в начало массива
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $push: { tags: { $each: ["new"], $position: 0 } } }
);

// $addToSet - добавить тег только если его нет (уникальность)
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $addToSet: { tags: "work" } }
);

// $pull - удалить тег из массива
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $pull: { tags: "urgent" } }
);

// $pull с условием - удалить теги по шаблону
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $pull: { tags: { $regex: /^temp/ } } }
);

// $pop - удалить последний элемент массива
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { $pop: { tags: 1 } }  // 1 = последний, -1 = первый
);
```

### 3.5 Обновление вложенных полей

```javascript
// Обновить метаданные
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  { 
    $set: { 
      "metadata.author": "Jane Smith",
      "metadata.version": 2,
      "metadata.last_modified_by": "user-002"
    } 
  }
);
```

### 3.6 Переписывание документа (замена)

```javascript
// Полная замена документа (кроме _id)
db.files.replaceOne(
  { id: "550e8400-e29b-41d4-a716-446655440001" },
  {
    id: "550e8400-e29b-41d4-a716-446655440001",
    name: "completely_new.txt",
    size: 4096,
    mime_type: "text/plain",
    directory_id: "dir-001",
    owner_id: "user-001",
    created_at: new Date("2024-01-01T00:00:00Z"),
    updated_at: new Date(),
    status: "available",
    content: "New content",
    tags: ["new"],
    metadata: {}
  }
);
```

### 3.7 Обновление с условием ($[])

```javascript
// Обновить все элементы массива, удовлетворяющие условию
db.files.updateOne(
  { id: "550e8400-e29b-41d4-a716-446655440001", tags: { $in: ["work"] } },
  { $set: { "tags.$[elem]": "WORK" } },
  { arrayFilters: [{ elem: "work" }] }
);
```

---

## 4. DELETE (Удаление документов)

### 4.1 Удаление одного документа

```javascript
// Удалить файл по ID
db.files.deleteOne({ id: "550e8400-e29b-41d4-a716-446655440001" });

// Удалить файл по имени и владельцу
db.files.deleteOne({ 
  name: "old_file.txt", 
  owner_id: "user-001" 
});
```

### 4.2 Удаление нескольких документов

```javascript
// Удалить все файлы владельца
db.files.deleteMany({ owner_id: "user-002" });

// Удалить все файлы со статусом "infected"
db.files.deleteMany({ status: "infected" });

// Удалить все файлы из определенной директории
db.files.deleteMany({ directory_id: "dir-001" });

// Удалить файлы по复合ному условию
db.files.deleteMany({
  $and: [
    { status: "pending" },
    { created_at: { $lt: new Date("2024-01-01T00:00:00Z") } }
  ]
});
```

### 4.3 Удаление всех документов (очистка коллекции)

```javascript
// Удалить все файлы
db.files.deleteMany({});
```

---

## 5. Агрегации (Aggregation Pipeline)

### 5.1 Базовый pipeline

```javascript
// Группировка файлов по владельцу с подсчетом количества и общего размера
db.files.aggregate([
  {
    $group: {
      _id: "$owner_id",
      fileCount: { $sum: 1 },
      totalSize: { $sum: "$size" },
      avgSize: { $avg: "$size" }
    }
  },
  {
    $sort: { fileCount: -1 }
  }
]);
```

### 5.2 Pipeline с $match, $group, $project, $sort

```javascript
// Статистика по файлам каждого владельца
db.files.aggregate([
  // Фильтрация только доступных файлов
  {
    $match: { status: "available" }
  },
  // Группировка по владельцу и директории
  {
    $group: {
      _id: { 
        owner: "$owner_id", 
        directory: "$directory_id" 
      },
      count: { $sum: 1 },
      totalSize: { $sum: "$size" },
      minSize: { $min: "$size" },
      maxSize: { $max: "$size" },
      files: { $push: { name: "$name", size: "$size" } }
    }
  },
  // Фильтрация групп с количеством > 1
  {
    $match: { count: { $gt: 1 } }
  },
  // Проекция финальных полей
  {
    $project: {
      _id: 0,
      owner: "$_id.owner",
      directory: "$_id.directory",
      fileCount: "$count",
      totalSize: "$totalSize",
      averageSize: { $round: [{ $divide: ["$totalSize", "$count"] }, 2] },
      files: 1
    }
  },
  // Сортировка по общему размеру
  {
    $sort: { totalSize: -1 }
  },
  // Ограничение результатов
  {
    $limit: 10
  }
]);
```

### 5.3 Агрегация с $lookup (соединение с directories)

```javascript
// Получить файлы с информацией о директории
db.files.aggregate([
  {
    $lookup: {
      from: "directories",
      localField: "directory_id",
      foreignField: "id",
      as: "directory"
    }
  },
  {
    $unwind: {
      path: "$directory",
      preserveNullAndEmptyArrays: true
    }
  },
  {
    $project: {
      _id: 0,
      fileId: "$id",
      fileName: "$name",
      fileSize: "$size",
      directoryName: "$directory.name",
      directoryPath: "$directory.path",
      owner: "$owner_id"
    }
  }
]);
```

### 5.4 Агрегация с группировкой по MIME-типам

```javascript
// Статистика по типам файлов
db.files.aggregate([
  {
    $group: {
      _id: "$mime_type",
      count: { $sum: 1 },
      totalSize: { $sum: "$size" },
      files: { $push: "$name" }
    }
  },
  {
    $project: {
      _id: 0,
      mimeType: "$_id",
      count: 1,
      totalSize: 1,
      averageSize: { $round: [{ $divide: ["$totalSize", "$count"] }, 0] },
      sampleFiles: { $slice: ["$files", 3] }
    }
  },
  {
    $sort: { count: -1 }
  }
]);
```

### 5.5 Агрегация с $facet (множественные результаты)

```javascript
// Получить несколько агрегаций в одном запросе
db.files.aggregate([
  {
    $facet: {
      "byStatus": [
        { $group: { _id: "$status", count: { $sum: 1 } } }
      ],
      "byOwner": [
        { $group: { _id: "$owner_id", count: { $sum: 1 }, totalSize: { $sum: "$size" } } }
      ],
      "largestFiles": [
        { $sort: { size: -1 } },
        { $limit: 5 },
        { $project: { _id: 0, name: 1, size: 1, owner_id: 1 } }
      ]
    }
  }
]);
```

---

## 6. Работа с индексами

### 6.1 Создание индексов

```javascript
// Индекс для поиска по владельцу
db.files.createIndex({ owner_id: 1 });

// Составной индекс для поиска файлов в директории
db.files.createIndex({ directory_id: 1, owner_id: 1 });

// Индекс для сортировки по дате
db.files.createIndex({ created_at: -1 });

// Текстовый индекс для полнотекстового поиска
db.files.createIndex({ name: "text", content: "text" });

// Уникальный индекс на бизнес-ключ
db.files.createIndex({ id: 1 }, { unique: true });
```

### 6.2 Просмотр индексов

```javascript
// Показать все индексы коллекции
db.files.getIndexes();
```

### 6.3 Удаление индексов

```javascript
// Удалить индекс по имени
db.files.dropIndex("owner_id_1");

// Удалить все индексы кроме _id
db.files.dropIndexes();
```

---

## 7. Полнотекстовый поиск

```javascript
// Поиск по тексту
db.files.find(
  { $text: { $search: "javascript code" } },
  { score: { $meta: "textScore" } }
).sort({ score: { $meta: "textScore" } });

// Поиск с фильтрацией
db.files.find({
  $text: { $search: "config" },
  owner_id: "user-003"
});
```

---

## 8. Валидация данных

```javascript
// Проверка количества документов
db.files.countDocuments();

// Проверка существования поля
db.files.find({ tags: { $exists: true } });

// Проверка типа поля
db.files.find({ size: { $type: "long" } });
db.files.find({ created_at: { $type: "date" } });
```
