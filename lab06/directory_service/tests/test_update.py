import json
import pytest
import uuid


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_update_directory_success(authorized_client, root_directory):
    """Успешное обновление имени директории."""
    new_name = "updated_dir_name"
    response = await authorized_client.put(
        f"/v1/directories/{root_directory['id']}",
        data=json.dumps({"name": new_name})
    )
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == root_directory["id"]
    assert data["name"] == new_name
    assert data["is_root"] is True


@pytest.mark.now(NOW)
async def test_update_directory_not_found(authorized_client):
    """Обновление несуществующей директории."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.put(
        f"/v1/directories/{fake_id}",
        data=json.dumps({"name": "new_name"})
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_update_directory_name_conflict(authorized_client, root_directory):
    """Обновление имени на уже существующее (конфликт)."""
    # Создаем вторую директорию
    await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "target_name"})
    )
    
    # Пытаемся переименовать root_directory в target_name
    response = await authorized_client.put(
        f"/v1/directories/{root_directory['id']}",
        data=json.dumps({"name": "target_name"})
    )
    assert response.status == 409


@pytest.mark.now(NOW)
async def test_update_directory_empty_name(authorized_client, root_directory):
    """Обновление имени на пустое."""
    response = await authorized_client.put(
        f"/v1/directories/{root_directory['id']}",
        data=json.dumps({"name": ""})
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_update_directory_unauthorized(service_client, root_directory):
    """Обновление директории без авторизации."""
    response = await service_client.put(
        f"/v1/directories/{root_directory['id']}",
        data=json.dumps({"name": "new_name"})
    )
    assert response.status == 401
