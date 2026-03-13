import json
import pytest


NOW = '2019-12-31T11:22:33Z'


@pytest.mark.now(NOW)
async def test_register_user_success(service_client):
    """Test successful user registration"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert response.status == 201
    response_json = response.json()
    assert response_json["login"] == "testuser"
    assert response_json["first_name"] == "Test"
    assert response_json["last_name"] == "User"
    assert "id" in response_json
    assert "created_at" in response_json


@pytest.mark.now(NOW)
async def test_register_user_empty_login(service_client):
    """Test registration with empty login returns 400"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


@pytest.mark.now(NOW)
async def test_register_user_empty_password(service_client):
    """Test registration with empty password returns 400"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser",
            "password": "",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


@pytest.mark.now(NOW)
async def test_register_user_duplicate(service_client):
    """Test registration with duplicate login returns 409"""
    # First registration
    response1 = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "duplicateuser",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert response1.status == 201

    # Second registration with same login
    response2 = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "duplicateuser",
            "password": "anotherpass",
            "first_name": "Another",
            "last_name": "User"
        })
    )
    assert response2.status == 409
    response_json = response2.json()
    assert "message" in response_json


@pytest.mark.now(NOW)
async def test_register_user_missing_fields(service_client):
    """Test registration with missing required fields returns 400"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser"
            # Missing password, first_name, last_name
        })
    )
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_register_user_min_length_login(service_client):
    """Test registration with login below minimum length"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "abc",  # Less than 4 characters
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    # Should return 400 due to validation
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_register_user_max_length_login(service_client):
    """Test registration with login above maximum length"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "a" * 21,  # More than 20 characters
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    # Should return 400 due to validation
    assert response.status == 400


@pytest.mark.now(NOW)
async def test_register_user_min_length_password(service_client):
    """Test registration with password below minimum length"""
    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testuser",
            "password": "abc",  # Less than 6 characters
            "first_name": "Test",
            "last_name": "User"
        })
    )
    # Should return 400 due to validation
    assert response.status == 400
