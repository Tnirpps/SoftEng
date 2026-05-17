import json
import pytest
import uuid


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_move_directory_success(authorized_client, root_directory, nested_directory):
    """Успешное перемещение директории в другую директорию."""
    # Создаем целевую директорию
    target_dir = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "target_dir"})
    )
    target_dir = target_dir.json()
    
    # Перемещаем nested_directory в target_dir
    response = await authorized_client.post(
        f"/v1/directories/{nested_directory['id']}/move",
        data=json.dumps({"new_parent_id": target_dir["id"]})
    )
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == nested_directory["id"]
    assert data["parent_id"] == target_dir["id"]
    assert data["is_root"] is False


@pytest.mark.now(NOW)
async def test_move_directory_to_root(authorized_client, nested_directory):
    """Перемещение директории в корень (new_parent_id = null)."""
    # Перемещаем в корень
    response = await authorized_client.post(
        f"/v1/directories/{nested_directory['id']}/move",
        data=json.dumps({"new_parent_id": None})
    )
    assert response.status == 200
    data = response.json()
    
    assert data["id"] == nested_directory["id"]
    assert "parent_id" not in data
    assert data["is_root"] is True


@pytest.mark.now(NOW)
async def test_move_directory_not_found(authorized_client, root_directory):
    """Перемещение несуществующей директории."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.post(
        f"/v1/directories/{fake_id}/move",
        data=json.dumps({"new_parent_id": str(uuid.uuid4())})
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_move_directory_invalid_parent(authorized_client, root_directory):
    """Перемещение в несуществующую родительскую директорию."""
    fake_parent_id = str(uuid.uuid4())
    response = await authorized_client.post(
        f"/v1/directories/{root_directory['id']}/move",
        data=json.dumps({"new_parent_id": fake_parent_id})
    )
    assert response.status == 404


@pytest.mark.now(NOW)
async def test_move_directory_into_itself(authorized_client, root_directory):
    """Попытка переместить директорию в саму себя."""
    response = await authorized_client.post(
        f"/v1/directories/{root_directory['id']}/move",
        data=json.dumps({"new_parent_id": root_directory["id"]})
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_move_directory_into_descendant(authorized_client, root_directory, nested_directory):
    """Попытка переместить директорию в свою собственную вложенную директорию (создание цикла)."""
    # Пытаемся переместить root_directory в nested_directory (который является его потомком)
    response = await authorized_client.post(
        f"/v1/directories/{root_directory['id']}/move",
        data=json.dumps({"new_parent_id": nested_directory["id"]})
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_move_directory_name_conflict(authorized_client, root_directory):
    """Перемещение директории с именем, которое уже существует в целевой директории."""
    # Создаем директорию dir1
    dir1 = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "dir1"})
    )
    dir1 = dir1.json()
    
    # Создаем директорию с именем "conflict" в корне
    await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "conflict"})
    )
    
    # Создаем директорию с именем "conflict" в dir1
    conflict_in_dir1 = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "conflict", "parent_id": dir1["id"]})
    )
    conflict_in_dir1 = conflict_in_dir1.json()
    
    # Пытаемся переместить conflict из dir1 в корень (где уже есть conflict)
    response = await authorized_client.post(
        f"/v1/directories/{conflict_in_dir1['id']}/move",
        data=json.dumps({"new_parent_id": None})
    )
    assert response.status == 409


@pytest.mark.now(NOW)
async def test_move_directory_unauthorized(service_client, root_directory):
    """Перемещение директории без авторизации."""
    response = await service_client.post(
        f"/v1/directories/{root_directory['id']}/move",
        data=json.dumps({"new_parent_id": str(uuid.uuid4())})
    )
    assert response.status == 401
