#include "Database.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Database::Database() : m_db(nullptr) {}
Database::~Database() { close(); }

string Database::escapeSQL(const string& str) {
    string result;
    for (char c : str) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

bool Database::open(const string &filename) {
    int rc = sqlite3_open(filename.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        cerr << "Ошибка открытия БД: " << sqlite3_errmsg(m_db) << endl;
        return false;
    }
    
    return initTables();
}

bool Database::initTables() {
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            login TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'user',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS threat_types (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        )
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS risk_levels (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            weight INTEGER NOT NULL
        )
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS threats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type_id INTEGER NOT NULL,
            risk_level_id INTEGER NOT NULL,
            description TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (type_id) REFERENCES threat_types(id),
            FOREIGN KEY (risk_level_id) REFERENCES risk_levels(id)
        )
    )");
    
    executeSQL(R"(
        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            event TEXT NOT NULL,
            details TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id)
        )
    )");
    
    initReferenceData();
    
    return true;
}

void Database::initReferenceData() {
    executeSQL(R"(
        INSERT OR IGNORE INTO threat_types (name) VALUES 
        ('Конфиденциальность'),
        ('Доступность'),
        ('Целостность'),
        ('Социальная инженерия'),
        ('Аппаратные сбои'),
        ('Программные уязвимости')
    )");
    
    executeSQL(R"(
        INSERT OR IGNORE INTO risk_levels (name, weight) VALUES 
        ('Низкий', 1),
        ('Средний', 2),
        ('Высокий', 3),
        ('Критический', 4)
    )");
    
    bool adminExists = false;
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && atoi(argv[0]) > 0) {
            *static_cast<bool*>(data) = true;
        }
        return 0;
    };
    
    executeSQLWithCallback("SELECT COUNT(*) FROM users WHERE login='admin'", callback, &adminExists);
    
    if (!adminExists) {
        executeSQL("INSERT INTO users (login, password, role) VALUES ('admin', 'admin123', 'admin')");
        logEvent(0, "system", "Создан администратор admin");
    }
}

bool Database::registerUser(const string &login, const string &password, 
                           const string &role) {
    stringstream ss;
    ss << "INSERT INTO users (login, password, role) VALUES ('"
       << escapeSQL(login) << "', '" << escapeSQL(password) << "', '" 
       << escapeSQL(role) << "')";
    
    if (!executeSQL(ss.str())) {
        return false;
    }
    
    int newUserId = 0;
    auto idCallback = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0) {
            *static_cast<int*>(data) = atoi(argv[0]);
        }
        return 0;
    };
    
    string getIdSQL = "SELECT id FROM users WHERE login='" + escapeSQL(login) + "'";
    executeSQLWithCallback(getIdSQL, idCallback, &newUserId);
    
    logEvent(newUserId, "user_register", "Зарегистрирован пользователь: " + login);
    return true;
}

User Database::loginUser(const string &login, const string &password) {
    struct UserData {
        int id;
        string role;
        bool found;
    } userData = {0, "", false};
    
    stringstream ss;
    ss << "SELECT id, role FROM users WHERE login='" << escapeSQL(login) 
       << "' AND password='" << escapeSQL(password) << "'";
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        UserData* ud = static_cast<UserData*>(data);
        if (argc >= 2) {
            ud->id = atoi(argv[0]);
            ud->role = argv[1] ? argv[1] : "";
            ud->found = true;
        }
        return 0;
    };
    
    executeSQLWithCallback(ss.str(), callback, &userData);
    
    if (userData.found) {
        logEvent(userData.id, "login", "Успешный вход пользователя: " + login);
        return User(userData.id, login, userData.role);
    }
    
    return User();
}

bool Database::userExists(const string &login) {
    int count = 0;
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0) {
            *static_cast<int*>(data) = atoi(argv[0]);
        }
        return 0;
    };
    
    string sql = "SELECT COUNT(*) FROM users WHERE login='" + escapeSQL(login) + "'";
    executeSQLWithCallback(sql, callback, &count);
    
    return count > 0;
}

