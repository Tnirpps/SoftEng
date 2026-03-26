import json
import pytest


NOW = '2019-12-31T11:22:33+00:00'

@pytest.mark.now(NOW)
async def test_login_success(service_client):
    """Test successful user login"""
    # First register a user
    
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Now login with the same credentials
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "testuser",
            "password": "123456"
        })
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["login"] == "testuser"
    assert "token" in response_json
    assert len(response_json["token"]) > 0


async def test_login_invalid_password(service_client):
    """Test login with invalid password returns 401"""
    # First register a user
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser2",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Try to login with wrong password
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "testuser2",
            "password": "wrongpassword"
        })
    )
    assert response.status == 401
    response_json = response.json()
    assert "message" in response_json


async def test_login_nonexistent_user(service_client):
    """Test login with non-existent user returns 401"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "nonexistentuser",
            "password": "123456"
        })
    )
    assert response.status == 401
    response_json = response.json()
    assert "message" in response_json


async def test_login_empty_login(service_client):
    """Test login with empty login returns 400"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "",
            "password": "123456"
        })
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


async def test_login_empty_password(service_client):
    """Test login with empty password returns 400"""
    response = await service_client.post(
        "/v1/users/login",
        data=json.dumps({
            "login": "testuser",
            "password": ""
        })
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json
