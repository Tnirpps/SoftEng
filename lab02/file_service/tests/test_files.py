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