bool Database::addThreat(const Threat &threat) {
    stringstream ss;
    ss << "INSERT INTO threats (name, type_id, risk_level_id, description) VALUES ('"
       << escapeSQL(threat.name) << "', " << threat.type_id << ", " 
       << threat.risk_level_id << ", '" << escapeSQL(threat.description) << "')";
    
    if (!executeSQL(ss.str())) {
        return false;
    }
    
    logEvent(0, "threat_add", "Добавлена угроза: " + threat.name);
    return true;
}

vector<Threat> Database::getAllThreats() {
    vector<Threat> threats;
    
    const char* sql = R"(
        SELECT t.id, t.name, t.type_id, t.risk_level_id, t.description, t.created_at,
               tt.name as type_name, rl.name as risk_level_name
        FROM threats t
        JOIN threat_types tt ON t.type_id = tt.id
        JOIN risk_levels rl ON t.risk_level_id = rl.id
        ORDER BY t.id DESC
    )";
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        vector<Threat>* threatsPtr = static_cast<vector<Threat>*>(data);
        
        Threat threat;
        if (argc >= 1) threat.id = atoi(argv[0]);
        if (argc >= 2) threat.name = argv[1] ? argv[1] : "";
        if (argc >= 3) threat.type_id = atoi(argv[2]);
        if (argc >= 4) threat.risk_level_id = atoi(argv[3]);
        if (argc >= 5) threat.description = argv[4] ? argv[4] : "";
        if (argc >= 6) threat.created_at = argv[5] ? argv[5] : "";
        if (argc >= 7) threat.type_name = argv[6] ? argv[6] : "";
        if (argc >= 8) threat.risk_level_name = argv[7] ? argv[7] : "";
        
        threatsPtr->push_back(threat);
        return 0;
    };
    
    executeSQLWithCallback(sql, callback, &threats);
    return threats;
}

vector<ThreatType> Database::getThreatTypes() {
    vector<ThreatType> types;
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        vector<ThreatType>* typesPtr = static_cast<vector<ThreatType>*>(data);
        
        ThreatType type;
        if (argc >= 1) type.id = atoi(argv[0]);
        if (argc >= 2) type.name = argv[1] ? argv[1] : "";
        
        typesPtr->push_back(type);
        return 0;
    };
    
    executeSQLWithCallback("SELECT id, name FROM threat_types ORDER BY name", callback, &types);
    return types;
}

vector<RiskLevel> Database::getRiskLevels() {
    vector<RiskLevel> levels;
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        vector<RiskLevel>* levelsPtr = static_cast<vector<RiskLevel>*>(data);
        
        RiskLevel level;
        if (argc >= 1) level.id = atoi(argv[0]);
        if (argc >= 2) level.name = argv[1] ? argv[1] : "";
        if (argc >= 3) level.weight = atoi(argv[2]);
        
        levelsPtr->push_back(level);
        return 0;
    };
    
    executeSQLWithCallback("SELECT id, name, weight FROM risk_levels ORDER BY weight DESC", 
                          callback, &levels);
    return levels;
}

bool Database::deleteThreat(int id) {
    stringstream ss;
    ss << "DELETE FROM threats WHERE id = " << id;
    
    if (!executeSQL(ss.str())) {
        return false;
    }
    
    logEvent(0, "threat_delete", "Удалена угроза ID: " + to_string(id));
    return true;
}

void Database::logEvent(int user_id, const string &event, const string &details) {
    stringstream ss;
    ss << "INSERT INTO logs (user_id, event, details) VALUES ("
       << user_id << ", '" << escapeSQL(event) << "', '" 
       << escapeSQL(details) << "')";
    executeSQL(ss.str());
}

vector<LogEntry> Database::getLogs(int limit) {
    vector<LogEntry> logs;
    
    stringstream ss;
    ss << "SELECT l.id, l.user_id, l.timestamp, l.event, l.details, u.login "
       << "FROM logs l LEFT JOIN users u ON l.user_id = u.id "
       << "ORDER BY l.timestamp DESC LIMIT " << limit;
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        vector<LogEntry>* logsPtr = static_cast<vector<LogEntry>*>(data);
        
        LogEntry entry;
        if (argc >= 1) entry.id = atoi(argv[0]);
        if (argc >= 2) entry.user_id = atoi(argv[1]);
        if (argc >= 3) entry.timestamp = argv[2] ? argv[2] : "";
        if (argc >= 4) entry.event = argv[3] ? argv[3] : "";
        if (argc >= 5) entry.details = argv[4] ? argv[4] : "";
        if (argc >= 6) entry.user_login = argv[5] ? argv[5] : "system";
        
        logsPtr->push_back(entry);
        return 0;
    };
    
    executeSQLWithCallback(ss.str(), callback, &logs);
    return logs;
}

