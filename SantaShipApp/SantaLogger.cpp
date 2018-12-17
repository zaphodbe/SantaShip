#include "SantaLogger.h"

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTextStream>

#if defined(Q_OS_WINRT) || defined(Q_OS_WIN)
  #include <Windows.h>
#else
  #include <stdio.h>
#endif

// really should just use a QMap, but I'm lazy
const QString DebugMessageLabel = QString("[DEBUG]");
const QString InfoMessageLabel = QString("[INFO]");
const QString WarningMessageLabel = QString("[WARNING]");
const QString CriticalMessageLabel = QString("[CRITICAL]");
const QString FatalMessageLabel = QString("[FATAL]");

const QString& getMessageLabel(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg:
            return DebugMessageLabel;
        case QtInfoMsg:
            return InfoMessageLabel;
        case QtWarningMsg:
            return WarningMessageLabel;
        case QtCriticalMsg:
            return CriticalMessageLabel;
        case QtFatalMsg:
            return FatalMessageLabel;
    }
    return DebugMessageLabel; // should never happen...
}

SantaLogger::SantaLogger(QObject* parent) : QObject(parent),
    m_rootDirectory(),
    m_logDir(),
    m_logFilePath()
{
    qInstallMessageHandler(SantaLogger::logMessageHandlerFn);
}

void SantaLogger::setRootDirectory(const QString& dir)
{
    // don't do anything on no change
    if (dir == m_rootDirectory) return;

    m_rootDirectory = dir;

    QDir rootDir(m_rootDirectory);
    if (!rootDir.exists(LOG_DIR_NAME)) {
        rootDir.mkdir(LOG_DIR_NAME);
    }

    QString logDirPath = rootDir.filePath(LOG_DIR_NAME);
    QDir newLogDir(logDirPath);
    QString newLogPath = newLogDir.filePath(newLogFileName());

    // Make sure we can access the new log dir
    QFile testFile(newLogPath);
    if (testFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        testFile.close();

        m_logDir = newLogDir;
        m_logFilePath = newLogPath;

        // do maintenance
        rotateLogs();
    }
    emit rootDirectoryChanged(m_rootDirectory);
}

void SantaLogger::logMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    Q_UNUSED(context);

    const QString typeLabel = getMessageLabel(type);

    // print to console, have to call OutputDebugString on Windoze
    // everything else uses stderr
#if defined(Q_OS_WINRT) || defined(Q_OS_WIN)
    OutputDebugString(reinterpret_cast<const wchar_t*>(message.utf16()));
#else
    fprintf(stderr, "%s %s\n", typeLabel.toLocal8Bit().constData(), message.toLocal8Bit().constData());
    fflush(stderr);
#endif

    // FIXME: it would be more efficient to open the log file once and keep it open until it changes
    if (m_logFilePath.isEmpty()) return;

    doSizeCheck();

    QFile logFile(m_logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream outStream(&logFile);
        outStream << typeLabel << " " << message << endl;
        logFile.close();
    }
}

// Generate a new logfile name
// Format is: SantaShip_yyyy_MM_dd_hh_mm.txt
QString SantaLogger::newLogFileName()
{
    return QString("SantaShip_%1_%2.txt")
            .arg(QDate::currentDate().toString("yyyy_MM_dd"))
            .arg(QTime::currentTime().toString("hh_mm"));
}

void SantaLogger::doSizeCheck()
{
    QFile logFile(m_logFilePath);
    if (logFile.size() > MAX_LOGFILE_SIZE) {
        // size exceeded, rotate log
        rotateLogs();
        m_logFilePath = m_logDir.filePath(newLogFileName());
    }
}

void SantaLogger::rotateLogs()
{
    // get a list of existing logfiles
    // FIXME: we really should glob/regex file names to be sure we only delete our own log files
    QFileInfoList list = m_logDir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks, QDir::Time | QDir::Reversed);
    if (list.size() > MAX_LOGFILE_COUNT) {
        // too many log files, delete the oldest until we're below the max
        int count = list.size() - MAX_LOGFILE_COUNT;
        qDebug().nospace() << "removing " << count << " old log files";
        for (int ii = 0; ii < count; ii ++) {
            m_logDir.remove(list.at(ii).fileName());
        }
    }
}

void SantaLogger::logMessageHandlerFn(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    instance().logMessage(type, context, msg);
}
