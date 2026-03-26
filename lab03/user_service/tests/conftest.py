import pytest
import json


pytest_plugins = [
    "pytest_userver.plugins.core",
]


@pytest.fixture(autouse=True)
async def clean_users_before_test(service_client):
    """Фикстура для очистки базы перед каждым тестом."""
    await service_client.run_task('delete-all-users')
    
    yield  # yield разделяет код 'до теста' и 'после теста'
    
    # Если хотите чистить ПОСЛЕ теста, можно перенести вызов сюда:
    # await service_client.run_task('delete-all-users')


class AuthorizedClient:
    def __init__(self, service_client, token: str):
        self._client = service_client
        self._token = token
    
    async def get(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.get(path, headers=headers, **kwargs)
    
    async def post(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.post(path, headers=headers, **kwargs)


async def _register_user(service_client, login: str, password: str, first_name: str, last_name: str):
    user_data = {
        "login": login,
        "password": password,
        "first_name": first_name,
        "last_name": last_name
    }
    
    response = await service_client.post(
        "/v1/users",
        data=json.dumps(user_data)
    )
    assert response.status == 201
    
    return user_data


@pytest.fixture
async def default_user(service_client):
    return await _register_user(service_client, "testuser", "123456", "Test", "Testov")


@pytest.fixture
async def token(service_client, default_user):
    login_response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": default_user["login"],
            "password": default_user["password"]
        })
    )
    assert login_response.status == 200
    
    return login_response.json()["token"]


@pytest.fixture
async def authorized_client(service_client, token):
    return AuthorizedClient(service_client, token)


@pytest.fixture
async def authorized_client_for_user(service_client):
    async def _create_client(login: str, password: str, first_name: str, last_name: str):
        await _register_user(service_client, login, password, first_name, last_name)
        
        login_response = await service_client.post(
            "/v1/users/login",
            data=json.dumps({
                "login": login,
                "password": password
            })
        )
        assert login_response.status == 200
        
        return AuthorizedClient(service_client, login_response.json()["token"])
    
    return _create_client
