import pytest
import json
import jwt
import uuid
from datetime import datetime, timedelta


pytest_plugins = [
    "pytest_userver.plugins.core",
]


# Секрет JWT из static_config.yaml
JWT_SECRET = "super-secret-jwt-key"
JWT_ISSUER = "user-service"


def generate_jwt_token(user_id: str, login: str) -> str:
    """Генерирует JWT токен для тестирования."""
    # Используем фиксированную дату в прошлом и очень долгий срок жизни
    now = datetime(2000, 1, 1, 0, 0, 0)
    payload = {
        "sub": user_id,
        "uuid": user_id,
        "login": login,
        "iss": JWT_ISSUER,
        "iat": int(now.timestamp()),
        "exp": int((now + timedelta(days=365 * 100)).timestamp())  # 100 лет
    }
    return jwt.encode(payload, JWT_SECRET, algorithm="HS256")


class AuthorizedClient:
    def __init__(self, service_client, token: str, user_id: str, login: str):
        self._client = service_client
        self._token = token
        self._user_id = user_id
        self._login = login
    
    async def get(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.get(path, headers=headers, **kwargs)
    
    async def post(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.post(path, headers=headers, **kwargs)
    
    async def put(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.put(path, headers=headers, **kwargs)
    
    async def delete(self, path: str, **kwargs):
        headers = kwargs.pop("headers", {})
        headers["Authorization"] = f"Bearer {self._token}"
        return await self._client.delete(path, headers=headers, **kwargs)


@pytest.fixture
def test_user():
    """Фикстура с данными тестового пользователя."""
    user_id = str(uuid.uuid4())
    login = "testuser"
    return {
        "user_id": user_id,
        "login": login,
        "token": generate_jwt_token(user_id, login)
    }


@pytest.fixture(autouse=True)
async def clean_directories(service_client):
    """Фикстура для очистки базы директорий перед каждым тестом."""
    await service_client.run_task('delete-all-directories')
    yield


@pytest.fixture
async def authorized_client(service_client, test_user):
    """Фикстура авторизованного клиента."""
    return AuthorizedClient(
        service_client, 
        test_user["token"],
        test_user["user_id"],
        test_user["login"]
    )


async def create_directory(client, name: str, parent_id: str = None) -> dict:
    """Вспомогательная функция для создания директории."""
    data = {"name": name}
    if parent_id:
        data["parent_id"] = parent_id
    
    response = await client.post(
        "/v1/directories",
        data=json.dumps(data)
    )
    assert response.status == 201
    return response.json()


@pytest.fixture
async def root_directory(authorized_client):
    """Фикстура с корневой директорией."""
    return await create_directory(authorized_client, "root_dir")


@pytest.fixture
async def nested_directory(authorized_client, root_directory):
    """Фикстура с вложенной директорией."""
    return await create_directory(
        authorized_client, 
        "nested_dir", 
        parent_id=root_directory["id"]
    )
