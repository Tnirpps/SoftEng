# MaDisk — Система хранения файлов

Микросервисная система облачного хранения файлов, реализованная на C++ с использованием фреймворка Userver.

## Архитектура

Система состоит из трёх сервисов:

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  User Service   │     │ Directory Svc   │     │  File Service   │
│   (порт 8081)   │     │   (порт 8082)   │     │   (порт 8083)   │
├─────────────────┤     ├─────────────────┤     ├─────────────────┤
│ • Регистрация   │     │ • CRUD папок    │     │ • Загрузка      │
│ • Аутентификация│     │ • Навигация     │     │ • Метаданные    │
│ • Поиск         │     │ • Перемещение   │     │ • Статусы       │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

## Сервисы

| Сервис | Порт | Описание |
|--------|------|----------|
| User Service | 8081 | Управление пользователями и аутентификация |
| Directory Service | 8082 | Управление иерархией директорий |
| File Service | 8083 | Управление файлами |

## Быстрый старт

### Требования

#### Для локальной разработки
- C++23 совместимый компилятор
- Conan Package Manager
- CMake 3.24+
- Python 3.11+

#### Для Docker
- Docker и Docker Compose

### Сборка и запуск

#### Локальная сборка (не рекомендуется)
Для такой сборки нужен установленный Conan и заранее собранный userver/2.16-rc (см. [документацию userver](https://userver.tech/docs/v2.0/da/de1/md_en_2userver_2tutorial_2build__userver.html#autotoc_md550)).

```bash
# Полная сборка (bootstrap + configure + build)
python3 build.py all

# Или по шагам:
python3 build.py bootstrap    # Установка зависимостей через Conan
python3 build.py configure    # Настройка CMake
python3 build.py build        # Сборка всех сервисов

# Запуск сервиса
python3 build.py start user_service
python3 build.py start directory_service
```

#### Запуск в Docker (рекомендуется)

```bash
# Сборка образов и запуск всех сервисов
python3 build.py docker-up

# Остановка сервисов
python3 build.py docker-down
```

После запуска сервисы доступны:
- User Service: http://localhost:8081
- Directory Service: http://localhost:8082
- File Service: http://localhost:8083

## Тестирование

```bash
# Unit-тесты (C++)
python3 build.py test

# Функциональные тесты (pytest)
python3 build.py ftest                    # Все тесты
python3 build.py ftest "test_register"    # Конкретный тест

# Тесты отдельного сервиса через pytest
python3 build.py ftest -k "test_login"    # Поиск по паттерну
```

## API Документация

Каждый сервис имеет OpenAPI спецификацию:

- [User Service API](user_service/schemas/openapi.yaml)
- [Directory Service API](directory_service/schemas/openapi.yaml)
- [File Service API](file_service/schemas/openapi.yaml)

### Примеры запросов

#### Регистрация пользователя
```bash
curl -X POST http://localhost:8081/v1/users \
  -H "Content-Type: application/json" \
  -d '{"login": "user", "password": "123456", "first_name": "Test", "last_name": "User"}'
```

#### Аутентификация
```bash
curl -X POST http://localhost:8081/v1/users/login \
  -H "Content-Type: application/json" \
  -d '{"login": "user", "password": "123456"}'
```

#### Создание директории
```bash
curl -X POST http://localhost:8082/v1/directories \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <TOKEN>" \
  -d '{"name": "Documents"}'
```

#### Загрузка файла
```bash
curl -X POST http://localhost:8083/v1/files \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <TOKEN>" \
  -d '{"name": "test.txt", "content": "Hello"}'
```

## Структура проекта

```
lab02/
├── common/                # Общие компоненты
│   ├── auth/              # JWT аутентификация
│   └── utils/             # Утилиты (UUID, request args)
├── user_service/          # Сервис пользователей
│   ├── src/
│   ├── tests/
│   └── schemas/
├── directory_service/     # Сервис директорий
│   ├── src/
│   ├── tests/
│   └── schemas/
├── file_service/          # Сервис файлов
│   ├── src/
│   ├── tests/
│   └── schemas/
├── Dockerfile             # Docker образ для всех сервисов
└── docker-compose.yml     # Оркестрация сервисов
```

## TODO

### Целевая архитектура (Lab 01)
- [ ] Интеграция с PostgreSQL для хранения данных
- [ ] Интеграция с MinIO для хранения файлов
- [ ] Реализация асинхронной проверки антивирусом через RabbitMQ
- [ ] gRPC коммуникация между сервисами
- [ ] API Gateway (Nginx) для маршрутизации запросов
