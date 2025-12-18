#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

using namespace std;

struct ThreatType {
    int id;
    string name;
    ThreatType() : id(0) {}
};

struct RiskLevel {
    int id;
    string name;
    int weight;
    RiskLevel() : id(0), weight(0) {}
};

struct Threat {
    int id;
    string name;
    int type_id;
    int risk_level_id;
    string description;
    string created_at;
    string type_name;
    string risk_level_name;
    Threat() : id(0), type_id(0), risk_level_id(0) {}
};

struct LogEntry {
    int id;
    int user_id;
    string timestamp;
    string event;
    string details;
    string user_login;
    LogEntry() : id(0), user_id(0) {}
};

class User {
private:
    int m_id;
    string m_login;
    string m_role;
public:
    User() : m_id(0) {}
    User(int id, const string& login, const string& role)
        : m_id(id), m_login(login), m_role(role) {}
    int getId() const { return m_id; }
    string getLogin() const { return m_login; }
    string getRole() const { return m_role; }
    bool isAdmin() const { return m_role == "admin"; }
};

class Database {
private:
    sqlite3* m_db;
    string escapeSQL(const string& str);

public:
    Database();
    ~Database();
    bool open(const string &filename);
    void close();

    bool registerUser(const string &login, const string &password, const string &role = "user");
    User loginUser(const string &login, const string &password);
    bool userExists(const string &login);

    bool addThreat(const Threat &threat);
    vector<Threat> getAllThreats();
    vector<ThreatType> getThreatTypes();
    vector<RiskLevel> getRiskLevels();
    bool deleteThreat(int id);
    int getThreatCount();

    void logEvent(int user_id, const string &event, const string &details);
    vector<LogEntry> getLogs(int limit = 100);
    vector<LogEntry> getUserLogs(int user_id, int limit = 50);

    struct Statistics {
        int total_threats = 0;
        int high_risk_threats = 0;
        int medium_risk_threats = 0;
        int low_risk_threats = 0;
        int total_users = 0;
        int total_logs = 0;
    };
    Statistics getStatistics();

private:
    bool initTables();
    bool executeSQL(const string &sql);
    bool executeSQLWithCallback(const string &sql, int (*callback)(void*, int, char**, char**), void* data);
    void initReferenceData();
};
#endif
