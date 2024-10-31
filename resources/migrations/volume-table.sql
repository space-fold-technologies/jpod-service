CREATE TABLE 
    IF NOT EXISTS volume_tb (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        identifier VARCHAR(36) NOT NULL,
        container_id INTEGER NOT NULL,
        driver VARCHAR(10) NOT NULL,
        filesystem VARCHAR(15) NOT NULL,
        path TEXT NOT NULL,
        source TEXT NOT NULL,
        options TEXT NOT NULL,
        CONSTRAINT container_volume_fk FOREIGN KEY (container_id) REFERENCES container_tb (id) ON DELETE CASCADE
    );