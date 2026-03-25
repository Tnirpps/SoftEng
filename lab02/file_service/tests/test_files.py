import json
import pytest


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_create_file(authorized_client):
    """Создание файла."""
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "test.txt",
            "content": "Hello, World!"
        })
    )
    assert response.status == 201
    data = response.json()
    
    assert data["name"] == "test.txt"
    assert data["size"] == 13
    assert data["mime_type"] == "text/plain"
    assert "id" in data
    assert "owner_id" in data
    assert "created_at" in data
    assert "updated_at" in data
    assert data["status"] == "available"


@pytest.mark.now(NOW)
async def test_create_file_with_parent_id(authorized_client, root_directory):
    """Создание файла с указанием parent_id."""
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "nested_file.txt",
            "content": "Nested content",
            "parent_id": root_directory["id"]
        })
    )
    assert response.status == 201
    data = response.json()
    
    assert data["name"] == "nested_file.txt"
    assert data["size"] == 14
    assert data["directory_id"] == root_directory["id"]


@pytest.mark.now(NOW)
async def test_create_file_with_existing_name(authorized_client):
    """Создание файла с существующим именем (конфликт)."""
    await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "duplicate.txt",
            "content": "Content 1"
        })
    )
    
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "duplicate.txt",
            "content": "Content 2"
        })
    )
    assert response.status == 409


@pytest.mark.now(NOW)
async def test_create_file_empty_name(authorized_client):
    """Создание файла с пустым именем."""
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "",
            "content": "Content"
        })
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_create_file_without_content(authorized_client):
    """Создание файла без content."""
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "test.txt"
        })
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_create_file_mime_types(authorized_client):
    """Проверка определения MIME-типов."""
    test_cases = [
        ("test.json", "application/json"),
        ("test.xml", "application/xml"),
        ("test.html", "text/html"),
        ("test.css", "text/css"),
        ("test.js", "application/javascript"),
        ("test.png", "image/png"),
        ("test.jpg", "image/jpeg"),
        ("test.pdf", "application/pdf"),
        ("test.md", "text/markdown"),
        ("test.unknown", "application/octet-stream"),
    ]
    
    for filename, expected_mime in test_cases:
        response = await authorized_client.post(
            "/v1/files",
            data=json.dumps({
                "name": filename,
                "content": "Content"
            })
        )
        assert response.status == 201
        data = response.json()
        assert data["mime_type"] == expected_mime, f"Wrong MIME type for {filename}"


@pytest.mark.now(NOW)
async def test_create_file_unauthorized(service_client):
    """Создание файла без авторизации."""
    response = await service_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "test.txt",
            "content": "Content"
        })
    )
    assert response.status == 401


@pytest.mark.now(NOW)
async def test_create_file_size_calculation(authorized_client):
    """Проверка расчёта размера файла."""
    content = "Hello, World! This is a test content."
    response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "size_test.txt",
            "content": content
        })
    )
    assert response.status == 201
    data = response.json()
    
    assert data["size"] == len(content)


@pytest.mark.now(NOW)
async def test_create_multiple_files(authorized_client):
    """Создание нескольких файлов."""
    file_names = ["file1.txt", "file2.txt", "file3.txt"]
    created_ids = []
    
    for name in file_names:
        response = await authorized_client.post(
            "/v1/files",
            data=json.dumps({
                "name": name,
                "content": f"Content of {name}"
            })
        )
        assert response.status == 201
        data = response.json()
        created_ids.append(data["id"])
    
    # Все ID должны быть уникальны
    assert len(set(created_ids)) == len(file_names)


@pytest.mark.now(NOW)
async def test_get_file(authorized_client):
    """Получение файла по ID."""
    # Сначала создаём файл
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "get_test.txt",
            "content": "Test content for get"
        })
    )
    assert create_response.status == 201
    file_data = create_response.json()
    file_id = file_data["id"]
    
    # Получаем файл
    response = await authorized_client.get(f"/v1/files/{file_id}")
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == file_id
    assert data["name"] == "get_test.txt"
    assert data["size"] == 20
    assert data["mime_type"] == "text/plain"


