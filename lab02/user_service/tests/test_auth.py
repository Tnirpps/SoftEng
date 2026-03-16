import json


async def test_search_without_authorization_header(service_client):
    """Test that /search returns 401 when no Authorization header is provided"""
    response = await service_client.get("/v1/users/search?pattern=test")
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Authorization header was not provided"


async def test_search_with_invalid_authorization_format(service_client):
    """Test that /search returns 401 when Authorization header has invalid format"""
    response = await service_client.get(
        "/v1/users/search?pattern=test",
        headers={"Authorization": "InvalidFormat token123"}
    )
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Invalid Authorization header format. Expected 'Bearer <token>'"


async def test_search_with_empty_token(service_client):
    """Test that /search returns 401 when Bearer token is empty"""
    response = await service_client.get(
        "/v1/users/search?pattern=test",
        headers={"Authorization": "Bearer "}
    )
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Token was not provided"


async def test_search_with_invalid_token(service_client):
    """Test that /search returns 401 when token is invalid/corrupted"""
    response = await service_client.get(
        "/v1/users/search?pattern=test",
        headers={"Authorization": "Bearer invalid_token_here"}
    )
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Invalid token"


async def test_search_with_valid_token(authorized_client):
    """Test that /search works with valid JWT token"""
    response = await authorized_client.get("/v1/users/search?pattern=test%")
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json


async def test_search_token_from_different_user(authorized_client_for_user):
    """Test that JWT token works for different users"""
    client = await authorized_client_for_user("anotheruser", "pass123", "Another", "User")
    
    response = await client.get("/v1/users/search?pattern=admin%")
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json


async def test_login_with_wrong_password(service_client, default_user):
    """Test that login returns 401 with wrong password"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": default_user["login"],
            "password": "wrong_password"
        })
    )
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Invalid credentials"


async def test_login_with_nonexistent_user(service_client):
    """Test that login returns 401 for non-existent user"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "nonexistent_user",
            "password": "some_password"
        })
    )
    assert response.status == 401
    response_json = response.json()
    assert response_json["message"] == "Invalid credentials"


async def test_login_success_returns_token(service_client, default_user):
    """Test that successful login returns JWT token"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": default_user["login"],
            "password": default_user["password"]
        })
    )
    assert response.status == 200
    response_json = response.json()
    assert "token" in response_json
    assert response_json["login"] == default_user["login"]
    # Token should be a non-empty string
    assert isinstance(response_json["token"], str)
    assert len(response_json["token"]) > 0

