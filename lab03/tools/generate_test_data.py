#!/usr/bin/env python3
"""
Script to generate test folders for existing users.
Creates realistic directory structures for performance testing
and index analysis.

Only uses user_service and directory_service (no file_service).
Generates JWT tokens locally using the same algorithm as the server.
"""

import asyncio
import asyncpg
import random
import argparse
import aiohttp
from datetime import datetime, timedelta
from typing import List, Dict, Optional
import json
import base64
import hmac
import hashlib


MAX_DEPTH = 4  # Maximum nesting depth for folders
FOLDERS_PER_LEVEL = 3  # Average folders per level

# JWT Configuration (must match server config)
JWT_SECRET = "super-secret-jwt-key"
JWT_ISSUER = "user-service"
JWT_EXPIRY_HOURS = 24

# Sample data for realistic folder names
FOLDER_NAMES = [
    "Documents", "Photos", "Videos", "Music", "Projects", "Work", "Personal",
    "Archive", "Downloads", "Uploads", "Backup", "Temp", "Reports", "Images",
    "Presentations", "Spreadsheets", "Notes", "Books", "Movies", "Software"
]


def base64url_encode(data: bytes) -> str:
    """Base64url encode without padding."""
    return base64.urlsafe_b64encode(data).rstrip(b'=').decode('utf-8')


def create_jwt(login: str, user_uuid: str) -> str:
    """
    Create a JWT token matching the server's format.
    
    Server code:
    auto token = jwt::create<jwt::traits::nlohmann_json>()
                     .set_issuer(jwt_credentials_.GetIssuer())
                     .set_type("JWT")
                     .set_subject(user.login)
                     .set_payload_claim("login", user.login)
                     .set_payload_claim("uuid", userver::utils::ToString(user.uuid))
                     .set_issued_at(now)
                     .set_expires_at(exp)
                     .sign(jwt::algorithm::hs256{jwt_credentials_.GetSecret()});
    """
    now = datetime.utcnow()
    exp = now + timedelta(hours=JWT_EXPIRY_HOURS)
    
    header = {
        "alg": "HS256",
        "typ": "JWT"
    }
    
    payload = {
        "iss": JWT_ISSUER,
        "sub": login,
        "login": login,
        "uuid": user_uuid,
        "iat": int(now.timestamp()),
        "exp": int(exp.timestamp())
    }
    
    header_b64 = base64url_encode(json.dumps(header, separators=(',', ':')).encode('utf-8'))
    payload_b64 = base64url_encode(json.dumps(payload, separators=(',', ':')).encode('utf-8'))
    
    message = f"{header_b64}.{payload_b64}"
    signature = hmac.new(
        JWT_SECRET.encode('utf-8'),
        message.encode('utf-8'),
        hashlib.sha256
    ).digest()
    signature_b64 = base64url_encode(signature)
    
    return f"{message}.{signature_b64}"


def generate_folder_name() -> str:
    """Generate a folder name."""
    return random.choice(FOLDER_NAMES)


