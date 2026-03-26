# User Service

## Описание

Микросервис управления пользователями системы MaDisk. Отвечает за регистрацию, аутентификацию и поиск пользователей.

**Технологии:**
- C++23
- Framework: Userver
- Аутентификация: JWT токены
- Хранилище: in-memory (для демонстрации)

## Функционал

| Endpoint | Метод | Описание | Auth |
|----------|-------|----------|------|
| `/v1/users` | POST | Регистрация нового пользователя | Нет |
| `/v1/users/login` | POST | Аутентификация, получение JWT токена | Нет |
| `/v1/users/search?pattern=` | GET | Поиск пользователя по маске логина | Да |

## Сборка и запуск

### Локальная сборка (не рекомендуется)
Для такой сборки нужен установленный Conan и заранее собранный userver/2.16-rc (см. [документацию userver](https://userver.tech/docs/v2.0/da/de1/md_en_2userver_2tutorial_2build__userver.html#autotoc_md550)).

```bash
# Полная сборка всех сервисов
python3 build.py all

# Запуск User Service
python3 build.py start user_service
```

### Запуск в Docker (рекомендуется)

```bash
# Сборка и запуск всех сервисов
python3 build.py docker-up

# User Service доступен на порту 8081
```

## Тестирование

### Запуск тестов (при локальной сборке)

```bash
# Unit-тесты (C++)
python3 build.py test

# Функциональные тесты (pytest)
python3 build.py ftest                    # Все тесты
python3 build.py ftest "test_register"    # Конкретный тест
python3 build.py ftest "test_login"       # Тесты логина
```

### Примеры запросов

#### Регистрация пользователя
```bash
curl -X POST http://localhost:8081/v1/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "testuser",
    "password": "123456",
    "first_name": "Test",
    "last_name": "User"
  }'
```

#### Аутентификация
```bash
curl -X POST http://localhost:8081/v1/users/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "testuser",
    "password": "123456"
  }'
```

#### Поиск пользователя
```bash
curl -X GET "http://localhost:8081/v1/users/search?pattern=test%" \
  -H "Authorization: Bearer <JWT_TOKEN>"
```

## TODO

### По целевой архитектуре (Lab 01)
- [ ] Интеграция с PostgreSQL для хранения пользователей
- [ ] Реализация проверки квот дискового пространства
- [ ] gRPC интерфейс для взаимодействия с Directory Service и File Service
- [ ] Rate limiting для endpoint'ов аутентификации
