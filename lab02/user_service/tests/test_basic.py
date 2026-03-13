import json
import pytest


NOW='2019-12-31T11:22:33Z'

@pytest.mark.now(NOW)
async def test_basic(service_client):

    response = await service_client.post(
        "/v1/users",
        # params={
        #     "name": "Tester"
        # },
        data=json.dumps({
            "login": "test",
            "password": "123456",
            "first_name": "test",
            "last_name": "test"
        })
    )
    assert response.status == 201
    response_json = response.json()

    assert "id" in response_json
    assert response_json["login"] == "test"
    assert response_json["first_name"] == "test"
    assert response_json["last_name"] == "test"
    # assert response_json["created_at"] == NOW
