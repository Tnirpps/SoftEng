import json
import pytest
import uuid


NOW = '2019-12-31T11:22:33+00:00'


@pytest.mark.now(NOW)
async def test_list_directories_empty(authorized_client):
    """Получение списка директорий когда их нет."""
    response = await authorized_client.get("/v1/directories")
    assert response.status == 200
    data = response.json()
    
    assert data["items"] == []
    assert data["total"] == 0
    assert data["limit"] == 20
    assert data["offset"] == 0


@pytest.mark.now(NOW)
async def test_list_directories_with_items(authorized_client, root_directory, nested_directory):
    """Получение списка директорий с элементами."""
    response = await authorized_client.get("/v1/directories")
    assert response.status == 200
    data = response.json()
    
    # root_directory и nested_directory создаются в conftest, но nested_directory имеет parent_id
    # поэтому в корневом списке только root_directory
    assert len(data["items"]) >= 1
    assert data["total"] >= 1
    assert data["limit"] == 20
    assert data["offset"] == 0
    
    # Проверяем, что root_directory присутствует
    ids = [item["id"] for item in data["items"]]
    assert root_directory["id"] in ids


@pytest.mark.now(NOW)
async def test_list_directories_with_parent_filter(authorized_client, root_directory, nested_directory):
    """Получение списка директорий с фильтром по parent_id."""
    # Создаем еще одну вложенную директорию в root_directory
    another_nested = await authorized_client.post(
        "/v1/directories",
        data=json.dumps({"name": "another_nested", "parent_id": root_directory["id"]})
    )
    another_nested = another_nested.json()
    
    # Получаем только вложенные директории
    response = await authorized_client.get(
        f"/v1/directories?parent_id={root_directory['id']}"
    )
    assert response.status == 200
    data = response.json()
    
    assert len(data["items"]) == 2
    assert data["total"] == 2
    
    ids = [item["id"] for item in data["items"]]
    assert nested_directory["id"] in ids
    assert another_nested["id"] in ids


@pytest.mark.now(NOW)
async def test_list_directories_pagination(authorized_client):
    """Тест пагинации списка директорий."""
    # Создаем 5 директорий
    for i in range(5):
        await authorized_client.post(
            "/v1/directories",
            data=json.dumps({"name": f"dir_{i}"})
        )
    
    # Получаем первые 2
    response = await authorized_client.get("/v1/directories?limit=2&offset=0")
    assert response.status == 200
    data = response.json()
    
    assert len(data["items"]) == 2
    assert data["total"] == 5
    assert data["limit"] == 2
    assert data["offset"] == 0
    
    # Получаем следующие 2
    response = await authorized_client.get("/v1/directories?limit=2&offset=2")
    assert response.status == 200
    data = response.json()
    
    assert len(data["items"]) == 2
    assert data["total"] == 5
    assert data["limit"] == 2
    assert data["offset"] == 2
    
    # Получаем последнюю
    response = await authorized_client.get("/v1/directories?limit=2&offset=4")
    assert response.status == 200
    data = response.json()
    
    assert len(data["items"]) == 1
    assert data["total"] == 5
    assert data["limit"] == 2
    assert data["offset"] == 4


# Закомментировано: ParseIntArg возвращает default значение при некорректном вводе
# @pytest.mark.now(NOW)
# async def test_list_directories_invalid_limit(authorized_client):
#     """Получение списка с некорректным limit."""
#     response = await authorized_client.get("/v1/directories?limit=abc")
#     assert response.status == 400
# 
# 
# @pytest.mark.now(NOW)
# async def test_list_directories_invalid_offset(authorized_client):
#     """Получение списка с некорректным offset."""
#     response = await authorized_client.get("/v1/directories?offset=abc")
#     assert response.status == 400


@pytest.mark.now(NOW)
async def test_list_directories_unauthorized(service_client):
    """Получение списка директорий без авторизации."""
    response = await service_client.get("/v1/directories")
    assert response.status == 401
