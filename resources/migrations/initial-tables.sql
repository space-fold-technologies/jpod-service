CREATE TABLE IF NOT EXISTS registry_tb(
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    uri TEXT NOT NULL,
    token TEXT,
    path NOT NULL
);

CREATE TABLE IF NOT EXISTS image_tb(
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    tag TEXT NOT NULL,
    os TEXT NOT NULL,
    variant TEXT NOT NULL,
    version TEXT NOT NULL,
    entry_point TEXT NULL,
    internals BLOB NOT NULL,
    identifier VARCHAR(36) NOT NULL,
    registry_id INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    FOREIGN KEY (registry_id) REFERENCES registry_tb(id)
);

CREATE TABLE IF NOT EXISTS container_tb(
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    identifier VARCHAR(36) NOT NULL,
    internals BLOB NOT NULL,
    status VARCHAR(10) NOT NULL,
    image_id INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, 
    FOREIGN KEY (image_id) REFERENCES image_tb(id)
);