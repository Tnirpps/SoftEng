#!/bin/bash

# Script to register a user in user_service and create a root folder in directory_service
# Usage: ./register_and_create_folder.sh [LOGIN] [PASSWORD] [FIRST_NAME] [LAST_NAME] [FOLDER_NAME]

set -e

# Default values
LOGIN="${1:-testuser}"
PASSWORD="${2:-testpassword123}"
FIRST_NAME="${3:-Test}"
LAST_NAME="${4:-User}"
FOLDER_NAME="${5:-MyFolder}"

USER_SERVICE_URL="${USER_SERVICE_URL:-http://localhost:8081/v1}"
DIRECTORY_SERVICE_URL="${DIRECTORY_SERVICE_URL:-http://localhost:8082/v1}"

echo "=== User Registration and Folder Creation Script ==="
echo "User Service URL: $USER_SERVICE_URL"
echo "Directory Service URL: $DIRECTORY_SERVICE_URL"
echo ""

echo "Step 1: Registering user '$LOGIN'..."
REGISTER_RESPONSE=$(curl -s -w $'\n%{http_code}' -X POST "$USER_SERVICE_URL/users" \
    -H "Content-Type: application/json" \
    -d "{
        \"login\": \"$LOGIN\",
        \"password\": \"$PASSWORD\",
        \"first_name\": \"$FIRST_NAME\",
        \"last_name\": \"$LAST_NAME\"
    }")

REGISTER_BODY=$(echo "$REGISTER_RESPONSE" | sed '$d')
REGISTER_CODE=$(echo "$REGISTER_RESPONSE" | tail -n 1)

echo "Register response code: $REGISTER_CODE"
echo "Register response body: $REGISTER_BODY"

if [ "$REGISTER_CODE" != "201" ] && [ "$REGISTER_CODE" != "200" ]; then
    if [ "$REGISTER_CODE" = "409" ]; then
        echo "User '$LOGIN' already exists. Proceeding to login..."
    else
        echo "Failed to register user. HTTP code: $REGISTER_CODE"
        echo "Response: $REGISTER_BODY"
        exit 1
    fi
fi

echo ""
echo "Step 2: Logging in to get JWT token..."
LOGIN_RESPONSE=$(curl -s -w $'\n%{http_code}' -X POST "$USER_SERVICE_URL/users/login" \
    -H "Content-Type: application/json" \
    -d "{
        \"login\": \"$LOGIN\",
        \"password\": \"$PASSWORD\"
    }")

LOGIN_BODY=$(echo "$LOGIN_RESPONSE" | sed '$d')
LOGIN_CODE=$(echo "$LOGIN_RESPONSE" | tail -n 1)

echo "Login response code: $LOGIN_CODE"
echo "Login response body: $LOGIN_BODY"

if [ "$LOGIN_CODE" != "200" ]; then
    echo "Failed to login. HTTP code: $LOGIN_CODE"
    echo "Response: $LOGIN_BODY"
    exit 1
fi

TOKEN=$(echo "$LOGIN_BODY" | sed -n 's/.*"token":"\([^"]*\)".*/\1/p')

if [ -z "$TOKEN" ]; then
    echo "Failed to extract JWT token from login response"
    echo "Login response: $LOGIN_BODY"
    exit 1
fi

echo ""
echo "JWT Token received: ${TOKEN:0:50}..."

echo ""
echo "Step 3: Creating folder '$FOLDER_NAME' in directory service..."
FOLDER_RESPONSE=$(curl -s -w $'\n%{http_code}' -X POST "$DIRECTORY_SERVICE_URL/directories" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d "{
        \"name\": \"$FOLDER_NAME\"
    }")

FOLDER_BODY=$(echo "$FOLDER_RESPONSE" | sed '$d')
FOLDER_CODE=$(echo "$FOLDER_RESPONSE" | tail -n 1)

echo "Folder creation response code: $FOLDER_CODE"
echo "Folder creation response body: $FOLDER_BODY"

if [ "$FOLDER_CODE" != "201" ] && [ "$FOLDER_CODE" != "200" ]; then
    echo "Failed to create folder. HTTP code: $FOLDER_CODE"
    echo "Response: $FOLDER_BODY"
    exit 1
fi

echo ""
echo "=== Success! ==="
echo "User '$LOGIN' registered and logged in."
echo "Folder '$FOLDER_NAME' created in directory service."
echo "JWT Token: $TOKEN"
