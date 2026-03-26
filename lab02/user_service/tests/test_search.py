import json
import pytest


async def test_search_user_found(authorized_client):
    """Test search for existing user pattern by last name"""
    response = await authorized_client.get(
        "/v1/users/search?pattern=Test%"
    )
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json
    assert response_json["found"] is True
    assert "user" in response_json
    assert response_json["user"]["last_name"] == "Testov"


async def test_search_user_not_found(authorized_client):
    """Test search for non-existent user pattern by last name"""
    response = await authorized_client.get(
        "/v1/users/search?pattern=nonexistent"
    )
    assert response.status == 200
    response_json = response.json()
    assert "found" in response_json
    assert response_json["found"] is False
    assert "user" not in response_json or response_json.get("user") is None


async def test_search_exact_match(authorized_client_for_user):
    """Test search with exact last name match"""
    client = await authorized_client_for_user("exactuser", "123456", "Test", "ExactUser")
    
    response = await client.get(
        "/v1/users/search?pattern=ExactUser"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True
    assert "user" in response_json
    assert response_json["user"]["last_name"] == "ExactUser"


async def test_search_wildcard_underscore(authorized_client_for_user):
    """Test search with underscore wildcard (single character)"""
    client = await authorized_client_for_user("admin", "secret", "Test", "Admin")
    
    response = await client.get(
        "/v1/users/search?pattern=A____"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True
    assert "user" in response_json
    assert response_json["user"]["last_name"] == "Admin"


async def test_search_missing_pattern(authorized_client):
    """Test search without pattern parameter returns 400"""
    response = await authorized_client.get(
        "/v1/users/search"
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


async def test_search_empty_pattern(authorized_client):
    """Test search with empty pattern returns 400"""
    response = await authorized_client.get(
        "/v1/users/search?pattern="
    )
    assert response.status == 400
    response_json = response.json()
    assert "message" in response_json


async def test_search_case_insensitive(authorized_client_for_user):
    """Test search is case insensitive by last name"""
    client = await authorized_client_for_user("CaseUser", "123456", "Test", "CaseUser")
    
    response = await client.get(
        "/v1/users/search?pattern=case%"
    )
    assert response.status == 200
    response_json = response.json()
    assert response_json["found"] is True
    assert "user" in response_json
    assert response_json["user"]["last_name"] == "CaseUser"
