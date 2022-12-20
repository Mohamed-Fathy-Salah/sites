#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

sqlite3 *db;

int toInt(char *s) {
  int n = 0;
  while (*s) {
    n = n * 10 + (*s - '0');
    s++;
  }
  return n;
}

void concat(char *sql, const char *str, char *val) {
  char tmp[250];
  sprintf(tmp, str, val);
  strcat(sql, tmp);
}

void exec(char *sql, void *cb) {
  char *err_msg = 0;
  int rc = sqlite3_exec(db, sql, cb, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    exit(1);
  }
}

void initDB() {
  const char *DATA_PATH = "/home/mofasa/.config/routine/data.db";
  char *err_msg = 0;

  int rc = sqlite3_open(DATA_PATH, &db);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  exec("CREATE TABLE IF NOT EXISTS sites (id INTEGER PRIMARY KEY "
       "AUTOINCREMENT, name VARCHAR(20) NOT NULL, url "
       "VARCHAR(200) NOT NULL, before_command VARCHAR(50), after_command "
       "VARCHAR(50), finished INTEGER DEFAULT 0);",
       0);
}

void newSite(int argc, char *argv[]) {
  int opt;
  char name[21], url[201], beforeCommand[51], afterCommand[51];

  while ((opt = getopt(argc, argv, "s:u:b:a:")) != -1) {
    switch (opt) {
    case 's':
      strcpy(name, optarg);
      break;
    case 'u':
      strcpy(url, optarg);
      break;
    case 'b':
      strcpy(beforeCommand, optarg);
      break;
    case 'a':
      strcpy(afterCommand, optarg);
      break;
    case ':':
      printf("option needs a value\n");
      break;
    case '?':
      puts("not an option");
      break;
    }
  }

  if (!name[0] || !url[0]) {
    puts("name, url are required");
    exit(1);
  }

  // no need to check for sql injection bcs it is user sided
  char sql[360];
  sprintf(sql,
          "INSERT INTO sites (name,url,before_command,after_command) "
          "VALUES(\"%s\",\"%s\",\"%s\",\"%s\");",
          name, url, beforeCommand, afterCommand);
  exec(sql, 0);
}

void modifySite(int argc, char *argv[]) {
  int opt, id = -1, finished = -1;
  char name[21], url[201], beforeCommand[51], afterCommand[51];

  while ((opt = getopt(argc, argv, "i:s:u:b:a:f:")) != -1) {
    switch (opt) {
    case 'i':
      id = toInt(optarg);
      break;
    case 's':
      strcpy(name, optarg);
      break;
    case 'u':
      strcpy(url, optarg);
      break;
    case 'b':
      strcpy(beforeCommand, optarg);
      break;
    case 'a':
      strcpy(afterCommand, optarg);
      break;
    case 'f':
      finished = toInt(optarg);
      break;
    case ':':
      printf("option needs a value\n");
      break;
    case '?':
      puts("not an option");
      break;
    }
  }

  if (id == -1) {
    puts("id is required");
    exit(1);
  }

  // no need to check for sql injection bcs it is user sided
  char sql[360] = "UPDATE sites SET";

  if (name[0]) concat(sql, " name = \"%s\",", name);
  if (url[0]) concat(sql, " url = \"%s\",", url);
  if (beforeCommand[0]) concat(sql, " before_command = \"%s\",", beforeCommand);
  if (afterCommand[0]) concat(sql, " after_command = \"%s\",", afterCommand);

  // remove last comma
  sql[strlen(sql) - 1] = '\0';

  // toggle finished
  if (finished == 2)
    strcat(sql, " finished = !finished");
  else if (finished == 0 || finished == 1) {
    char tmp[100];
    sprintf(tmp, " finished = %d, ", finished);
    strcat(sql, tmp);
  }

  char tmp[20];
  sprintf(tmp, " WHERE id = %d;", id);
  strcat(sql, tmp);

  exec(sql, 0);
}

void removeSite(int argc, char *argv[]) {}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  NotUsed = 0;
  for (int i = 0; i < argc; i++)
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  return 0;
}

void listSites(int argc, char *argv[]) {
  exec("SELECT * from sites;", callback);
}

void open() {
  // reset completed if today != last day modified
}

int main(int argc, char *argv[]) {
  initDB();

  switch (getopt(argc, argv, "nmrl")) {
  case 'n':
    newSite(argc, argv);
    break;
  case 'm':
    modifySite(argc, argv);
    break;
  case 'r':
    removeSite(argc, argv);
    break;
  case 'l':
    listSites(argc, argv);
    break;
  case '?':
    printf("unknown option: %c\n", optopt);
    break;
  default:
    open();
  }

  sqlite3_close(db);
  return 0;
}
