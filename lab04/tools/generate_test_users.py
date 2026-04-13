#!/usr/bin/env python3

import asyncio
import asyncpg
import random
import string
from datetime import datetime
from typing import List, Tuple
import argparse


# Sample data for generating realistic test users
FIRST_NAMES = [
    "James", "Mary", "John", "Patricia", "Robert", "Jennifer", "Michael", "Linda",
    "William", "Elizabeth", "David", "Barbara", "Richard", "Susan", "Joseph", "Jessica",
    "Thomas", "Sarah", "Charles", "Karen", "Christopher", "Nancy", "Daniel", "Lisa",
    "Matthew", "Betty", "Anthony", "Margaret", "Mark", "Sandra", "Donald", "Ashley",
    "Steven", "Kimberly", "Paul", "Emily", "Andrew", "Donna", "Joshua", "Michelle",
    "Kenneth", "Dorothy", "Kevin", "Carol", "Brian", "Amanda", "George", "Melissa",
    "Edward", "Deborah", "Ronald", "Stephanie", "Timothy", "Rebecca", "Jason", "Sharon",
    "Jeffrey", "Laura", "Ryan", "Cynthia", "Jacob", "Kathleen", "Gary", "Amy",
    "Nicholas", "Angela", "Eric", "Shirley", "Jonathan", "Anna", "Stephen", "Brenda"
]

LAST_NAMES = [
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
    "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson",
    "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Perez", "Thompson",
    "White", "Harris", "Sanchez", "Clark", "Ramirez", "Lewis", "Robinson", "Walker",
    "Young", "Allen", "King", "Wright", "Scott", "Torres", "Nguyen", "Hill", "Flores",
    "Green", "Adams", "Nelson", "Baker", "Hall", "Rivera", "Campbell", "Mitchell",
    "Carter", "Roberts", "Gomez", "Phillips", "Evans", "Turner", "Diaz", "Parker",
    "Cruz", "Edwards", "Collins", "Reyes", "Stewart", "Morris", "Morales", "Murphy"
]


def generate_login(first_name: str, last_name: str, unique_id: int) -> str:
    """Generate a unique login from name and ID."""
    # Put unique_id first to guarantee uniqueness (names can collide)
    # Format: {id}.{firstname}.{lastname} truncated to 20 chars
    base = f"{unique_id}.{first_name.lower()}.{last_name.lower()}"
    return base[:20]  # Max 20 chars as per schema


def generate_password() -> str:
    """Generate a random password."""
    chars = string.ascii_letters + string.digits + "!@#$%"
    return ''.join(random.choices(chars, k=12))


def generate_user(unique_id: int) -> Tuple[str, str, str, str]:
    """Generate a single test user."""
    first_name = random.choice(FIRST_NAMES)
    last_name = random.choice(LAST_NAMES)
    login = generate_login(first_name, last_name, unique_id)
    password = generate_password()
    return (login, password, first_name, last_name)


async def insert_users_batch(
    conn: asyncpg.Connection,
    users: List[Tuple[str, str, str, str]]
) -> int:
    """Insert a batch of users using COPY for maximum performance."""
    try:
        # Use copy_records_to_table for bulk insert
        async with conn.transaction():
            await conn.copy_records_to_table(
                'users',
                records=users,
                columns=['login', 'password', 'first_name', 'last_name']
            )
        return len(users)
    except Exception as e:
        print(f"Error during batch insert: {e}")
        return 0


async def generate_and_insert_users(
    host: str,
    port: int,
    user: str,
    password: str,
    database: str,
    count: int,
    batch_size: int = 1000
):
    """Generate and insert test users into the database."""
    print(f"Connecting to PostgreSQL at {host}:{port}...")
    
    conn = await asyncpg.connect(
        host=host,
        port=port,
        user=user,
        password=password,
        database=database
    )
    
    try:
        print(f"Generating {count} test users...")
        start_time = datetime.now()
        
        total_inserted = 0
        batch = []
        
        for i in range(count):
            batch.append(generate_user(i))
            
            if len(batch) >= batch_size:
                inserted = await insert_users_batch(conn, batch)
                total_inserted += inserted
                batch = []
                
                if total_inserted % 10000 == 0:
                    elapsed = (datetime.now() - start_time).total_seconds()
                    rate = total_inserted / elapsed if elapsed > 0 else 0
                    print(f"  Inserted {total_inserted:,} users ({rate:.0f} users/sec)")
        
        # Insert remaining users
        if batch:
            inserted = await insert_users_batch(conn, batch)
            total_inserted += inserted
        
        elapsed = (datetime.now() - start_time).total_seconds()
        rate = total_inserted / elapsed if elapsed > 0 else 0
        
        print(f"\n✓ Successfully inserted {total_inserted:,} users")
        print(f"  Time elapsed: {elapsed:.2f} seconds")
        print(f"  Average rate: {rate:.0f} users/second")
        
        # Verify count
        async with conn.transaction():
            row = await conn.fetchrow("SELECT COUNT(*) FROM users")
            print(f"  Total users in database: {row[0]:,}")
        
    finally:
        await conn.close()


async def clear_users(
    host: str,
    port: int,
    user: str,
    password: str,
    database: str
):
    """Clear all users from the database."""
    print(f"Connecting to PostgreSQL at {host}:{port}...")
    
    conn = await asyncpg.connect(
        host=host,
        port=port,
        user=user,
        password=password,
        database=database
    )
    
    try:
        async with conn.transaction():
            result = await conn.execute("DELETE FROM users")
        print(f"✓ Cleared all users from database")
    finally:
        await conn.close()


async def main():
    parser = argparse.ArgumentParser(
        description="Generate test users for performance testing"
    )
    parser.add_argument(
        "--host",
        default="localhost",
        help="PostgreSQL host (default: localhost)"
    )
    parser.add_argument(
        "--port",
        type=int,
        default=5432,
        help="PostgreSQL port (default: 5432)"
    )
    parser.add_argument(
        "--user",
        default="postgres",
        help="PostgreSQL user (default: postgres)"
    )
    parser.add_argument(
        "--password",
        default="postgres",
        help="PostgreSQL password (default: postgres)"
    )
    parser.add_argument(
        "--database",
        default="user_service",
        help="PostgreSQL database (default: user_service)"
    )
    parser.add_argument(
        "--count",
        type=int,
        default=10000,
        help="Number of users to generate (default: 10000)"
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=1000,
        help="Batch size for inserts (default: 1000)"
    )
    parser.add_argument(
        "--clear",
        action="store_true",
        help="Clear all users before inserting"
    )
    parser.add_argument(
        "--clear-only",
        action="store_true",
        help="Only clear users, don't insert new ones"
    )
    
    args = parser.parse_args()
    
    if args.clear or args.clear_only:
        await clear_users(
            args.host, args.port, args.user, args.password, args.database
        )
        if args.clear_only:
            return
    
    await generate_and_insert_users(
        host=args.host,
        port=args.port,
        user=args.user,
        password=args.password,
        database=args.database,
        count=args.count,
        batch_size=args.batch_size
    )


if __name__ == "__main__":
    asyncio.run(main())
