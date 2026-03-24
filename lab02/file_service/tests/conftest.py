import pytest
import json
import jwt
import uuid
from datetime import datetime, timedelta


pytest_plugins = [
    "pytest_userver.plugins.core",
]


# JWT secret from static_config.yaml
JWT_SECRET = "super-secret-jwt-key"
JWT_ISSUER = "user-service"


def generate_jwt_token(user_id: str, login: str) -> str:
    """Generate JWT token for testing."""
    # Use fixed date in the past and very long expiration
    now = datetime(2000, 1, 1, 0, 0, 0)
    payload = {
        "sub": user_id,
        "uuid": user_id,
        "login": login,
        "iss": JWT_ISSUER,
        "iat": int(now.timestamp()),
        "exp": int((now + timedelta(days=365 * 100)).timestamp())  # 100 years
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
    """Fixture with test user data."""
    user_id = str(uuid.uuid4())
    login = "testuser"
    return {
        "user_id": user_id,
        "login": login,
        "token": generate_jwt_token(user_id, login)
    }


@pytest.fixture(autouse=True)
async def clean_files(service_client):
    """Fixture to clean up files before each test."""
    await service_client.run_task('delete-all-files')
    yield


@pytest.fixture
async def authorized_client(service_client, test_user):
    """Fixture for authorized client."""
    return AuthorizedClient(
        service_client, 
        test_user["token"],
        test_user["user_id"],
        test_user["login"]
    )


async def create_file(client, name: str, content: str, parent_id: str = None) -> dict:
    """Helper function to create a file."""
    data = {"name": name, "content": content}
    if parent_id:
        data["parent_id"] = parent_id
    
    response = await client.post(
        "/v1/files",
        data=json.dumps(data)
    )
    assert response.status == 201
    return response.json()


@pytest.fixture
async def test_file(authorized_client):
    """Fixture with a test file."""
    return await create_file(authorized_client, "test_file.txt", "test content")


@pytest.fixture
async def root_directory(authorized_client):
    """Fixture with a root directory (for file tests with parent_id)."""
    # Create a directory using the directory service endpoint if available
    # For now, just return a mock structure for tests that need parent_id
    return {"id": str(uuid.uuid4()), "name": "test_dir"}