class TestDataGenerator:
    def __init__(
        self,
        user_service_url: str,
        directory_service_url: str,
        db_host: str,
        db_port: int,
        db_user: str,
        db_password: str,
        db_name: str
    ):
        self.user_service_url = user_service_url
        self.directory_service_url = directory_service_url
        self.db_host = db_host
        self.db_port = db_port
        self.db_user = db_user
        self.db_password = db_password
        self.db_name = db_name
        self.session: Optional[aiohttp.ClientSession] = None

    async def connect_db(self) -> asyncpg.Connection:
        """Connect to the database."""
        return await asyncpg.connect(
            host=self.db_host,
            port=self.db_port,
            user=self.db_user,
            password=self.db_password,
            database=self.db_name
        )

    async def start_session(self):
        """Start HTTP session."""
        self.session = aiohttp.ClientSession()

    async def close_session(self):
        """Close HTTP session."""
        if self.session:
            await self.session.close()

    async def get_user_uuid(self, login: str) -> Optional[str]:
        """Get user UUID from database."""
        conn = await self.connect_db()
        try:
            row = await conn.fetchrow(
                "SELECT uuid FROM users WHERE login = $1",
                login
            )
            return str(row["uuid"]) if row else None
        finally:
            await conn.close()

    def generate_jwt_token(self, login: str, user_uuid: str) -> str:
        """Generate JWT token locally."""
        return create_jwt(login, user_uuid)

    async def create_directory(
        self,
        token: str,
        name: str,
        parent_id: Optional[str] = None
    ) -> Dict:
        """Create a directory."""
        url = f"{self.directory_service_url}/directories"
        payload = {"name": name}
        if parent_id:
            payload["parent_id"] = parent_id
        
        headers = {"Authorization": f"Bearer {token}"}
        
        async with self.session.post(url, json=payload, headers=headers) as resp:
            if resp.status in [200, 201]:
                return await resp.json()
            else:
                text = await resp.text()
                raise Exception(f"Failed to create directory '{name}': {resp.status} - {text}")

    async def list_directories(self, token: str, parent_id: Optional[str] = None) -> List[Dict]:
        """List directories."""
        url = f"{self.directory_service_url}/directories"
        params = {}
        if parent_id:
            params["parent_id"] = parent_id
        
        headers = {"Authorization": f"Bearer {token}"}
        
        async with self.session.get(url, params=params, headers=headers) as resp:
            if resp.status == 200:
                data = await resp.json()
                return data.get("items", [])
            else:
                text = await resp.text()
                raise Exception(f"Failed to list directories: {resp.status} - {text}")

    async def create_directory_tree(
        self,
        token: str,
        user_uuid: str,
        parent_id: Optional[str] = None,
        depth: int = 0,
        max_depth: int = MAX_DEPTH
    ) -> List[str]:
        """
        Create a tree of directories recursively.
        Returns list of created directory IDs.
        """
        created_ids = []
        
        if depth >= max_depth:
            return created_ids
        
        num_folders = random.randint(1, FOLDERS_PER_LEVEL)
        
        for i in range(num_folders):
            folder_name = f"{generate_folder_name()}_{depth}_{i}"
            
            try:
                result = await self.create_directory(token, folder_name, parent_id)
                dir_id = result["id"]
                created_ids.append(dir_id)
                
                sub_ids = await self.create_directory_tree(
                    token, user_uuid, dir_id, depth + 1, max_depth
                )
                created_ids.extend(sub_ids)
            except Exception as e:
                print(f"    Warning: {e}")
        
        return created_ids

    async def process_user(
        self,
        login: str,
        user_num: int,
        total_users: int
    ):
        """Process a single user: create directory tree."""
        print(f"[{user_num}/{total_users}] Processing user: {login}")
        
        try:
            user_uuid = await self.get_user_uuid(login)
            if not user_uuid:
                print(f"  ✗ User {login} not found in database")
                return
            
            token = self.generate_jwt_token(login, user_uuid)
            print(f"  Generated JWT for user {login} (UUID: {user_uuid})")
            
            root_dir_name = f"user_{login}_root"
            try:
                root_result = await self.create_directory(token, root_dir_name)
                root_id = root_result["id"]
                print(f"  Created root directory: {root_id}")
            except Exception as e:
                print(f"  Warning: Could not create root directory: {e}")
                dirs = await self.list_directories(token)
                if dirs:
                    root_id = dirs[0]["id"]
                    print(f"  Using existing root directory: {root_id}")
                else:
                    root_id = None
            
            print(f"  Creating directory tree (max depth: {MAX_DEPTH})...")
            all_dir_ids = [root_id] if root_id else []
            tree_ids = await self.create_directory_tree(token, user_uuid, root_id)
            all_dir_ids.extend(tree_ids)
            
            print(f"  ✓ Created {len(all_dir_ids)} directories for user {login}")
            
        except Exception as e:
            print(f"  ✗ Error processing user {login}: {e}")

    async def get_test_users(self, limit: int) -> List[str]:
        """Get list of test user logins from database."""
        conn = await self.connect_db()
        try:
            rows = await conn.fetch(
                "SELECT login FROM users WHERE login ~ '^[0-9]+\\.' ORDER BY login LIMIT $1",
                limit
            )
            return [row["login"] for row in rows]
        finally:
            await conn.close()

    async def run(self, user_count: int, skip_users: int = 0):
        """Main execution method."""
        print(f"=== Test Data Generator ===")
        print(f"User Service: {self.user_service_url}")
        print(f"Directory Service: {self.directory_service_url}")
        print(f"Database: {self.db_host}:{self.db_port}/{self.db_name}")
        print(f"JWT Secret: {JWT_SECRET[:10]}...")
        print(f"JWT Issuer: {JWT_ISSUER}")
        print()
        
        print(f"Fetching up to {user_count} test users from database...")
        users = await self.get_test_users(user_count + skip_users)
        
        if skip_users:
            users = users[skip_users:]
        
        if not users:
            print("No test users found! Run generate_test_users.py first.")
            return
        
        users = users[:user_count]
        total_users = len(users)
        print(f"Found {total_users} test users")
        print()
        
        await self.start_session()
        
        try:
            start_time = datetime.now()
            
            for i, login in enumerate(users, 1):
                await self.process_user(login, i, total_users)
                
                if i % 10 == 0 or i == total_users:
                    elapsed = (datetime.now() - start_time).total_seconds()
                    rate = i / elapsed if elapsed > 0 else 0
                    remaining = total_users - i
                    eta = remaining / rate if rate > 0 else 0
                    print(f"  Progress: {i}/{total_users} ({i*100//total_users}%) - ETA: {eta:.0f}s")

                await asyncio.sleep(0.1)
            
            elapsed = (datetime.now() - start_time).total_seconds()
            print()
            print(f"=== Generation Complete ===")
            print(f"  Processed {len(users)} users")
            print(f"  Time elapsed: {elapsed:.2f} seconds")
            print(f"  Average: {elapsed/len(users):.2f} seconds/user")
            
        finally:
            await self.close_session()


