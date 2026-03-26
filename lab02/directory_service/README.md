# Directory Service

## Описание

Микросервис управления директориями (папками) системы MaDisk. Отвечает за создание, чтение, обновление, удаление и перемещение директорий в иерархической структуре.

**Технологии:**
- C++23
- Framework: Userver
- Аутентификация: JWT токены (middleware)
- Хранилище: in-memory (для демонстрации)

## Функционал

| Endpoint | Метод | Описание | Auth |
|----------|-------|----------|------|
| `/v1/directories` | POST | Создание новой директории | Да |
| `/v1/directories` | GET | Список директорий (с пагинацией) | Да |
| `/v1/directories/{id}` | GET | Получение информации о директории | Да |
| `/v1/directories/{id}` | PUT | Обновление имени директории | Да |
| `/v1/directories/{id}` | DELETE | Удаление директории | Да |
| `/v1/directories/{id}/move` | POST | Перемещение директории | Да |
| `/v1/directories/{id}/files` | GET | Список файлов в директории | Да |

## Сборка и запуск

### Локальная сборка (не рекомендуется)
Для такой сборки нужен установленный Conan и заранее собранный userver/2.16-rc (см. [документацию userver](https://userver.tech/docs/v2.0/da/de1/md_en_2userver_2tutorial_2build__userver.html#autotoc_md550)).

```bash
# Полная сборка всех сервисов
python3 build.py all

# Запуск Directory Service
python3 build.py start directory_service
```

### Запуск в Docker (рекомендуется)

```bash
# Сборка и запуск всех сервисов
python3 build.py docker-up

# Directory Service доступен на порту 8082
```

## Тестирование

### Запуск тестов (при локальной сборке)

```bash
# Unit-тесты (C++)
python3 build.py test

# Функциональные тесты (pytest)
python3 build.py ftest                      # Все тесты
python3 build.py ftest "test_create"        # Тесты создания
python3 build.py ftest "test_move"          # Тесты перемещения
python3 build.py ftest "test_delete"        # Тесты удаления
```

### Примеры запросов

#### Создание директории
```bash
curl -X POST http://localhost:8082/v1/directories \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{
    "name": "Documents"
  }'
```

#### Создание вложенной директории
```bash
curl -X POST http://localhost:8082/v1/directories \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{
    "name": "Work",
    "parent_id": "550e8400-e29b-41d4-a716-446655440000"
  }'
```

#### Получение списка директорий
```bash
curl -X GET "http://localhost:8082/v1/directories?limit=20&offset=0" \
  -H "Authorization: Bearer <JWT_TOKEN>"
```

#### Перемещение директории
```bash
curl -X POST http://localhost:8082/v1/directories/{id}/move \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <JWT_TOKEN>" \
  -d '{
    "new_parent_id": "550e8400-e29b-41d4-a716-446655440001"
  }'
```

#### Удаление директории
```bash
# Без рекурсивного удаления (ошибка если есть дочерние)
curl -X DELETE http://localhost:8082/v1/directories/{id} \
  -H "Authorization: Bearer <JWT_TOKEN>"

# С рекурсивным удалением
curl -X DELETE "http://localhost:8082/v1/directories/{id}?recursive=true" \
  -H "Authorization: Bearer <JWT_TOKEN>"
```

## TODO


### По целевой архитектуре (Lab 01)
- [ ] Интеграция с PostgreSQL для хранения иерархии директорий
- [ ] Кэширование часто запрашиваемых директорий (Redis)
