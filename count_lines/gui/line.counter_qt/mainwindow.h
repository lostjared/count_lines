#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr);
    void setParameters(const QString &directory, const QStringList &extensions);

signals:
    void progressUpdate(const QString &message);
    void resultReady(const QString &result);
    void finished();

public slots:
    void process();

private:
    QString m_directory;
    QStringList m_extensions;

    QStringList gatherFiles(const QString &directory, const QStringList &extensions);
    unsigned long countFile(const QString &filePath, unsigned long &blankLines);
    bool lineEmpty(const QString &line) const;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseDirectory();
    void onCountLines();
    void onCopyResults();
    void handleProgressUpdate(const QString &message);
    void handleResultReady(const QString &result);
    void handleWorkerFinished();

private:
    Ui::MainWindow *ui;
    QThread *workerThread;
    Worker *worker;

    void appendResult(const QString &text);
};

#endif 