#pragma once

#include <QObject>
#include <QString>
#include <QDir>

// Maximum size of each logfile before a new one is created
#define MAX_LOGFILE_SIZE (10 * 1024 * 1024)

// Maximum number of log files
#define MAX_LOGFILE_COUNT 20

// Name of log directory, will be created in the directory we're processing pictures in
#define LOG_DIR_NAME "logs"

class SantaLogger : public QObject {
public:
    Q_OBJECT

    // this allows us to create a binding so we don't have to explicitly set the root path
    Q_PROPERTY(QString rootDirectory MEMBER m_rootDirectory WRITE setRootDirectory NOTIFY rootDirectoryChanged)

public:
    static SantaLogger& instance() {
        // C++11 guarantees this is thread safe and can only happen once
        static SantaLogger globalInstance;
        return globalInstance;
    }

    void setRootDirectory(const QString& dir);

signals:
    void rootDirectoryChanged(const QString& dir);

private:
    // Prevent creation of additional instances
    explicit SantaLogger(QObject* parent = nullptr);
    ~SantaLogger() = default;
    SantaLogger(const SantaLogger&) = delete;
    SantaLogger& operator=(const SantaLogger&) = delete;

    static void logMessageHandlerFn(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    void logMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    void doSizeCheck();
    void rotateLogs();
    QString newLogFileName();

    QString m_rootDirectory;
    QDir m_logDir;
    QString m_logFilePath;
};
