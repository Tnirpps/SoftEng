# MaDisk, ЛР6

В этой лабораторной я добавил в проект событийное взаимодействие через RabbitMQ. До этого сервисы в основном общались синхронно и напрямую работали со своими хранилищами, а теперь часть изменений можно передавать как события.

Основной реализованный пример: `file_service` сообщает о создании, изменении и удалении файлов, а `directory_service` слушает эти события и инвалидирует read-модель списка файлов в директории.

## Что появилось

- RabbitMQ в `docker-compose.yml`.
- Producer в `file_service`: компонент `file-event-publisher`.
- Consumer в `directory_service`: компонент `file-events-consumer`.
- Exchange `madisk.events` типа `direct`.
- Очередь `directory_service.file_events`.
- События `FileCreated`, `FileUpdated`, `FileDeleted`.

## Как это работает

Когда пользователь создает, переименовывает или удаляет файл, `file_service` сначала успешно меняет данные в MongoDB. После этого он публикует событие в RabbitMQ.

`directory_service` подписан на file-события. Если событие относится к файлу внутри директории, сервис сбрасывает кэш списка файлов этой директории в Redis. Следующий запрос на чтение заново соберет актуальный список.

Так получилось разделить write-часть и read-часть: запись остается в основных БД, а read-модель обновляется асинхронно через события.

## Запуск

```bash
cd lab06
docker compose up --build
```

Сервисы:

- `user_service`: `http://localhost:8081`
- `directory_service`: `http://localhost:8082`
- `file_service`: `http://localhost:8083`
- RabbitMQ Management UI: `http://localhost:15672`

Для RabbitMQ Management:

- login: `guest`
- password: `guest`

## Как проверить руками

1. Зарегистрировать пользователя через `POST /v1/users`.
2. Выполнить login через `POST /v1/users/login`.
3. Создать директорию через `POST /v1/directories`.
4. Создать файл в этой директории через `POST /v1/files`.
5. Посмотреть в логах, что `file_service` опубликовал событие, а `directory_service` его обработал.

## Документация

- [event_driven_design.md](/home/egortest/Desktop/MyProject/SoftEng/lab06/event_driven_design.md:1)
- [event_catalog.md](/home/egortest/Desktop/MyProject/SoftEng/lab06/event_catalog.md:1)
