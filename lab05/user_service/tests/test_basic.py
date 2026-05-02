import json
import pytest


async def test_basic(service_client):

    response = await service_client.post(
        "/v1/users",
        data=json.dumps({
            "login": "test",
            "password": "123456",
            "first_name": "test",
            "last_name": "test"
        })
    )
    assert response.status == 201
    response_json = response.json()

    assert response_json["login"] == "test"
    assert response_json["first_name"] == "test"
    assert response_json["last_name"] == "test"
