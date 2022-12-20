#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

sqlite3 *db;

int toInt(char *s);
void concat(char *sql, const char *str, char *val);
void exec(char *sql, void *cb);
void initDB();
void newSite(int argc, char *argv[]);
void modifySite(int argc, char *argv[]);
void removeSite(int argc, char *argv[]);
int callback(void *NotUsed, int argc, char **argv, char **azColName);
void listSites(int argc, char *argv[]);
void open();

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
  // todo: get env $HOME
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
      exit(1);
    case '?':
      puts("not an option");
      exit(1);
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
      exit(1);
    case '?':
      puts("not an option");
      exit(1);
    }
  }

  if (id == -1) {
    puts("id is required");
    exit(1);
  }

  // no need to check for sql injection bcs it is user sided
  char sql[360] = "UPDATE sites SET";

  if (name[0])
    concat(sql, " name = \"%s\",", name);
  if (url[0])
    concat(sql, " url = \"%s\",", url);
  if (beforeCommand[0])
    concat(sql, " before_command = \"%s\",", beforeCommand);
  if (afterCommand[0])
    concat(sql, " after_command = \"%s\",", afterCommand);

  // toggle finished
  if (finished == 2)
    strcat(sql, " finished = 1 - finished,");
  else if (finished == 0 || finished == 1) {
    char tmp[100];
    sprintf(tmp, " finished = %d,", finished);
    strcat(sql, tmp);
  }

  // remove last comma
  sql[strlen(sql) - 1] = '\0';

  char tmp[20];
  sprintf(tmp, " WHERE id = %d;", id);
  strcat(sql, tmp);

  exec(sql, 0);
}

void removeSite(int argc, char *argv[]) {
  int id = -1;

  switch (getopt(argc, argv, "i:")) {
  case 'i':
    id = toInt(optarg);
    break;
  case ':':
    printf("option needs a value\n");
    exit(1);
  case '?':
    puts("not an option");
    exit(1);
  }

  char sql[50];
  sprintf(sql, "DELETE FROM sites WHERE id = %d;", id);
  exec(sql, 0);
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  NotUsed = 0;
  for (int i = 0; i < argc; i++)
    printf("%s\t", argv[i] ? argv[i] : "NULL");
  printf("\n");
  return 0;
}

void listSites(int argc, char *argv[]) {
  int opt;
  char s = 0, u = 0, b = 0, a = 0, c = 0, f = 2;
  while ((opt = getopt(argc, argv, "subacf:")) != -1) {
    switch (opt) {
    case 's':
      s = 1;
      break;
    case 'u':
      u = 1;
      break;
    case 'b':
      b = 1;
      break;
    case 'a':
      a = 1;
      break;
    case 'c':
      c = 1;
      break;
    case 'f':
      f = toInt(optarg);
      break;
    case ':':
      printf("option needs a value\n");
      exit(1);
    case '?':
      puts("not an option");
      exit(1);
    }
  }

  char sql[250] = "SELECT ";
  if (!s && !u && !b && !a && !c)
    strcat(sql, "*,");
  else {
    if (s)
      strcat(sql, "name,");
    if (u)
      strcat(sql, "url,");
    if (b)
      strcat(sql, "before_command,");
    if (a)
      strcat(sql, "after_command,");
    if (c)
      strcat(sql, "finished,");
  }

  // remove last comma
  sql[strlen(sql) - 1] = '\0';

  strcat(sql, " FROM sites");

  if (f == 0 || f == 1) {
    char tmp[100];
    sprintf(tmp, " WHERE finished = %d;", f);
    strcat(sql, tmp);
  }

  // todo: use stmt as in open
  //https://gist.github.com/jsok/2936764
  exec(sql, callback);
}

void open() {
  // todo: reset completed if today != last day modified
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id || '_' || name FROM sites WHERE finished = 0;", -1, &stmt, NULL);

    char names[500];
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        char name[30];
        sprintf(name, "%s\n", sqlite3_column_text(stmt, 0));
        strcat(names, name);
    }
    sqlite3_finalize(stmt);

  FILE *pp;
  char cmd[600];
  sprintf(cmd, "echo '%s' | dmenu -i", names);

  pp = popen(cmd, "r");
  if (pp != NULL) {
      char *line;
      char buf[30];
      line = fgets(buf, sizeof buf, pp);
      if (line == NULL) exit(1);
      printf("%s", line); 
    pclose(pp);
  }

  // todo: select before_command, url, after_command from sites where id = ?
  // todo: run before_command && $BROWSER url && after_command
}
