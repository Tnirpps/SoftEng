import json
import pytest
import uuid


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_create_root_directory(authorized_client):
    """Создание корневой директории."""
    response = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "test_dir"})
    )
    assert response.status == 201
    data = response.json()
    
    assert data["name"] == "test_dir"
    # parent_id должно отсутствовать в ответе для корневой директории
    assert "parent_id" not in data
    assert data["is_root"] is True
    assert "id" in data
    assert "owner_id" in data
    assert "created_at" in data
    assert "updated_at" in data


@pytest.mark.now(NOW)
async def test_create_nested_directory(authorized_client, root_directory):
    """Создание вложенной директории."""
    # Явно передаем parent_id в запросе
    response = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({
            "name": "child_dir",
            "parent_id": root_directory["id"]
        })
    )
    assert response.status == 201, f"Failed to create nested directory: {response.json()}"
    data = response.json()
    
    assert data["name"] == "child_dir"
    assert data["parent_id"] == root_directory["id"]
    assert data["is_root"] is False


@pytest.mark.now(NOW)
async def test_create_directory_with_existing_name(authorized_client):
    """Создание директории с существующим именем (конфликт)."""
    await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "duplicate_dir"})
    )
    
    response = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "duplicate_dir"})
    )
    assert response.status == 409


@pytest.mark.now(NOW)
async def test_create_directory_empty_name(authorized_client):
    """Создание директории с пустым именем."""
    response = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": ""})
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_create_directory_invalid_parent(authorized_client):
    """Создание директории с несуществующим parent_id."""
    fake_parent_id = str(uuid.uuid4())
    response = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({
            "name": "test_dir",
            "parent_id": fake_parent_id
        })
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_get_directory(authorized_client, root_directory):
    """Получение существующей директории."""
    response = await authorized_client.get(
        f"/v1/directories/{root_directory['id']}"
    )
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == root_directory["id"]
    assert data["name"] == root_directory["name"]
    assert "parent_id" not in data


@pytest.mark.now(NOW)
async def test_get_nonexistent_directory(authorized_client):
    """Получение несуществующей директории."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.get(
        f"/v1/directories/{fake_id}"
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_delete_directory(authorized_client, root_directory):
    """Удаление существующей директории."""
    response = await authorized_client.delete(
        f"/v1/directories/{root_directory['id']}"
    )
    assert response.status == 200
    data = response.json()
    assert "message" in data


@pytest.mark.now(NOW)
async def test_delete_nonexistent_directory(authorized_client):
    """Удаление несуществующей директории."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.delete(
        f"/v1/directories/{fake_id}"
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_delete_directory_with_children_not_recursive(authorized_client, root_directory, nested_directory):
    """Удаление директории с дочерними элементами без recursive."""
    response = await authorized_client.delete(
        f"/v1/directories/{root_directory['id']}"
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_delete_directory_with_children_recursive(authorized_client, root_directory, nested_directory):
    """Удаление директории с дочерними элементами с recursive=true."""
    response = await authorized_client.delete(
        f"/v1/directories/{root_directory['id']}?recursive=true"
    )
    assert response.status == 200
    data = response.json()
    assert "message" in data
