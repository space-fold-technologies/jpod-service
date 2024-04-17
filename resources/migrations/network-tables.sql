CREATE TABLE
    IF NOT EXISTS network_tb (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        identifier VARCHAR(36) NOT NULL,
        name VARCHAR(25) NOT NULL,
        code VARCHAR(38) NOT NULL,
        driver TEXT NOT NULL,
        scope TEXT NOT NULL,
        subnet TEXT NOT NULL,
        status TEXT DEFAULT 'DOWN' NOT NULL,
        UNIQUE(name),
        UNIQUE(code) 
    );

CREATE TABLE
    IF NOT EXISTS network_member_tb (
        network_id INTEGER NOT NULL,
        container_id INTEGER NOT NULL,
        members VARCHAR(34) NOT NULL,
        CONSTRAINT network_member_fk FOREIGN KEY (network_id) REFERENCES network_tb(id) ON DELETE CASCADE,
        CONSTRAINT container_member_fk FOREIGN KEY (container_id) REFERENCES container_tb(id) ON UPDATE CASCADE ON DELETE CASCADE
    );