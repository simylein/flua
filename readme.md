## flua flights

a simple web application which presents flight and airtime statistics

### compile the source code

```sh
make
```

### create a database file

```sh
touch flua.sqlite
sqlite3 flua.sqlite
```

### initialize the database schema

```sql
create table user (
  id BLOB NOT NULL PRIMARY KEY,
  username TEXT NOT NULL UNIQUE,
  password TEXT NOT NULL
);
create index index_username on user(username);
create table flight (
  id BLOB NOT NULL PRIMARY KEY,
  hash BLOB NOT NULL UNIQUE,
  starts_at DATETIME NOT NULL,
  ends_at DATETIME NOT NULL,
  user_id BLOB NOT NULL,
  foreign key (user_id) references user(id) on delete cascade
);
create index index_user_id on flight(user_id);
create index index_starts_at on flight(starts_at);
```

### start the application

```sh
./flua
```

and enjoy ;^)
