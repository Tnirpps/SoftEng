# File Service

## Описание

Микросервис управления файлами системы MaDisk. Отвечает за загрузку, хранение метаданных и управление файлами пользователей.

**Технологии:**
- C++23
- Framework: Userver
- Аутентификация: JWT токены (middleware)
- Хранилище: in-memory (для демонстрации)

## Функционал

| Endpoint | Метод | Описание | Auth |
|----------|-------|----------|------|
| `/v1/files` | POST | Загрузка нового файла | Да |
| `/v1/files/{id}` | GET | Получение файла | Да |
| `/v1/files/{id}` | PUT | Обновление файла | Да |
| `/v1/files/{id}` | DELETE | Удаление файла | Да |

## Сборка и запуск

### Локальная сборка (не рекоммендуется)
Для такой сборки нужен установленный conan и заранее собранный userver/2.16-rc (см документацию [userver](https://userver.tech/docs/v2.0/da/de1/md_en_2userver_2tutorial_2build__userver.html#autotoc_md550))

```bash
# Полная сборка всех сервисов
python3 build.py all

# Запуск File Service
python3 build.py start file_service
```

### Запуск в Docker (рекомендуется)

```bash
# Сборка и запуск всех сервисов
python3 build.py docker-up
```


## Тестирование

### Запуск тестов (при локальной сборке)

```bash
# Unit-тесты (C++)
python3 build.py test

# Функциональные тесты (pytest)
python3 build.py ftest                    # Все тесты
python3 build.py ftest "test_create"      # Тесты создания файлов
python3 build.py ftest "test_mime"        # Тесты MIME-типов
```

### Примеры запросов

#### Загрузка файла
```bash
curl -X POST http://localhost:8083/v1/files \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{
    "name": "document.txt",
    "content": "Hello, World!",
    "parent_id": "550e8400-e29b-41d4-a716-446655440000"
  }'
```

> **Примечание:** Операции списка файлов и перемещения файлов реализуются в Directory Service.

## Примеры запросов

#### Получение файла
```bash
curl -X GET http://localhost:8083/v1/files/<FILE_ID> \
  -H "Authorization: Bearer <JWT_TOKEN>"
```

#### Обновление файла
```bash
curl -X PUT http://localhost:8083/v1/files/<FILE_ID> \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{
    "name": "new_name.txt"
  }'
```

#### Удаление файла
```bash
curl -X DELETE http://localhost:8083/v1/files/<FILE_ID> \
  -H "Authorization: Bearer <JWT_TOKEN>"
```

## TODO

### По целевой архитектуре (Lab 01)
- [ ] Интеграция с PostgreSQL для хранения метаданных файлов
- [ ] Интеграция с MinIO/S3 для физического хранения файлов
- [ ] Асинхронная проверка файлов антивирусом через RabbitMQ
- [ ] Определение MIME-типов по содержимому, а не по расширению
- [ ] gRPC интерфейс для взаимодействия с Directory Service
