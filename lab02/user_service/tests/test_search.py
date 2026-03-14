import json
import pytest


@pytest.mark.asyncio
async def test_search_user_found(service_client):
    """Test search for existing user pattern"""
    # First register a user
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "testsearch",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Search for user with pattern
    response = await service_client.get(
        "/v1/users/search?pattern=test%"
    )
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json
    assert response_json["found"] is True


@pytest.mark.asyncio
async def test_search_user_not_found(service_client):
    """Test search for non-existent user pattern"""
    response = await service_client.get(
        "/v1/users/search?pattern=nonexistent"
    )
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json
    assert response_json["found"] is False


@pytest.mark.asyncio
async def test_search_exact_match(service_client):
    """Test search with exact login match"""
    # Register a user
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "exactuser",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Search with exact pattern
    response = await service_client.get(
        "/v1/users/search?pattern=exactuser"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True


@pytest.mark.asyncio
async def test_search_wildcard_underscore(service_client):
    """Test search with underscore wildcard (single character)"""
    # Register a user
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "admin",
            "password": "secret",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Search with underscore pattern (a_min matches admin)
    response = await service_client.get(
        "/v1/users/search?pattern=a____"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True


@pytest.mark.asyncio
async def test_search_missing_pattern(service_client):
    """Test search without pattern parameter returns 400"""
    response = await service_client.get(
        "/v1/users/search"
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


@pytest.mark.asyncio
async def test_search_empty_pattern(service_client):
    """Test search with empty pattern returns 400"""
    response = await service_client.get(
        "/v1/users/search?pattern="
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


@pytest.mark.asyncio
async def test_search_case_insensitive(service_client):
    """Test search is case insensitive"""
    # Register a user
    register_response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "CaseUser",
            "password": "123456",
            "first_name": "Test",
            "last_name": "User"
        })
    )
    assert register_response.status == 201

    # Search with lowercase pattern
    response = await service_client.get(
        "/v1/users/search?pattern=case%"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True
