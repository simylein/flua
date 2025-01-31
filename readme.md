## flua flights

the source code of https://flua.dev

a simple web application which presents flight and airtime statistics

### compile the source code

for development

```sh
make develop
```

for production

```sh
make release
```

### create a database file

```sh
touch flua.sqlite
sqlite3 flua.sqlite
```

### initialize the database schema

```sql
create table user (
  id blob not null primary key,
  username text not null unique,
  password blob not null,
  public int not null
);
create index index_username on user(username);
create table flight (
  id blob not null primary key,
  hash blob not null unique,
  starts_at datetime not null,
  ends_at datetime not null,
  altitude blob not null,
  thermal blob not null,
  speed blob not null,
  glide blob not null,
  user_id blob not null,
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

### configure the application

check out the available command line flags

```sh
./flua --help
```