@pytest.mark.now(NOW)
async def test_get_file_not_found(authorized_client):
    """Получение несуществующего файла."""
    import uuid
    fake_id = str(uuid.uuid4())
    response = await authorized_client.get(f"/v1/files/{fake_id}")
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_get_file_unauthorized(service_client, authorized_client):
    """Получение файла без авторизации."""
    # Создаём файл
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "auth_test.txt",
            "content": "Test"
        })
    )
    file_id = create_response.json()["id"]
    
    # Пытаемся получить без авторизации
    response = await service_client.get(f"/v1/files/{file_id}")
    assert response.status == 401


@pytest.mark.now(NOW)
async def test_update_file(authorized_client):
    """Обновление файла."""
    # Создаём файл
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "update_test.txt",
            "content": "Original content"
        })
    )
    assert create_response.status == 201
    file_data = create_response.json()
    file_id = file_data["id"]
    
    # Обновляем файл
    response = await authorized_client.put(
        f"/v1/files/{file_id}",
        data=json.dumps({
            "name": "updated_name.txt"
        })
    )
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == file_id
    assert data["name"] == "updated_name.txt"
    assert data["mime_type"] == "text/plain"


@pytest.mark.now(NOW)
async def test_update_file_not_found(authorized_client):
    """Обновление несуществующего файла."""
    import uuid
    fake_id = str(uuid.uuid4())
    response = await authorized_client.put(
        f"/v1/files/{fake_id}",
        data=json.dumps({
            "name": "new_name.txt"
        })
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_update_file_name_conflict(authorized_client):
    """Обновление файла с конфликтом имён."""
    # Создаём два файла
    await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "file1.txt",
            "content": "Content 1"
        })
    )
    create_response2 = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "file2.txt",
            "content": "Content 2"
        })
    )
    file2_id = create_response2.json()["id"]
    
    # Пытаемся переименовать file2 в file1.txt
    response = await authorized_client.put(
        f"/v1/files/{file2_id}",
        data=json.dumps({
            "name": "file1.txt"
        })
    )
    assert response.status == 409


@pytest.mark.now(NOW)
async def test_update_file_unauthorized(service_client, authorized_client):
    """Обновление файла без авторизации."""
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "unauth_update.txt",
            "content": "Test"
        })
    )
    file_id = create_response.json()["id"]
    
    response = await service_client.put(
        f"/v1/files/{file_id}",
        data=json.dumps({
            "name": "new_name.txt"
        })
    )
    assert response.status == 401


@pytest.mark.now(NOW)
async def test_delete_file(authorized_client):
    """Удаление файла."""
    # Создаём файл
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "delete_test.txt",
            "content": "To be deleted"
        })
    )
    assert create_response.status == 201
    file_id = create_response.json()["id"]
    
    # Удаляем файл
    response = await authorized_client.delete(f"/v1/files/{file_id}")
    assert response.status == 200
    data = response.json()
    assert data["message"] == "File successfully deleted"
    
    # Проверяем, что файл удалён
    get_response = await authorized_client.get(f"/v1/files/{file_id}")
    assert get_response.status == 404


@pytest.mark.now(NOW)
async def test_delete_file_not_found(authorized_client):
    """Удаление несуществующего файла."""
    import uuid
    fake_id = str(uuid.uuid4())
    response = await authorized_client.delete(f"/v1/files/{fake_id}")
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_delete_file_unauthorized(service_client, authorized_client):
    """Удаление файла без авторизации."""
    create_response = await authorized_client.post(
        "/v1/files",
        data=json.dumps({
            "name": "unauth_delete.txt",
            "content": "Test"
        })
    )
    file_id = create_response.json()["id"]
    
    response = await service_client.delete(f"/v1/files/{file_id}")
    assert response.status == 401
