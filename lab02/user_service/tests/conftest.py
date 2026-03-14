import pytest


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