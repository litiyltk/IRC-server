#!/bin/bash

DB_NAME="chat_db"
DB_USER="chat_user"
DB_PASSWORD="chat_pass"

execute_psql() {
    sudo -u postgres psql -v ON_ERROR_STOP=1 -c "$1"
}

echo "1. Создание пользователя и базы данных для чата"

USER_EXISTS=$(sudo -u postgres psql -tAqc "SELECT 1 FROM pg_roles WHERE rolname='${DB_USER}'")
if [ "$USER_EXISTS" != "1" ]; then
    execute_psql "CREATE ROLE ${DB_USER} LOGIN PASSWORD '${DB_PASSWORD}';"
else
    echo "Пользователь ${DB_USER} уже существует"
fi

DB_EXISTS=$(sudo -u postgres psql -tAqc "SELECT 1 FROM pg_database WHERE datname='${DB_NAME}'")
if [ "$DB_EXISTS" != "1" ]; then
    execute_psql "CREATE DATABASE ${DB_NAME} OWNER ${DB_USER};"
else
    echo "База данных ${DB_NAME} уже существует"
fi

echo "2. Выдача прав"
execute_psql "GRANT ALL PRIVILEGES ON DATABASE ${DB_NAME} TO ${DB_USER};"

echo "3. Создание таблиц"

export PGPASSWORD=${DB_PASSWORD}

psql -h localhost -U ${DB_USER} -d ${DB_NAME} -v ON_ERROR_STOP=1 <<EOF

CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY,
    username VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    registered_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS rooms (
    id UUID PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS messages (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
    message TEXT NOT NULL,
    sent_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS room_members (
    id UUID PRIMARY KEY,
    room_id UUID NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    joined_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE(room_id, user_id)
);

EOF

unset PGPASSWORD

echo "БД и таблицы созданы"
