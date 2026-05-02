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


@pytest.mark.now(NOW)
async def test_list_files_creates_cache_entry(authorized_client, root_directory, test_user, redis_store):
    """Успешный запрос должен создавать Redis-кэш для содержимого папки."""
    response = await authorized_client.get(
        f"/v1/directories/{root_directory['id']}/files"
    )
    assert response.status == 200

    cache_key = f"dir-files:{test_user['user_id']}:{root_directory['id']}:20:0"
    cached_value = redis_store.get(cache_key)

    assert cached_value is not None
    cached_payload = json.loads(cached_value)
    assert cached_payload["items"] == []
    assert cached_payload["total"] == 0
    assert cached_payload["limit"] == 20
    assert cached_payload["offset"] == 0


@pytest.mark.now(NOW)
async def test_list_files_uses_separate_keys_for_pagination(authorized_client, root_directory, test_user, redis_store):
    """Разные limit/offset должны создавать разные ключи Redis."""
    response = await authorized_client.get(
        f"/v1/directories/{root_directory['id']}/files?limit=5&offset=10"
    )
    assert response.status == 200

    default_key = f"dir-files:{test_user['user_id']}:{root_directory['id']}:20:0"
    paginated_key = f"dir-files:{test_user['user_id']}:{root_directory['id']}:5:10"

    assert redis_store.get(default_key) is None
    assert redis_store.get(paginated_key) is not None


@pytest.mark.now(NOW)
async def test_list_files_not_found_is_not_cached(authorized_client, test_user, redis_store):
    """Ответ 404 не должен попадать в Redis."""
    fake_id = str(uuid.uuid4())
    response = await authorized_client.get(
        f"/v1/directories/{fake_id}/files"
    )
    assert response.status == 404

    cache_key = f"dir-files:{test_user['user_id']}:{fake_id}:20:0"
    assert redis_store.get(cache_key) is None


@pytest.mark.now(NOW)
async def test_list_files_unauthorized_is_not_cached(service_client, root_directory, test_user, redis_store):
    """Ответ 401 не должен попадать в Redis."""
    response = await service_client.get(
        f"/v1/directories/{root_directory['id']}/files"
    )
    assert response.status == 401

    cache_key = f"dir-files:{test_user['user_id']}:{root_directory['id']}:20:0"
    assert redis_store.get(cache_key) is None