async def main():
    global MAX_DEPTH, FOLDERS_PER_LEVEL
    
    parser = argparse.ArgumentParser(
        description="Generate test folders for existing users"
    )
    
    # Service URLs
    parser.add_argument(
        "--user-service-url",
        default="http://localhost:8081/v1",
        help="User service URL (default: http://localhost:8081/v1)"
    )
    parser.add_argument(
        "--directory-service-url",
        default="http://localhost:8082/v1",
        help="Directory service URL (default: http://localhost:8082/v1)"
    )
    
    # Database connection
    parser.add_argument(
        "--db-host",
        default="localhost",
        help="PostgreSQL host (default: localhost)"
    )
    parser.add_argument(
        "--db-port",
        type=int,
        default=5432,
        help="PostgreSQL port (default: 5432)"
    )
    parser.add_argument(
        "--db-user",
        default="postgres",
        help="PostgreSQL user (default: postgres)"
    )
    parser.add_argument(
        "--db-password",
        default="postgres",
        help="PostgreSQL password (default: postgres)"
    )
    parser.add_argument(
        "--db-name",
        default="user_service",
        help="PostgreSQL database (default: user_service)"
    )
    
    # Generation parameters
    parser.add_argument(
        "--users",
        type=int,
        default=10,
        help="Number of users to process (default: 10)"
    )
    parser.add_argument(
        "--skip-users",
        type=int,
        default=0,
        help="Skip first N users (default: 0)"
    )
    parser.add_argument(
        "--max-depth",
        type=int,
        default=4,
        help="Maximum directory depth (default: 4)"
    )
    parser.add_argument(
        "--folders-per-level",
        type=int,
        default=3,
        help="Average folders per level (default: 3)"
    )
    
    args = parser.parse_args()
    
    MAX_DEPTH = args.max_depth
    FOLDERS_PER_LEVEL = args.folders_per_level
    
    generator = TestDataGenerator(
        user_service_url=args.user_service_url,
        directory_service_url=args.directory_service_url,
        db_host=args.db_host,
        db_port=args.db_port,
        db_user=args.db_user,
        db_password=args.db_password,
        db_name=args.db_name
    )
    
    await generator.run(
        user_count=args.users,
        skip_users=args.skip_users
    )


if __name__ == "__main__":
    asyncio.run(main())
