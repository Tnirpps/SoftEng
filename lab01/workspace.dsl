workspace "Cloud File Storage" "Архитектура системы хранения файлов (Вариант 11)" {
    model {
        user = person "Пользователь" "Загружает и скачивает файлы, управляет папками."
        
        antivirusEngine = softwareSystem "Антивирусное API" "Внешний сервис проверки на вирусы." "External"
        cloudDrive = softwareSystem "Облачное хранилище файлов" "Сервис облачного хранения файлов и каталогов пользователей." {
            
            webUI = container "Web UI" "Веб-интерфейс файлового менеджера." "React" "Web Browser"
            gateway = container "API Gateway" "Входная точка API, маршрутизация запросов." "Nginx"
            
            userService = container "User Service" "Сервис управления профилями пользователей и дисковыми квотами." "C++ (userver) / gRPC"
            directoryApp = container "Directory Service" "Сервис управления иерархией каталогов и файлов." "C++ (userver) / gRPC"
            fileTransferApp = container "File Node" "Сервис приёма и передачи файлов." "C++ (userver)"
            backgroundWorker = container "Background Worker" "Сервис фоновой обработки задач." "C++ (userver)"
            
            rabbitmq = container "Message Broker" "Брокер сообщений для асинхронной обработки." "RabbitMQ" "Infrastructure"
            metadataDb = container "Relational DB" "Хранение данных пользователей, каталогов и файлов." "PostgreSQL" "Database"
            s3Storage = container "S3 Object Store" "Объектное хранилище файловых данных." "MinIO" "ObjectStore"
        }
        user -> webUI "Взаимодействует с веб-интерфейсом"
        webUI -> gateway "HTTPS запросы" "JSON/REST"
        
        gateway -> userService "Маршрутизирует запросы к /api/users" "REST"
        gateway -> directoryApp "Маршрутизирует запросы к /api/folders" "REST"
        gateway -> fileTransferApp "Передаёт файлы на /api/files/upload" "Multipart"
        
        fileTransferApp -> userService "Синхронный вызов: проверка дисковой квоты" "gRPC"
        fileTransferApp -> directoryApp "Синхронный вызов: регистрация метаданных файла" "gRPC"
        
        userService -> metadataDb "Операции CRUD: профили и квоты" "TCP"
        directoryApp -> metadataDb "Операции CRUD: структура каталогов" "TCP"
        fileTransferApp -> s3Storage "Запись объекта в хранилище" "S3 API"
        
        fileTransferApp -> rabbitmq "Публикация события 'File Uploaded'" "AMQP"
        rabbitmq -> backgroundWorker "Доставка события потребителю (Consumer)" "AMQP"
        
        backgroundWorker -> metadataDb "Обновление статуса файла" "TCP"
        backgroundWorker -> s3Storage "Чтение объекта для проверки" "S3 API"
        backgroundWorker -> antivirusEngine "Отправка содержимого на проверку" "HTTPS"
    }
    views {
        systemContext cloudDrive "SystemContext" {
            include *
            autoLayout tb
            description "Системный контекст платформы облачного диска."
        }
        container cloudDrive "Containers" {
            include *
            autoLayout tb 300 200
            description "Архитектура с выделением сервиса пользователей, метаданных и файловой ноды."
        }
        # Сценарий загрузки нового файла пользователем
        dynamic cloudDrive "AsyncUploadFlow" "Сценарий: Загрузка и проверка файла" {
            description "Процесс загрузки файла: валидация квоты, запись в S3, быстрый ответ клиенту и фоновая антивирусная проверка."
            
            # Вход от пользователя
            user -> webUI "Инициирует загрузку файла"
            webUI -> gateway "Формирует POST-запрос на /files/upload"
            gateway -> fileTransferApp "Направляет поток данных в сервис файлов"
            
            # Проверки и сохранение
            fileTransferApp -> userService "Запрос к сервису квот: проверка свободного места"
            fileTransferApp -> directoryApp "Регистрация метаданных нового файла в каталоге"
            directoryApp -> metadataDb "Транзакция записи метаданных (статус: 'На проверке')"
            fileTransferApp -> s3Storage "Потоковая запись байтов в объектное хранилище (PutObject)"
            
            # Асинхронная делегация задачи
            fileTransferApp -> rabbitmq "Публикация события 'Файл загружен'"
            rabbitmq -> backgroundWorker "Доставка события фоновому обработчику"
            
            # Фоновая проверка
            backgroundWorker -> antivirusEngine "Антивирусная проверка содержимого"
            backgroundWorker -> metadataDb "Обновление статуса файла в БД ('Доступен')"
            
            autoLayout tb 300 300
        }
        styles {
            element "Software System" {
                background #4f4f4f
                color #ffffff
            }
            element "External" {
                background #999999
                color #ffffff
            }
            element "Person" {
                shape person
                background #1369b0
                color #ffffff
            }
            element "Container" {
                background #1b8adb
                color #ffffff
            }
            element "Web Browser" {
                shape WebBrowser
            }
            element "Database" {
                shape Cylinder
                background #f39c12
                color #ffffff
            }
            element "ObjectStore" {
                shape Folder
                background #27ae60
                color #ffffff
            }
            element "Infrastructure" {
                shape Pipe
                background #e74c3c
                color #ffffff
            }
        }
    }
}
