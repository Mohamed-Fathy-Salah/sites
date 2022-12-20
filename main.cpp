#include <cstdlib>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

sqlite3 *db;

void initDB();
void newSite(int argc, char *argv[]);
void modifySite(int argc, char *argv[]);
void removeSite(int argc, char *argv[]);
void listSites(int argc, char *argv[]);
void openSite();

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
    openSite();
  }

  sqlite3_close(db);
  return 0;
}

void exec(std::string sql) {
  char *err_msg = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    exit(1);
  }
}

void initDB() {
  // todo: get env $HOME
  const std::string DATA_PATH = std::string(getenv("HOME")) + "/.config/sites.db";
  char *err_msg = 0;

  int rc = sqlite3_open(DATA_PATH.c_str(), &db);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  exec("CREATE TABLE IF NOT EXISTS sites (id INTEGER PRIMARY KEY "
       "AUTOINCREMENT, name VARCHAR(20) NOT NULL, url "
       "VARCHAR(200) NOT NULL, before_command VARCHAR(50), after_command "
       "VARCHAR(50), finished INTEGER DEFAULT 0);");
}

void newSite(int argc, char *argv[]) {
  int opt;
  std::string name, url, beforeCommand, afterCommand;

  while ((opt = getopt(argc, argv, "s:u:b:a:")) != -1) {
    switch (opt) {
    case 's':
      name = optarg;
      break;
    case 'u':
      url = optarg;
      break;
    case 'b':
      beforeCommand = optarg;
      break;
    case 'a':
      afterCommand = optarg;
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
  exec("INSERT INTO sites (name,url,before_command,after_command) VALUES(\"" +
       name + "\",\"" + url + "\",\"" + beforeCommand + "\",\"" + afterCommand +
       "\");");
}

void modifySite(int argc, char *argv[]) {
  int opt;
  std::string name, url, beforeCommand, afterCommand, finished, id;

  while ((opt = getopt(argc, argv, "i:s:u:b:a:f:")) != -1) {
    switch (opt) {
    case 'i':
      id = std::stoi(optarg);
      break;
    case 's':
      name = optarg;
      break;
    case 'u':
      url = optarg;
      break;
    case 'b':
      beforeCommand = optarg;
      break;
    case 'a':
      afterCommand = optarg;
      break;
    case 'f':
      finished = std::stoi(optarg);
      break;
    case ':':
      printf("option needs a value\n");
      exit(1);
    case '?':
      puts("not an option");
      exit(1);
    }
  }

  if (id.empty()) {
    puts("id is required");
    exit(1);
  }

  // no need to check for sql injection bcs it is user sided
  std::string sql = "UPDATE sites SET";

  if (name[0])
    sql += " name = \"" + name + "\",";
  if (url[0])
    sql += " url = \"" + url + "\",";
  if (beforeCommand[0])
    sql += " before_command = \"" + beforeCommand + "\",";
  if (afterCommand[0])
    sql += " after_command = \"" + afterCommand + "\",";

  // toggle finished
  if (finished[0] == '2')
    sql += " finished = 1 - finished,";
  else if (finished[0] == '0' || finished[0] == '1')
    sql += " finished = " + finished + ",";

  // remove last comma
  sql.pop_back();

  sql += " WHERE id = " + id + ",";

  exec(sql);
}

void removeSite(int argc, char *argv[]) {
  std::string id;

  switch (getopt(argc, argv, "i:")) {
  case 'i':
    id = optarg;
    break;
  case ':':
    printf("option needs a value\n");
    exit(1);
  case '?':
    puts("not an option");
    exit(1);
  }

  exec("DELETE FROM sites WHERE id = " + id);
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
      f = optarg[0];
      break;
    case ':':
      printf("option needs a value\n");
      exit(1);
    case '?':
      puts("not an option");
      exit(1);
    }
  }

  std::string sql = "SELECT ";
  if (!s && !u && !b && !a && !c)
    sql += "*";
  else {
    sql += "id";
    if (s)
      sql += ",name";
    if (u)
      sql += ",url";
    if (b)
      sql += ",before_command";
    if (a)
      sql += ",after_command";
    if (c)
      sql += ",finished";
  }

  sql += " FROM sites";

  if (f == 0 || f == 1)
    sql += &" WHERE finished = "[f];

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

  while (sqlite3_step(stmt) != SQLITE_DONE) {
    int num_cols = sqlite3_column_count(stmt);
    for (int i = 0; i < num_cols; i++)
      printf("%s\t", sqlite3_column_text(stmt, i));
    printf("\n");
  }
  sqlite3_finalize(stmt);
}

void openSite() {
  // todo: reset completed if today != last day modified
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db,
                     "SELECT id || '_' || name FROM sites WHERE finished = 0;",
                     -1, &stmt, NULL);

  std::string names;
  while (sqlite3_step(stmt) != SQLITE_DONE)
    names += std::string(
                 reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))) +
             "\n";
  sqlite3_finalize(stmt);

  FILE *pp;
  std::string cmd = "echo '" + names + "' | dmenu -i";

  pp = popen(cmd.c_str(), "r");
  char *line;

  if (pp == NULL)
    exit(1);
  char buf[30];
  line = fgets(buf, sizeof buf, pp);
  if (line == NULL)
    exit(1);
  pclose(pp);

  // keep the id only
  char *p = line;
  while (*p != '_')
    p++;
  *p = '\0';

  std::string sql =
      "SELECT before_command, url, after_command FROM sites WHERE id = " +
      std::string(line);

  // cmd = {before_command, url, after_command}
  sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
  if (sqlite3_step(stmt) == SQLITE_DONE)
    exit(1);

  cmd = "";

  std::string before =
      std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
  if (before.size())
    cmd += before + " && ";

  cmd +=
      "$BROWSER " +
      std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));

  std::string after =
      std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
  if (after.size())
    cmd += " && " + after;

  sqlite3_finalize(stmt);
  system(cmd.c_str());
}
