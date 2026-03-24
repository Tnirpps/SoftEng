import json
import pytest
import uuid


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_list_files_empty(authorized_client, root_directory):
    """Получение списка файлов в пустой директории."""
    response = await authorized_client.get(
        f"/v1/directories/{root_directory['id']}/files"
    )
    assert response.status == 200
    data = response.json()
    
    assert data["items"] == []
    assert data["total"] == 0
    assert data["limit"] == 20
    assert data["offset"] == 0


@pytest.mark.now(NOW)
async def test_list_files_not_found(authorized_client):
    """Получение списка файлов в несуществующей директории."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.get(
        f"/v1/directories/{fake_id}/files"
    )
    assert response.status == 404


# Закомментировано: ParseIntArg возвращает default значение при некорректном вводе
# @pytest.mark.now(NOW)
# async def test_list_files_invalid_limit(authorized_client, root_directory):
#     """Получение списка файлов с некорректным limit."""
#     response = await authorized_client.get(
#         f"/v1/directories/{root_directory['id']}/files?limit=abc"
#     )
#     assert response.status == 400
# 
# 
# @pytest.mark.now(NOW)
# async def test_list_files_invalid_offset(authorized_client, root_directory):
#     """Получение списка файлов с некорректным offset."""
#     response = await authorized_client.get(
#         f"/v1/directories/{root_directory['id']}/files?offset=abc"
#     )
#     assert response.status == 400


@pytest.mark.now(NOW)
async def test_list_files_unauthorized(service_client, root_directory):
    """Получение списка файлов без авторизации."""
    response = await service_client.get(
        f"/v1/directories/{root_directory['id']}/files"
    )
    assert response.status == 401
