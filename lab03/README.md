# MaDisk - Система управления файлами и директориями

## Схема базы данных

### Файлы схемы

Схема базы данных разделена на два файла в директории [`init-db/`](./init-db/). Эти файлы автоматически выполняются при первом запуске PostgreSQL через Docker Compose:

- [`init-db/01-users.sql`](./init-db/01-users.sql) - таблица `users` для User Service
- [`init-db/02-directories.sql`](./init-db/02-directories.sql) - таблицы `directories` и `files` для Directory Service

### Структура таблиц

#### users (User Service)

| Поле | Тип | Описание |
|------|-----|----------|
| uuid | UUID | Первичный ключ (auto-generated) |
| login | VARCHAR(20) | Уникальный логин пользователя |
| password | VARCHAR(40) | Хэш пароля |
| first_name | VARCHAR(20) | Имя |
| last_name | VARCHAR(20) | Фамилия |
| created_at | TIMESTAMPTZ | Дата создания записи |

**Индексы:**
- PRIMARY KEY на `uuid`
- UNIQUE на `login`
- GIN индекс на `last_name` с `gin_trgm_ops` для поиска по маске (ILIKE)

#### directories (Directory Service)

| Поле | Тип | Описание |
|------|-----|----------|
| uuid | UUID | Первичный ключ (auto-generated) |
| name | VARCHAR(255) | Имя директории |
| parent_uuid | UUID | Ссылка на родительскую директорию (NULL для корневых) |
| owner_uuid | UUID | Владелец директории |
| created_at | TIMESTAMPTZ | Дата создания |
| updated_at | TIMESTAMPTZ | Дата последнего обновления |
| is_root | BOOLEAN | Флаг корневой директории |

**Индексы:**
- PRIMARY KEY на `uuid`
- UNIQUE на `(owner_uuid, parent_uuid, name)`
- B-Tree индекс на `parent_uuid` (проверка наличия дочерних элементов)
- B-Tree индекс на `owner_uuid` (листинг директорий пользователя)
- Composite индекс на `(owner_uuid, parent_uuid)` (фильтрация по владельцу и родителю)

#### files (Directory Service)

| Поле | Тип | Описание |
|------|-----|----------|
| uuid | UUID | Первичный ключ (auto-generated) |
| name | VARCHAR(255) | Имя файла |
| size | BIGINT | Размер в байтах |
| mime_type | VARCHAR(255) | MIME-тип |
| directory_uuid | UUID | Ссылка на родительскую директорию |
| owner_uuid | UUID | Владелец файла |
| created_at | TIMESTAMPTZ | Дата создания |
| updated_at | TIMESTAMPTZ | Дата последнего обновления |
| status | VARCHAR(50) | Статус файла (pending/scanning/available/infected) |

**Индексы:**
- PRIMARY KEY на `uuid`
- B-Tree индекс на `directory_uuid` (листинг файлов в директории)
- B-Tree индекс на `owner_uuid` (листинг файлов пользователя)
- Composite индекс на `(directory_uuid, owner_uuid)` (комбинированная фильтрация)

## Запуск

### Требования

- Docker и Docker Compose
- PostgreSQL 15+ (создается автоматически через Docker)

### Быстрый старт

1. Клонируйте репозиторий
2. Запустите все сервисы через Docker Compose:

```bash
cd lab03
docker-compose up --build
```

После запуска сервисы будут доступны по адресам:
- User Service: http://localhost:8081
- Directory Service: http://localhost:8082
- File Service: http://localhost:8083
- PostgreSQL: localhost:5432 (postgres/postgres)

### Проверка здоровья

```bash
curl http://localhost:8081/ping  # User Service
curl http://localhost:8082/ping  # Directory Service
curl http://localhost:8083/ping  # File Service
```

## Оптимизация базы данных

[`local/DESIGN_REPORT.md`](./local/DESIGN_REPORT.md).

## API Документация

Полная документация API доступна в OpenAPI спецификациях:

- [User Service API](./user_service/schemas/openapi.yaml)
- [Directory Service API](./directory_service/schemas/openapi.yaml)
- [File Service API](./file_service/schemas/openapi.yaml)

### Основные эндпоинты

#### User Service

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| POST | `/api/v1/users/register` | Регистрация пользователя |
| POST | `/api/v1/users/login` | Аутентификация |
| GET | `/api/v1/users/search` | Поиск пользователей по фамилии |

#### Directory Service

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| POST | `/api/v1/directories` | Создание директории |
| GET | `/api/v1/directories/:uuid` | Получение директории |
| PUT | `/api/v1/directories/:uuid` | Обновление директории |
| DELETE | `/api/v1/directories/:uuid` | Удаление директории |
| POST | `/api/v1/directories/:uuid/move` | Перемещение директории |
| GET | `/api/v1/directories` | Листинг директорий |
| GET | `/api/v1/directories/:uuid/files` | Листинг файлов в директории |

### Генерация тестовых данных для БД

```bash
cd lab03/tools
python generate_test_data.py
python generate_test_users.py
```

## Структура проекта

```
lab03/
├── init-db/                    # Скрипты инициализации БД
│   ├── 01-users.sql           # Схема users
│   └── 02-directories.sql     # Схема directories + files
├── user_service/              # Сервис пользователей
│   ├── src/
│   ├── queries/
│   ├── postgresql/schemas/
│   └── tests/
├── directory_service/         # Сервис директорий
│   ├── src/
│   ├── queries/
│   ├── postgresql/schemas/
│   └── tests/
├── file_service/              # Сервис файлов
│   ├── src/
│   └── tests/
├── common/                    # Общие утилиты и модели
├── tools/                     # Утилиты для генерации данных
├── docker-compose.yml         # Конфигурация Docker Compose
├── Dockerfile                 # Docker образ для сборки
└── README.md                  # Этот файл
```
