CREATE TABLE
    IF NOT EXISTS registry_tb (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        uri TEXT NOT NULL,
        authorization_type VARCHAR(10),
        authorization_url TEXT,
        path TEXT,
        CONSTRAINT registry_unq UNIQUE (name)
    );

CREATE TABLE
    IF NOT EXISTS image_tb (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        repository TEXT NOT NULL,
        tag TEXT NOT NULL,
        os TEXT NOT NULL,
        size INTEGER NOT NULL,
        variant TEXT NOT NULL,
        version TEXT NOT NULL,
        internals BLOB NOT NULL,
        identifier VARCHAR(36) NOT NULL,
        registry_id INTEGER NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        CONSTRAINT registry_fk FOREIGN KEY (registry_id) REFERENCES registry_tb (id) ON UPDATE RESTRICT ON DELETE RESTRICT,
        CONSTRAINT image_unq UNIQUE (repository, version, tag)
    );

CREATE TABLE
    IF NOT EXISTS container_tb (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        identifier VARCHAR(36) NOT NULL,
        internals BLOB NOT NULL,
        status VARCHAR(10) NOT NULL,
        image_id INTEGER NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        CONSTRAINT image_fk FOREIGN KEY (image_id) REFERENCES image_tb (id) ON UPDATE RESTRICT ON DELETE RESTRICT,
        CONSTRAINT container_unq UNIQUE (name)
    );