vector<LogEntry> Database::getUserLogs(int user_id, int limit) {
    vector<LogEntry> logs;
    
    stringstream ss;
    ss << "SELECT id, user_id, timestamp, event, details FROM logs "
       << "WHERE user_id = " << user_id << " ORDER BY timestamp DESC LIMIT " << limit;
    
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        vector<LogEntry>* logsPtr = static_cast<vector<LogEntry>*>(data);
        
        LogEntry entry;
        if (argc >= 1) entry.id = atoi(argv[0]);
        if (argc >= 2) entry.user_id = atoi(argv[1]);
        if (argc >= 3) entry.timestamp = argv[2] ? argv[2] : "";
        if (argc >= 4) entry.event = argv[3] ? argv[3] : "";
        if (argc >= 5) entry.details = argv[4] ? argv[4] : "";
        
        logsPtr->push_back(entry);
        return 0;
    };
    
    executeSQLWithCallback(ss.str(), callback, &logs);
    return logs;
}

Database::Statistics Database::getStatistics() {
    Statistics stats = {0, 0, 0, 0, 0, 0};
    
    auto threatCallback = [](void* data, int argc, char** argv, char**) -> int {
        Statistics* s = static_cast<Statistics*>(data);
        if (argc >= 1) s->total_threats = atoi(argv[0]);
        return 0;
    };
    
    executeSQLWithCallback("SELECT COUNT(*) FROM threats", threatCallback, &stats);
    
    auto riskCallback = [](void* data, int argc, char** argv, char**) -> int {
        Statistics* s = static_cast<Statistics*>(data);
        if (argc >= 2) {
            string level = argv[0] ? argv[0] : "";
            int count = atoi(argv[1]);
            
            if (level == "Высокий" || level == "Критический") s->high_risk_threats += count;
            else if (level == "Средний") s->medium_risk_threats = count;
            else if (level == "Низкий") s->low_risk_threats = count;
        }
        return 0;
    };
    
    executeSQLWithCallback(R"(
        SELECT rl.name, COUNT(*) 
        FROM threats t 
        JOIN risk_levels rl ON t.risk_level_id = rl.id 
        GROUP BY rl.name
    )", riskCallback, &stats);
    
    auto userCallback = [](void* data, int argc, char** argv, char**) -> int {
        Statistics* s = static_cast<Statistics*>(data);
        if (argc >= 1) s->total_users = atoi(argv[0]);
        return 0;
    };
    
    executeSQLWithCallback("SELECT COUNT(*) FROM users", userCallback, &stats);
    
    auto logCallback = [](void* data, int argc, char** argv, char**) -> int {
        Statistics* s = static_cast<Statistics*>(data);
        if (argc >= 1) s->total_logs = atoi(argv[0]);
        return 0;
    };
    
    executeSQLWithCallback("SELECT COUNT(*) FROM logs", logCallback, &stats);
    
    return stats;
}

int Database::getThreatCount() {
    int count = 0;
    auto callback = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0) {
            *static_cast<int*>(data) = atoi(argv[0]);
        }
        return 0;
    };
    
    executeSQLWithCallback("SELECT COUNT(*) FROM threats", callback, &count);
    return count;
}

void Database::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::executeSQL(const string &sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        cerr << "SQL ошибка: " << errMsg << " в запросе: " << sql << endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool Database::executeSQLWithCallback(const string &sql, 
                                     int (*callback)(void*, int, char**, char**), 
                                     void* data) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), callback, data, &errMsg);
    
    if (rc != SQLITE_OK) {
        cerr << "SQL ошибка: " << errMsg << " в запросе: " << sql << endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}
